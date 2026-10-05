using Godot;

namespace WorldMapStudio.WoW;

/// <summary>A `dbc_taxinodes` row as a streamed scene entity: a flight master.</summary>
public sealed class WowTaxiNode : SceneEntity
{
    /// <summary>`dbc_taxinodes.ID`; null until the entity is first committed.</summary>
    public int? NodeId { get; set; }

    [ScriptProperty(Mutable = true)]
    public int MountCreatureId1 { get; set; }

    [ScriptProperty(Mutable = true)]
    public int MountCreatureId2 { get; set; }

    [ScriptProperty(Mutable = true)]
    public int Flags { get; set; }

    /// <summary>World-map icon offset — not a world position. See <c>DbcTaxinodesRecord</c>.</summary>
    [ScriptProperty(Mutable = true)]
    public float MapOffset1 { get; set; }

    [ScriptProperty(Mutable = true)]
    public float MapOffset2 { get; set; }

    public override string DisplayName => Name.Length > 0 ? Name : $"Taxi Node {NodeId}";
}
