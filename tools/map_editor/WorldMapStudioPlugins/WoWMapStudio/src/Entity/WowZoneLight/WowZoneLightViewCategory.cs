namespace WorldMapStudio.WoW;

/// <summary>One toggle over every loaded <see cref="WowZoneLight"/>.</summary>
[Subsystem(nameof(ViewCategorySystem))]
public sealed class WowZoneLightViewCategory : IViewCategory
{
    public string Id => "wow.view.zone-lights";

    public string DisplayName => "Zone Lights";

    public string Group => "Lighting";

    public WowZoneLightViewCategory(ViewCategorySystem system)
    {
    }

    public bool Includes(SceneEntity entity) => entity is WowZoneLight;
}
