namespace WorldMapStudio.WoW;

/// <summary>
/// A <c>dbc_groundeffecttexture</c> row as a catalog entity — the client's ground-effect table (which
/// doodads scatter over a terrain texture, their weights, and how densely). Read-only reference data:
/// <see cref="GroundEffectTextureFactory"/> loads it whole for browsing and lookups but gives it no
/// create/edit surface. Its reason to exist is naming the id an MCLY layer's <c>effectId</c> points
/// at, once ground-effect ids are authored on a landscape material.
/// </summary>
public sealed class GroundEffectTexture : CatalogEntity
{
    [ScriptProperty]
    public int GroundEffectTextureId { get; set; }

    /// <summary><c>DoodadID1..4</c>: rows in <c>dbc_groundeffectdoodad</c> scattered over the texture.</summary>
    public int[] DoodadIds { get; set; } = new int[4];

    /// <summary><c>DoodadWeight1..4</c>: relative spawn chance of each doodad.</summary>
    public int[] DoodadWeights { get; set; } = new int[4];

    [ScriptProperty]
    public int Density { get; set; }

    [ScriptProperty]
    public int Sound { get; set; }

    public override string DisplayName => $"Ground Effect Texture {GroundEffectTextureId}";
}
