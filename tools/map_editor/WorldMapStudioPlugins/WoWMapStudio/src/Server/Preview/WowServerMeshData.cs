using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// One mesh's worth of preview geometry as plain arrays, so a layer can build it on a worker thread and
/// the host can turn it into a Godot mesh on the frame thread. Vertices are in Godot space.
/// </summary>
public sealed class WowServerMeshData
{
    public required Vector3[] Vertices { get; init; }

    public required Color[] Colors { get; init; }

    /// <summary>Null for a non-indexed mesh.</summary>
    public int[]? Indices { get; init; }

    public Mesh.PrimitiveType Primitive { get; init; } = Mesh.PrimitiveType.Triangles;

    /// <summary>Drawn with alpha blending rather than opaque.</summary>
    public bool Translucent { get; init; }
}
