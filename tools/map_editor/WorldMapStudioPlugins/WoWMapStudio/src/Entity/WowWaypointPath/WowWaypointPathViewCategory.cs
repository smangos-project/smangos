namespace WorldMapStudio.WoW;

/// <summary>One toggle over every loaded <see cref="WowWaypointPath"/>.</summary>
[Subsystem(nameof(ViewCategorySystem))]
public sealed class WowWaypointPathViewCategory : IViewCategory
{
    public string Id => "wow.view.waypoint-paths";

    public string DisplayName => "Waypoint Paths";

    public string Group => "Paths";

    public WowWaypointPathViewCategory(ViewCategorySystem system)
    {
    }

    public bool Includes(SceneEntity entity) => entity is WowWaypointPath;
}
