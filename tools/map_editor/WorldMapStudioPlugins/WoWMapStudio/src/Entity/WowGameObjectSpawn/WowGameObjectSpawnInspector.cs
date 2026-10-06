namespace WorldMapStudio.WoW;

[Subsystem(nameof(InspectorWindow))]
public sealed class WowGameObjectSpawnInspector : WowFieldInspector<WowGameObjectSpawn>
{
    public WowGameObjectSpawnInspector(InspectorWindow window) : base(window)
    {
    }

    protected override void DrawFields(InspectorContext context, WowGameObjectSpawn e)
    {
        UInt(context, e, "Template", e.TemplateEntry, v => e.TemplateEntry = v);
        UInt(context, e, "Respawn min (s)", e.SpawnTimeMin, v => e.SpawnTimeMin = v);
        UInt(context, e, "Respawn max (s)", e.SpawnTimeMax, v => e.SpawnTimeMax = v);
    }
}
