# C# Client for OpenVR Tracker Reader

This client provides a C# interface to read tracker data from the OpenVR Tracker Reader service.

## Basic Usage

```csharp
// Simple async reading
var reader = new TrackerData();
await reader.ConnectAsync();
var poses = await reader.ReadTrackersAsync();

foreach (var pose in poses)
{
    if (pose.Valid)
    {
        Console.WriteLine($"Tracker {pose.Serial}:");
        Console.WriteLine($"Position: ({pose.X}, {pose.Y}, {pose.Z})");
        Console.WriteLine($"Rotation: [w:{pose.Qw}, x:{pose.Qx}, y:{pose.Qy}, z:{pose.Qz}]");
    }
}
```

## Frame-Loop Integration

For applications with a frame loop (games, visualizations, etc.), use the `FrameLoopExample` class:

```csharp
class App
{
    private FrameLoopExample trackerLoop;

    public async Task Startup()
    {
        trackerLoop = new FrameLoopExample();
        await trackerLoop.Initialize();
    }

    public void Update() // Called every frame
    {
        if (trackerLoop.TryGetLatestPoses(out var poses))
        {
            foreach (var pose in poses)
            {
                if (pose.Valid)
                {
                    // Use the tracker data
                    // Position: pose.X, pose.Y, pose.Z (in meters)
                    // Rotation: pose.Qw, pose.Qx, pose.Qy, pose.Qz (quaternion)
                    // ID: pose.Serial
                }
            }
        }
    }

    public void Cleanup()
    {
        trackerLoop?.Shutdown();
    }
}
```

## Data Format

- Position: X, Y, Z in meters
- Rotation: Quaternion (Qw, Qx, Qy, Qz)
- Serial: Unique identifier for each tracker
- Valid: Indicates if the pose data is valid

## Performance Notes

- The frame-loop implementation uses a background task for reading
- Only the latest data is kept to prevent queue buildup
- All frame-loop operations are non-blocking
- Auto-reconnect on errors with 1-second delay