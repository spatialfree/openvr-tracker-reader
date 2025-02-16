# OpenVR Tracker Reader

A C++ application that reads and represents data from VR trackers (besides head/hands) using OpenVR.

## Features
- Detects and tracks VR trackers (excluding HMD/controllers)
- Provides real-time position (x,y,z) and rotation (quaternion) data
- Identifies trackers by serial numbers
- Updates at 60Hz

## Requirements
- CMake 3.12 or higher
- C++17 compiler
- OpenVR library

## Building
```bash
mkdir build
cd build
cmake ..
make
```

## Environment Variables
- `OpenVR_INCLUDE_DIR`: Path to OpenVR include directory (optional)

## Usage
Run the executable:
```bash
./vr_tracker_reader
```

The program will display:
- Number of detected trackers
- Position and rotation data for each tracker
- Serial numbers for identification
## Building with System OpenVR
If OpenVR is installed on your system, the CMake configuration will automatically find and use it.
