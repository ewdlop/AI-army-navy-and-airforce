use actix_web::{web, App, HttpServer};
use dotenv::dotenv;
use std::env;

mod models;
mod api;
mod radio;
mod db;
mod error;

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    dotenv().ok();
    env_logger::init();

    let database_url = env::var("DATABASE_URL")
        .expect("DATABASE_URL must be set");
    
    let pool = db::create_pool(&database_url)
        .await
        .expect("Failed to create database pool");

    HttpServer::new(move || {
        App::new()
            .app_data(web::Data::new(pool.clone()))
            .configure(api::config)
    })
    .bind("127.0.0.1:8080")?
    .run()
    .await
}