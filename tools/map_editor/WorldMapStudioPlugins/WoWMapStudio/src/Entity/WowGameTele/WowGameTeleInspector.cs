namespace WorldMapStudio.WoW;

[Subsystem(nameof(InspectorWindow))]
public sealed class WowGameTeleInspector : WowFieldInspector<WowGameTele>
{
    public WowGameTeleInspector(InspectorWindow window) : base(window)
    {
    }

    protected override void DrawFields(InspectorContext context, WowGameTele e)
    {
        Text(context, e, "Name", e.Name, v => e.Name = v);
    }
}
