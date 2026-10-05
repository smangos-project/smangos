namespace WorldMapStudio.WoW;

/// <summary>One toggle over every loaded <see cref="WowAreaTrigger"/>.</summary>
[Subsystem(nameof(ViewCategorySystem))]
public sealed class WowAreaTriggerViewCategory : IViewCategory
{
    public string Id => "wow.view.area-triggers";

    public string DisplayName => "Area Triggers";

    // Placed points, not route topology.
    public string Group => "Scene";

    public WowAreaTriggerViewCategory(ViewCategorySystem system)
    {
    }

    public bool Includes(SceneEntity entity) => entity is WowAreaTrigger;
}
