// safer.h - Main header file
#ifndef SAFER_H
#define SAFER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

// Risk level enumeration
typedef enum {
    RISK_LOW,
    RISK_MEDIUM,
    RISK_HIGH,
    RISK_CRITICAL
} RiskLevel;

// Weather conditions structure
typedef struct {
    float temperature;
    float visibility;
    float wind_speed;
    float precipitation;
} WeatherCondition;

// Maintenance record structure
typedef struct {
    char aircraft_id[16];
    time_t last_inspection;
    time_t maintenance_due;
    char **reported_issues;
    int num_issues;
} MaintenanceRecord;

// Crew member structure
typedef struct {
    char id[16];
    char name[64];
    char role[32];
    char certification[32];
    int flight_hours;
    time_t last_training;
} CrewMember;

// Aircraft structure
typedef struct {
    char id[16];
    char model[32];
    time_t manufacture_date;
    int total_flight_hours;
    MaintenanceRecord *maintenance_records;
    int num_records;
} Aircraft;

// Mission structure
typedef struct {
    char id[16];
    Aircraft *aircraft;
    CrewMember **crew;
    int crew_size;
    time_t departure_time;
    float estimated_duration;
    char mission_type[32];
    WeatherCondition weather;
    RiskLevel risk_level;
} Mission;

// Safety Management System structure
typedef struct {
    Aircraft **aircraft_registry;
    int num_aircraft;
    CrewMember **crew_registry;
    int num_crew;
    Mission **missions;
    int num_missions;
} SafetyManagementSystem;

// Function declarations
RiskLevel assess_weather_risk(WeatherCondition *weather);
RiskLevel assess_maintenance_risk(MaintenanceRecord *record);
RiskLevel assess_crew_risk(CrewMember *crew);
void perform_risk_assessment(Mission *mission);
void generate_safety_report(SafetyManagementSystem *sms, time_t start_date, time_t end_date);

#endif // SAFER_H

// safer.c - Implementation file
#include "safer.h"

RiskLevel assess_weather_risk(WeatherCondition *weather) {
    int risk_score = 0;
    
    // Assess visibility risk
    if (weather->visibility < 1000) {
        risk_score += 3;
    } else if (weather->visibility < 3000) {
        risk_score += 2;
    } else if (weather->visibility < 5000) {
        risk_score += 1;
    }
    
    // Assess wind risk
    if (weather->wind_speed > 50) {
        risk_score += 3;
    } else if (weather->wind_speed > 30) {
        risk_score += 2;
    } else if (weather->wind_speed > 15) {
        risk_score += 1;
    }
    
    // Convert score to risk level
    if (risk_score >= 5) return RISK_CRITICAL;
    if (risk_score >= 3) return RISK_HIGH;
    if (risk_score >= 1) return RISK_MEDIUM;
    return RISK_LOW;
}

RiskLevel assess_maintenance_risk(MaintenanceRecord *record) {
    time_t current_time = time(NULL);
    double days_since_inspection = difftime(current_time, record->last_inspection) / (24 * 3600);
    
    if (days_since_inspection > 180 || record->num_issues > 2) {
        return RISK_CRITICAL;
    } else if (days_since_inspection > 90 || record->num_issues > 0) {
        return RISK_HIGH;
    } else if (days_since_inspection > 45) {
        return RISK_MEDIUM;
    }
    return RISK_LOW;
}

RiskLevel assess_crew_risk(CrewMember *crew) {
    time_t current_time = time(NULL);
    double days_since_training = difftime(current_time, crew->last_training) / (24 * 3600);
    
    if (days_since_training > 180) {
        return RISK_CRITICAL;
    }
    
    if (crew->flight_hours < 100) {
        return RISK_HIGH;
    } else if (crew->flight_hours < 500) {
        return RISK_MEDIUM;
    }
    return RISK_LOW;
}

