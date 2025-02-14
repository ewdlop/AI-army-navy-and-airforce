use chrono::{DateTime, Duration, Utc};
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use uuid::Uuid;

// Diving and Swimming Physics Types
#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct DivingConditions {
    depth: f64,              // in meters
    water_temperature: f32,  // in Celsius
    visibility: f32,         // in meters
    current_speed: f32,      // in knots
    water_density: f32,      // in kg/m³
    salinity: f32,          // in PSU (Practical Salinity Unit)
    pressure: f32,          // in bar
    oxygen_partial_pressure: f32, // in ata
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct DiveProfile {
    id: Uuid,
    diver_id: Uuid,
    start_time: DateTime<Utc>,
    end_time: DateTime<Utc>,
    max_depth: f64,
    bottom_time: Duration,
    decompression_stops: Vec<DecompressionStop>,
    air_consumption: f32,    // in liters
    remaining_air: f32,      // in bar
    no_decompression_limit: Duration,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct DecompressionStop {
    depth: f64,
    duration: Duration,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct DivingEquipment {
    id: Uuid,
    equipment_type: DivingGearType,
    last_inspection: DateTime<Utc>,
    next_inspection_due: DateTime<Utc>,
    max_depth_rating: f64,
    maintenance_history: Vec<MaintenanceRecord>,
}

#[derive(Debug, Serialize, Deserialize, Clone, PartialEq)]
pub enum DivingGearType {
    Tank,
    Regulator,
    BCD,
    Wetsuit,
    Computer,
    Gauge,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct SwimmingPhysics {
    buoyancy: f32,          // in Newtons
    drag_coefficient: f32,
    reynolds_number: f32,
    propulsion_force: f32,  // in Newtons
    velocity: f32,          // in m/s
    energy_expenditure: f32, // in calories/hour
}

impl SwimmingPhysics {
    pub fn calculate_buoyant_force(&self, volume: f32, fluid_density: f32) -> f32 {
        const GRAVITY: f32 = 9.81;
        volume * fluid_density * GRAVITY
    }

    pub fn calculate_drag_force(&self, velocity: f32, area: f32, fluid_density: f32) -> f32 {
        0.5 * self.drag_coefficient * fluid_density * area * velocity * velocity
    }

    pub fn calculate_reynolds_number(&self, velocity: f32, characteristic_length: f32, kinematic_viscosity: f32) -> f32 {
        velocity * characteristic_length / kinematic_viscosity
    }
}

// Diving Safety Calculator
pub struct DivingSafetyCalculator {
    pressure_conversion_factor: f32,  // meters to bar
    oxygen_toxicity_limit: f32,      // in OTUs
    nitrogen_loading: f32,           // in tissue pressure
}

impl DivingSafetyCalculator {
    pub fn new() -> Self {
        DivingSafetyCalculator {
            pressure_conversion_factor: 0.1,
            oxygen_toxicity_limit: 650.0,
            nitrogen_loading: 0.0,
        }
    }

    pub fn calculate_pressure_at_depth(&self, depth: f64) -> f32 {
        (depth as f32 * self.pressure_conversion_factor) + 1.0 // Adding 1 atm
    }

    pub fn calculate_no_decompression_limit(&self, depth: f64, previous_dives: &[DiveProfile]) -> Duration {
        // Simplified NDL calculation based on depth and previous nitrogen loading
        let pressure = self.calculate_pressure_at_depth(depth);
        let base_limit = match depth as i32 {
            0..=10 => Duration::minutes(219),
            11..=20 => Duration::minutes(147),
            21..=30 => Duration::minutes(72),
            31..=40 => Duration::minutes(37),
            _ => Duration::minutes(10),
        };

        if previous_dives.is_empty() {
            base_limit
        } else {
            // Reduce time based on previous nitrogen loading
            base_limit / 2 // Simplified reduction - real implementation would use tissue loading calculation
        }
    }

    pub fn calculate_oxygen_partial_pressure(&self, depth: f64, oxygen_fraction: f32) -> f32 {
        self.calculate_pressure_at_depth(depth) * oxygen_fraction
    }

    pub fn estimate_air_consumption(&self, depth: f64, duration: Duration, work_rate: f32) -> f32 {
        let pressure = self.calculate_pressure_at_depth(depth);
        let surface_consumption = 20.0; // Liters per minute at surface
        let depth_adjusted_consumption = surface_consumption * pressure;
        depth_adjusted_consumption * work_rate * (duration.num_minutes() as f32)
    }
}

// Enhanced SafetyProgram with diving components
pub struct SafetyProgram {
    // Previous fields...
    [Previous fields from the last implementation...]
    
    // New diving-related fields
    diving_equipment: HashMap<Uuid, DivingEquipment>,
    dive_profiles: Vec<DiveProfile>,
    diving_conditions: Vec<DivingConditions>,
    diving_calculator: DivingSafetyCalculator,
}

impl SafetyProgram {
    // Previous methods remain...
    [Previous methods from the last implementation...]

    // New diving-related methods
    pub fn add_diving_equipment(&mut self, equipment: DivingEquipment) -> Result<(), String> {
        if self.diving_equipment.contains_key(&equipment.id) {
            return Err("Equipment already exists".to_string());
        }
        self.diving_equipment.insert(equipment.id, equipment);
        Ok(())
    }

    pub fn record_dive_profile(&mut self, profile: DiveProfile) {
        self.dive_profiles.push(profile);
    }

    pub fn get_diver_profiles(&self, diver_id: Uuid) -> Vec<&DiveProfile> {
        self.dive_profiles
            .iter()
            .filter(|profile| profile.diver_id == diver_id)
            .collect()
    }

    pub fn calculate_safe_dive_time(&self, depth: f64, diver_id: Uuid) -> Duration {
        let previous_dives = self.get_diver_profiles(diver_id);
        self.diving_calculator.calculate_no_decompression_limit(depth, &previous_dives)
    }

    pub fn check_equipment_safety(&self, equipment_id: Uuid) -> Result<bool, String> {
        if let Some(equipment) = self.diving_equipment.get(&equipment_id) {
            Ok(equipment.next_inspection_due > Utc::now())
        } else {
            Err("Equipment not found".to_string())
        }
    }

    pub fn log_diving_conditions(&mut self, conditions: DivingConditions) {
        self.diving_conditions.push(conditions);
    }

    pub fn get_latest_diving_conditions(&self) -> Option<&DivingConditions> {
        self.diving_conditions.last()
    }

    pub fn generate_dive_safety_report(&self, diver_id: Uuid) -> DiveSafetyReport {
        let profiles = self.get_diver_profiles(diver_id);
        let latest_conditions = self.get_latest_diving_conditions();
        
        DiveSafetyReport {
            diver_id,
            total_dives: profiles.len(),
            cumulative_bottom_time: profiles.iter()
                .map(|p| p.bottom_time)
                .sum(),
            max_depth_reached: profiles.iter()
                .map(|p| p.max_depth)
                .max_by(|a, b| a.partial_cmp(b).unwrap())
                .unwrap_or(0.0),
            current_conditions: latest_conditions.cloned(),
            timestamp: Utc::now(),
        }
    }
}

#[derive(Debug, Serialize, Deserialize)]
pub struct DiveSafetyReport {
    diver_id: Uuid,
    total_dives: usize,
    cumulative_bottom_time: Duration,
    max_depth_reached: f64,
    current_conditions: Option<DivingConditions>,
    timestamp: DateTime<Utc>,
}

// Tests
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_diving_calculator() {
        let calculator = DivingSafetyCalculator::new();
        let pressure = calculator.calculate_pressure_at_depth(30.0);
        assert!(pressure > 1.0); // Pressure should be greater than surface pressure

        let ndl = calculator.calculate_no_decompression_limit(30.0, &[]);
        assert!(ndl > Duration::minutes(0));
    }

    #[test]
    fn test_swimming_physics() {
        let physics = SwimmingPhysics {
            buoyancy: 0.0,
            drag_coefficient: 0.3,
            reynolds_number: 0.0,
            propulsion_force: 100.0,
            velocity: 1.5,
            energy_expenditure: 500.0,
        };

        let volume = 0.07; // 70 liters
        let water_density = 1025.0; // kg/m³ (seawater)
        let buoyant_force = physics.calculate_buoyant_force(volume, water_density);
        assert!(buoyant_force > 0.0);
    }

    // Add more tests as needed...
}
