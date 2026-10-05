using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Per-vertex metadata of a zone light polygon; vertices carry no data beyond position.</summary>
public readonly record struct ZoneLightNodeData;

/// <summary>A zone light polygon: an ordered chain of `dbc_zonelightpoint` vertices. See
/// <see cref="WowPathComponent{TNode}"/> for the shared shape.</summary>
public sealed class WowZoneLightComponent : WowPathComponent<ZoneLightNodeData>
{
    public const string Kind = "wow-zone-light";

    public override string TypeId => Kind;

    public override string DisplayName => "Zone Light Outline";

    protected override ZoneLightNodeData DefaultNode => default;

    protected override Color MeshColor => new(1.0f, 0.9f, 0.3f);

    public override SceneComponent Clone()
    {
        var clone = new WowZoneLightComponent();
        CopyStateInto(clone);
        return clone;
    }
}
