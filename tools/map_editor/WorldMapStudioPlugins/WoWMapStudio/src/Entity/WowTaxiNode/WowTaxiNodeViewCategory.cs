namespace WorldMapStudio.WoW;

/// <summary>One toggle over every loaded <see cref="WowTaxiNode"/>.</summary>
[Subsystem(nameof(ViewCategorySystem))]
public sealed class WowTaxiNodeViewCategory : IViewCategory
{
    public string Id => "wow.view.taxi-nodes";

    public string DisplayName => "Taxi Nodes";

    public string Group => "Paths";

    public WowTaxiNodeViewCategory(ViewCategorySystem system)
    {
    }

    public bool Includes(SceneEntity entity) => entity is WowTaxiNode;
}
