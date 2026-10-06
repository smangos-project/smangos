namespace WorldMapStudio.WoW;

/// <summary>One toggle over every loaded <see cref="WowSpellTargetPosition"/>.</summary>
[Subsystem(nameof(ViewCategorySystem))]
public sealed class WowSpellTargetPositionViewCategory : IViewCategory
{
    public string Id => "wow.view.spell-target-positions";

    public string DisplayName => "Spell Target Positions";

    public string Group => "Scene";

    public WowSpellTargetPositionViewCategory(ViewCategorySystem system)
    {
    }

    public bool Includes(SceneEntity entity) => entity is WowSpellTargetPosition;
}
