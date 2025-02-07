from fastapi import FastAPI, HTTPException
from sqlalchemy import create_engine, Column, Integer, String, Float, DateTime, JSON, Enum as SQLEnum
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker
from datetime import datetime, timedelta
import threading
import logging
from typing import List, Dict, Optional
import json
import asyncio
import websockets
from prometheus_client import start_http_server, Counter, Gauge

# Database Setup
DATABASE_URL = "postgresql://user:password@localhost/safer_db"
engine = create_engine(DATABASE_URL)
SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)
Base = declarative_base()

# Logging Configuration
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    filename='safer.log'
)
logger = logging.getLogger(__name__)

# Prometheus Metrics
RISK_LEVELS = Counter('risk_levels_total', 'Risk levels by category', ['level'])
ACTIVE_MISSIONS = Gauge('active_missions', 'Number of active missions')
MAINTENANCE_ALERTS = Counter('maintenance_alerts_total', 'Number of maintenance alerts')

# Database Models
class MissionDB(Base):
    __tablename__ = "missions"
    
    id = Column(String, primary_key=True)
    aircraft_id = Column(String)
    departure_time = Column(DateTime)
    estimated_duration = Column(Float)
    mission_type = Column(String)
    risk_assessment = Column(JSON)
    status = Column(String)
    weather_conditions = Column(JSON)

class AircraftDB(Base):
    __tablename__ = "aircraft"
    
    id = Column(String, primary_key=True)
    model = Column(String)
    manufacture_date = Column(DateTime)
    total_flight_hours = Column(Integer)
    maintenance_records = Column(JSON)

class CrewMemberDB(Base):
    __tablename__ = "crew_members"
    
    id = Column(String, primary_key=True)
    name = Column(String)
    role = Column(String)
    certification_level = Column(String)
    flight_hours = Column(Integer)
    last_training_date = Column(DateTime)

# Create all database tables
Base.metadata.create_all(bind=engine)

# FastAPI Application
app = FastAPI(title="SAFER API")

# WebSocket Manager for Real-time Updates
class WebSocketManager:
    def __init__(self):
        self.active_connections = set()
        
    async def connect(self, websocket):
        await websocket.accept()
        self.active_connections.add(websocket)
        
    def disconnect(self, websocket):
        self.active_connections.remove(websocket)
        
    async def broadcast(self, message: str):
        for connection in self.active_connections:
            await connection.send_text(message)

ws_manager = WebSocketManager()

# Real-time Monitoring System
class MonitoringSystem:
    def __init__(self):
        self.alert_thresholds = {
            'weather': {
                'wind_speed': 30.0,
                'visibility': 3000.0
            },
            'maintenance': {
                'days_since_inspection': 90
            }
        }
    
    async def check_conditions(self, mission: Mission) -> List[str]:
        alerts = []
        
        # Check weather conditions
        if mission.weather_conditions:
            if mission.weather_conditions.wind_speed > self.alert_thresholds['weather']['wind_speed']:
                alert = f"High wind speed alert for mission {mission.id}"
                alerts.append(alert)
                await self.send_alert(alert)
            
            if mission.weather_conditions.visibility < self.alert_thresholds['weather']['visibility']:
                alert = f"Low visibility alert for mission {mission.id}"
                alerts.append(alert)
                await self.send_alert(alert)
        
        # Check maintenance status
        maintenance_status = mission.aircraft.get_current_maintenance_status()
        if maintenance_status:
            days_since_inspection = (datetime.now() - maintenance_status.last_inspection_date).days
            if days_since_inspection > self.alert_thresholds['maintenance']['days_since_inspection']:
                alert = f"Maintenance overdue for aircraft {mission.aircraft.id}"
                alerts.append(alert)
                await self.send_alert(alert)
                MAINTENANCE_ALERTS.inc()
        
        return alerts
    
    async def send_alert(self, alert: str):
        await ws_manager.broadcast(json.dumps({
            "type": "alert",
            "message": alert,
            "timestamp": datetime.now().isoformat()
        }))
        logger.warning(alert)

monitoring_system = MonitoringSystem()

# API Endpoints
@app.post("/missions/")
async def create_mission(mission_data: dict):
    try:
        db = SessionLocal()
        mission_db = MissionDB(**mission_data)
        db.add(mission_db)
        db.commit()
        ACTIVE_MISSIONS.inc()
        return {"message": "Mission created successfully", "id": mission_db.id}
    except Exception as e:
        logger.error(f"Error creating mission: {str(e)}")
        raise HTTPException(status_code=500, detail=str(e))
    finally:
        db.close()

@app.get("/missions/{mission_id}")
async def get_mission(mission_id: str):
    db = SessionLocal()
    mission = db.query(MissionDB).filter(MissionDB.id == mission_id).first()
    db.close()
    if not mission:
        raise HTTPException(status_code=404, detail="Mission not found")
    return mission

@app.get("/safety-report/")
async def generate_safety_report(start_date: str, end_date: str):
    try:
        db = SessionLocal()
        missions = db.query(MissionDB).filter(
            MissionDB.departure_time.between(start_date, end_date)
        ).all()
        
        report = {
            "total_missions": len(missions),
            "risk_distribution": {},
            "incidents": [],
            "maintenance_alerts": []
        }
        
        for mission in missions:
            if mission.risk_assessment:
                risk_level = mission.risk_assessment.get("overall_risk")
                report["risk_distribution"][risk_level] = report["risk_distribution"].get(risk_level, 0) + 1
                RISK_LEVELS.labels(level=risk_level).inc()
        
        return report
    except Exception as e:
        logger.error(f"Error generating safety report: {str(e)}")
        raise HTTPException(status_code=500, detail=str(e))
    finally:
        db.close()

# WebSocket endpoint for real-time updates
@app.websocket("/ws")
async def websocket_endpoint(websocket):
    await ws_manager.connect(websocket)
    try:
        while True:
            data = await websocket.receive_text()
            # Handle incoming WebSocket messages if needed
    except Exception as e:
        logger.error(f"WebSocket error: {str(e)}")
    finally:
        ws_manager.disconnect(websocket)

# Analytics System
class AnalyticsSystem:
    def __init__(self):
        self.db = SessionLocal()
    
    def calculate_fleet_statistics(self) -> Dict:
        try:
            aircraft = self.db.query(AircraftDB).all()
            return {
                "total_aircraft": len(aircraft),
                "average_flight_hours": sum(a.total_flight_hours for a in aircraft) / len(aircraft),
                "maintenance_status": self.analyze_maintenance_status()
            }
        finally:
            self.db.close()
    
    def analyze_maintenance_status(self) -> Dict:
        aircraft = self.db.query(AircraftDB).all()
        status_count = {"current": 0, "due": 0, "overdue": 0}
        
        for aircraft in aircraft:
            records = aircraft.maintenance_records
            if records:
                latest = max(records, key=lambda x: x["inspection_date"])
                days_since = (datetime.now() - datetime.fromisoformat(latest["inspection_date"])).days
                
                if days_since > 90:
                    status_count["overdue"] += 1
                elif days_since > 60:
                    status_count["due"] += 1
                else:
                    status_count["current"] += 1
        
        return status_count

# Initialize Prometheus metrics server
def start_metrics_server():
    start_http_server(8000)
    logger.info("Metrics server started on port 8000")

# Startup Events
@app.on_event("startup")
async def startup_event():
    # Start Prometheus metrics server
    threading.Thread(target=start_metrics_server, daemon=True).start()
    
    # Initialize analytics system
    analytics = AnalyticsSystem()
    
    logger.info("SAFER system initialized successfully")

# Example usage
if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8080)
