namespace WorldMapStudio.WoW;

[Subsystem(nameof(InspectorWindow))]
public sealed class WowTaxiNodeInspector : WowFieldInspector<WowTaxiNode>
{
    public WowTaxiNodeInspector(InspectorWindow window) : base(window)
    {
    }

    protected override void DrawFields(InspectorContext context, WowTaxiNode e)
    {
        Text(context, e, "Name", e.Name, v => e.Name = v);
        Int(context, e, "Mount creature (alliance)", e.MountCreatureId1, v => e.MountCreatureId1 = v);
        Int(context, e, "Mount creature (horde)", e.MountCreatureId2, v => e.MountCreatureId2 = v);
        Int(context, e, "Flags", e.Flags, v => e.Flags = v);
    }
}
