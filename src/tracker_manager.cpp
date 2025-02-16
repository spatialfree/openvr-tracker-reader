#include "tracker_manager.hpp"
#include <cstring>
#include <cmath>

TrackerManager::TrackerManager() : m_vrSystem(nullptr) {
    m_poses.resize(vr::k_unMaxTrackedDeviceCount);
}

TrackerManager::~TrackerManager() {
    if (m_vrSystem) {
        vr::VR_Shutdown();
        m_vrSystem = nullptr;
    }
}

bool TrackerManager::initialize() {
    vr::EVRInitError error = vr::VRInitError_None;
    m_vrSystem = vr::VR_Init(&error, vr::VRApplication_Background);
    
    if (error != vr::VRInitError_None) {
        m_vrSystem = nullptr;
        return false;
    }

    updateTrackerList();
    return true;
}

void TrackerManager::updatePoses() {
    if (!m_vrSystem) return;

    m_vrSystem->GetDeviceToAbsoluteTrackingPose(
        vr::TrackingUniverseStanding,
        0.0f,
        m_poses.data(),
        vr::k_unMaxTrackedDeviceCount
    );
}

size_t TrackerManager::getTrackerCount() const {
    return m_trackerIndices.size();
}

TrackerManager::TrackerPose TrackerManager::getTrackerPose(size_t index) const {
    TrackerPose pose = {0};
    
    if (index >= m_trackerIndices.size()) {
        pose.valid = false;
        return pose;
    }

    const auto& devicePose = m_poses[m_trackerIndices[index]];
    if (!devicePose.bPoseIsValid) {
        pose.valid = false;
        return pose;
    }

    const auto& mat = devicePose.mDeviceToAbsoluteTracking.m;
    
    // Position
    pose.x = mat[0][3];
    pose.y = mat[1][3];
    pose.z = mat[2][3];

    // Convert rotation matrix to quaternion
    float trace = mat[0][0] + mat[1][1] + mat[2][2];
    if (trace > 0) {
        float s = 0.5f / sqrt(trace + 1.0f);
        pose.qw = 0.25f / s;
        pose.qx = (mat[2][1] - mat[1][2]) * s;
        pose.qy = (mat[0][2] - mat[2][0]) * s;
        pose.qz = (mat[1][0] - mat[0][1]) * s;
    } else {
        if (mat[0][0] > mat[1][1] && mat[0][0] > mat[2][2]) {
            float s = 2.0f * sqrt(1.0f + mat[0][0] - mat[1][1] - mat[2][2]);
            pose.qw = (mat[2][1] - mat[1][2]) / s;
            pose.qx = 0.25f * s;
            pose.qy = (mat[0][1] + mat[1][0]) / s;
            pose.qz = (mat[0][2] + mat[2][0]) / s;
        } else if (mat[1][1] > mat[2][2]) {
            float s = 2.0f * sqrt(1.0f + mat[1][1] - mat[0][0] - mat[2][2]);
            pose.qw = (mat[0][2] - mat[2][0]) / s;
            pose.qx = (mat[0][1] + mat[1][0]) / s;
            pose.qy = 0.25f * s;
            pose.qz = (mat[1][2] + mat[2][1]) / s;
        } else {
            float s = 2.0f * sqrt(1.0f + mat[2][2] - mat[0][0] - mat[1][1]);
            pose.qw = (mat[1][0] - mat[0][1]) / s;
            pose.qx = (mat[0][2] + mat[2][0]) / s;
            pose.qy = (mat[1][2] + mat[2][1]) / s;
            pose.qz = 0.25f * s;
        }
    }

    pose.valid = true;
    return pose;
}

std::string TrackerManager::getTrackerSerial(size_t index) const {
    if (!m_vrSystem || index >= m_trackerIndices.size()) {
        return "";
    }

    char buffer[vr::k_unMaxPropertyStringSize];
    vr::ETrackedPropertyError error;
    m_vrSystem->GetStringTrackedDeviceProperty(
        m_trackerIndices[index],
        vr::Prop_SerialNumber_String,
        buffer,
        vr::k_unMaxPropertyStringSize,
        &error
    );

    if (error != vr::TrackedProp_Success) {
        return "";
    }

    return std::string(buffer);
}

bool TrackerManager::isTracker(vr::TrackedDeviceIndex_t deviceIndex) const {
    if (!m_vrSystem) return false;

    return m_vrSystem->GetTrackedDeviceClass(deviceIndex) == vr::TrackedDeviceClass_GenericTracker;
}

void TrackerManager::updateTrackerList() {
    m_trackerIndices.clear();
    
    if (!m_vrSystem) return;

    for (vr::TrackedDeviceIndex_t i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i) {
        if (isTracker(i)) {
            m_trackerIndices.push_back(i);
        }
    }
}