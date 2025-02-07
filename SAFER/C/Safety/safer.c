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