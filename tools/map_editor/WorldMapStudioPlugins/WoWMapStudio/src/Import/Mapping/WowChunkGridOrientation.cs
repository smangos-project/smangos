namespace WorldMapStudio.WoW;

/// <summary>
/// The editor's chunk grids run their fast index along world +X and their slow index along +Z; a WoW
/// MCNK grid runs fast East and slow South, which this map's axes land on editor +Z and -X. Grids
/// inside a chunk are therefore a quarter-turn apart even though the chunk sits in the right slot.
/// </summary>
public static class WowChunkGridOrientation
{
    /// <summary>Where an on-disk <paramref name="fast"/>/<paramref name="slow"/> index pair lands on the
    /// editor grid, for a square grid of <paramref name="edge"/> entries.</summary>
    public static (int X, int Z) ToEditorGrid(int fast, int slow, int edge) => (edge - 1 - slow, fast);
}
