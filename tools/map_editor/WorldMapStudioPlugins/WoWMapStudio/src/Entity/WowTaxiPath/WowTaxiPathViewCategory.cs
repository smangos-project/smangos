namespace WorldMapStudio.WoW;

/// <summary>One toggle over every loaded <see cref="WowTaxiPath"/>.</summary>
[Subsystem(nameof(ViewCategorySystem))]
public sealed class WowTaxiPathViewCategory : IViewCategory
{
    public string Id => "wow.view.flight-paths";

    public string DisplayName => "Flight Paths";

    public string Group => "Paths";

    public WowTaxiPathViewCategory(ViewCategorySystem system)
    {
    }

    public bool Includes(SceneEntity entity) => entity is WowTaxiPath;
}
