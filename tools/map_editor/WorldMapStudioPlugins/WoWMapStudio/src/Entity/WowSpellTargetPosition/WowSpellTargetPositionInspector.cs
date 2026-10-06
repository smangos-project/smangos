using ImGuiNET;

namespace WorldMapStudio.WoW;

[Subsystem(nameof(InspectorWindow))]
public sealed class WowSpellTargetPositionInspector : WowFieldInspector<WowSpellTargetPosition>
{
    public WowSpellTargetPositionInspector(InspectorWindow window) : base(window)
    {
    }

    protected override void DrawFields(InspectorContext context, WowSpellTargetPosition e)
    {
        context.Fields.Field("Spell Entry", () => ImGui.TextDisabled($"Spell Entry: {e.SpellId}"));
    }
}
