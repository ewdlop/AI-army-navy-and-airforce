using Valve.VR;
using UnityEngine;
using System.Collections.Generic;
using System;

namespace SAFER.OpenVR
{
    public class SAFEROpenVRManager : MonoBehaviour
    {
        private CVRSystem vrSystem;
        private TrackedDevicePose_t[] renderPoses;
        private TrackedDevicePose_t[] gamePoses;
        private string[] serialNumbers;

        [Header("OpenVR Settings")]
        [SerializeField] private EVRApplicationType applicationType = EVRApplicationType.VRApplication_Scene;
        [SerializeField] private string applicationName = "SAFER Training Environment";

        [Header("Safety Settings")]
        [SerializeField] private float warningDistance = 0.5f;
        [SerializeField] private float criticalDistance = 0.2f;

        private void Start()
        {
            InitializeOpenVR();
            SetupSafetyBoundaries();
            InitializeTrackingSystem();
        }

        private void InitializeOpenVR()
        {
            EVRInitError initError = EVRInitError.None;
            vrSystem = OpenVR.Init(ref initError, applicationType);

            if (initError != EVRInitError.None)
            {
                Debug.LogError($"Failed to initialize OpenVR: {initError}");
                return;
            }

            renderPoses = new TrackedDevicePose_t[OpenVR.k_unMaxTrackedDeviceCount];
            gamePoses = new TrackedDevicePose_t[OpenVR.k_unMaxTrackedDeviceCount];
            serialNumbers = new string[OpenVR.k_unMaxTrackedDeviceCount];

            // Get device serial numbers
            for (uint deviceIndex = 0; deviceIndex < OpenVR.k_unMaxTrackedDeviceCount; deviceIndex++)
            {
                ETrackedDeviceClass deviceClass = vrSystem.GetTrackedDeviceClass(deviceIndex);
                if (deviceClass != ETrackedDeviceClass.Invalid)
                {
                    var error = ETrackedPropertyError.TrackedProp_Success;
                    var capacity = vrSystem.GetStringTrackedDeviceProperty(deviceIndex, 
                        ETrackedDeviceProperty.Prop_SerialNumber_String, null, 0, ref error);
                    
                    if (capacity > 1)
                    {
                        var buffer = new System.Text.StringBuilder((int)capacity);
                        vrSystem.GetStringTrackedDeviceProperty(deviceIndex, 
                            ETrackedDeviceProperty.Prop_SerialNumber_String, buffer, capacity, ref error);
                        serialNumbers[deviceIndex] = buffer.ToString();
                    }
                }
            }
        }

        private void SetupSafetyBoundaries()
        {
            var chaperone = OpenVR.Chaperone;
            if (chaperone != null)
            {
                // Configure chaperone bounds
                chaperone.GetPlayAreaSize(out float sizeX, out float sizeZ);
                var playArea = new Vector2(sizeX, sizeZ);
                InitializeSafetyZones(playArea);
            }
        }

        private class SafetyZone
        {
            public Vector3 Center { get; set; }
            public float Radius { get; set; }
            public float RiskLevel { get; set; }
            public Action<float> OnRiskLevelChanged;

            public void UpdateRiskLevel(float newLevel)
            {
                RiskLevel = newLevel;
                OnRiskLevelChanged?.Invoke(newLevel);
            }
        }

        private Dictionary<string, SafetyZone> safetyZones = new Dictionary<string, SafetyZone>();

        private void InitializeSafetyZones(Vector2 playArea)
        {
            // Create safety zones based on play area
            var centerZone = new SafetyZone
            {
                Center = Vector3.zero,
                Radius = Mathf.Min(playArea.x, playArea.y) * 0.4f,
                RiskLevel = 0f
            };

            safetyZones.Add("center", centerZone);
        }

        // SAFER-specific OpenVR implementations
        public class SAFERTrainingModule
        {
            private readonly CVRSystem vrSystem;
            private Dictionary<string, TrainingScenario> scenarios;

            public SAFERTrainingModule(CVRSystem system)
            {
                vrSystem = system;
                scenarios = new Dictionary<string, TrainingScenario>();
                InitializeTrainingScenarios();
            }

