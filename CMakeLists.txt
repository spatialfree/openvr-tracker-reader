cmake_minimum_required(VERSION 3.12)
project(openxr_tracker_extenuation)

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

# Platform-specific sources
if(WIN32)
    list(APPEND SOURCES src/win_pipe_server.cpp)
    add_definitions(-DUSE_WINDOWS_PIPE)
else()
    list(APPEND SOURCES src/unix_socket_server.cpp)
    add_definitions(-DUSE_UNIX_SOCKET)
endif()

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include OpenVR headers
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENVR_INCLUDE_DIRS})

# Link OpenVR and platform-specific libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENVR_LIBRARIES})

# Platform-specific configuration
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /W4)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
    endif()
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif()