namespace WorldMapStudio.WoW;

/// <summary>Per-node metadata for a <see cref="WowWaypointPathComponent"/> vertex: one `creature_movement`
/// row's non-positional columns.</summary>
public readonly record struct WaypointNodeData(float Orientation, int WaitTime, float WanderDistance, int ScriptId)
{
    public static readonly WaypointNodeData Default = new(0.0f, 0, 0.0f, 0);
}
