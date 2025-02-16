using System;
using System.IO.Pipes;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Collections.Concurrent;

public static class TrackerReader
{
    public struct Pose
    {
        public float X, Y, Z;           // Position in meters
        public float Qw, Qx, Qy, Qz;    // Rotation quaternion
        public bool Valid;
        public string Serial;
    }

    private static NamedPipeClientStream pipeClient;
    private static ConcurrentQueue<Pose[]> poseQueue = new ConcurrentQueue<Pose[]>();
    private static CancellationTokenSource cancellationSource;
    private static Task readerTask;
    private static bool isInitialized;
    private static readonly byte[] buffer = new byte[8192]; // Match server buffer size

    /// <summary>
    /// Initializes the tracker reader and starts the background reading task.
    /// Call this once at startup.
    /// </summary>
    public static async Task Initialize(string pipeName = "vr_tracker_data")
    {
        if (isInitialized) return;

        try
        {
            pipeClient = new NamedPipeClientStream(".", pipeName, PipeDirection.In);
            Console.WriteLine($"Connecting to pipe: {pipeName}");
            await pipeClient.ConnectAsync();
            
            cancellationSource = new CancellationTokenSource();
            readerTask = RunReaderLoop(cancellationSource.Token);
            
            isInitialized = true;
            Console.WriteLine("Connected to pipe server!");
        }
        catch (Exception e)
        {
            Console.WriteLine($"Failed to initialize: {e.Message}");
            throw;
        }
    }

    private static async Task RunReaderLoop(CancellationToken token)
    {
        while (!token.IsCancellationRequested)
        {
            try
            {
                var poses = await ReadTrackersAsync();
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

    private static async Task<List<Pose>> ReadTrackersAsync()
    {
        var poses = new List<Pose>();

        try
        {
            // Read number of trackers
            await ReadExactAsync(buffer, 0, sizeof(uint));
            uint numTrackers = BitConverter.ToUInt32(buffer, 0);

            for (int i = 0; i < numTrackers; i++)
            {
                var pose = new Pose();

                // Read position (3 floats)
                await ReadExactAsync(buffer, 0, sizeof(float) * 3);
                pose.X = BitConverter.ToSingle(buffer, 0);
                pose.Y = BitConverter.ToSingle(buffer, 4);
                pose.Z = BitConverter.ToSingle(buffer, 8);

                // Read rotation (4 floats)
                await ReadExactAsync(buffer, 0, sizeof(float) * 4);
                pose.Qw = BitConverter.ToSingle(buffer, 0);
                pose.Qx = BitConverter.ToSingle(buffer, 4);
                pose.Qy = BitConverter.ToSingle(buffer, 8);
                pose.Qz = BitConverter.ToSingle(buffer, 12);

                // Read validity
                await ReadExactAsync(buffer, 0, sizeof(byte));
                pose.Valid = buffer[0] != 0;

                // Read serial number
                await ReadExactAsync(buffer, 0, sizeof(uint));
                uint serialLength = BitConverter.ToUInt32(buffer, 0);
                await ReadExactAsync(buffer, 0, (int)serialLength);
                pose.Serial = System.Text.Encoding.ASCII.GetString(buffer, 0, (int)serialLength);

                poses.Add(pose);
            }
        }
        catch (Exception)
        {
            throw;
        }

        return poses;
    }

    private static async Task ReadExactAsync(byte[] buffer, int offset, int count)
    {
        int bytesRead = 0;
        while (bytesRead < count)
        {
            bytesRead += await pipeClient.ReadAsync(buffer, offset + bytesRead, count - bytesRead);
        }
    }

    /// <summary>
    /// Non-blocking method to get the latest tracker poses.
    /// Call this from your frame update/step/tick method.
    /// </summary>
    /// <param name="poses">Array of latest tracker poses if available.</param>
    /// <returns>True if new poses were available, false otherwise.</returns>
    public static bool TryGetLatestPoses(out Pose[] poses)
    {
        return poseQueue.TryDequeue(out poses);
    }

    /// <summary>
    /// Cleanly shuts down the tracker reader.
    /// Call this when your application exits.
    /// </summary>
    public static void Shutdown()
    {
        if (!isInitialized) return;

        cancellationSource?.Cancel();
        readerTask?.Wait();
        pipeClient?.Close();
        
        isInitialized = false;
    }
}

// Example usage in a frame-based application:
/*
// At startup:
await TrackerReader.Initialize();

// In your frame loop:
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

// At shutdown:
TrackerReader.Shutdown();
*/