namespace WorldMapStudio.WoW;

/// <summary>One toggle over every loaded <see cref="WowLight"/>.</summary>
[Subsystem(nameof(ViewCategorySystem))]
public sealed class WowLightViewCategory : IViewCategory
{
    public string Id => "wow.view.lights";

    public string DisplayName => "Lights";

    // Placed points, not route topology.
    public string Group => "Scene";

    public WowLightViewCategory(ViewCategorySystem system)
    {
    }

    public bool Includes(SceneEntity entity) => entity is WowLight;
}
