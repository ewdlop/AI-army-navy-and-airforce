use thiserror::Error;

#[derive(Error, Debug)]
pub enum SaferError {
    #[error("Database error: {0}")]
    Database(#[from] sqlx::Error),

    #[error("Validation error: {0}")]
    Validation(String),

    #[error("Internal server error")]
    Internal(#[from] anyhow::Error),
}

impl actix_web::ResponseError for SaferError {
    fn error_response(&self) -> HttpResponse {
        match self {
            SaferError::Database(_) => HttpResponse::InternalServerError().json(json!({
                "error": "Database error occurred"
            })),
            SaferError::Validation(msg) => HttpResponse::BadRequest().json(json!({
                "error": msg
            })),
            SaferError::Internal(_) => HttpResponse::InternalServerError().json(json!({
                "error": "Internal server error"
            })),
        }
    }
}