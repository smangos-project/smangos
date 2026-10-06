using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Snaps a water vertex's XZ onto the MH2O subchunk lattice, so hand-authored water is exactly
/// representable — a covered subchunk's corners are then triangle vertices, and
/// <see cref="WowLiquidRasterizer"/> reads their heights back without interpolating.
///
/// The lattice is <c>ChunkSize / 8</c> apart (the ADT unit size on a real WoW map) and, because
/// <see cref="LandscapeGrid.OriginOf"/> puts the origin chunk at world zero, aligned to world zero.
/// Height is left untouched — a water surface is contoured, not flattened.
/// </summary>
internal static class WaterLatticeSnap
{
    public static Vector3 ToSubchunkCorner(EditorContext context, Vector3 worldPosition)
    {
        if (context.Landscape.Grid is not { } grid)
        {
            return worldPosition;
        }

        float step = grid.ChunkSize / WowLiquidLattice.SubchunksPerCell;
        return new Vector3(
            Mathf.Round(worldPosition.X / step) * step,
            worldPosition.Y,
            Mathf.Round(worldPosition.Z / step) * step);
    }
}
