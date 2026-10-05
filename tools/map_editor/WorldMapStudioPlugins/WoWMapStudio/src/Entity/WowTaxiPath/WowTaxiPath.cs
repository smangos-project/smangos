using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// One continent-run of a `dbc_taxipath`'s nodes as a streamed scene entity. A path that crosses
/// continents becomes two of these, one per run, sharing <see cref="PathId"/> and distinguished by
/// <see cref="RunIndex"/>. The nodes themselves live in an always-attached
/// <see cref="WowTaxiPathComponent"/>, edited with the shared path tool; this entity carries just
/// the row's scalar fields.
/// </summary>
public sealed class WowTaxiPath : SceneEntity
{
    /// <summary>`dbc_taxipath.ID`. Shared by every run of a split path. Storage-internal identity, so
    /// not a <see cref="ScriptPropertyAttribute"/> — the same reason <c>WowZoneLight.ZoneLightId</c>
    /// isn't.</summary>
    public int PathId { get; set; }

    /// <summary>Which contiguous continent run of <see cref="PathId"/> this entity is. 0 for every
    /// path that doesn't cross continents (1565 of 1601); 0 or 1 for the 36 that do.</summary>
    public int RunIndex { get; set; }

    [ScriptProperty(Mutable = true)]
    public int FromTaxiNodeId { get; set; }

    [ScriptProperty(Mutable = true)]
    public int ToTaxiNodeId { get; set; }

    [ScriptProperty(Mutable = true)]
    public int Cost { get; set; }

    /// <summary>Whether a `dbc_taxipath` row already exists for this entity.</summary>
    public bool IsSaved { get; set; }

    public override string DisplayName =>
        $"Taxi Path {FromTaxiNodeId} → {ToTaxiNodeId}" + (RunIndex != 0 ? $" (run {RunIndex})" : string.Empty);

    public override Aabb LocalBounds =>
        Components.OfType<ISceneBoundsProvider>().Any() ? EffectiveLocalBounds : new Aabb(Vector3.One * -0.5f, Vector3.One);
}
