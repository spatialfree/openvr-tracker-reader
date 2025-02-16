#include "tracker_manager.hpp"
#ifdef USE_WINDOWS_PIPE
#include "win_pipe_server.hpp"
#else
#include "unix_socket_server.hpp"
#endif
#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <vector>
#include <memory>

void printPose(const TrackerManager::TrackerPose& pose) {
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Position: (" << pose.x << ", " << pose.y << ", " << pose.z << ") m\n";
    std::cout << "Rotation: [w:" << pose.qw << ", x:" << pose.qx
              << ", y:" << pose.qy << ", z:" << pose.qz << "]\n";
}

int main() {
    TrackerManager manager;

    if (!manager.initialize()) {
        std::cerr << "Failed to initialize OpenVR\n";
        return 1;
    }

    std::cout << "OpenVR initialized successfully\n";

    // Initialize IPC server with platform-specific path
#ifdef USE_WINDOWS_PIPE
    const char* DEFAULT_IPC_PATH = "\\\\.\\pipe\\openxr_tracker_extenuation";  // Windows named pipe
#else
    const char* DEFAULT_IPC_PATH = "/tmp/openxr_tracker_extenuation";          // Unix domain socket
#endif

    std::unique_ptr<IPCServer> ipcServer;
#ifdef USE_WINDOWS_PIPE
    ipcServer = std::make_unique<WinPipeServer>(DEFAULT_IPC_PATH);
#else
    ipcServer = std::make_unique<UnixSocketServer>(DEFAULT_IPC_PATH);
#endif

    if (!ipcServer->initialize()) {
        std::cerr << "Failed to initialize IPC server\n";
        return 1;
    }

    std::vector<TrackerManager::TrackerPose> poses;
    std::vector<std::string> serials;

    // Main loop
    while (true) {
        // Update tracker list periodically to handle hot-plugging
        static auto lastUpdateTime = std::chrono::steady_clock::now();
        auto currentTime = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastUpdateTime).count() >= 1) {
            manager.updateTrackerList();
            lastUpdateTime = currentTime;
        }

        manager.updatePoses();
        size_t trackerCount = manager.getTrackerCount();

        // Clear previous data
        poses.clear();
        serials.clear();

        std::cout << "\033[2J\033[H";  // Clear screen and move cursor to top
        std::cout << "Found " << trackerCount << " trackers\n\n";

        // Collect all tracker data
        for (size_t i = 0; i < trackerCount; ++i) {
            std::string serial = manager.getTrackerSerial(i);
            auto pose = manager.getTrackerPose(i);

            poses.push_back(pose);
            serials.push_back(serial);

            std::cout << "Tracker " << i + 1 << " (Serial: " << serial << ")\n";
            if (pose.valid) {
                printPose(pose);
            } else {
                std::cout << "Invalid pose data\n";
            }
            std::cout << "------------------------\n";
        }

        // Send data through IPC with retry logic
        if (trackerCount > 0) {
            static int failureCount = 0;
            static bool wasConnected = true;
            const int maxRetries = 3;
            bool sendSuccess = false;

            for (int retry = 0; retry < maxRetries && !sendSuccess; retry++) {
                if (ipcServer->sendTrackerData(poses, serials)) {
                    sendSuccess = true;
                    if (!wasConnected) {
                        std::cout << "IPC connection restored\n";
                        wasConnected = true;
                    }
                    failureCount = 0;
                } else {
                    failureCount++;
                    if (retry < maxRetries - 1) {
                        std::cerr << "Failed to send tracker data, retrying (" << retry + 1 << "/" << maxRetries << ")...\n";
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                }
            }

            if (!sendSuccess) {
                if (wasConnected) {
                    std::cerr << "IPC connection lost\n";
                    wasConnected = false;
                }
                // Try to reinitialize IPC server after consecutive failures
                if (failureCount > 10) {
                    std::cout << "Attempting to reinitialize IPC server...\n";
                    ipcServer->initialize();
                    failureCount = 0;
                }
            }
        }

        // Use OpenVR's frame timing for optimal update rate
        vr::Compositor_FrameTiming timing = {0};
        timing.m_nSize = sizeof(vr::Compositor_FrameTiming);
        if (vr::VRCompositor() && vr::VRCompositor()->GetFrameTiming(&timing)) {
            float frameTime = timing.m_flSystemTimeInSeconds;
            static float lastFrameTime = frameTime;
            float deltaTime = frameTime - lastFrameTime;
            lastFrameTime = frameTime;

            // Target 1000Hz max update rate (1ms per frame)
            const float targetFrameTime = 0.001f;
            if (deltaTime < targetFrameTime) {
                float sleepTime = (targetFrameTime - deltaTime) * 1000000.0f; // Convert to microseconds
                std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(sleepTime)));
            }

            // Calculate and log running average frame rate using exponential moving average
            static float avgFrameRate = 0.0f;
            const float alpha = 0.1f; // Smoothing factor (0.1 = 10% weight to new samples)
            float instantFrameRate = 1.0f / deltaTime;
            avgFrameRate = (alpha * instantFrameRate) + ((1.0f - alpha) * avgFrameRate);

            // Log frame rate every second
            static auto lastLogTime = std::chrono::steady_clock::now();
            auto currentLogTime = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(currentLogTime - lastLogTime).count() >= 1) {
                std::cout << "Average frame rate: " << std::fixed << std::setprecision(1)
                         << avgFrameRate << " Hz (instant: " << instantFrameRate << " Hz)\n";
                lastLogTime = currentLogTime;
            }
        } else {
            // Fallback if compositor timing isn't available
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    return 0;
}