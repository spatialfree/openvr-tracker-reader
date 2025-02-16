# OpenVR Tracker Reader

A cross-platform C++ application that reads VR tracker data from OpenVR and makes it available to other applications (particularly useful for OpenXR applications that can't directly access trackers).

## Use Case
This project bridges a specific gap: While OpenXR is becoming the standard for VR development, it currently doesn't support additional trackers (beyond HMD and controllers). This tool allows OpenXR applications (like those built with StereoKit) to access tracker data by:
1. Reading tracker data through OpenVR
2. Making this data available through IPC (Inter-Process Communication)

## Features
- Detects and tracks VR trackers (excluding HMD/controllers)
- Real-time position (x,y,z) and rotation (quaternion) data
- Tracker identification via serial numbers
- Dynamic update rate (up to 1000Hz, automatically matches system capabilities)
- Cross-platform IPC support:
  - Windows: Named Pipes
  - Linux/Unix: Domain Sockets
- Low latency (~0.1-0.5ms)
- C# client included for easy integration with StereoKit/Unity

## Requirements
- CMake 3.12 or higher
- C++17 compiler
- OpenVR library
- Windows or Linux/Unix operating system

## Building

### Windows
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Linux
```bash
mkdir build
cd build
cmake ..
make
```

## Environment Variables
- `OpenVR_INCLUDE_DIR`: Path to OpenVR include directory (optional)

## Usage

### Running the Tracker Server
```bash
./vr_tracker_reader
```

### Integrating with C# Applications
1. Add the provided `TrackerReader.cs` to your project
2. Initialize the connection:
```csharp
var trackerReader = new TrackerData();
await trackerReader.ConnectAsync();
```
3. Read tracker data:
```csharp
var trackers = await trackerReader.ReadTrackersAsync();
foreach (var tracker in trackers)
{
    if (tracker.Valid)
    {
        // Use tracker.X, Y, Z for position
        // Use tracker.Qw, Qx, Qy, Qz for rotation
        // tracker.Serial for identification
    }
}
```

## Performance
- IPC Latency: ~0.1-0.5ms
- Update Rate: Dynamic (up to 1000Hz, typically matches HMD refresh rate)
  - Base station tracking: 100Hz
  - Raw sensor data: up to 1000Hz
  - Pose updates: matches system capabilities (90Hz-144Hz typical)
- Memory Usage: Minimal (< 10MB)

## Architecture
```
OpenVR -> C++ Server <-> IPC Channel <-> C# Client -> Your Application
```

## Contributing
Contributions welcome! Particularly interested in:
- Additional client implementations (Unity, Unreal, etc.)
- Performance optimizations
- Additional IPC methods
## Building with System OpenVR
If OpenVR is installed on your system, the CMake configuration will automatically find and use it.
