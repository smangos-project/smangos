namespace WorldMapStudio.WoW;

[Subsystem(nameof(InspectorWindow))]
public sealed class WowCreatureSpawnInspector : WowFieldInspector<WowCreatureSpawn>
{
    public WowCreatureSpawnInspector(InspectorWindow window) : base(window)
    {
    }

    protected override void DrawFields(InspectorContext context, WowCreatureSpawn e)
    {
        UInt(context, e, "Template", e.TemplateEntry, v => e.TemplateEntry = v);
        UInt(context, e, "Respawn min (s)", e.SpawnTimeMin, v => e.SpawnTimeMin = v);
        UInt(context, e, "Respawn max (s)", e.SpawnTimeMax, v => e.SpawnTimeMax = v);
        Float(context, e, "Wander distance", e.WanderDistance, v => e.WanderDistance = v);
        Int(context, e, "Movement type", e.MovementType, v => e.MovementType = (byte)System.Math.Clamp(v, 0, 255));
    }
}
