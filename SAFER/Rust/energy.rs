// src/energy.rs
use serde::{Deserialize, Serialize};
use chrono::{DateTime, Utc};
use crate::models::RiskLevel;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum EnergySourceType {
    HydrogenFuelCell,
    SolidStateElectric,
    NuclearThermal,
    Hybrid,
    SolarElectric,
    ChemicalFuel,  // For comparison
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct EnergySource {
    pub source_type: EnergySourceType,
    pub capacity: f64,         // kWh
    pub current_level: f64,    // Percentage
    pub efficiency: f64,       // Percentage
    pub power_output: f64,     // kW
    pub temperature: f64,      // Celsius
    pub maintenance_hours: i32,
    pub last_inspection: DateTime<Utc>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct HydrogenSystem {
    pub storage_pressure: f64,    // MPa
    pub tank_temperature: f64,    // Celsius
    pub purity_level: f64,        // Percentage
    pub cell_stack_voltage: f64,  // Volts
    pub hydrogen_level: f64,      // Percentage
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ElectricSystem {
    pub battery_health: f64,      // Percentage
    pub charge_cycles: i32,
    pub cell_temperature: f64,    // Celsius
    pub voltage: f64,             // Volts
    pub current: f64,             // Amperes
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NuclearSystem {
    pub reactor_temperature: f64,  // Celsius
    pub coolant_pressure: f64,    // MPa
    pub radiation_level: f64,     // mSv/h
    pub power_output: f64,        // MW
    pub core_life_remaining: f64, // Percentage
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct EnergyAnalysis {
    pub source_type: EnergySourceType,
    pub risk_level: RiskLevel,
    pub range_estimate: f64,      // km
    pub efficiency_rating: f64,   // Percentage
    pub environmental_impact: f64, // CO2 equivalent kg/kWh
    pub safety_rating: f64,       // 0-100
    pub recommendations: Vec<String>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct MissionEnergyRequirements {
    pub distance: f64,            // km
    pub duration: f64,            // hours
    pub altitude: f64,            // meters
    pub payload_weight: f64,      // kg
    pub emergency_reserve: f64,   // Percentage
}

impl EnergySource {
    pub fn analyze(&self, requirements: &MissionEnergyRequirements) -> EnergyAnalysis {
        match self.source_type {
            EnergySourceType::HydrogenFuelCell => self.analyze_hydrogen(requirements),
            EnergySourceType::SolidStateElectric => self.analyze_electric(requirements),
            EnergySourceType::NuclearThermal => self.analyze_nuclear(requirements),
            EnergySourceType::SolarElectric => self.analyze_solar(requirements),
            EnergySourceType::Hybrid => self.analyze_hybrid(requirements),
            EnergySourceType::ChemicalFuel => self.analyze_chemical(requirements),
        }
    }

    fn analyze_hydrogen(&self, requirements: &MissionEnergyRequirements) -> EnergyAnalysis {
        let efficiency_rating = 0.60; // 60% typical for fuel cells
        let range_estimate = self.capacity * efficiency_rating * 0.5; // km/kWh approximation
        let environmental_impact = 0.0; // Zero direct emissions
        
        let safety_rating = self.calculate_hydrogen_safety();
        let risk_level = self.determine_risk_level(safety_rating);
        
        let mut recommendations = Vec::new();
        
        if safety_rating < 80.0 {
            recommendations.push("Schedule hydrogen system inspection".to_string());
        }
        
        if self.current_level < requirements.emergency_reserve {
            recommendations.push("Insufficient hydrogen reserves for mission".to_string());
        }

        EnergyAnalysis {
            source_type: EnergySourceType::HydrogenFuelCell,
            risk_level,
            range_estimate,
            efficiency_rating,
            environmental_impact,
            safety_rating,
            recommendations,
        }
    }

    fn analyze_electric(&self, requirements: &MissionEnergyRequirements) -> EnergyAnalysis {
        let efficiency_rating = 0.95; // 95% typical for electric systems
        let range_estimate = self.capacity * efficiency_rating * 0.3; // km/kWh approximation
        let environmental_impact = 0.0; // Zero direct emissions
        
        let safety_rating = self.calculate_electric_safety();
        let risk_level = self.determine_risk_level(safety_rating);
        
        let mut recommendations = Vec::new();
        
        if self.temperature > 45.0 {
            recommendations.push("Battery temperature exceeds recommended limits".to_string());
        }
        
        if self.current_level < requirements.emergency_reserve {
            recommendations.push("Battery charge below mission requirements".to_string());
        }

        EnergyAnalysis {
            source_type: EnergySourceType::SolidStateElectric,
            risk_level,
            range_estimate,
            efficiency_rating,
            environmental_impact,
            safety_rating,
            recommendations,
        }
    }

    fn analyze_nuclear(&self, requirements: &MissionEnergyRequirements) -> EnergyAnalysis {
        let efficiency_rating = 0.30; // 30% typical for small nuclear systems
        let range_estimate = f64::INFINITY; // Theoretical unlimited range
        let environmental_impact = 0.0; // Zero direct emissions
        
        let safety_rating = self.calculate_nuclear_safety();
        let risk_level = self.determine_risk_level(safety_rating);
        
        let mut recommendations = Vec::new();
        
        if self.temperature > 800.0 {
            recommendations.push("Core temperature approaching limits".to_string());
        }
        
        if self.maintenance_hours > 1000 {
            recommendations.push("Schedule reactor maintenance".to_string());
        }

        EnergyAnalysis {
            source_type: EnergySourceType::NuclearThermal,
            risk_level,
            range_estimate,
            efficiency_rating,
            environmental_impact,
            safety_rating,
            recommendations,
        }
    }

    fn analyze_solar(&self, requirements: &MissionEnergyRequirements) -> EnergyAnalysis {
        let efficiency_rating = 0.20; // 20% typical for solar cells
        let range_estimate = self.calculate_solar_range(requirements.altitude);
        let environmental_impact = 0.0; // Zero direct emissions
        
        let safety_rating = 95.0; // Very safe technology
        let risk_level = self.determine_risk_level(safety_rating);
        
        let mut recommendations = Vec::new();
        
        if requirements.altitude < 10000.0 {
            recommendations.push("Increase altitude for optimal solar collection".to_string());
        }
        
        if self.efficiency < 0.15 {
            recommendations.push("Solar panel efficiency below optimal".to_string());
        }

        EnergyAnalysis {
            source_type: EnergySourceType::SolarElectric,
            risk_level,
            range_estimate,
            efficiency_rating,
            environmental_impact,
            safety_rating,
            recommendations,
        }
    }

    fn calculate_hydrogen_safety(&self) -> f64 {
        let mut safety_score = 100.0;
        
        // Temperature factors
        if self.temperature > 80.0 {
            safety_score -= (self.temperature - 80.0) * 2.0;
        }
        
        // Maintenance factors
        let hours_since_maintenance = self.maintenance_hours;
        if hours_since_maintenance > 500 {
            safety_score -= (hours_since_maintenance - 500) as f64 * 0.1;
        }
        
        safety_score.max(0.0).min(100.0)
    }

    fn calculate_electric_safety(&self) -> f64 {
        let mut safety_score = 100.0;
        
        // Temperature factors
        if self.temperature > 45.0 {
            safety_score -= (self.temperature - 45.0) * 2.0;
        }
        
        // Efficiency factors
        if self.efficiency < 0.8 {
            safety_score -= (0.8 - self.efficiency) * 100.0;
        }
        
        safety_score.max(0.0).min(100.0)
    }

    fn calculate_nuclear_safety(&self) -> f64 {
        let mut safety_score = 100.0;
        
        // Temperature factors
        if self.temperature > 800.0 {
            safety_score -= (self.temperature - 800.0) * 0.5;
        }
        
        // Maintenance factors
        let hours_since_maintenance = self.maintenance_hours;
        if hours_since_maintenance > 1000 {
            safety_score -= (hours_since_maintenance - 1000) as f64 * 0.05;
        }
        
        safety_score.max(0.0).min(100.0)
    }

    fn calculate_solar_range(&self, altitude: f64) -> f64 {
        // Basic solar range calculation based on altitude and efficiency
        let base_solar_power = 1000.0; // W/m² at sea level
        let altitude_factor = 1.0 + (altitude / 10000.0) * 0.2;
        let effective_power = base_solar_power * altitude_factor * self.efficiency;
        
        effective_power * self.capacity // Simplified range estimation
    }

    fn determine_risk_level(&self, safety_rating: f64) -> RiskLevel {
        match safety_rating {
            s if s >= 90.0 => RiskLevel::Low,
            s if s >= 75.0 => RiskLevel::Medium,
            s if s >= 60.0 => RiskLevel::High,
            _ => RiskLevel::Critical,
        }
    }
}

// Example usage in API endpoint
pub async fn analyze_energy_source(
    source: web::Json<EnergySource>,
    requirements: web::Json<MissionEnergyRequirements>,
) -> Result<HttpResponse, SaferError> {
    let analysis = source.analyze(&requirements);
    Ok(HttpResponse::Ok().json(analysis))
}

// Database schema additions
pub const ENERGY_MIGRATIONS: &str = "
ALTER TABLE missions ADD COLUMN energy_source_type energy_source_type;
ALTER TABLE missions ADD COLUMN energy_analysis jsonb;

CREATE TYPE energy_source_type AS ENUM (
    'HydrogenFuelCell',
    'SolidStateElectric',
    'NuclearThermal',
    'Hybrid',
    'SolarElectric',
    'ChemicalFuel'
);

CREATE TABLE energy_readings (
    id SERIAL PRIMARY KEY,
    mission_id TEXT REFERENCES missions(id),
    source_type energy_source_type NOT NULL,
    capacity DOUBLE PRECISION NOT NULL,
    current_level DOUBLE PRECISION NOT NULL,
    efficiency DOUBLE PRECISION NOT NULL,
    power_output DOUBLE PRECISION NOT NULL,
    temperature DOUBLE PRECISION NOT NULL,
    recorded_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);
";
