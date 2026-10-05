using System;
using System.Collections.Generic;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Which texture (as a material) sits in one MCNK slot of one chunk. The MCLY
/// <c>effectId</c> is a per-texture property of the bound
/// <c>LandscapeMaterial.SurfaceEffectId</c>.</summary>
public sealed record AdtSlotAssignment(int SlotIndex, int? LayerId, int? MaterialId);

/// <summary>One MCNK's non-raster facts. The <c>Has*</c> flags gate whether this chunk claims the
/// corresponding layer at all — the grids themselves live in the <c>adt_holes</c> / <c>adt_vcolor</c> /
/// <c>adt_vlight</c> images. The area id and MCNK flags ride the <c>adt_chunk</c> image into the
/// <c>wow.area</c> / <c>wow.mcnk_flags</c> terrain attributes.</summary>
public sealed record AdtChunkFacts(
    IReadOnlyList<AdtSlotAssignment> Slots,
    bool HasHoles = false,
    bool HasVertexColor = false,
    bool HasVertexLight = false)
{
    public static readonly AdtChunkFacts Empty = new(Array.Empty<AdtSlotAssignment>());

    /// <summary>Whether two facts describe the same chunk. Slot lists compare by content, which record
    /// equality does not.</summary>
    public bool SameAs(AdtChunkFacts other) =>
        HasHoles == other.HasHoles &&
        HasVertexColor == other.HasVertexColor &&
        HasVertexLight == other.HasVertexLight &&
        Slots.SequenceEqual(other.Slots);
}

/// <summary>
/// One imported ADT tile's non-raster per-chunk facts, and the anchor re-import, removal and the
/// exporter all key on. Per chunk it carries the texture slot assignments (which material sits in
/// which of the 4 MCNK slots, in slot order), the area id, the MCNK flags worth keeping, and each
/// slot's ground-effect id.
///
/// As an <see cref="ILandscapeDeformer"/> it claims, per chunk, the map-wide height layer plus one
/// texture-slot layer per assigned slot — so a single component covers a whole tile while still
/// resolving differently in every chunk, keyed on <see cref="LandscapeClaimContext.Coord"/>.
/// </summary>
public sealed class AdtTileComponent : SceneComponent, ILandscapeDeformer, ISceneBoundsProvider, IIncrementalLandscapeDeformer
{
    public const string Kind = "wow-adt-tile";

    /// <summary>Texture slots per MCNK. Fixed by the format.</summary>
    public const int SlotCount = 4;

    private static readonly float TileWorldSize = WowTileAddressing.ChunksPerTileEdge * WowTileAddressing.CellSize;

    private readonly Dictionary<(int Col, int Row), AdtChunkFacts> _chunks = new();

    // Chunks whose facts changed since the last ConsumeDirtyRegions, so an edit to one chunk's slots
    // rebuilds that chunk rather than the tile.
    private readonly HashSet<(int Col, int Row)> _dirtyChunks = [];
    private bool _dirtyAll = true;
    private int? _consumedHeaderVersion;

    /// <summary>The source tile's filename indices — how a <see cref="ChunkCoord"/> is matched back to
    /// one of this tile's 256 slots without the map's directory name (<see cref="WowTileAddressing.SlotOf"/>).</summary>
    public int FileX { get; set; }

    public int FileY { get; set; }

    /// <summary>Map-wide catalog ids the claims resolve against — set by the importer from
    /// <see cref="AdtImportCatalog"/>.</summary>
    public int? HeightLayerId { get; set; }

    public int? HeightMaterialId { get; set; }

    public int? HoleLayerId { get; set; }

    public int? HoleMaterialId { get; set; }

    public int? VertexColorLayerId { get; set; }

    public int? VertexColorMaterialId { get; set; }

    public int? VertexLightLayerId { get; set; }

    public int? VertexLightMaterialId { get; set; }

    /// <summary>The map-wide layer/material whose terrain-attribute writes carry <c>wow.area</c> and
    /// <c>wow.mcnk_flags</c> — claimed by every chunk, since every MCNK has an area id and flags.</summary>
    public int? ChunkDataLayerId { get; set; }

    public int? ChunkDataMaterialId { get; set; }

