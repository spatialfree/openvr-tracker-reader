#include "win_pipe_server.hpp"
#include <iostream>

WinPipeServer::WinPipeServer(const std::string& pipeName) 
    : m_pipeName(pipeName), m_pipe(INVALID_HANDLE_VALUE), m_isConnected(false) {
}

WinPipeServer::~WinPipeServer() {
    if (m_pipe != INVALID_HANDLE_VALUE) {
        CloseHandle(m_pipe);
        m_pipe = INVALID_HANDLE_VALUE;
    }
}

bool WinPipeServer::initialize() {
    m_pipe = CreateNamedPipeA(
        m_pipeName.c_str(),
        PIPE_ACCESS_OUTBOUND,          // One-way pipe (server to client)
        PIPE_TYPE_MESSAGE |            // Message-based communication
        PIPE_READMODE_MESSAGE |        // Message-based reading
        PIPE_WAIT,                     // Blocking mode
        1,                            // Only one instance
        8192,                         // Output buffer size
        1024,                         // Input buffer size
        0,                            // Default timeout
        nullptr                       // Default security attributes
    );

    if (m_pipe == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create named pipe. Error: " << GetLastError() << std::endl;
        return false;
    }

    std::cout << "Waiting for client connection on " << m_pipeName << "..." << std::endl;
    
    // Wait for a client to connect
    if (!ConnectNamedPipe(m_pipe, nullptr)) {
        if (GetLastError() != ERROR_PIPE_CONNECTED) {
            std::cerr << "Failed to connect to client. Error: " << GetLastError() << std::endl;
            CloseHandle(m_pipe);
            m_pipe = INVALID_HANDLE_VALUE;
            return false;
        }
    }

    m_isConnected = true;
    std::cout << "Client connected successfully!" << std::endl;
    return true;
}

bool WinPipeServer::writeData(const void* data, size_t size) {
    if (!m_isConnected || m_pipe == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD bytesWritten;
    if (!WriteFile(m_pipe, data, static_cast<DWORD>(size), &bytesWritten, nullptr)) {
        std::cerr << "Failed to write to pipe. Error: " << GetLastError() << std::endl;
        return false;
    }

    return bytesWritten == size;
}

bool WinPipeServer::sendTrackerData(const std::vector<TrackerManager::TrackerPose>& poses,
                                  const std::vector<std::string>& serials) {
    if (poses.size() != serials.size()) {
        return false;
    }

    // Write number of trackers
    uint32_t numTrackers = static_cast<uint32_t>(poses.size());
    if (!writeData(&numTrackers, sizeof(numTrackers))) {
        return false;
    }

    // Write data for each tracker
    for (size_t i = 0; i < poses.size(); ++i) {
        const auto& pose = poses[i];
        const auto& serial = serials[i];

        // Write position
        if (!writeData(&pose.x, sizeof(float) * 3)) {
            return false;
        }

        // Write rotation
        float rotation[4] = {pose.qw, pose.qx, pose.qy, pose.qz};
        if (!writeData(rotation, sizeof(float) * 4)) {
            return false;
        }

        // Write validity
        uint8_t valid = pose.valid ? 1 : 0;
        if (!writeData(&valid, sizeof(uint8_t))) {
            return false;
        }

        // Write serial number
        uint32_t serialLength = static_cast<uint32_t>(serial.length());
        if (!writeData(&serialLength, sizeof(uint32_t))) {
            return false;
        }
        if (!writeData(serial.c_str(), serialLength)) {
            return false;
        }
    }

    // Flush the pipe
    FlushFileBuffers(m_pipe);
    return true;
}