void perform_risk_assessment(Mission *mission) {
    RiskLevel weather_risk = assess_weather_risk(&mission->weather);
    RiskLevel maintenance_risk = assess_maintenance_risk(mission->aircraft->maintenance_records);
    
    // Find highest crew risk
    RiskLevel max_crew_risk = RISK_LOW;
    for (int i = 0; i < mission->crew_size; i++) {
        RiskLevel crew_risk = assess_crew_risk(mission->crew[i]);
        if (crew_risk > max_crew_risk) {
            max_crew_risk = crew_risk;
        }
    }
    
    // Overall risk is the highest of all risks
    mission->risk_level = weather_risk;
    if (maintenance_risk > mission->risk_level) mission->risk_level = maintenance_risk;
    if (max_crew_risk > mission->risk_level) mission->risk_level = max_crew_risk;
}

void generate_safety_report(SafetyManagementSystem *sms, time_t start_date, time_t end_date) {
    int risk_distribution[4] = {0}; // Count of missions at each risk level
    int total_missions = 0;
    
    for (int i = 0; i < sms->num_missions; i++) {
        Mission *mission = sms->missions[i];
        if (mission->departure_time >= start_date && mission->departure_time <= end_date) {
            risk_distribution[mission->risk_level]++;
            total_missions++;
        }
    }
    
    // Print report
    printf("\nSAFER Safety Report\n");
    printf("Period: %s", ctime(&start_date));
    printf("To: %s\n", ctime(&end_date));
    printf("Total Missions: %d\n\n", total_missions);
    printf("Risk Distribution:\n");
    printf("Low Risk: %d (%.1f%%)\n", risk_distribution[RISK_LOW], 
           (float)risk_distribution[RISK_LOW] * 100 / total_missions);
    printf("Medium Risk: %d (%.1f%%)\n", risk_distribution[RISK_MEDIUM],
           (float)risk_distribution[RISK_MEDIUM] * 100 / total_missions);
    printf("High Risk: %d (%.1f%%)\n", risk_distribution[RISK_HIGH],
           (float)risk_distribution[RISK_HIGH] * 100 / total_missions);
    printf("Critical Risk: %d (%.1f%%)\n", risk_distribution[RISK_CRITICAL],
           (float)risk_distribution[RISK_CRITICAL] * 100 / total_missions);
}

// database.h - Database management header
#ifndef DATABASE_H
#define DATABASE_H

#include "safer.h"
#include <sqlite3.h>

typedef struct {
    sqlite3 *db;
    char *err_msg;
} Database;

// Database function declarations
int init_database(Database *db);
int save_mission(Database *db, Mission *mission);
int save_aircraft(Database *db, Aircraft *aircraft);
int save_crew_member(Database *db, CrewMember *crew);
Mission* load_mission(Database *db, const char *mission_id);
Aircraft* load_aircraft(Database *db, const char *aircraft_id);
CrewMember* load_crew_member(Database *db, const char *crew_id);

#endif // DATABASE_H

// database.c - Database implementation
#include "database.h"

int init_database(Database *db) {
    int rc = sqlite3_open("safer.db", &db->db);
    if (rc) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }
    
    // Create tables
    const char *sql = 
        "CREATE TABLE IF NOT EXISTS aircraft ("
        "id TEXT PRIMARY KEY,"
        "model TEXT,"
        "manufacture_date INTEGER,"
        "total_flight_hours INTEGER"
        ");"
        
        "CREATE TABLE IF NOT EXISTS crew_members ("
        "id TEXT PRIMARY KEY,"
        "name TEXT,"
        "role TEXT,"
        "certification TEXT,"
        "flight_hours INTEGER,"
        "last_training INTEGER"
        ");"
        
        "CREATE TABLE IF NOT EXISTS missions ("
        "id TEXT PRIMARY KEY,"
        "aircraft_id TEXT,"
        "departure_time INTEGER,"
        "estimated_duration REAL,"
        "mission_type TEXT,"
        "risk_level INTEGER,"
        "FOREIGN KEY(aircraft_id) REFERENCES aircraft(id)"
        ");";
    
    char *err_msg = 0;
    rc = sqlite3_exec(db->db, sql, 0, 0, &err_msg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return rc;
    }
    
    return SQLITE_OK;
}

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
