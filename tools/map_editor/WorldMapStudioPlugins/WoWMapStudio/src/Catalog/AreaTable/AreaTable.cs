namespace WorldMapStudio.WoW;

/// <summary>
/// A <c>dbc_areatable</c> row as a catalog entity — the client's area table (id, continent, parent
/// area, name, and the exploration/sound/liquid columns a zone carries). Read-only reference data:
/// <see cref="AreaTableFactory"/> loads it whole for lookups and browsing but gives it no
/// create/edit surface. Its reason to exist is resolving an MCNK <c>areaId</c> to a name in the
/// terrain-attribute overlay legend and readout without a query per chunk.
/// </summary>
public sealed class AreaTable : CatalogEntity
{
    [ScriptProperty]
    public int AreaId { get; set; }

    /// <summary>The <c>dbc_map</c> id this area sits on — 0 for Eastern Kingdoms, 1 for Kalimdor, ...</summary>
    [ScriptProperty]
    public int ContinentId { get; set; }

    /// <summary>The enclosing area's id, or 0 for a top-level zone. A subzone points at its zone.</summary>
    [ScriptProperty]
    public int ParentAreaId { get; set; }

    [ScriptProperty]
    public int AreaBit { get; set; }

    [ScriptProperty]
    public int Flags { get; set; }

    [ScriptProperty]
    public int Flags2 { get; set; }

    [ScriptProperty]
    public string AreaName { get; set; } = string.Empty;

    [ScriptProperty]
    public int ExplorationLevel { get; set; }

    [ScriptProperty]
    public int ZoneMusic { get; set; }

    [ScriptProperty]
    public int AmbienceId { get; set; }

    /// <summary><c>LiquidTypeID1..4</c>: the liquid ids allowed in this area, one per liquid class.</summary>
    public int[] LiquidTypeIds { get; set; } = new int[4];

    public override string DisplayName =>
        $"Area {AreaId}: {(AreaName.Length > 0 ? AreaName : "(unnamed)")}";
}
