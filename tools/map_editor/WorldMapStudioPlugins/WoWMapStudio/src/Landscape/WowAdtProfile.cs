using System.Collections.Generic;

namespace WorldMapStudio.WoW;

/// <summary>
/// WoW's own ADT terrain layout. Chunk sizing matches the grid constants <see cref="WowTileAddressing"/>
/// already uses for water (a WoW map tile is 16x16 MCNK chunks, <see cref="WowTileAddressing.TileSize"/>
/// yards across), so authoring against this profile lines up with the water rasterizer without either
/// having to be re-derived from the other.
/// </summary>
[Subsystem(nameof(LandscapeSystem))]
public sealed class WowAdtProfile : ILandscapeProfile
{
    public WowAdtProfile(LandscapeSystem landscape)
    {
    }

    public string Name => "WoW (ADT)";

    public string Description => "Matches a WoW ADT's MCNK layout: 33.33-yard chunks, a 4-texture blend, and 64-texel alpha maps.";

    public LandscapeSettings CreateSettings() => new()
    {
        ProfileName = Name,

        // One MCNK cell: a WoW map tile (WowTileAddressing.TileSize yards) split 16x16.
        ChunkWorldSize = WowTileAddressing.CellSize,

        // MCVT's outer height grid is 9x9, with edge rows shared between neighbouring chunks.
        ChunkHeightResolution = 9,

        // MCVT also stores one vertex at the centre of each of the 8x8 cells.
        HeightVertexLayout = HeightVertexLayout.GridWithCellCentres,

        // Standard uncompressed MCAL alpha layer resolution.
        ChunkAlphaResolution = 64,

        // MCNK holes are a 4x4 grid of low-resolution hole bits.
        ChunkHoleResolution = 4,

        // MCVT stores per-vertex height as IEEE floats relative to the chunk's base height, not a
        // quantized integer.
        HeightEncoding = HeightEncoding.Float32,
        AlphaBitDepth = 8,

        // Layers 1-3 blend over the base independently (alpha-over), rather than being normalized to
        // sum to 1 across the stack.
        AlphaNormalized = false,

        // A full map is a 64x64 grid of tiles; with the origin tile centred, chunks extend up to 32
        // tiles (512 chunks) from the origin in either direction.
        OriginChunkX = 0,
        OriginChunkY = 0,
        ChunkLimit = 512,

        // MCNK's texture layer count, including the base.
        TextureLimit = 4,
    };

    /// <summary>The stable key an MCNK area-id write binds to.</summary>
    public const string AreaAttributeKey = "wow.area";

    /// <summary>The stable key an MCNK-flags write binds to.</summary>
    public const string FlagsAttributeKey = "wow.mcnk_flags";

    /// <summary>MCNK header flag: skip the client's alpha-map edge fix-up. Set on every MCNK this
    /// exporter writes, so it is the flags attribute's default.</summary>
    public const uint DoNotFixAlphaMapFlag = 0x8000;

    public IReadOnlyList<TerrainAttributeSeed> SeedAttributes() =>
    [
        new TerrainAttributeSeed(new TerrainAttribute
        {
            Key = AreaAttributeKey,
            Name = "MCNK Area",
            Description = "The dbc_areatable id written into every MCNK header.",
            CellsPerChunkEdge = 1,
            Components = 1,
            ElementWidth = 32,
            Kind = TerrainAttributeKind.CatalogRef,
            CatalogName = "Area",
            DefaultValue = 0,
        }),
        new TerrainAttributeSeed(
            new TerrainAttribute
            {
                Key = FlagsAttributeKey,
                Name = "MCNK Flags",
                Description = "The MCNK header flag bits the ADT round-trips.",
                CellsPerChunkEdge = 1,
                Components = 1,
                ElementWidth = 32,
                Kind = TerrainAttributeKind.Flags,
                DefaultValue = DoNotFixAlphaMapFlag,
            },
            [
                new TerrainAttributeValueSeed(0x2, "impassable"),
                new TerrainAttributeValueSeed(0x4, "river"),
                new TerrainAttributeValueSeed(0x8, "ocean"),
                new TerrainAttributeValueSeed(0x10, "magma"),
                new TerrainAttributeValueSeed(0x20, "slime"),
                new TerrainAttributeValueSeed(0x8000, "do_not_fix_alpha_map"),
                new TerrainAttributeValueSeed(0x10000, "high_res_holes"),
            ]),
    ];
}
