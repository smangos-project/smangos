namespace WorldMapStudio.WoW;

[Subsystem(nameof(InspectorWindow))]
public sealed class WowSoundEmitterInspector : WowFieldInspector<WowSoundEmitter>
{
    public WowSoundEmitterInspector(InspectorWindow window) : base(window)
    {
    }

    protected override void DrawFields(InspectorContext context, WowSoundEmitter e)
    {
        Text(context, e, "Name", e.Name, v => e.Name = v);
        Int(context, e, "Sound entry", e.SoundEntry, v => e.SoundEntry = v);
    }
}
