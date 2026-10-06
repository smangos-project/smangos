using System;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Decides whether an extracted tile differs from the tile as the client has it, so unedited tiles are not rewritten.</summary>
public static class ImportedTileComparer
{
    private const float HeightTolerance = 0.01f;
    private const float ColorTolerance = 1f / 255f;

    public static bool IsUnchanged(ImportedTile a, ImportedTile b)
    {
        if (a.Chunks.Count != b.Chunks.Count || !a.Textures.SequenceEqual(b.Textures) || !a.HeightTextures.SequenceEqual(b.HeightTextures)
            || !a.Doodads.SequenceEqual(b.Doodads) || !a.Objects.SequenceEqual(b.Objects))
        {
            return false;
        }

        for (int i = 0; i < a.Chunks.Count; i++)
        {
            if (!ChunkEqual(a.Chunks[i], b.Chunks[i]))
            {
                return false;
            }
        }

        return true;
    }

    private static bool ChunkEqual(ImportedChunk a, ImportedChunk b) =>
        a.AreaId == b.AreaId && a.Flags == b.Flags
        && a.Holes.AsSpan().SequenceEqual(b.Holes)
        && Near(a.OuterHeights, b.OuterHeights) && Near(a.InnerHeights, b.InnerHeights)
        && Near(a.VertexColors, b.VertexColors) && Near(a.InnerVertexColors, b.InnerVertexColors)
        && Near(a.VertexLight, b.VertexLight) && Near(a.InnerVertexLight, b.InnerVertexLight)
        && a.Layers.Count == b.Layers.Count
        && a.Layers.Zip(b.Layers).All(p => p.First.TextureIndex == p.Second.TextureIndex && p.First.EffectId == p.Second.EffectId
            && (p.First.Alpha == null ? p.Second.Alpha == null : p.Second.Alpha != null && p.First.Alpha.AsSpan().SequenceEqual(p.Second.Alpha)));

    private static bool Near(float[] a, float[] b) =>
        a.Length == b.Length && a.Zip(b).All(p => MathF.Abs(p.First - p.Second) <= HeightTolerance);

    private static bool Near(Color[]? a, Color[]? b)
    {
        if (a == null || b == null)
        {
            return a == b;
        }

        return a.Length == b.Length && a.Zip(b).All(p =>
            MathF.Abs(p.First.R - p.Second.R) <= ColorTolerance && MathF.Abs(p.First.G - p.Second.G) <= ColorTolerance
            && MathF.Abs(p.First.B - p.Second.B) <= ColorTolerance && MathF.Abs(p.First.A - p.Second.A) <= ColorTolerance);
    }
}
