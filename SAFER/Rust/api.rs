use actix_web::{web, HttpResponse, Scope};
use serde_json::json;

use crate::models::{Mission, RadioEnvironment};
use crate::db::DbPool;
use crate::error::SaferError;

pub fn config(cfg: &mut web::ServiceConfig) {
    cfg.service(
        web::scope("/api")
            .service(web::resource("/mission")
                .route(web::post().to(create_mission))
                .route(web::get().to(get_mission)))
            .service(web::resource("/radio-analysis")
                .route(web::post().to(analyze_radio)))
    );
}

async fn create_mission(
    pool: web::Data<DbPool>,
    mission: web::Json<Mission>,
) -> Result<HttpResponse, SaferError> {
    let mission = mission.into_inner();
    
    // Perform radio analysis
    let radio_env = RadioEnvironment {
        sources: vec![], // Populate from mission data
        background_noise: -90.0,
        weather_factor: 1.0,
    };
    let radio_analysis = radio_env.analyze();

    // Store mission in database
    sqlx::query!(
        "INSERT INTO missions (id, aircraft_id, departure_time, estimated_duration, mission_type, risk_level)
         VALUES ($1, $2, $3, $4, $5, $6)",
        mission.id,
        mission.aircraft_id,
        mission.departure_time,
        mission.estimated_duration,
        mission.mission_type,
        radio_analysis.risk_level as RiskLevel,
    )
    .execute(pool.as_ref())
    .await?;

    Ok(HttpResponse::Ok().json(json!({
        "mission_id": mission.id,
        "risk_level": radio_analysis.risk_level,
        "radio_analysis": radio_analysis
    })))
}

async fn get_mission(
    pool: web::Data<DbPool>,
    id: web::Query<String>,
) -> Result<HttpResponse, SaferError> {
    let mission = sqlx::query_as!(
        Mission,
        "SELECT * FROM missions WHERE id = $1",
        id.into_inner()
    )
    .fetch_one(pool.as_ref())
    .await?;

    Ok(HttpResponse::Ok().json(mission))
}

async fn analyze_radio(
    env: web::Json<RadioEnvironment>,
) -> Result<HttpResponse, SaferError> {
    let analysis = env.into_inner().analyze();
    Ok(HttpResponse::Ok().json(analysis))
}