using System;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Decides whether an extracted tile differs from the tile as the client has it, so unedited tiles are not rewritten.</summary>
public static class ImportedTileComparer
{
    private const float HeightTolerance = 0.01f;
    private const float ColorTolerance = 1f / 255f;

    public static bool IsUnchanged(ImportedTile a, ImportedTile b) => FirstDifference(a, b) == null;

    /// <summary>What differs between the tiles, or null when they match.</summary>
    public static string? FirstDifference(ImportedTile a, ImportedTile b)
    {
        if (a.Chunks.Count != b.Chunks.Count) return "chunk count";
        if (!a.Textures.SequenceEqual(b.Textures)) return "textures";
        if (!a.HeightTextures.SequenceEqual(b.HeightTextures)) return "height textures";
        if (!a.Doodads.Zip(b.Doodads).All(p => DoodadEqual(p.First, p.Second)) || a.Doodads.Count != b.Doodads.Count) {
            int at = a.Doodads.Zip(b.Doodads).TakeWhile(p => DoodadEqual(p.First, p.Second)).Count();
            return $"doodads ({a.Doodads.Count} vs {b.Doodads.Count}) at {at}: {(at < a.Doodads.Count ? a.Doodads[at] : default)} vs {(at < b.Doodads.Count ? b.Doodads[at] : default)}";
        }
        if (!a.Objects.Zip(b.Objects).All(p => ObjectEqual(p.First, p.Second)) || a.Objects.Count != b.Objects.Count) return $"objects ({a.Objects.Count} vs {b.Objects.Count})";

        for (int i = 0; i < a.Chunks.Count; i++)
        {
            if (!ChunkEqual(a.Chunks[i], b.Chunks[i]))
            {
                return $"chunk {i}";
            }
        }

        return null;
    }

    private static bool Near(Vector3 a, Vector3 b) => (a - b).Length() <= HeightTolerance;

    private static bool DoodadEqual(ImportedDoodad a, ImportedDoodad b) =>
        a.ModelFdid == b.ModelFdid && a.UniqueId == b.UniqueId && a.Scale == b.Scale && a.Flags == b.Flags
        && Near(a.Position, b.Position) && WowPlacementTransform.SameRotation(a.RotationDegrees, b.RotationDegrees);

    private static bool ObjectEqual(ImportedObject a, ImportedObject b) =>
        a.WmoFdid == b.WmoFdid && a.UniqueId == b.UniqueId && a.Flags == b.Flags && a.DoodadSet == b.DoodadSet
        && a.NameSet == b.NameSet && a.Scale == b.Scale
        && Near(a.Position, b.Position) && WowPlacementTransform.SameRotation(a.RotationDegrees, b.RotationDegrees)
        && Near(a.LowerBounds, b.LowerBounds) && Near(a.UpperBounds, b.UpperBounds);

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
