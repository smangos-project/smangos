namespace WorldMapStudio.WoW;

/// <summary>A `dbc_light` row as a streamed scene entity: a point with a falloff sphere.</summary>
public sealed class WowLight : SceneEntity
{
    /// <summary>`dbc_light.ID`; null until first committed.</summary>
    public uint? LightId { get; set; }

    public float FalloffStart { get; set; }

    public float FalloffEnd { get; set; }

    /// <summary>`dbc_light.LightParamsID1`, the clear-weather parameter set.</summary>
    public int LightParams { get; set; }

    /// <summary>A light at the origin with no falloff is the map's default, blended as the base.</summary>
    public bool IsDefault => Transform.Origin == Godot.Vector3.Zero;

    public override string DisplayName => $"Light {LightId}";
}
