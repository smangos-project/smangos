namespace WorldMapStudio.WoW;

/// <summary>One toggle over every loaded <see cref="WowGraveyard"/>.</summary>
[Subsystem(nameof(ViewCategorySystem))]
public sealed class WowGraveyardViewCategory : IViewCategory
{
    public string Id => "wow.view.game-locs";

    public string DisplayName => "Graveyards";

    // Placed points, not route topology.
    public string Group => "Scene";

    public WowGraveyardViewCategory(ViewCategorySystem system)
    {
    }

    public bool Includes(SceneEntity entity) => entity is WowGraveyard;
}
