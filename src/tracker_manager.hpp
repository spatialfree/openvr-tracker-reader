#pragma once
#include <openvr.h>
#include <vector>
#include <memory>
#include <string>

class TrackerManager {
public:
    struct TrackerPose {
        float x, y, z;           // Position in meters
        float qw, qx, qy, qz;    // Rotation quaternion
        bool valid;              // Whether the pose is valid
    };

    TrackerManager();
    ~TrackerManager();

    // Initialize OpenVR system
    bool initialize();

    // Update poses for all tracked devices
    void updatePoses();

    // Get number of active trackers (excluding HMD and controllers)
    size_t getTrackerCount() const;

    // Get pose data for a specific tracker
    TrackerPose getTrackerPose(size_t index) const;

    // Get serial number for a specific tracker
    std::string getTrackerSerial(size_t index) const;

    // Update list of connected trackers
    void updateTrackerList();

private:
    vr::IVRSystem* m_vrSystem;
    std::vector<vr::TrackedDeviceIndex_t> m_trackerIndices;
    std::vector<vr::TrackedDevicePose_t> m_poses;
    
    // Helper to check if device is a tracker
    bool isTracker(vr::TrackedDeviceIndex_t deviceIndex) const;
};