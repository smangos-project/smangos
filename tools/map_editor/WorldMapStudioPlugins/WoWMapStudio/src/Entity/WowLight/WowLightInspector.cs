namespace WorldMapStudio.WoW;

[Subsystem(nameof(InspectorWindow))]
public sealed class WowLightInspector : WowFieldInspector<WowLight>
{
    public WowLightInspector(InspectorWindow window) : base(window)
    {
    }

    protected override void DrawFields(InspectorContext context, WowLight e)
    {
        Float(context, e, "Falloff start", e.FalloffStart, v => e.FalloffStart = v, 1.0f);
        Float(context, e, "Falloff end", e.FalloffEnd, v => e.FalloffEnd = v, 1.0f);
        Int(context, e, "Light params", e.LightParams, v => e.LightParams = v);
    }
}
