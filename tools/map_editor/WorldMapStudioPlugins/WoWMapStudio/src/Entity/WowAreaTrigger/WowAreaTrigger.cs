using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// A `dbc_areatrigger` row as a streamed scene entity. A sphere when <see cref="Radius"/> is positive,
/// otherwise a yawed box. Owns the optional `areatrigger_teleport` row with the same id.
/// </summary>
public sealed class WowAreaTrigger : SceneEntity
{
    /// <summary>`dbc_areatrigger.ID`; null until first committed.</summary>
    public uint? TriggerId { get; set; }

    public float Radius { get; set; }

    public float BoxLength { get; set; }

    public float BoxWidth { get; set; }

    public float BoxHeight { get; set; }

    public float BoxYaw { get; set; }

    /// <summary>Teleport destination; null when the trigger has no `areatrigger_teleport` row.</summary>
    public WowAreaTriggerTeleport? Teleport { get; set; }

    public override string DisplayName => Teleport is { Message.Length: > 0 } t ? t.Message : $"Area Trigger {TriggerId}";
}

/// <summary>An `areatrigger_teleport` row; the destination is in world (WoW) coordinates.</summary>
public sealed class WowAreaTriggerTeleport
{
    public string Message { get; set; } = string.Empty;

    public int RequiredLevel { get; set; }

    public int RequiredCondition { get; set; }

    public int TargetMap { get; set; }

    public Vector3 TargetPosition { get; set; }

    public float TargetOrientation { get; set; }
}
