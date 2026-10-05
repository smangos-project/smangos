using System;
using System.Collections.Generic;
using System.Data.Common;
using System.Globalization;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

public sealed class SceneAdtTileComponentRecord
{
    public int EntityId { get; set; }

    public int FileX { get; set; }

    public int FileY { get; set; }

    public int? HeightLayerId { get; set; }

    public int? HeightMaterialId { get; set; }

    public int? HoleLayerId { get; set; }

    public int? HoleMaterialId { get; set; }

    public int? VertexColorLayerId { get; set; }

    public int? VertexColorMaterialId { get; set; }

    public int? VertexLightLayerId { get; set; }

    public int? VertexLightMaterialId { get; set; }

    public int? ChunkDataLayerId { get; set; }

    public int? ChunkDataMaterialId { get; set; }

    public int? SlotLayer0 { get; set; }

    public int? SlotLayer1 { get; set; }

    public int? SlotLayer2 { get; set; }

    public int? SlotLayer3 { get; set; }

    public EntityRecord? Entity { get; set; }
}

public sealed class SceneAdtTileChunkRecord
{
    public int EntityId { get; set; }

    public int LocalCol { get; set; }

    public int LocalRow { get; set; }

    public bool HasHoles { get; set; }

    public bool HasVertexColor { get; set; }

    public bool HasVertexLight { get; set; }

    /// <summary>The chunk's slot assignments, as <c>slot:layer:material</c> records joined by
    /// <c>|</c> — <c>layer</c>/<c>material</c> blank for null. At most 4. (Rows written before the
    /// effect id moved to the material carry a fourth <c>:effect</c> field, which is ignored on read.)</summary>
    public string Slots { get; set; } = "";

    public SceneAdtTileComponentRecord? Component { get; set; }
}

[Subsystem(nameof(EditorStorage))]
public sealed class AdtTileComponentPersistence : ISceneComponentPersistence
{
    private readonly EditorStorage _storage;

    public AdtTileComponentPersistence(EditorStorage storage)
    {
        _storage = storage;
    }

    public string TypeId => AdtTileComponent.Kind;

    public void Configure(ModelBuilder model)
    {
        model.Entity<SceneAdtTileComponentRecord>(entity =>
        {
            entity.ToTable("wow_adt_tile_components");
            entity.HasKey(record => record.EntityId);
            entity.HasOne(record => record.Entity)
                .WithOne()
                .HasForeignKey<SceneAdtTileComponentRecord>(record => record.EntityId)
                .OnDelete(DeleteBehavior.Cascade);
        });

        model.Entity<SceneAdtTileChunkRecord>(entity =>
        {
            entity.ToTable("wow_adt_tile_chunks");
            entity.HasKey(record => new { record.EntityId, record.LocalCol, record.LocalRow });
            entity.HasOne(record => record.Component)
                .WithMany()
                .HasForeignKey(record => record.EntityId)
                .OnDelete(DeleteBehavior.Cascade);
        });
    }

    public async Task LoadAsync(EditorDbContext context, IReadOnlyDictionary<int, SceneEntity> byId, IReadOnlyList<int> ids, SceneEntityScanCatalog catalog)
    {
        List<SceneAdtTileComponentRecord> rows = await context.Set<SceneAdtTileComponentRecord>().AsNoTracking()
            .Where(record => ids.Contains(record.EntityId))
            .ToListAsync()
            .ConfigureAwait(false);

        // See TerrainValueComponentPersistence: the chunk table's key is (EntityId, LocalCol,
        // LocalRow), and a scan-wide IN against its leading column costs hundreds of milliseconds
        // whatever it returns — here for the chunks of at most a handful of tiles.
        int[] owners = rows.Select(row => row.EntityId).ToArray();
        Dictionary<int, List<SceneAdtTileChunkRecord>> chunkRows = owners.Length == 0
            ? []
            : await context.Set<SceneAdtTileChunkRecord>().AsNoTracking()
                .Where(record => owners.Contains(record.EntityId))
                .GroupBy(record => record.EntityId)
                .ToDictionaryAsync(group => group.Key, group => group.ToList())
                .ConfigureAwait(false);

        foreach (SceneAdtTileComponentRecord row in rows)
        {
            if (!byId.TryGetValue(row.EntityId, out SceneEntity? entity))
            {
                continue;
            }

            var component = new AdtTileComponent
            {
                FileX = row.FileX,
                FileY = row.FileY,
                HeightLayerId = row.HeightLayerId,
                HeightMaterialId = row.HeightMaterialId,
                HoleLayerId = row.HoleLayerId,
                HoleMaterialId = row.HoleMaterialId,
                VertexColorLayerId = row.VertexColorLayerId,
                VertexColorMaterialId = row.VertexColorMaterialId,
                VertexLightLayerId = row.VertexLightLayerId,
                VertexLightMaterialId = row.VertexLightMaterialId,
                ChunkDataLayerId = row.ChunkDataLayerId,
                ChunkDataMaterialId = row.ChunkDataMaterialId,
                SlotLayerIds = [row.SlotLayer0, row.SlotLayer1, row.SlotLayer2, row.SlotLayer3],
            };

            if (chunkRows.TryGetValue(row.EntityId, out List<SceneAdtTileChunkRecord>? chunks))
            {
                foreach (SceneAdtTileChunkRecord chunk in chunks)
                {
                    component.SetChunk(chunk.LocalCol, chunk.LocalRow, new AdtChunkFacts(
                        ParseSlots(chunk.Slots),
                        chunk.HasHoles, chunk.HasVertexColor, chunk.HasVertexLight));
                }
            }

            entity.LoadComponent(component);
        }
    }

