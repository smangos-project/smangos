namespace WorldMapStudio.WoW;

/// <summary>
/// A <c>dbc_terraintype</c> row as a catalog entity — the client's terrain-type table (footstep
/// sounds and spray effects for a ground surface). Read-only reference data:
/// <see cref="TerrainTypeFactory"/> loads it whole for browsing and lookups.
/// </summary>
public sealed class TerrainType : CatalogEntity
{
    [ScriptProperty]
    public int TerrainTypeId { get; set; }

    /// <summary><c>TerrainID</c>: the value an MCLY <c>effectId</c> resolves against for footstep audio.</summary>
    [ScriptProperty]
    public int TerrainId { get; set; }

    [ScriptProperty]
    public string Description { get; set; } = string.Empty;

    [ScriptProperty]
    public int FootstepSprayRun { get; set; }

    [ScriptProperty]
    public int FootstepSprayWalk { get; set; }

    [ScriptProperty]
    public int SoundId { get; set; }

    [ScriptProperty]
    public int Flags { get; set; }

    public override string DisplayName =>
        $"Terrain Type {TerrainTypeId}: {(Description.Length > 0 ? Description : "(unnamed)")}";
}
