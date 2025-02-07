use chrono::{DateTime, Utc};
use serde::{Deserialize, Serialize};
use sqlx::FromRow;

#[derive(Debug, Clone, Copy, Serialize, Deserialize, PartialEq)]
pub enum RiskLevel {
    Low,
    Medium,
    High,
    Critical,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct WeatherCondition {
    pub temperature: f64,
    pub visibility: f64,
    pub wind_speed: f64,
    pub precipitation: f64,
}

#[derive(Debug, Serialize, Deserialize, FromRow)]
pub struct MaintenanceRecord {
    pub aircraft_id: String,
    pub last_inspection: DateTime<Utc>,
    pub maintenance_due: DateTime<Utc>,
    pub reported_issues: Vec<String>,
}

#[derive(Debug, Serialize, Deserialize, FromRow)]
pub struct CrewMember {
    pub id: String,
    pub name: String,
    pub role: String,
    pub certification_level: String,
    pub flight_hours: i32,
    pub last_training: DateTime<Utc>,
}

#[derive(Debug, Serialize, Deserialize, FromRow)]
pub struct Aircraft {
    pub id: String,
    pub model: String,
    pub manufacture_date: DateTime<Utc>,
    pub total_flight_hours: i32,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Mission {
    pub id: String,
    pub aircraft_id: String,
    pub crew_ids: Vec<String>,
    pub departure_time: DateTime<Utc>,
    pub estimated_duration: f64,
    pub mission_type: String,
    pub weather: WeatherCondition,
    pub risk_level: Option<RiskLevel>,
}

// src/radio.rs
use crate::models::RiskLevel;
use num_complex::Complex64;
use serde::{Deserialize, Serialize};
use std::f64::consts::PI;

const SPEED_OF_LIGHT: f64 = 299_792_458.0;

#[derive(Debug, Serialize, Deserialize)]
pub struct RadioSource {
    pub frequency: f64,    // MHz
    pub power: f64,       // dBm
    pub distance: f64,    // km
    pub terrain_factor: f64,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct RadioEnvironment {
    pub sources: Vec<RadioSource>,
    pub background_noise: f64,  // dBm
    pub weather_factor: f64,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct RadioInterferenceAnalysis {
    pub interference_level: f64,
    pub signal_to_noise: f64,
    pub risk_level: RiskLevel,
    pub recommendations: String,
}

impl RadioEnvironment {
    pub fn analyze(&self) -> RadioInterferenceAnalysis {
        let total_interference: f64 = self.sources.iter()
            .map(|source| {
                let path_loss = source.calculate_path_loss();
                let received_power = source.power - path_loss;
                10.0_f64.powf(received_power / 10.0)
            })
            .sum();

        let interference_level = 10.0 * total_interference.log10();
        let signal_to_noise = interference_level - self.background_noise;

        let (risk_level, recommendations) = match signal_to_noise {
            snr if snr > 30.0 => (
                RiskLevel::Low,
                "Normal operations can proceed".to_string()
            ),
            snr if snr > 20.0 => (
                RiskLevel::Medium,
                "Consider frequency adjustment or power increase".to_string()
            ),
            snr if snr > 10.0 => (
                RiskLevel::High,
                "Immediate frequency reallocation recommended".to_string()
            ),
            _ => (
                RiskLevel::Critical,
                "Unsafe for critical communications. Abort mission if communication dependent".to_string()
            ),
        };

        RadioInterferenceAnalysis {
            interference_level,
            signal_to_noise,
            risk_level,
            recommendations,
        }
    }
}

impl RadioSource {
    fn calculate_path_loss(&self) -> f64 {
        let wavelength = SPEED_OF_LIGHT / (self.frequency * 1e6);
        let distance_m = self.distance * 1000.0;
        
        let basic_loss = 20.0 * (4.0 * PI * distance_m / wavelength).log10();
        basic_loss + self.terrain_factor * self.distance.log10()
    }
}