    public void Stage(EditorDbContext context, SceneEntity entity, EntityRecord entityRow)
    {
        AdtTileComponent? component = entity.Attached<AdtTileComponent>();
        if (component == null)
        {
            if (entity.RecordId is int id)
            {
                StageDelete(context, id);
            }

            return;
        }

        var row = new SceneAdtTileComponentRecord
        {
            Entity = entity.RecordId is null ? entityRow : null,
            EntityId = entity.RecordId ?? 0,
            FileX = component.FileX,
            FileY = component.FileY,
            HeightLayerId = component.HeightLayerId,
            HeightMaterialId = component.HeightMaterialId,
            HoleLayerId = component.HoleLayerId,
            HoleMaterialId = component.HoleMaterialId,
            VertexColorLayerId = component.VertexColorLayerId,
            VertexColorMaterialId = component.VertexColorMaterialId,
            VertexLightLayerId = component.VertexLightLayerId,
            VertexLightMaterialId = component.VertexLightMaterialId,
            ChunkDataLayerId = component.ChunkDataLayerId,
            ChunkDataMaterialId = component.ChunkDataMaterialId,
            SlotLayer0 = component.SlotLayerIds.ElementAtOrDefault(0),
            SlotLayer1 = component.SlotLayerIds.ElementAtOrDefault(1),
            SlotLayer2 = component.SlotLayerIds.ElementAtOrDefault(2),
            SlotLayer3 = component.SlotLayerIds.ElementAtOrDefault(3),
        };
        EditorComponentPersistenceHelpers.StageRow(context, row, entity.RecordId);
        StageChunks(context, component, row, entity.RecordId);
    }

    public void StageDelete(EditorDbContext context, int entityId) =>
        EditorComponentPersistenceHelpers.StageDelete<SceneAdtTileComponentRecord>(context, entityId);

    public async Task DeleteForMapAsync(EditorDbContext context, DbTransaction transaction, MapId map)
    {
        await _storage.DeleteForMapEntitiesAsync<SceneAdtTileChunkRecord>(context, transaction, nameof(SceneAdtTileChunkRecord.EntityId), map)
            .ConfigureAwait(false);
        await _storage.DeleteForMapEntitiesAsync<SceneAdtTileComponentRecord>(context, transaction, nameof(SceneAdtTileComponentRecord.EntityId), map)
            .ConfigureAwait(false);
    }

    private static void StageChunks(
        EditorDbContext context,
        AdtTileComponent component,
        SceneAdtTileComponentRecord row,
        int? entityId)
    {
        DbSet<SceneAdtTileChunkRecord> set = context.Set<SceneAdtTileChunkRecord>();

        var wanted = component.Chunks.ToDictionary(
            pair => (pair.Key.Col, pair.Key.Row),
            pair => pair.Value);

        if (entityId is int id)
        {
            List<SceneAdtTileChunkRecord> existing = set.Where(record => record.EntityId == id).ToList();
            foreach (SceneAdtTileChunkRecord chunk in existing)
            {
                if (wanted.Remove((chunk.LocalCol, chunk.LocalRow), out AdtChunkFacts? facts))
                {
                    chunk.HasHoles = facts.HasHoles;
                    chunk.HasVertexColor = facts.HasVertexColor;
                    chunk.HasVertexLight = facts.HasVertexLight;
                    chunk.Slots = SerializeSlots(facts.Slots);
                }
                else
                {
                    set.Remove(chunk);
                }
            }
        }

        foreach (((int col, int row) key, AdtChunkFacts facts) in wanted)
        {
            set.Add(new SceneAdtTileChunkRecord
            {
                Component = entityId is null ? row : null,
                EntityId = entityId ?? 0,
                LocalCol = key.col,
                LocalRow = key.row,
                HasHoles = facts.HasHoles,
                HasVertexColor = facts.HasVertexColor,
                HasVertexLight = facts.HasVertexLight,
                Slots = SerializeSlots(facts.Slots),
            });
        }
    }

    private static string SerializeSlots(IReadOnlyList<AdtSlotAssignment> slots) => string.Join("|", slots.Select(slot =>
        string.Join(":",
            slot.SlotIndex.ToString(CultureInfo.InvariantCulture),
            slot.LayerId?.ToString(CultureInfo.InvariantCulture) ?? "",
            slot.MaterialId?.ToString(CultureInfo.InvariantCulture) ?? "")));

    private static IReadOnlyList<AdtSlotAssignment> ParseSlots(string serialized)
    {
        if (string.IsNullOrEmpty(serialized))
        {
            return Array.Empty<AdtSlotAssignment>();
        }

        var result = new List<AdtSlotAssignment>();
        foreach (string record in serialized.Split('|', StringSplitOptions.RemoveEmptyEntries))
        {
            string[] parts = record.Split(':');
            if (parts.Length < 3)
            {
                continue;
            }

            result.Add(new AdtSlotAssignment(
                int.Parse(parts[0], CultureInfo.InvariantCulture),
                parts[1].Length == 0 ? null : int.Parse(parts[1], CultureInfo.InvariantCulture),
                parts[2].Length == 0 ? null : int.Parse(parts[2], CultureInfo.InvariantCulture)));
        }

        return result;
    }
}
