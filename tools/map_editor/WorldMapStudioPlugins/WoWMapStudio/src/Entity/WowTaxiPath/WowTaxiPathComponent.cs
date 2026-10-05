using Godot;

namespace WorldMapStudio.WoW;

/// <summary>The flight path itself: an ordered chain of `dbc_taxipathnode` rows. See
/// <see cref="WowPathComponent{TNode}"/> for the shared shape.</summary>
public sealed class WowTaxiPathComponent : WowPathComponent<TaxiNodeData>
{
    public const string Kind = "wow-taxi-path";

    public override string TypeId => Kind;

    public override string DisplayName => "Taxi Path Nodes";

    protected override TaxiNodeData DefaultNode => TaxiNodeData.Default;

    protected override Color MeshColor => new(1.0f, 0.8f, 0.15f);

    public override SceneComponent Clone()
    {
        var clone = new WowTaxiPathComponent();
        CopyStateInto(clone);
        return clone;
    }
}
