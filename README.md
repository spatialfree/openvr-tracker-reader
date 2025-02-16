# OpenVR Tracker Reader

A cross-platform bridge that makes OpenVR tracker data available to OpenXR applications through a simple IPC interface.

## Why?
OpenXR is becoming the standard for VR development but doesn't support additional trackers (beyond HMD and controllers). This tool bridges that gap by:
1. Reading tracker data through OpenVR
2. Making it available through IPC (Named Pipes on Windows, Domain Sockets on Linux)

## Features
- Real-time position and rotation data from VR trackers
- Cross-platform: Windows and Linux support
- Low latency (~0.1-0.5ms)
- High performance (matches VR system capabilities, up to 1000Hz)
- Simple C# client included

## Quick Start

### 1. Build and Run Server
```bash
mkdir build && cd build
cmake ..
cmake --build .  # or 'make' on Linux
./vr_tracker_reader
```

### 2. Use in Your C# Application
```csharp
// At startup:
await TrackerReader.Initialize();

// In your frame loop:
if (TrackerReader.TryGetLatestPoses(out var poses))
{
    foreach (var pose in poses)
    {
        if (pose.Valid)
        {
            // Use tracker data:
            // Position: pose.X, pose.Y, pose.Z (meters)
            // Rotation: pose.Qw, pose.Qx, pose.Qy, pose.Qz
            // ID: pose.Serial
        }
    }
}

// At shutdown:
TrackerReader.Shutdown();
```

## Requirements
- CMake 3.12+
- C++17 compiler
- OpenVR library
- Windows or Linux

## Performance
- Latency: ~0.1-0.5ms
- Update Rate: Matches system capabilities (typically 90-144Hz)
- Memory: < 10MB

## Architecture
```
OpenVR -> C++ Server <-> IPC Channel <-> C# Client -> Your Application
```

## Contributing
Contributions welcome! Areas of interest:
- Additional client implementations
- Performance optimizations
- Enhanced error handling

## Notes
- Set `OpenVR_INCLUDE_DIR` environment variable if OpenVR headers aren't found
- See `csharp_client/README.md` for detailed C# usage