            private void InitializeTrainingScenarios()
            {
                // Initialize predefined training scenarios
                scenarios.Add("emergency_response", new TrainingScenario
                {
                    Name = "Emergency Response",
                    Difficulty = TrainingDifficulty.Advanced,
                    Requirements = new List<string> { "motion_tracking", "hand_tracking" }
                });
            }

            public void LoadScenario(string scenarioId)
            {
                if (scenarios.TryGetValue(scenarioId, out var scenario))
                {
                    scenario.Initialize(vrSystem);
                }
            }
        }

        // OpenVR Input System
        public class OpenVRInputSystem
        {
            private readonly CVRSystem vrSystem;
            private VRActiveActionSet_t[] actionSets;
            private ulong actionSetHandle;

            public OpenVRInputSystem(CVRSystem system)
            {
                vrSystem = system;
                InitializeActionSets();
            }

            private void InitializeActionSets()
            {
                var initInfo = new VRActiveActionSet_t
                {
                    ulActionSet = actionSetHandle,
                    ulRestrictedToDevice = OpenVR.k_ulInvalidActionSetHandle,
                    nPriority = 0
                };

                actionSets = new VRActiveActionSet_t[] { initInfo };
            }

            public void UpdateActionStates()
            {
                OpenVR.Input.UpdateActionState(actionSets, (uint)System.Runtime.InteropServices.Marshal.SizeOf(typeof(VRActiveActionSet_t)));
            }
        }

        // Risk Assessment System
        public class RiskAssessmentSystem
        {
            private readonly Dictionary<string, float> riskLevels = new Dictionary<string, float>();
            private readonly Action<string, float> onRiskLevelChanged;

            public RiskAssessmentSystem(Action<string, float> riskCallback)
            {
                onRiskLevelChanged = riskCallback;
            }

            public void UpdateRiskLevel(string zoneId, float riskLevel)
            {
                riskLevels[zoneId] = riskLevel;
                onRiskLevelChanged?.Invoke(zoneId, riskLevel);
            }
        }

        // Equipment Monitoring System
        public class EquipmentMonitoringSystem
        {
            private readonly Dictionary<string, EquipmentStatus> equipmentStatuses = new Dictionary<string, EquipmentStatus>();

            public void UpdateEquipmentStatus(string equipmentId, EquipmentStatus status)
            {
                equipmentStatuses[equipmentId] = status;
                OnEquipmentStatusChanged(equipmentId, status);
            }

            private void OnEquipmentStatusChanged(string equipmentId, EquipmentStatus status)
            {
                // Handle equipment status changes
            }
        }

        // Supporting Classes
        public class TrainingScenario
        {
            public string Name { get; set; }
            public TrainingDifficulty Difficulty { get; set; }
            public List<string> Requirements { get; set; }

            public void Initialize(CVRSystem vrSystem)
            {
                // Scenario initialization logic
            }
        }

        public enum TrainingDifficulty
        {
            Basic,
            Intermediate,
            Advanced,
            Expert
        }

        public class EquipmentStatus
        {
            public bool IsOperational { get; set; }
            public float Condition { get; set; }
            public List<string> Warnings { get; set; }
        }

        private void OnApplicationQuit()
        {
            OpenVR.Shutdown();
        }

        private void Update()
        {
            if (vrSystem != null)
            {
                vrSystem.GetDeviceToAbsoluteTrackingPose(ETrackingUniverseOrigin.TrackingUniverseStanding, 
                    Time.deltaTime, renderPoses);
                UpdateSafetySystem();
            }
        }

        private void UpdateSafetySystem()
        {
            // Update safety zones based on current poses
            foreach (var pose in renderPoses)
            {
                if (pose.bPoseIsValid)
                {
                    CheckSafetyBoundaries(pose);
                }
            }
        }

        private void CheckSafetyBoundaries(TrackedDevicePose_t pose)
        {
            var matrix = new SteamVR_Utils.RigidTransform(pose.mDeviceToAbsoluteTracking);
            var position = matrix.pos;

            foreach (var zone in safetyZones)
            {
                float distance = Vector3.Distance(position, zone.Value.Center);
                float riskLevel = CalculateRiskLevel(distance, zone.Value.Radius);
                zone.Value.UpdateRiskLevel(riskLevel);
            }
        }

        private float CalculateRiskLevel(float distance, float zoneRadius)
        {
            // Calculate risk level based on distance from zone center
            return Mathf.Clamp01(1 - (distance / zoneRadius));
        }
    }
}