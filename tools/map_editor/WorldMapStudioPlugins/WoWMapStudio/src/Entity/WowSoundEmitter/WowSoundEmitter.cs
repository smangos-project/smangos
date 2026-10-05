namespace WorldMapStudio.WoW;

/// <summary>A `dbc_soundemitters` row as a streamed scene entity; the basis carries the emitter direction.</summary>
public sealed class WowSoundEmitter : SceneEntity
{
    /// <summary>`dbc_soundemitters.ID`; null until first committed.</summary>
    public uint? EmitterId { get; set; }

    /// <summary>`dbc_soundemitters.SoundEntriesID`.</summary>
    public int SoundEntry { get; set; }

    public override string DisplayName => Name.Length > 0 ? Name : $"Sound Emitter {EmitterId}";
}
