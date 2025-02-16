#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include "tracker_manager.hpp"

class PipeServer {
public:
    PipeServer(const std::string& pipeName = "\\\\.\\pipe\\vr_tracker_data");
    ~PipeServer();

    // Initialize the pipe server
    bool initialize();

    // Send tracker data through the pipe
    bool sendTrackerData(const std::vector<TrackerManager::TrackerPose>& poses, 
                        const std::vector<std::string>& serials);

private:
    HANDLE m_pipe;
    std::string m_pipeName;
    bool m_isConnected;

    // Binary packet format:
    // [uint32_t: number of trackers]
    // For each tracker:
    //   [float x, y, z]
    //   [float qw, qx, qy, qz]
    //   [uint8_t valid]
    //   [uint32_t serial_length]
    //   [char[] serial]
    
    // Helper to write data to pipe
    bool writeData(const void* data, size_t size);
};