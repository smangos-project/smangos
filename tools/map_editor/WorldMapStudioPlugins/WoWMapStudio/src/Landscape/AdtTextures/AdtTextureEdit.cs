using System;
using System.Collections.Generic;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>What a one-shot edit did: chunks it changed, and chunks it could not touch.</summary>
public readonly record struct AdtTextureEditResult(int Changed, int Skipped)
{
    public static readonly AdtTextureEditResult None = new(0, 0);
}

/// <summary>
/// One-shot texture edits on whole chunks, with no brush: replace, remove, swap, clean up. Each call is one
/// undo step. They load and store chunks through the same <see cref="AdtChunkEdit"/> the paint stroke uses.
/// </summary>
public sealed class AdtTextureEdit
{
    private readonly AdtTextureWorld _world;
    private readonly EditSessionManager _sessions;

    public AdtTextureEdit(AdtTextureWorld world, EditSessionManager sessions)
    {
        _world = world;
        _sessions = sessions;
    }

    /// <summary>Swaps <paramref name="from"/> for <paramref name="to"/> in whole chunks.</summary>
    public AdtTextureEditResult Replace(string from, string to, IEnumerable<ChunkCoord> chunks)
    {
        _world.Materials.EnsureTexture(to);
        return Run($"Replace {from}", chunks, edit => edit.Set.Replace(from, to));
    }

    /// <summary>Swaps <paramref name="from"/> for <paramref name="to"/> only where a disc covers a chunk's texels.</summary>
    public AdtTextureEditResult ReplaceInDisc(string from, string to, Vector3 centre, float radius)
    {
        _world.Materials.EnsureTexture(to);
        var dab = new BrushDab(centre, radius, 1.0f, Invert: false, Hardness: 1.0f);
        ChunkCoord min = _world.Grid.CoordAt(new Vector3(centre.X - radius, 0.0f, centre.Z - radius));
        ChunkCoord max = _world.Grid.CoordAt(new Vector3(centre.X + radius, 0.0f, centre.Z + radius));
        IEnumerable<ChunkCoord> coords = Range(min, max);

        return Run($"Replace {from}", coords, edit =>
        {
            var mask = new bool[AdtChunkTextureSet.TexelCount];
            BrushRaster.Walk(_world.GridOf(edit.Chunk.Coord), dab, (x, y, _) => mask[(y * AdtChunkTextureSet.Edge) + x] = true);
            return edit.Set.Replace(from, to, mask);
        });
    }

    /// <summary>Removes <paramref name="texture"/> from whole chunks, its weight going to the others.</summary>
    public AdtTextureEditResult Remove(string texture, IEnumerable<ChunkCoord> chunks) =>
        Run($"Remove {texture}", chunks, edit => edit.Set.IndexOf(texture) is var slot && slot >= 0 && edit.Set.Remove(slot));

    /// <summary>Removes the layer in one slot of one chunk.</summary>
    public AdtTextureEditResult RemoveSlot(ChunkCoord chunk, int slot) =>
        Run("Remove layer", [chunk], edit => edit.Set.Remove(slot));

    /// <summary>Swaps the stacking order of two layers of one chunk.</summary>
    public AdtTextureEditResult Swap(ChunkCoord chunk, int slotA, int slotB) =>
        Run("Swap layers", [chunk], edit => edit.Set.Swap(slotA, slotB));

    /// <summary>Drops layers that nothing shows.</summary>
    public AdtTextureEditResult Cleanup(IEnumerable<ChunkCoord> chunks, float threshold = AdtChunkTextureSet.VisibleThreshold) =>
        Run("Clean up textures", chunks, edit => edit.Set.EraseUnused(threshold));

    /// <summary>Merges layers that hold the same texture.</summary>
    public AdtTextureEditResult RemoveDuplicates(IEnumerable<ChunkCoord> chunks) =>
        Run("Remove duplicate textures", chunks, edit => edit.Set.RemoveDuplicates() > 0);

    /// <summary>Every imported chunk of one tile.</summary>
    public static IEnumerable<ChunkCoord> ChunksOfTile(AdtTileComponent tile) =>
        tile.Chunks.Keys.Select(key => WowTileAddressing.ChunkAt(new WowTileId("", tile.FileX, tile.FileY), key.Col, key.Row));

    public static IEnumerable<ChunkCoord> Range(ChunkCoord min, ChunkCoord max)
    {
        for (int x = min.X; x <= max.X; x++)
        {
            for (int y = min.Y; y <= max.Y; y++)
            {
                yield return new ChunkCoord(x, y);
            }
        }
    }

    private AdtTextureEditResult Run(string description, IEnumerable<ChunkCoord> chunks, Func<AdtChunkEdit, bool> operation)
    {
        var changed = new List<AdtChunkEdit>();
        int skipped = 0;

        _world.Image.BeginStroke();
        try
        {
            foreach (ChunkCoord coord in chunks.Distinct())
            {
                if (AdtChunkEdit.TryLoad(_world, coord, out AdtChunkEdit? edit) != AdtChunkLoadResult.Loaded)
                {
                    skipped++;
                    continue;
                }

                if (operation(edit!))
                {
                    edit!.Store();
                    changed.Add(edit);
                }
            }
        }
        catch
        {
            _world.Image.EndStroke();
            throw;
        }

        if (AdtChunkEdit.Commit(_world, changed, description) is { } command)
        {
            _sessions.Record(command);
            _world.NoticeChanged();
        }

        return new AdtTextureEditResult(changed.Count, skipped);
    }
}
