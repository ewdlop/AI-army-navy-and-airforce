// SAFER.hpp
#pragma once
#include <openvr.h>
#include <vector>
#include <memory>
#include <map>
#include <string>
#include <functional>

namespace SAFER {

class SafetySystem;
class TrainingModule;
class RiskAssessment;

class SAFERSystem {
public:
    SAFERSystem();
    ~SAFERSystem();

    bool Initialize();
    void Shutdown();
    void Update();

    // Core systems
    std::shared_ptr<SafetySystem> GetSafetySystem() { return m_safetySystem; }
    std::shared_ptr<TrainingModule> GetTrainingModule() { return m_trainingModule; }
    std::shared_ptr<RiskAssessment> GetRiskAssessment() { return m_riskAssessment; }

private:
    vr::IVRSystem* m_vrSystem;
    std::shared_ptr<SafetySystem> m_safetySystem;
    std::shared_ptr<TrainingModule> m_trainingModule;
    std::shared_ptr<RiskAssessment> m_riskAssessment;

    std::vector<vr::TrackedDevicePose_t> m_trackedDevicePoses;
    bool InitializeOpenVR();
};

class SafetySystem {
public:
    struct SafetyZone {
        float x, y, z;        // Position
        float radius;         // Zone radius
        float warningLevel;   // Current warning level
        std::string id;       // Zone identifier
    };

    SafetySystem(vr::IVRSystem* vrSystem);
    void Update(const std::vector<vr::TrackedDevicePose_t>& poses);
    void AddSafetyZone(const SafetyZone& zone);
    void SetWarningCallback(std::function<void(const std::string&, float)> callback);

private:
    vr::IVRSystem* m_vrSystem;
    std::vector<SafetyZone> m_safetyZones;
    std::function<void(const std::string&, float)> m_warningCallback;
    
    void CheckSafetyBoundaries(const vr::TrackedDevicePose_t& pose);
    float CalculateRiskLevel(float distance, float zoneRadius);
};

class TrainingModule {
public:
    enum class Difficulty {
        Basic,
        Intermediate,
        Advanced,
        Expert
    };

    struct Scenario {
        std::string id;
        std::string name;
        Difficulty difficulty;
        std::vector<std::string> requirements;
    };

    TrainingModule(vr::IVRSystem* vrSystem);
    bool LoadScenario(const std::string& scenarioId);
    void UpdateScenario();
    void AddScenario(const Scenario& scenario);

private:
    vr::IVRSystem* m_vrSystem;
    std::map<std::string, Scenario> m_scenarios;
    Scenario* m_currentScenario;
};

class RiskAssessment {
public:
    struct RiskZone {
        float position[3];
        float risk;
        std::string id;
    };

    RiskAssessment();
    void UpdateRiskLevels(const std::vector<vr::TrackedDevicePose_t>& poses);
    void AddRiskZone(const RiskZone& zone);
    float GetRiskLevel(const std::string& zoneId) const;

private:
    std::vector<RiskZone> m_riskZones;
    float CalculateRisk(const vr::HmdMatrix34_t& pose, const RiskZone& zone);
};
