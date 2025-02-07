// main.c - Example usage
#include "safer.h"
#include "database.h"

int main() {
    // Initialize safety management system
    SafetyManagementSystem sms = {0};
    Database db = {0};
    
    // Initialize database
    if (init_database(&db) != SQLITE_OK) {
        fprintf(stderr, "Failed to initialize database\n");
        return 1;
    }
    
    // Create example aircraft
    Aircraft aircraft = {
        .id = "AC001",
        .model = "F-35",
        .manufacture_date = time(NULL) - 365 * 24 * 3600, // 1 year ago
        .total_flight_hours = 1000
    };
    
    // Create maintenance record
    MaintenanceRecord maintenance = {
        .aircraft_id = "AC001",
        .last_inspection = time(NULL) - 30 * 24 * 3600, // 30 days ago
        .maintenance_due = time(NULL) + 60 * 24 * 3600, // 60 days from now
        .num_issues = 0
    };
    
    aircraft.maintenance_records = &maintenance;
    aircraft.num_records = 1;
    
    // Create crew members
    CrewMember pilot = {
        .id = "P001",
        .name = "John Doe",
        .role = "Pilot",
        .certification = "Senior",
        .flight_hours = 2000,
        .last_training = time(NULL) - 90 * 24 * 3600 // 90 days ago
    };
    
    CrewMember copilot = {
        .id = "P002",
        .name = "Jane Smith",
        .role = "Co-Pilot",
        .certification = "Mid-Level",
        .flight_hours = 1000,
        .last_training = time(NULL) - 60 * 24 * 3600 // 60 days ago
    };
    
    // Create mission
    CrewMember *crew[] = {&pilot, &copilot};
    Mission mission = {
        .id = "M001",
        .aircraft = &aircraft,
        .crew = crew,
        .crew_size = 2,
        .departure_time = time(NULL) + 24 * 3600, // 24 hours from now
        .estimated_duration = 2.5,
        .mission_type = "Training",
        .weather = {
            .temperature = 25.0,
            .visibility = 5000.0,
            .wind_speed = 10.0,
            .precipitation = 0.0
        }
    };
    
    // Perform risk assessment
    perform_risk_assessment(&mission);
    
    // Save to database
    save_aircraft(&db, &aircraft);
    save_crew_member(&db, &pilot);
    save_crew_member(&db, &copilot);
    save_mission(&db, &mission);
    
    // Generate safety report
    time_t start_date = time(NULL) - 30 * 24 * 3600; // 30 days ago
    time_t end_date = time(NULL) + 30 * 24 * 3600;   // 30 days from now
    generate_safety_report(&sms, start_date, end_date);
    
    // Cleanup
    sqlite3_close(db.db);
    
    return 0;
}
