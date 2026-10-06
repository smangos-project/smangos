namespace WorldMapStudio.WoW;

/// <summary>
/// The two per-body MH2O attribute flags <c>WaterSurfaceMeshFunction</c> and
/// <c>RiverSurfaceMeshFunction</c> both expose. They drive <c>MH2O_Attributes</c>'
/// <c>fishable</c>/<c>fatigue</c> subchunk bitmasks: <see cref="WowLiquidRasterizer"/> sets the bit
/// for every subchunk a body covers, and only writes an attributes block when a body's flags differ
/// from the format's own defaults (fishable set, fatigue clear).
///
/// Per body, not per subchunk — a source cell whose bits vary within one water body cannot be
/// reproduced by these, which the importer flags rather than flattening silently.
/// </summary>
public static class WaterAttributeParameters
{
    public static readonly MeshParameter Fishable =
        MeshParameter.Bool("fishable", "Fishable", true, "Whether this water can be fished (MH2O per-subchunk fishable bit).");

    public static readonly MeshParameter Fatigue =
        MeshParameter.Bool("fatigue", "Fatigue", false, "Whether this water applies swim fatigue — the deep-ocean exhaustion mechanic (MH2O per-subchunk fatigue bit).");
}
