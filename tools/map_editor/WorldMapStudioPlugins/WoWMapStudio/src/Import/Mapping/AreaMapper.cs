namespace WorldMapStudio.WoW;

/// <summary>
/// Turns one MCNK's non-raster header numbers — its <c>dbc_areatable</c> id and the flag bits the ADT
/// round-trips — into the two float components the map-wide <c>adt_chunk</c> image stores, read back
/// through <c>builtin.attribute.channel_value</c> into the <c>wow.area</c> and <c>wow.mcnk_flags</c>
/// terrain attributes. <c>float32</c> holds every id and flag word below 2^24 exactly, so the
/// round-trip is lossless. Joins <see cref="HeightMapper"/>/<see cref="HoleMapper"/>/
/// <see cref="VertexColorMapper"/> with the same pure <c>ImportedChunk</c>-in, pixel-value-out shape.
/// </summary>
public static class AreaMapper
{
    /// <summary>The MCNK header flag bits carried on <c>wow.mcnk_flags</c> — impassable, the four
    /// liquid-type hints, <c>do_not_fix_alpha_map</c> and <c>high_res_holes</c>. Bits the exporter
    /// computes itself (MCSH, MCCV) are masked out so importing them does not fight that computation.</summary>
    public const uint RoundTrippedFlagMask = 0x2u | 0x4u | 0x8u | 0x10u | 0x20u | 0x8000u | 0x10000u;

    public static float AreaValue(ImportedChunk chunk) => chunk.AreaId;

    public static float FlagsValue(ImportedChunk chunk) => chunk.Flags & RoundTrippedFlagMask;
}
