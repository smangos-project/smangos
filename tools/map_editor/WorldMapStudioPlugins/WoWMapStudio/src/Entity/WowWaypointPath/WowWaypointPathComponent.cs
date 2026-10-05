using Godot;

namespace WorldMapStudio.WoW;

/// <summary>A creature's patrol route: an ordered chain of `creature_movement` rows. See
/// <see cref="WowPathComponent{TNode}"/> for the shared shape.</summary>
public sealed class WowWaypointPathComponent : WowPathComponent<WaypointNodeData>
{
    public const string Kind = "wow-waypoint-path";

    public override string TypeId => Kind;

    public override string DisplayName => "Waypoint Nodes";

    protected override WaypointNodeData DefaultNode => WaypointNodeData.Default;

    protected override Color MeshColor => new(0.25f, 0.85f, 0.4f);

    public override SceneComponent Clone()
    {
        var clone = new WowWaypointPathComponent();
        CopyStateInto(clone);
        return clone;
    }
}
