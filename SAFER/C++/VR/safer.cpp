
} // namespace SAFER

// SAFER.cpp
#include "SAFER.hpp"
#include <cmath>
#include <iostream>

namespace SAFER {

SAFERSystem::SAFERSystem() : m_vrSystem(nullptr) {
    m_trackedDevicePoses.resize(vr::k_unMaxTrackedDeviceCount);
}

SAFERSystem::~SAFERSystem() {
    Shutdown();
}

bool SAFERSystem::Initialize() {
    if (!InitializeOpenVR()) {
        return false;
    }

    m_safetySystem = std::make_shared<SafetySystem>(m_vrSystem);
    m_trainingModule = std::make_shared<TrainingModule>(m_vrSystem);
    m_riskAssessment = std::make_shared<RiskAssessment>();

    return true;
}

bool SAFERSystem::InitializeOpenVR() {
    vr::EVRInitError error = vr::VRInitError_None;
    m_vrSystem = vr::VR_Init(&error, vr::VRApplication_Scene);

    if (error != vr::VRInitError_None) {
        std::cerr << "Failed to initialize OpenVR: " 
                  << vr::VR_GetVRInitErrorAsEnglishDescription(error) << std::endl;
        return false;
    }

    return true;
}

void SAFERSystem::Update() {
    if (!m_vrSystem) return;

    // Update tracked device poses
    m_vrSystem->GetDeviceToAbsoluteTrackingPose(
        vr::TrackingUniverseStanding, 
        0.0f,
        m_trackedDevicePoses.data(), 
        vr::k_unMaxTrackedDeviceCount
    );

    // Update subsystems
    m_safetySystem->Update(m_trackedDevicePoses);
    m_riskAssessment->UpdateRiskLevels(m_trackedDevicePoses);
}

void SAFERSystem::Shutdown() {
    if (m_vrSystem) {
        vr::VR_Shutdown();
        m_vrSystem = nullptr;
    }
}

// SafetySystem Implementation
SafetySystem::SafetySystem(vr::IVRSystem* vrSystem) : m_vrSystem(vrSystem) {}

void SafetySystem::Update(const std::vector<vr::TrackedDevicePose_t>& poses) {
    for (uint32_t i = 0; i < poses.size(); i++) {
        if (poses[i].bPoseIsValid) {
            CheckSafetyBoundaries(poses[i]);
        }
    }
}

void SafetySystem::CheckSafetyBoundaries(const vr::TrackedDevicePose_t& pose) {
    const auto& matrix = pose.mDeviceToAbsoluteTracking;
    
    for (auto& zone : m_safetyZones) {
        float dx = matrix.m[0][3] - zone.x;
        float dy = matrix.m[1][3] - zone.y;
        float dz = matrix.m[2][3] - zone.z;
        
        float distance = std::sqrt(dx*dx + dy*dy + dz*dz);
        float risk = CalculateRiskLevel(distance, zone.radius);
        
        if (m_warningCallback) {
            m_warningCallback(zone.id, risk);
        }
    }
}

float SafetySystem::CalculateRiskLevel(float distance, float zoneRadius) {
    return std::max(0.0f, std::min(1.0f, 1.0f - (distance / zoneRadius)));
}

// TrainingModule Implementation
TrainingModule::TrainingModule(vr::IVRSystem* vrSystem) 
    : m_vrSystem(vrSystem), m_currentScenario(nullptr) {}

bool TrainingModule::LoadScenario(const std::string& scenarioId) {
    auto it = m_scenarios.find(scenarioId);
    if (it == m_scenarios.end()) {
        return false;
    }

    m_currentScenario = &it->second;
    return true;
}

void TrainingModule::AddScenario(const Scenario& scenario) {
    m_scenarios[scenario.id] = scenario;
}

// RiskAssessment Implementation
RiskAssessment::RiskAssessment() {}

void RiskAssessment::UpdateRiskLevels(const std::vector<vr::TrackedDevicePose_t>& poses) {
    for (auto& zone : m_riskZones) {
        float maxRisk = 0.0f;
        
        for (const auto& pose : poses) {
            if (pose.bPoseIsValid) {
                float risk = CalculateRisk(pose.mDeviceToAbsoluteTracking, zone);
                maxRisk = std::max(maxRisk, risk);
            }
        }
        
        zone.risk = maxRisk;
    }
}

float RiskAssessment::CalculateRisk(const vr::HmdMatrix34_t& pose, const RiskZone& zone) {
    float dx = pose.m[0][3] - zone.position[0];
    float dy = pose.m[1][3] - zone.position[1];
    float dz = pose.m[2][3] - zone.position[2];
    
    float distance = std::sqrt(dx*dx + dy*dy + dz*dz);
    return std::max(0.0f, std::min(1.0f, 1.0f - distance));
}

} 
// namespace SAFER

// Example usage:
/*
int main() {
    SAFER::SAFERSystem saferSystem;
    
    if (!saferSystem.Initialize()) {
        std::cerr << "Failed to initialize SAFER system" << std::endl;
        return -1;
    }

    // Configure safety zones
    SAFER::SafetySystem::SafetyZone zone{
        0.0f, 0.0f, 0.0f,  // position
        2.0f,              // radius
        0.0f,              // initial warning level
        "main_zone"        // id
    };
    saferSystem.GetSafetySystem()->AddSafetyZone(zone);

    // Add training scenarios
    SAFER::TrainingModule::Scenario scenario{
        "emergency_response",
        "Emergency Response Training",
        SAFER::TrainingModule::Difficulty::Advanced,
        {"motion_tracking", "hand_tracking"}
    };
    saferSystem.GetTrainingModule()->AddScenario(scenario);

    // Main loop
    while (true) {
        saferSystem.Update();
        // Your application logic here
    }

    return 0;
}
*/