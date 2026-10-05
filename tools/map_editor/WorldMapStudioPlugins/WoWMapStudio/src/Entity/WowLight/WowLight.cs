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

    public override string DisplayName => $"Light {LightId}";
}
