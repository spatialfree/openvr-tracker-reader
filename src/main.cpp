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

    // Initialize IPC server
#ifdef USE_WINDOWS_PIPE
    std::unique_ptr<IPCServer> ipcServer = std::make_unique<WinPipeServer>("\\\\.\\pipe\\vr_tracker_data");
#else
    std::unique_ptr<IPCServer> ipcServer = std::make_unique<UnixSocketServer>("/tmp/vr_tracker_data");
#endif

    if (!ipcServer->initialize()) {
        std::cerr << "Failed to initialize IPC server\n";
        return 1;
    }
    
    std::vector<TrackerManager::TrackerPose> poses;
    std::vector<std::string> serials;

    // Main loop
    while (true) {
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

        // Send data through IPC
        if (trackerCount > 0) {
            if (!ipcServer->sendTrackerData(poses, serials)) {
                std::cerr << "Failed to send tracker data through IPC\n";
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
            
            if (deltaTime < 0.001f) { // Avoid spinning too fast
                std::this_thread::sleep_for(std::chrono::microseconds(500));
            }
            
            // Log frame rate every second
            static float timeAccumulator = 0;
            static int frameCount = 0;
            timeAccumulator += deltaTime;
            frameCount++;
            if (timeAccumulator >= 1.0f) {
                std::cout << "Average frame rate: " << frameCount / timeAccumulator << " Hz\n";
                timeAccumulator = 0;
                frameCount = 0;
            }
        } else {
            // Fallback if compositor timing isn't available
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    
    return 0;
}