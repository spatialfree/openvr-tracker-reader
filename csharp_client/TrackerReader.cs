using System;
using System.IO.Pipes;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Collections.Generic;

public class TrackerData
{
    public struct Pose
    {
        public float X, Y, Z;           // Position in meters
        public float Qw, Qx, Qy, Qz;    // Rotation quaternion
        public bool Valid;
        public string Serial;
    }

    private NamedPipeClientStream pipeClient;
    private bool isConnected = false;
    private readonly string pipeName;
    private byte[] buffer = new byte[8192]; // Match server buffer size

    public TrackerData(string pipeName = "vr_tracker_data")
    {
        this.pipeName = pipeName;
    }

    public async Task ConnectAsync()
    {
        if (isConnected) return;

        pipeClient = new NamedPipeClientStream(".", pipeName, PipeDirection.In);
        Console.WriteLine($"Connecting to pipe: {pipeName}");
        
        await pipeClient.ConnectAsync();
        isConnected = true;
        Console.WriteLine("Connected to pipe server!");
    }

    public async Task<List<Pose>> ReadTrackersAsync()
    {
        if (!isConnected) return new List<Pose>();

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
        catch (Exception e)
        {
            Console.WriteLine($"Error reading from pipe: {e.Message}");
            isConnected = false;
        }

        return poses;
    }

    private async Task ReadExactAsync(byte[] buffer, int offset, int count)
    {
        int bytesRead = 0;
        while (bytesRead < count)
        {
            bytesRead += await pipeClient.ReadAsync(buffer, offset + bytesRead, count - bytesRead);
        }
    }

    public void Disconnect()
    {
        if (isConnected)
        {
            pipeClient.Close();
            isConnected = false;
        }
    }
}

// Example usage in StereoKit:
/*
private TrackerData trackerReader;

public async Task Initialize()
{
    trackerReader = new TrackerData();
    await trackerReader.ConnectAsync();
}

public async Task UpdateTrackers()
{
    var trackers = await trackerReader.ReadTrackersAsync();
    foreach (var tracker in trackers)
    {
        if (tracker.Valid)
        {
            // Convert to StereoKit types
            Vec3 position = new Vec3(tracker.X, tracker.Y, tracker.Z);
            Quat rotation = new Quat(tracker.Qx, tracker.Qy, tracker.Qz, tracker.Qw);
            
            // Use the tracker data in your StereoKit app
            // Example: Update visual representation of tracker
            Mesh trackerMesh = GetTrackerMesh(tracker.Serial);
            trackerMesh.Draw(Matrix.TRS(position, rotation, Vec3.One));
        }
    }
}
*/