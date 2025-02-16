cmake_minimum_required(VERSION 3.12)
project(vr_tracker_reader)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add cmake modules path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Find OpenVR package
find_package(OpenVR REQUIRED)

# Add source files
set(SOURCES
    src/main.cpp
    src/tracker_manager.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include OpenVR headers
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENVR_INCLUDE_DIRS})

# Link OpenVR
target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENVR_LIBRARIES})