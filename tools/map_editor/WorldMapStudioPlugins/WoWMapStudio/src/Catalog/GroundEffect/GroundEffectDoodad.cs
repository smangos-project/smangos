namespace WorldMapStudio.WoW;

/// <summary>
/// A <c>dbc_groundeffectdoodad</c> row as a catalog entity — one scatter model a
/// <see cref="GroundEffectTexture"/> can place (grass tuft, pebble, …). Read-only reference data:
/// <see cref="GroundEffectDoodadFactory"/> loads it whole for browsing and lookups.
/// </summary>
public sealed class GroundEffectDoodad : CatalogEntity
{
    [ScriptProperty]
    public int GroundEffectDoodadId { get; set; }

    [ScriptProperty]
    public int ModelFileID { get; set; }

    [ScriptProperty]
    public int Flags { get; set; }

    public override string DisplayName =>
        $"Ground Effect Doodad {GroundEffectDoodadId}: {ModelFileID}";
}
