import datetime
from typing import List, Dict, Optional
from dataclasses import dataclass
import pandas as pd
import numpy as np
from enum import Enum

class RiskLevel(Enum):
    LOW = 1
    MEDIUM = 2
    HIGH = 3
    CRITICAL = 4

@dataclass
class WeatherCondition:
    temperature: float
    visibility: float
    wind_speed: float
    precipitation: float
    
    def calculate_risk_level(self) -> RiskLevel:
        risk_score = 0
        
        # Evaluate visibility risk
        if self.visibility < 1000:
            risk_score += 3
        elif self.visibility < 3000:
            risk_score += 2
        elif self.visibility < 5000:
            risk_score += 1
            
        # Evaluate wind risk
        if self.wind_speed > 50:
            risk_score += 3
        elif self.wind_speed > 30:
            risk_score += 2
        elif self.wind_speed > 15:
            risk_score += 1
            
        # Convert score to RiskLevel
        if risk_score >= 5:
            return RiskLevel.CRITICAL
        elif risk_score >= 3:
            return RiskLevel.HIGH
        elif risk_score >= 1:
            return RiskLevel.MEDIUM
        return RiskLevel.LOW

@dataclass
class MaintenanceRecord:
    aircraft_id: str
    last_inspection_date: datetime.datetime
    maintenance_due_date: datetime.datetime
    reported_issues: List[str]
    maintenance_history: List[Dict]
    
    def is_maintenance_required(self) -> bool:
        return (
            datetime.datetime.now() >= self.maintenance_due_date or
            len(self.reported_issues) > 0
        )
    
    def calculate_risk_level(self) -> RiskLevel:
        days_since_inspection = (
            datetime.datetime.now() - self.last_inspection_date
        ).days
        
        if days_since_inspection > 180 or len(self.reported_issues) > 2:
            return RiskLevel.CRITICAL
        elif days_since_inspection > 90 or len(self.reported_issues) > 0:
            return RiskLevel.HIGH
        elif days_since_inspection > 45:
            return RiskLevel.MEDIUM
        return RiskLevel.LOW

class CrewMember:
    def __init__(
        self,
        id: str,
        name: str,
        role: str,
        certification_level: str,
        flight_hours: int,
        last_training_date: datetime.datetime
    ):
        self.id = id
        self.name = name
        self.role = role
        self.certification_level = certification_level
        self.flight_hours = flight_hours
        self.last_training_date = last_training_date
        self.training_history = []
        self.incident_history = []
    
    def is_training_current(self) -> bool:
        days_since_training = (
            datetime.datetime.now() - self.last_training_date
        ).days
        return days_since_training <= 180
    
    def calculate_risk_level(self) -> RiskLevel:
        if not self.is_training_current():
            return RiskLevel.CRITICAL
            
        if self.flight_hours < 100:
            return RiskLevel.HIGH
        elif self.flight_hours < 500:
            return RiskLevel.MEDIUM
        return RiskLevel.LOW

class Aircraft:
    def __init__(
        self,
        id: str,
        model: str,
        manufacture_date: datetime.datetime,
        total_flight_hours: int
    ):
        self.id = id
        self.model = model
        self.manufacture_date = manufacture_date
        self.total_flight_hours = total_flight_hours
        self.maintenance_records = []
        self.incident_history = []
    
    def add_maintenance_record(self, record: MaintenanceRecord):
        self.maintenance_records.append(record)
    
    def get_current_maintenance_status(self) -> Optional[MaintenanceRecord]:
        if self.maintenance_records:
            return max(
                self.maintenance_records,
                key=lambda x: x.last_inspection_date
            )
        return None

