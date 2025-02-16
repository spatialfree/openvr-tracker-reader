#include "tracker_manager.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>

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
    
    // Main loop
    while (true) {
        manager.updatePoses();
        size_t trackerCount = manager.getTrackerCount();
        
        std::cout << "\033[2J\033[H";  // Clear screen and move cursor to top
        std::cout << "Found " << trackerCount << " trackers\n\n";
        
        for (size_t i = 0; i < trackerCount; ++i) {
            std::string serial = manager.getTrackerSerial(i);
            auto pose = manager.getTrackerPose(i);
            
            std::cout << "Tracker " << i + 1 << " (Serial: " << serial << ")\n";
            if (pose.valid) {
                printPose(pose);
            } else {
                std::cout << "Invalid pose data\n";
            }
            std::cout << "------------------------\n";
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(16));  // ~60 Hz update rate
    }
    
    return 0;
}