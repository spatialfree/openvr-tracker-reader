# C# Client for OpenVR Tracker Reader

This client provides a C# interface to read tracker data from the OpenVR Tracker Reader service, designed for frame-based applications.

## Usage

Just three steps:

```csharp
// 1. At startup:
await TrackerReader.Initialize();

// 2. In your frame loop:
if (TrackerReader.TryGetLatestPoses(out var poses))
{
    foreach (var pose in poses)
    {
        if (pose.Valid)
        {
            // Use the tracker data:
            // Position: pose.X, pose.Y, pose.Z (in meters)
            // Rotation: pose.Qw, pose.Qx, pose.Qy, pose.Qz (quaternion)
            // ID: pose.Serial
        }
    }
}

// 3. At shutdown:
TrackerReader.Shutdown();
```

## How It Works

- Uses a background task to read tracker data asynchronously
- Provides non-blocking access to the latest data in your frame loop
- Automatically handles reconnection on errors
- Only keeps the latest data to prevent queue buildup
- Automatically uses the correct IPC method for each platform:
  - Windows: Named Pipes
  - Linux: Unix Domain Sockets

## Data Format

Each tracker pose contains:
- Position (X, Y, Z) in meters
- Rotation as quaternion (Qw, Qx, Qy, Qz)
- Serial number for identification
- Valid flag indicating data reliability