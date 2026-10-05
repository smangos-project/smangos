namespace WorldMapStudio.WoW;

/// <summary>One toggle over every loaded <see cref="WowGameTele"/>.</summary>
[Subsystem(nameof(ViewCategorySystem))]
public sealed class WowGameTeleViewCategory : IViewCategory
{
    public string Id => "wow.view.game-teles";

    public string DisplayName => "Game Teles";

    // Placed points, not route topology.
    public string Group => "Scene";

    public WowGameTeleViewCategory(ViewCategorySystem system)
    {
    }

    public bool Includes(SceneEntity entity) => entity is WowGameTele;
}
