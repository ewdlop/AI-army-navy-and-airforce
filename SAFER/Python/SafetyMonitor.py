import pandas as pd
import numpy as np
from datetime import datetime
from typing import Dict, List, Optional
import logging

class SafetyMonitor:
    def __init__(self):
        """Initialize the Safety Monitoring System"""
        self.incidents_db = pd.DataFrame(columns=[
            'date', 'incident_type', 'severity', 'location',
            'personnel_involved', 'equipment_affected', 'description'
        ])
        self.flight_data = pd.DataFrame(columns=[
            'date', 'aircraft_id', 'flight_hours', 'maintenance_status',
            'weather_conditions', 'pilot_hours', 'incidents'
        ])
        self.risk_thresholds = {
            'low': 2.0,
            'medium': 5.0,
            'high': 8.0,
            'critical': 10.0
        }
        
        # Setup logging
        logging.basicConfig(
            filename='safety_monitor.log',
            level=logging.INFO,
            format='%(asctime)s - %(levelname)s - %(message)s'
        )

    def record_incident(self, incident_data: Dict) -> bool:
        """
        Record a new safety incident
        
        Args:
            incident_data (Dict): Dictionary containing incident details
            
        Returns:
            bool: True if recording was successful, False otherwise
        """
        try:
            new_incident = pd.DataFrame([incident_data])
            self.incidents_db = pd.concat([self.incidents_db, new_incident], ignore_index=True)
            logging.info(f"Incident recorded: {incident_data['incident_type']} - {incident_data['date']}")
            return True
        except Exception as e:
            logging.error(f"Error recording incident: {str(e)}")
            return False

    def calculate_risk_score(self, flight_conditions: Dict) -> float:
        """
        Calculate risk score based on multiple factors
        
        Args:
            flight_conditions (Dict): Dictionary containing flight conditions
            
        Returns:
            float: Calculated risk score
        """
        base_score = 0.0
        
        # Weather risk factors
        weather_risks = {
            'clear': 0.0,
            'cloudy': 1.0,
            'rain': 2.0,
            'thunderstorm': 4.0,
            'snow': 3.5
        }
        base_score += weather_risks.get(flight_conditions.get('weather', 'clear'), 0.0)
        
        # Maintenance status
        if flight_conditions.get('maintenance_due', False):
            base_score += 2.0
        
        # Pilot experience
        pilot_hours = flight_conditions.get('pilot_hours', 1000)
        if pilot_hours < 100:
            base_score += 3.0
        elif pilot_hours < 500:
            base_score += 1.5
        
        # Equipment age
        equipment_age = flight_conditions.get('equipment_age', 0)
        base_score += min(equipment_age * 0.1, 2.0)
        
        return round(base_score, 2)

    def analyze_trends(self, timeframe: str = 'month') -> Dict:
        """
        Analyze safety trends over specified timeframe
        
        Args:
            timeframe (str): Time period for analysis ('week', 'month', 'year')
            
        Returns:
            Dict: Analysis results
        """
        current_date = datetime.now()
        self.incidents_db['date'] = pd.to_datetime(self.incidents_db['date'])
        
        if timeframe == 'week':
            mask = self.incidents_db['date'] > current_date - pd.Timedelta(days=7)
        elif timeframe == 'month':
            mask = self.incidents_db['date'] > current_date - pd.Timedelta(days=30)
        else:  # year
            mask = self.incidents_db['date'] > current_date - pd.Timedelta(days=365)
            
        period_incidents = self.incidents_db[mask]
        
        analysis = {
            'total_incidents': len(period_incidents),
            'severity_breakdown': period_incidents['severity'].value_counts().to_dict(),
            'type_breakdown': period_incidents['incident_type'].value_counts().to_dict(),
            'high_severity_rate': len(period_incidents[period_incidents['severity'] > 7]) / len(period_incidents) if len(period_incidents) > 0 else 0
        }
        
        return analysis

    def generate_safety_report(self, start_date: str, end_date: str) -> Dict:
        """
        Generate comprehensive safety report for specified period
        
        Args:
            start_date (str): Start date for report period (YYYY-MM-DD)
            end_date (str): End date for report period (YYYY-MM-DD)
            
        Returns:
            Dict: Safety report data
        """
        mask = (self.incidents_db['date'] >= start_date) & (self.incidents_db['date'] <= end_date)
        period_data = self.incidents_db[mask]
        
        report = {
            'period': f"{start_date} to {end_date}",
            'incident_summary': {
                'total_incidents': len(period_data),
                'average_severity': period_data['severity'].mean() if len(period_data) > 0 else 0,
                'incident_types': period_data['incident_type'].value_counts().to_dict()
            },
            'risk_metrics': {
                'high_risk_incidents': len(period_data[period_data['severity'] > self.risk_thresholds['high']]),
                'medium_risk_incidents': len(period_data[period_data['severity'].between(
                    self.risk_thresholds['medium'],
                    self.risk_thresholds['high']
                )]),
                'low_risk_incidents': len(period_data[period_data['severity'] < self.risk_thresholds['medium']])
            },
            'recommendations': self._generate_recommendations(period_data)
        }
        
        return report

    def _generate_recommendations(self, incident_data: pd.DataFrame) -> List[str]:
        """
        Generate safety recommendations based on incident data
        
        Args:
            incident_data (pd.DataFrame): Incident data for analysis
            
        Returns:
            List[str]: List of safety recommendations
        """
        recommendations = []
        
        # Analyze high severity incidents
        high_severity = incident_data[incident_data['severity'] > self.risk_thresholds['high']]
        if len(high_severity) > 0:
            common_types = high_severity['incident_type'].mode()
            if not common_types.empty:
                recommendations.append(
                    f"Priority: Address {common_types.iloc[0]} incidents - high severity trend detected"
                )
        
        # Check for recurring incidents
        incident_counts = incident_data['incident_type'].value_counts()
        frequent_incidents = incident_counts[incident_counts > 3]
        for incident_type, count in frequent_incidents.items():
            recommendations.append(
                f"Review procedures for {incident_type} incidents - occurred {count} times in period"
            )
        
        # Equipment-related recommendations
        equipment_incidents = incident_data[incident_data['equipment_affected'].notna()]
        if len(equipment_incidents) > 0:
            common_equipment = equipment_incidents['equipment_affected'].mode()
            if not common_equipment.empty:
                recommendations.append(
                    f"Conduct thorough inspection of {common_equipment.iloc[0]} - frequent incident involvement"
                )
        
        return recommendations

# Example usage
if __name__ == "__main__":
    # Initialize the safety monitor
    monitor = SafetyMonitor()
    
    # Record a sample incident
    sample_incident = {
        'date': '2025-02-17',
        'incident_type': 'maintenance_delay',
        'severity': 3,
        'location': 'Hangar B',
        'personnel_involved': ['Tech1', 'Tech2'],
        'equipment_affected': 'Aircraft A1',
        'description': 'Routine maintenance exceeded scheduled time due to parts delay'
    }
    monitor.record_incident(sample_incident)
    
    # Calculate risk score for a flight
    flight_conditions = {
        'weather': 'rain',
        'maintenance_due': False,
        'pilot_hours': 750,
        'equipment_age': 5
    }
    risk_score = monitor.calculate_risk_score(flight_conditions)
    print(f"Flight Risk Score: {risk_score}")
    
    # Generate a safety report
    report = monitor.generate_safety_report('2025-01-01', '2025-02-17')
    print("\nSafety Report:")
    print(report)