    /// <summary>The 4 slot layers, indexed by MCNK slot. Populated once per map by the importer.</summary>
    public int?[] SlotLayerIds { get; set; } = new int?[SlotCount];

    public IReadOnlyDictionary<(int Col, int Row), AdtChunkFacts> Chunks => _chunks;

    public void SetChunk(int col, int row, AdtChunkFacts facts)
    {
        if (!_chunks.TryGetValue((col, row), out AdtChunkFacts? previous) || !previous.SameAs(facts))
        {
            _dirtyChunks.Add((col, row));
        }

        _chunks[(col, row)] = facts;
    }

    /// <summary>The world bounds of one of this tile's chunks.</summary>
    public Aabb ChunkWorldBounds(int col, int row)
    {
        ChunkCoord coord = WowTileAddressing.ChunkAt(new WowTileId("", FileX, FileY), col, row);
        float cell = WowTileAddressing.CellSize;
        return new Aabb(
            new Vector3(coord.X * cell, -LandscapeGrid.NominalHeightExtent, coord.Y * cell),
            new Vector3(cell, LandscapeGrid.NominalHeightExtent * 2.0f, cell));
    }

    public IReadOnlyList<Aabb> ConsumeDirtyRegions()
    {
        int header = HeaderVersion();
        bool wide = _dirtyAll || _consumedHeaderVersion != header;
        _consumedHeaderVersion = header;
        _dirtyAll = false;

        var regions = new List<Aabb>();
        if (wide)
        {
            regions.Add(InfluenceBounds);
        }
        else
        {
            foreach ((int col, int row) in _dirtyChunks)
            {
                regions.Add(ChunkWorldBounds(col, row));
            }
        }

        _dirtyChunks.Clear();
        return regions;
    }

    public void ReplaceChunks(IEnumerable<KeyValuePair<(int Col, int Row), AdtChunkFacts>> chunks)
    {
        _dirtyAll = true;
        _chunks.Clear();
        foreach ((var key, AdtChunkFacts facts) in chunks)
        {
            _chunks[key] = facts;
        }
    }

    public override string TypeId => Kind;

    public override string DisplayName => "WoW ADT Tile";

    public string DeformerKey => Entity.RecordId is { } id
        ? $"entity:{id}:wow-adt-tile"
        : $"entity:new:{Entity.Id.Value}:wow-adt-tile";

    public Aabb InfluenceBounds => Entity.WorldBounds;

    /// <summary>The tile's own footprint, so <c>LandscapeBuilder.Touching</c> filters this component to
    /// its tile's chunks rather than the whole map. The owning entity is placed at the tile centre.</summary>
    public Aabb LocalBounds => new(
        new Vector3(-TileWorldSize * 0.5f, -LandscapeGrid.NominalHeightExtent, -TileWorldSize * 0.5f),
        new Vector3(TileWorldSize, LandscapeGrid.NominalHeightExtent * 2.0f, TileWorldSize));

    // Everything a claim depends on except the per-chunk facts.
    private int HeaderVersion()
    {
        var hash = new HashCode();
        hash.Add(FileX);
        hash.Add(FileY);
        hash.Add(HeightLayerId);
        hash.Add(HeightMaterialId);
        hash.Add(HoleLayerId);
        hash.Add(HoleMaterialId);
        hash.Add(VertexColorLayerId);
        hash.Add(VertexColorMaterialId);
        hash.Add(VertexLightLayerId);
        hash.Add(VertexLightMaterialId);
        hash.Add(ChunkDataLayerId);
        hash.Add(ChunkDataMaterialId);
        foreach (int? slotLayer in SlotLayerIds)
        {
            hash.Add(slotLayer);
        }

        return hash.ToHashCode();
    }

    public override int ContentVersion
    {
        get
        {
            var hash = new HashCode();
            hash.Add(HeaderVersion());

            foreach ((var key, AdtChunkFacts facts) in _chunks.OrderBy(pair => (pair.Key.Row, pair.Key.Col)))
            {
                hash.Add(key.Col);
                hash.Add(key.Row);
                hash.Add(facts.HasHoles);
                hash.Add(facts.HasVertexColor);
                hash.Add(facts.HasVertexLight);
                foreach (AdtSlotAssignment slot in facts.Slots)
                {
                    hash.Add(slot.SlotIndex);
                    hash.Add(slot.MaterialId);
                }
            }

            return hash.ToHashCode();
        }
    }

