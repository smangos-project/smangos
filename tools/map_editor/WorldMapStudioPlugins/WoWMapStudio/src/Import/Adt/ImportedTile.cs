using System.Collections.Generic;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Identity of one ADT tile: the map directory name and the two filename indices.</summary>
public readonly record struct ImportedTileId(string Map, int FileX, int FileY);

/// <summary>One MDDF record as stored on disk. <see cref="ModelFdid"/> is 0 when the model could not be
/// resolved to a FileDataID.</summary>
public readonly record struct ImportedDoodad(
    uint ModelFdid,
    uint UniqueId,
    Vector3 Position,
    Vector3 RotationDegrees,
    ushort Scale,
    ushort Flags);

/// <summary>One MODF record as stored on disk, including the bounds the client uses to place the WMO
/// before its own geometry is loaded.</summary>
public readonly record struct ImportedObject(
    uint WmoFdid,
    uint UniqueId,
    Vector3 Position,
    Vector3 RotationDegrees,
    Vector3 LowerBounds,
    Vector3 UpperBounds,
    ushort Flags,
    ushort DoodadSet,
    ushort NameSet,
    ushort Scale);

/// <summary>One texture layer of one chunk. <see cref="TextureIndex"/> indexes
/// <see cref="ImportedTile.Textures"/>; <see cref="Alpha"/> is a 64×64 byte grid, null for the base layer.</summary>
public readonly record struct ImportedLayer(
    int TextureIndex,
    int EffectId,
    byte[]? Alpha);

/// <summary>One MH2O liquid instance of a chunk. <see cref="Heights"/> and <see cref="Depths"/> are
/// per-vertex ((Width+1)*(Height+1)) and null when the vertex format carries none.</summary>
public sealed class ImportedLiquidInstance
{
    public required ushort LiquidType { get; init; }
    public required ushort LiquidObjectOrFormat { get; init; }
    public required float MinHeight { get; init; }
    public required float MaxHeight { get; init; }
    public required int XOffset { get; init; }
    public required int YOffset { get; init; }
    public required int Width { get; init; }
    public required int Height { get; init; }

    /// <summary>One bit per cell (row-major, LSB first) of the Width×Height rectangle; null means all cells.</summary>
    public byte[]? Exists { get; init; }
    public float[]? Heights { get; init; }
    public byte[]? Depths { get; init; }
}

/// <summary>The MH2O contribution of one chunk; <see cref="Fishable"/> and <see cref="Fatigue"/> are 8×8 subchunk bitmasks.</summary>
public sealed class ImportedChunkLiquid
{
    public required bool HasAttributes { get; init; }
    public required ulong Fishable { get; init; }
    public required ulong Fatigue { get; init; }
    public required IReadOnlyList<ImportedLiquidInstance> Instances { get; init; }
}

/// <summary>One decoded MCNK. Heights are absolute world height.</summary>
public sealed class ImportedChunk
{
    public const int OuterEdge = 9;
    public const int InnerEdge = 8;
    public const int HoleEdge = 4;

    public required int LocalCol { get; init; }
    public required int LocalRow { get; init; }

    /// <summary>The 9×9 outer height grid, row-major.</summary>
    public required float[] OuterHeights { get; init; }

    /// <summary>The 8×8 inner (cell-centre) height grid, row-major.</summary>
    public required float[] InnerHeights { get; init; }

    /// <summary>The 9×9 MCCV grid as <c>byte/255</c>; null when the chunk has none.</summary>
    public Color[]? VertexColors { get; init; }
    public Color[]? InnerVertexColors { get; init; }

    /// <summary>The 9×9 MCLV grid; null when the chunk has none.</summary>
    public Color[]? VertexLight { get; init; }
    public Color[]? InnerVertexLight { get; init; }

    public required IReadOnlyList<ImportedLayer> Layers { get; init; }

    /// <summary>The 4×4 hole grid, row-major. True cuts the cell.</summary>
    public required bool[] Holes { get; init; }

    public required uint AreaId { get; init; }
    public required uint Flags { get; init; }
    public ImportedChunkLiquid? Liquid { get; init; }
}

/// <summary>One ADT tile as plain data: no editor or wowlib types, and file references are FileDataIDs.</summary>
public sealed class ImportedTile
{
    public const int ChunkEdge = 16;
    public const int ChunkCount = ChunkEdge * ChunkEdge;

    public required ImportedTileId TileId { get; init; }

    /// <summary>Diffuse texture FileDataIDs; MCLY indexes into this.</summary>
    public required IReadOnlyList<uint> Textures { get; init; }

    /// <summary>Height texture FileDataIDs, parallel to <see cref="Textures"/> (0 where absent).</summary>
    public required IReadOnlyList<uint> HeightTextures { get; init; }

    public required IReadOnlyList<ImportedDoodad> Doodads { get; init; }
    public required IReadOnlyList<ImportedObject> Objects { get; init; }

    /// <summary>The tile's 256 MCNKs in on-disk order (<c>row * 16 + col</c>).</summary>
    public required IReadOnlyList<ImportedChunk> Chunks { get; init; }
}
