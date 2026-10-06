using System.Collections.Generic;

namespace WorldMapStudio.WoW;

/// <summary>Which per-vertex arrays a <see cref="RasterLayer"/> carries; values match the MH2O vertex format ids.</summary>
public enum WaterVertexFormat
{
    HeightDepth = 0,
    HeightUv = 1,
    DepthOnly = 2,
    HeightUvDepth = 3,
}

/// <summary>One liquid layer of one cell, in its 8x8 subchunk grid. Corner arrays are
/// (Width + 1) * (Height + 1); a null <see cref="ExistsBitmap"/> means every subchunk renders.</summary>
public sealed class RasterLayer
{
    public required int LiquidTypeId { get; init; }
    public required WaterVertexFormat VertexFormat { get; init; }
    public required float MinHeight { get; init; }
    public required float MaxHeight { get; init; }
    public required int XOffset { get; init; }
    public required int YOffset { get; init; }
    public required int Width { get; init; }
    public required int Height { get; init; }
    public bool[]? ExistsBitmap { get; init; }
    public float[]? Heightmap { get; init; }
    public byte[]? Depthmap { get; init; }
}

/// <summary>One cell's liquid: layers plus the per-subchunk fishable/fatigue bitmasks (bit <c>z * 8 + x</c>).</summary>
public sealed class RasterCell
{
    public bool HasAttributes { get; set; }
    public ulong Fishable { get; set; } = ulong.MaxValue;
    public ulong Fatigue { get; set; }
    public List<RasterLayer> Instances { get; } = [];
}
