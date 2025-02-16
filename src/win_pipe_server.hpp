#pragma once
#include "ipc_server.hpp"
#include <windows.h>

class WinPipeServer : public IPCServer {
public:
    WinPipeServer(const std::string& pipeName = "\\\\.\\pipe\\openxr_tracker_extenuation");
    ~WinPipeServer();

    bool initialize() override;
    bool sendTrackerData(const std::vector<TrackerManager::TrackerPose>& poses,
                        const std::vector<std::string>& serials) override;

private:
    bool writeData(const void* data, size_t size) override;

    HANDLE m_pipe;
    std::string m_pipeName;
    bool m_isConnected;
};