#pragma once
#include <string>
#include <vector>
#include "tracker_manager.hpp"

class IPCServer {
public:
    virtual ~IPCServer() = default;

    // Initialize the IPC server
    virtual bool initialize() = 0;

    // Send tracker data through IPC
    virtual bool sendTrackerData(const std::vector<TrackerManager::TrackerPose>& poses, 
                               const std::vector<std::string>& serials) = 0;

protected:
    // Helper to write data to IPC channel
    virtual bool writeData(const void* data, size_t size) = 0;
};