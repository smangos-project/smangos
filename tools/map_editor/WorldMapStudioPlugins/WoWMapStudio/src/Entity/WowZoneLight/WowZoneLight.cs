using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>A `dbc_zonelight` row as a streamed scene entity. The polygon outline lives in an
/// always-attached <see cref="WowZoneLightComponent"/> and maps to `dbc_zonelightpoint`; the last node
/// implicitly connects back to the first.</summary>
public sealed class WowZoneLight : SceneEntity
{
    /// <summary>`dbc_zonelight.ID`; null until first committed.</summary>
    public uint? ZoneLightId { get; set; }

    public string Name { get; set; } = string.Empty;

    /// <summary>`dbc_zonelight.LightID`, the `dbc_light` row applied inside the polygon.</summary>
    public int LightId { get; set; }

    public int Flags { get; set; }

    public float ZMin { get; set; } = -50000.0f;

    public float ZMax { get; set; } = 50000.0f;

    public override string DisplayName => Name.Length > 0 ? Name : $"Zone Light {ZoneLightId}";

    public override Aabb LocalBounds =>
        Components.OfType<ISceneBoundsProvider>().Any() ? EffectiveLocalBounds : new Aabb(Vector3.One * -0.5f, Vector3.One);
}
