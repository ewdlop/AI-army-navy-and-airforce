use chrono::{DateTime, Duration, Utc};
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use uuid::Uuid;

// Marine Safety Types
#[derive(Debug, Serialize, Deserialize, Clone)]
pub enum WeatherCondition {
    Clear,
    Cloudy,
    Rain,
    Storm,
    Hurricane,
    Fog,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct MarineReport {
    timestamp: DateTime<Utc>,
    position: GpsCoordinates,
    weather: WeatherCondition,
    wave_height: f32,
    wind_speed: f32,
    wind_direction: f32,
    visibility: f32,
    warnings: Vec<String>,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct GpsCoordinates {
    latitude: f64,
    longitude: f64,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct NavigationLog {
    timestamp: DateTime<Utc>,
    position: GpsCoordinates,
    heading: f32,
    speed: f32,
    destination: GpsCoordinates,
    eta: DateTime<Utc>,
}

// Food and Supply Management Types
#[derive(Debug, Serialize, Deserialize, Clone)]
pub enum StorageType {
    Refrigerated,
    Frozen,
    DryStorage,
    ChemicalStorage,
    MedicalStorage,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub enum SupplyCategory {
    Food,
    Water,
    Medical,
    Fuel,
    Ammunition,
    Maintenance,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct SupplyItem {
    id: Uuid,
    name: String,
    category: SupplyCategory,
    storage_type: StorageType,
    quantity: f64,
    unit: String,
    expiration_date: Option<DateTime<Utc>>,
    minimum_threshold: f64,
    storage_location: String,
    temperature_requirements: Option<TemperatureRange>,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct TemperatureRange {
    min_temp: f32,
    max_temp: f32,
    current_temp: f32,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct MealPlan {
    date: DateTime<Utc>,
    meals: Vec<Meal>,
    total_calories: u32,
    special_diets: Vec<String>,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct Meal {
    meal_type: String,
    items: Vec<String>,
    serving_size: u32,
    calories_per_serving: u32,
}

// Existing types from previous implementation...
[Previous SafetyStatus, EmergencyType, CrewMember, Certification, TrainingRecord, 
Equipment, MaintenanceRecord, EmergencyProcedure, EnvironmentalCompliance definitions...]

// Enhanced SafetyProgram struct
pub struct SafetyProgram {
    crew_members: HashMap<Uuid, CrewMember>,
    equipment: HashMap<Uuid, Equipment>,
    emergency_procedures: HashMap<EmergencyType, EmergencyProcedure>,
    environmental_compliance: Vec<EnvironmentalCompliance>,
    incident_reports: Vec<IncidentReport>,
    // New fields for marine safety and supplies
    marine_logs: Vec<MarineReport>,
    navigation_logs: Vec<NavigationLog>,
    supplies: HashMap<Uuid, SupplyItem>,
    meal_plans: Vec<MealPlan>,
    storage_temperatures: HashMap<String, TemperatureRange>,
}

impl SafetyProgram {
    // Previous methods remain the same...
    [Previous methods for crew, equipment, emergency procedures, etc...]

    // New methods for marine safety
    pub fn log_marine_conditions(&mut self, report: MarineReport) {
        self.marine_logs.push(report);
    }

    pub fn get_weather_warnings(&self) -> Vec<&MarineReport> {
        self.marine_logs
            .iter()
            .filter(|report| !report.warnings.is_empty())
            .collect()
    }

    pub fn add_navigation_log(&mut self, log: NavigationLog) {
        self.navigation_logs.push(log);
    }

    pub fn get_latest_position(&self) -> Option<&GpsCoordinates> {
        self.navigation_logs
            .last()
            .map(|log| &log.position)
    }

    // Supply management methods
    pub fn add_supply_item(&mut self, item: SupplyItem) -> Result<(), String> {
        if self.supplies.contains_key(&item.id) {
            return Err("Supply item already exists".to_string());
        }
        self.supplies.insert(item.id, item);
        Ok(())
    }

    pub fn update_supply_quantity(&mut self, id: Uuid, new_quantity: f64) -> Result<(), String> {
        if let Some(item) = self.supplies.get_mut(&id) {
            item.quantity = new_quantity;
            Ok(())
        } else {
            Err("Supply item not found".to_string())
        }
    }

    pub fn get_low_supplies(&self) -> Vec<&SupplyItem> {
        self.supplies
            .values()
            .filter(|item| item.quantity <= item.minimum_threshold)
            .collect()
    }

    pub fn get_expiring_supplies(&self, days_threshold: i64) -> Vec<&SupplyItem> {
        let threshold_date = Utc::now() + Duration::days(days_threshold);
        self.supplies
            .values()
            .filter(|item| {
                if let Some(exp_date) = item.expiration_date {
                    exp_date <= threshold_date
                } else {
                    false
                }
            })
            .collect()
    }

    pub fn add_meal_plan(&mut self, plan: MealPlan) {
        self.meal_plans.push(plan);
    }

    pub fn get_meal_plan(&self, date: DateTime<Utc>) -> Option<&MealPlan> {
        self.meal_plans
            .iter()
            .find(|plan| {
                plan.date.date() == date.date()
            })
    }

    pub fn monitor_storage_temperature(&mut self, location: String, temp_range: TemperatureRange) {
        self.storage_temperatures.insert(location, temp_range);
    }

    pub fn get_temperature_violations(&self) -> Vec<(&String, &TemperatureRange)> {
        self.storage_temperatures
            .iter()
            .filter(|(_, range)| {
                range.current_temp < range.min_temp || range.current_temp > range.max_temp
            })
            .collect()
    }

    // Enhanced safety report including marine and supply status
    pub fn generate_comprehensive_report(&self) -> ComprehensiveReport {
        ComprehensiveReport {
            safety_report: self.generate_safety_report(),
            low_supplies: self.get_low_supplies().len(),
            expiring_supplies: self.get_expiring_supplies(30).len(),
            temperature_violations: self.get_temperature_violations().len(),
            weather_warnings: self.get_weather_warnings().len(),
            timestamp: Utc::now(),
        }
    }
}

#[derive(Debug, Serialize, Deserialize)]
pub struct ComprehensiveReport {
    safety_report: SafetyReport,
    low_supplies: usize,
    expiring_supplies: usize,
    temperature_violations: usize,
    weather_warnings: usize,
    timestamp: DateTime<Utc>,
}

// Tests
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_supply_management() {
        let mut program = SafetyProgram::new();
        let supply_item = SupplyItem {
            id: Uuid::new_v4(),
            name: "Drinking Water".to_string(),
            category: SupplyCategory::Water,
            storage_type: StorageType::DryStorage,
            quantity: 1000.0,
            unit: "Liters".to_string(),
            expiration_date: Some(Utc::now() + Duration::days(90)),
            minimum_threshold: 200.0,
            storage_location: "Main Storage".to_string(),
            temperature_requirements: None,
        };

        assert!(program.add_supply_item(supply_item.clone()).is_ok());
        assert!(program.add_supply_item(supply_item).is_err());
    }

    #[test]
    fn test_temperature_monitoring() {
        let mut program = SafetyProgram::new();
        let temp_range = TemperatureRange {
            min_temp: 2.0,
            max_temp: 8.0,
            current_temp: 10.0,
        };

        program.monitor_storage_temperature("Refrigerator 1".to_string(), temp_range);
        assert_eq!(program.get_temperature_violations().len(), 1);
    }

    // Add more tests as needed
}
