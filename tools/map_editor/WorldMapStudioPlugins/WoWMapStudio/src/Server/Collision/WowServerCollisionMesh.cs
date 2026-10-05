using System;
using System.Collections.Generic;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// A tile's collision as a wireframe: every placed model's unique triangle edges, transformed to its
/// spawn's position, coloured by kind so a doodad and a building read apart. A spawn whose model is missing
/// is left out — that a model failed to extract is exactly what this view should make visible.
/// </summary>
public static class WowServerCollisionMesh
{
    public static readonly Color M2Color = new(1.0f, 0.6f, 0.1f);
    public static readonly Color WmoColor = new(0.2f, 0.9f, 0.9f);

    /// <summary>Beyond this many vertices the rest of the tile is not drawn: a crowded tile would otherwise cost more than it shows.</summary>
    public const int MaxVertices = 1_500_000;

    public sealed record Result(WowServerMeshData? Mesh, int Drawn, int MissingModels, bool Truncated);

    public static Result Build(IReadOnlyList<WowVmapSpawn> spawns, Func<string, WowWorldModel?> modelOf)
    {
        var vertices = new List<Vector3>();
        var colors = new List<Color>();
        var indices = new List<int>();
        int drawn = 0;
        int missing = 0;
        bool truncated = false;

        foreach (WowVmapSpawn spawn in spawns)
        {
            if (modelOf(spawn.Name) is not { } model)
            {
                missing++;
                continue;
            }

            (float[] modelVertices, int[] edges) = model.Wireframe;
            if (edges.Length == 0)
            {
                continue;
            }

            if (vertices.Count + (modelVertices.Length / 3) > MaxVertices)
            {
                truncated = true;
                break;
            }

            var transform = new WowVmapTransform(spawn);
            Color color = spawn.IsM2 ? M2Color : WmoColor;
            int vertexBase = vertices.Count;
            for (int i = 0; i < modelVertices.Length; i += 3)
            {
                vertices.Add(transform.ToGodot(modelVertices[i], modelVertices[i + 1], modelVertices[i + 2]));
                colors.Add(color);
            }

            foreach (int edge in edges)
            {
                indices.Add(vertexBase + edge);
            }

            drawn++;
        }

        WowServerMeshData? mesh = vertices.Count == 0
            ? null
            : new WowServerMeshData
            {
                Vertices = vertices.ToArray(),
                Colors = colors.ToArray(),
                Indices = indices.ToArray(),
                Primitive = Mesh.PrimitiveType.Lines,
            };
        return new Result(mesh, drawn, missing, truncated);
    }
}
