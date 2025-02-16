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

    // Convert rotation matrix to quaternion using a numerically stable method
    float r11 = mat[0][0], r12 = mat[0][1], r13 = mat[0][2];
    float r21 = mat[1][0], r22 = mat[1][1], r23 = mat[1][2];
    float r31 = mat[2][0], r32 = mat[2][1], r33 = mat[2][2];

    // Compute quaternion components squared
    float qw_sq = (1.0f + r11 + r22 + r33) / 4.0f;
    float qx_sq = (1.0f + r11 - r22 - r33) / 4.0f;
    float qy_sq = (1.0f - r11 + r22 - r33) / 4.0f;
    float qz_sq = (1.0f - r11 - r22 + r33) / 4.0f;

    // Find maximum component squared
    float max_sq = qw_sq;
    int max_idx = 0;
    if (qx_sq > max_sq) { max_sq = qx_sq; max_idx = 1; }
    if (qy_sq > max_sq) { max_sq = qy_sq; max_idx = 2; }
    if (qz_sq > max_sq) { max_sq = qz_sq; max_idx = 3; }

    // Compute the maximum component and remaining components
    float max_val = sqrt(max_sq);
    float mult = 1.0f / (4.0f * max_val);

    switch (max_idx) {
        case 0: // qw is max
            pose.qw = max_val;
            pose.qx = (r32 - r23) * mult;
            pose.qy = (r13 - r31) * mult;
            pose.qz = (r21 - r12) * mult;
            break;
        case 1: // qx is max
            pose.qx = max_val;
            pose.qw = (r32 - r23) * mult;
            pose.qy = (r12 + r21) * mult;
            pose.qz = (r13 + r31) * mult;
            break;
        case 2: // qy is max
            pose.qy = max_val;
            pose.qw = (r13 - r31) * mult;
            pose.qx = (r12 + r21) * mult;
            pose.qz = (r23 + r32) * mult;
            break;
        case 3: // qz is max
            pose.qz = max_val;
            pose.qw = (r21 - r12) * mult;
            pose.qx = (r13 + r31) * mult;
            pose.qy = (r23 + r32) * mult;
            break;
    }

    // Normalize quaternion
    float norm = sqrt(pose.qw * pose.qw + pose.qx * pose.qx + 
                     pose.qy * pose.qy + pose.qz * pose.qz);
    if (norm > 0.0001f) {
        float inv_norm = 1.0f / norm;
        pose.qw *= inv_norm;
        pose.qx *= inv_norm;
        pose.qy *= inv_norm;
        pose.qz *= inv_norm;
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