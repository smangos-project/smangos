namespace WorldMapStudio.WoW;

/// <summary>One toggle over every loaded <see cref="WowSoundEmitter"/>.</summary>
[Subsystem(nameof(ViewCategorySystem))]
public sealed class WowSoundEmitterViewCategory : IViewCategory
{
    public string Id => "wow.view.sound-emitters";

    public string DisplayName => "Sound Emitters";

    // Placed points, not route topology.
    public string Group => "Scene";

    public WowSoundEmitterViewCategory(ViewCategorySystem system)
    {
    }

    public bool Includes(SceneEntity entity) => entity is WowSoundEmitter;
}