class Mission:
    def __init__(
        self,
        id: str,
        aircraft: Aircraft,
        crew: List[CrewMember],
        departure_time: datetime.datetime,
        estimated_duration: float,
        mission_type: str
    ):
        self.id = id
        self.aircraft = aircraft
        self.crew = crew
        self.departure_time = departure_time
        self.estimated_duration = estimated_duration
        self.mission_type = mission_type
        self.weather_conditions = None
        self.risk_assessment = None
        
    def set_weather_conditions(self, weather: WeatherCondition):
        self.weather_conditions = weather
    
    def perform_risk_assessment(self) -> Dict:
        if not self.weather_conditions:
            raise ValueError("Weather conditions must be set before risk assessment")
            
        maintenance_status = self.aircraft.get_current_maintenance_status()
        if not maintenance_status:
            raise ValueError("No maintenance records found for aircraft")
            
        risk_factors = {
            "weather": self.weather_conditions.calculate_risk_level(),
            "maintenance": maintenance_status.calculate_risk_level(),
            "crew": max(crew.calculate_risk_level() for crew in self.crew)
        }
        
        overall_risk = max(risk_factors.values())
        
        self.risk_assessment = {
            "risk_factors": risk_factors,
            "overall_risk": overall_risk,
            "assessment_time": datetime.datetime.now()
        }
        
        return self.risk_assessment

class SafetyManagementSystem:
    def __init__(self):
        self.aircraft_registry = {}
        self.crew_registry = {}
        self.missions = []
        self.incident_reports = []
        
    def register_aircraft(self, aircraft: Aircraft):
        self.aircraft_registry[aircraft.id] = aircraft
    
    def register_crew_member(self, crew_member: CrewMember):
        self.crew_registry[crew_member.id] = crew_member
    
    def create_mission(self, mission: Mission):
        self.missions.append(mission)
    
    def generate_safety_report(self, start_date: datetime.datetime, 
                             end_date: datetime.datetime) -> Dict:
        relevant_missions = [
            m for m in self.missions 
            if start_date <= m.departure_time <= end_date
        ]
        
        risk_levels = [
            m.risk_assessment["overall_risk"] 
            for m in relevant_missions 
            if m.risk_assessment
        ]
        
        report = {
            "total_missions": len(relevant_missions),
            "risk_distribution": {
                level: risk_levels.count(level) 
                for level in RiskLevel
            },
            "average_risk": np.mean([r.value for r in risk_levels]) 
                if risk_levels else 0,
            "high_risk_missions": len([
                r for r in risk_levels 
                if r in (RiskLevel.HIGH, RiskLevel.CRITICAL)
            ])
        }
        
        return report

# Example usage:
if __name__ == "__main__":
    # Initialize the safety management system
    sms = SafetyManagementSystem()
    
    # Create an aircraft
    aircraft = Aircraft(
        "AC001",
        "F-35",
        datetime.datetime(2020, 1, 1),
        1000
    )
    
    # Add maintenance record
    maintenance = MaintenanceRecord(
        "AC001",
        datetime.datetime.now() - datetime.timedelta(days=30),
        datetime.datetime.now() + datetime.timedelta(days=60),
        [],
        []
    )
    aircraft.add_maintenance_record(maintenance)
    
    # Create crew members
    pilot = CrewMember(
        "P001",
        "John Doe",
        "Pilot",
        "Senior",
        2000,
        datetime.datetime.now() - datetime.timedelta(days=90)
    )
    
    copilot = CrewMember(
        "P002",
        "Jane Smith",
        "Co-Pilot",
        "Mid-Level",
        1000,
        datetime.datetime.now() - datetime.timedelta(days=60)
    )
    
    # Register with SMS
    sms.register_aircraft(aircraft)
    sms.register_crew_member(pilot)
    sms.register_crew_member(copilot)
    
    # Create a mission
    mission = Mission(
        "M001",
        aircraft,
        [pilot, copilot],
        datetime.datetime.now() + datetime.timedelta(hours=24),
        2.5,
        "Training"
    )
    
    # Set weather conditions
    weather = WeatherCondition(
        temperature=25.0,
        visibility=5000.0,
        wind_speed=10.0,
        precipitation=0.0
    )
    mission.set_weather_conditions(weather)
    
    # Perform risk assessment
    risk_assessment = mission.perform_risk_assessment()
    print(f"Mission Risk Assessment: {risk_assessment}")
    
    # Add mission to SMS
    sms.create_mission(mission)
    
    # Generate safety report
    report = sms.generate_safety_report(
        datetime.datetime.now() - datetime.timedelta(days=30),
        datetime.datetime.now() + datetime.timedelta(days=30)
    )
    print(f"Safety Report: {report}")
