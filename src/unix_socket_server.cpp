#include "unix_socket_server.hpp"
#include <iostream>
#include <cstring>
#include <sys/stat.h>
#include <fcntl.h>

UnixSocketServer::UnixSocketServer(const std::string& socketPath)
    : m_socketPath(socketPath), m_socket(-1), m_clientSocket(-1), m_isConnected(false) {
}

UnixSocketServer::~UnixSocketServer() {
    if (m_clientSocket != -1) {
        close(m_clientSocket);
    }
    if (m_socket != -1) {
        close(m_socket);
    }
    unlink(m_socketPath.c_str());
}

bool UnixSocketServer::initialize() {
    // Create socket
    m_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (m_socket == -1) {
        std::cerr << "Failed to create socket. Error: " << strerror(errno) << std::endl;
        return false;
    }

    // Remove existing socket file if it exists
    unlink(m_socketPath.c_str());

    // Prepare socket address
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, m_socketPath.c_str(), sizeof(addr.sun_path) - 1);

    // Bind socket
    if (bind(m_socket, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        std::cerr << "Failed to bind socket. Error: " << strerror(errno) << std::endl;
        close(m_socket);
        m_socket = -1;
        return false;
    }

    // Listen for connections
    if (listen(m_socket, 1) == -1) {
        std::cerr << "Failed to listen on socket. Error: " << strerror(errno) << std::endl;
        close(m_socket);
        m_socket = -1;
        return false;
    }

    std::cout << "Waiting for client connection on " << m_socketPath << "..." << std::endl;

    // Accept client connection
    m_clientSocket = accept(m_socket, nullptr, nullptr);
    if (m_clientSocket == -1) {
        std::cerr << "Failed to accept client connection. Error: " << strerror(errno) << std::endl;
        close(m_socket);
        m_socket = -1;
        return false;
    }

    m_isConnected = true;
    std::cout << "Client connected successfully!" << std::endl;
    return true;
}

bool UnixSocketServer::writeData(const void* data, size_t size) {
    if (!m_isConnected || m_clientSocket == -1) {
        return false;
    }

    const char* buffer = static_cast<const char*>(data);
    size_t totalWritten = 0;

    while (totalWritten < size) {
        ssize_t written = send(m_clientSocket, buffer + totalWritten, size - totalWritten, 0);
        if (written <= 0) {
            if (errno == EINTR) continue;  // Interrupted by signal, retry
            std::cerr << "Failed to write to socket. Error: " << strerror(errno) << std::endl;
            return false;
        }
        totalWritten += written;
    }

    return true;
}

bool UnixSocketServer::sendTrackerData(const std::vector<TrackerManager::TrackerPose>& poses,
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

    return true;
}