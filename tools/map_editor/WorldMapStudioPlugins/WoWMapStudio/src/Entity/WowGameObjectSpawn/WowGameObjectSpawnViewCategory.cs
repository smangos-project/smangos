namespace WorldMapStudio.WoW;

/// <summary>One toggle over every loaded <see cref="WowGameObjectSpawn"/>.</summary>
[Subsystem(nameof(ViewCategorySystem))]
public sealed class WowGameObjectSpawnViewCategory : IViewCategory
{
    public string Id => "wow.view.gameobject-spawns";

    public string DisplayName => "Game Object Spawns";

    public string Group => "Scene";

    public WowGameObjectSpawnViewCategory(ViewCategorySystem system)
    {
    }

    public bool Includes(SceneEntity entity) => entity is WowGameObjectSpawn;
}
