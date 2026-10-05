namespace WorldMapStudio.WoW;

/// <summary>One toggle over every loaded <see cref="WowCreatureSpawn"/>.</summary>
[Subsystem(nameof(ViewCategorySystem))]
public sealed class WowCreatureSpawnViewCategory : IViewCategory
{
    public string Id => "wow.view.creature-spawns";

    public string DisplayName => "Creature Spawns";

    public string Group => "Scene";

    public WowCreatureSpawnViewCategory(ViewCategorySystem system)
    {
    }

    public bool Includes(SceneEntity entity) => entity is WowCreatureSpawn;
}
