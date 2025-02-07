/ safer.h - Main header file
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
