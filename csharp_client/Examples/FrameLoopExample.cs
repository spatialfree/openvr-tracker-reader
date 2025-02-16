using System;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Collections.Concurrent;

/// <summary>
/// A frame-loop friendly wrapper for TrackerData that handles the async reading
/// in a background task while providing non-blocking access to the latest data.
/// </summary>
public class FrameLoopExample
{
    private TrackerData trackerReader;
    private ConcurrentQueue<TrackerData.Pose[]> poseQueue = new ConcurrentQueue<TrackerData.Pose[]>();
    private CancellationTokenSource cancellationSource;
    private Task readerTask;
    private bool isInitialized;

    /// <summary>
    /// Initializes the tracker reader and starts the background reading task.
    /// </summary>
    public async Task Initialize()
    {
        if (isInitialized) return;

        trackerReader = new TrackerData();
        await trackerReader.ConnectAsync();
        
        cancellationSource = new CancellationTokenSource();
        readerTask = RunReaderLoop(cancellationSource.Token);
        
        isInitialized = true;
    }

    private async Task RunReaderLoop(CancellationToken token)
    {
        while (!token.IsCancellationRequested)
        {
            try
            {
                var poses = await trackerReader.ReadTrackersAsync();
                poseQueue.Enqueue(poses.ToArray());
                
                // Only keep the latest frame
                while (poseQueue.Count > 1)
                {
                    poseQueue.TryDequeue(out _);
                }
            }
            catch (Exception e)
            {
                Console.WriteLine($"Error in reader loop: {e.Message}");
                await Task.Delay(1000, token); // Wait before retrying
            }
        }
    }

    /// <summary>
    /// Non-blocking method to get the latest tracker poses.
    /// Call this from your frame update/step/tick method.
    /// </summary>
    /// <param name="poses">Array of latest tracker poses if available.</param>
    /// <returns>True if new poses were available, false otherwise.</returns>
    public bool TryGetLatestPoses(out TrackerData.Pose[] poses)
    {
        return poseQueue.TryDequeue(out poses);
    }

    /// <summary>
    /// Cleanly shuts down the tracker reader and background task.
    /// </summary>
    public void Shutdown()
    {
        if (!isInitialized) return;

        cancellationSource?.Cancel();
        readerTask?.Wait();
        trackerReader?.Disconnect();
        
        isInitialized = false;
    }
}

// Example usage in a frame-based application:
/*
class ExampleApp
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
                if (!pose.Valid) continue;
                
                // Use the tracker data:
                // pose.X, pose.Y, pose.Z - Position in meters
                // pose.Qw, pose.Qx, pose.Qy, pose.Qz - Rotation as quaternion
                // pose.Serial - Tracker identifier
            }
        }
    }

    public void Cleanup()
    {
        trackerLoop?.Shutdown();
    }
}
*/