// radio_interference.h
#ifndef RADIO_INTERFERENCE_H
#define RADIO_INTERFERENCE_H

#include <math.h>
#include <complex.h>

typedef struct {
    double frequency;      // MHz
    double power;         // dBm
    double distance;      // km
    double terrain_factor; // terrain roughness factor
} RadioSource;

typedef struct {
    RadioSource *sources;
    int num_sources;
    double background_noise; // dBm
    double weather_factor;   // Attenuation due to weather
} RadioEnvironment;

typedef struct {
    double interference_level; // dBm
    double signal_to_noise;    // dB
    RiskLevel risk_level;
    char *recommendations;
} RadioInterferenceAnalysis;

RadioInterferenceAnalysis analyze_radio_interference(RadioEnvironment *env);
double calculate_path_loss(RadioSource *source);
RiskLevel assess_radio_risk(RadioInterferenceAnalysis *analysis);

#endif // RADIO_INTERFERENCE_H