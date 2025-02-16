#pragma once
#include "ipc_server.hpp"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

class UnixSocketServer : public IPCServer {
public:
    UnixSocketServer(const std::string& socketPath = "/tmp/openxr_tracker_extenuation");
    ~UnixSocketServer();

    bool initialize() override;
    bool sendTrackerData(const std::vector<TrackerManager::TrackerPose>& poses,
                        const std::vector<std::string>& serials) override;

private:
    bool writeData(const void* data, size_t size) override;

    int m_socket;
    int m_clientSocket;
    std::string m_socketPath;
    bool m_isConnected;
};