    public override SceneComponent Clone()
    {
        var clone = new AdtTileComponent
        {
            FileX = FileX,
            FileY = FileY,
            HeightLayerId = HeightLayerId,
            HeightMaterialId = HeightMaterialId,
            HoleLayerId = HoleLayerId,
            HoleMaterialId = HoleMaterialId,
            VertexColorLayerId = VertexColorLayerId,
            VertexColorMaterialId = VertexColorMaterialId,
            VertexLightLayerId = VertexLightLayerId,
            VertexLightMaterialId = VertexLightMaterialId,
            ChunkDataLayerId = ChunkDataLayerId,
            ChunkDataMaterialId = ChunkDataMaterialId,
            SlotLayerIds = (int?[])SlotLayerIds.Clone(),
        };
        clone.ReplaceChunks(_chunks);
        return clone;
    }

    public IEnumerable<LandscapeClaimGroup> Claim(in LandscapeClaimContext context)
    {
        (int fileX, int fileY, int localCol, int localRow) = WowTileAddressing.SlotOf(context.Coord);
        if (fileX != FileX || fileY != FileY || !_chunks.TryGetValue((localCol, localRow), out AdtChunkFacts? facts))
        {
            return [];
        }

        var groups = new List<LandscapeClaimGroup>();

        if (context.Layer(HeightLayerId) is { } heightLayer)
        {
            groups.Add(new LandscapeClaimGroup
            {
                Key = $"{DeformerKey}:{localCol}:{localRow}:height",
                Label = $"{Entity.DisplayName}: terrain {localCol},{localRow}",
                Priority = 0,
                Claims = [new LandscapeClaim { Layer = heightLayer, Material = context.Material(HeightMaterialId) }],
                Source = Entity.Id,
            });
        }

        // Every MCNK carries an area id and flags, so the chunk-data layer is always claimed — unlike
        // holes / vertex colour / vertex light, which the Has* gates make conditional.
        AddClaim(groups, ChunkDataLayerId is not null, ChunkDataLayerId, ChunkDataMaterialId, "chunkdata", localCol, localRow, context);

        AddClaim(groups, facts.HasHoles, HoleLayerId, HoleMaterialId, "holes", localCol, localRow, context);
        AddClaim(groups, facts.HasVertexColor, VertexColorLayerId, VertexColorMaterialId, "vcolor", localCol, localRow, context);
        AddClaim(groups, facts.HasVertexLight, VertexLightLayerId, VertexLightMaterialId, "vlight", localCol, localRow, context);

        foreach (AdtSlotAssignment slot in facts.Slots)
        {
            if (slot.SlotIndex < 0 || slot.SlotIndex >= SlotCount ||
                context.Layer(SlotLayerIds[slot.SlotIndex]) is not { } slotLayer ||
                context.Material(slot.MaterialId) is not { } slotMaterial)
            {
                continue;
            }

            groups.Add(new LandscapeClaimGroup
            {
                Key = $"{DeformerKey}:{localCol}:{localRow}:slot{slot.SlotIndex}",
                Label = $"{Entity.DisplayName}: {slotLayer.Name} {localCol},{localRow}",
                Priority = slot.SlotIndex,
                Claims = [new LandscapeClaim { Layer = slotLayer, Material = slotMaterial }],
                Source = Entity.Id,
            });
        }

        return groups;
    }

    private void AddClaim(
        List<LandscapeClaimGroup> groups,
        bool present,
        int? layerId,
        int? materialId,
        string tag,
        int localCol,
        int localRow,
        in LandscapeClaimContext context)
    {
        if (!present || context.Layer(layerId) is not { } layer || context.Material(materialId) is not { } material)
        {
            return;
        }

        groups.Add(new LandscapeClaimGroup
        {
            Key = $"{DeformerKey}:{localCol}:{localRow}:{tag}",
            Label = $"{Entity.DisplayName}: {tag} {localCol},{localRow}",
            Priority = 0,
            Claims = [new LandscapeClaim { Layer = layer, Material = material }],
            Source = Entity.Id,
        });
    }

    public void Rasterize(in LandscapeRasterContext context)
    {
    }
}
