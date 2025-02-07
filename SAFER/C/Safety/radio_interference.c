// radio_interference.c
#include "radio_interference.h"

#define C 299792458.0  // Speed of light in m/s
#define PI 3.14159265359

double calculate_path_loss(RadioSource *source) {
    // Implementation of Extended Hata Model for path loss
    double wavelength = C / (source->frequency * 1e6);
    double path_loss = 20 * log10(4 * PI * source->distance * 1000 / wavelength);
    
    // Add terrain and weather effects
    path_loss += source->terrain_factor * log10(source->distance);
    
    return path_loss;
}

RadioInterferenceAnalysis analyze_radio_interference(RadioEnvironment *env) {
    RadioInterferenceAnalysis analysis = {0};
    double total_interference = 0;
    
    // Calculate cumulative interference from all sources
    for (int i = 0; i < env->num_sources; i++) {
        double path_loss = calculate_path_loss(&env->sources[i]);
        double received_power = env->sources[i].power - path_loss;
        total_interference += pow(10, received_power/10);
    }
    
    // Convert back to dBm
    analysis.interference_level = 10 * log10(total_interference);
    
    // Calculate Signal to Noise Ratio
    analysis.signal_to_noise = analysis.interference_level - env->background_noise;
    
    // Assess risk level based on interference
    if (analysis.signal_to_noise > 30) {
        analysis.risk_level = RISK_LOW;
        analysis.recommendations = "Normal operations can proceed";
    } else if (analysis.signal_to_noise > 20) {
        analysis.risk_level = RISK_MEDIUM;
        analysis.recommendations = "Consider frequency adjustment or power increase";
    } else if (analysis.signal_to_noise > 10) {
        analysis.risk_level = RISK_HIGH;
        analysis.recommendations = "Immediate frequency reallocation recommended";
    } else {
        analysis.risk_level = RISK_CRITICAL;
        analysis.recommendations = "Unsafe for critical communications. Abort mission if communication dependent";
    }
    
    return analysis;
}
