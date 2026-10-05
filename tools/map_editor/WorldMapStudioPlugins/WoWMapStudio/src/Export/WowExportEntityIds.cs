using System;
using System.Collections.Generic;
using System.Data.Common;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>
/// The uid an exported placement was given, keyed on its persistent key
/// (<see cref="SceneEntity.RecordId"/>; the <see cref="WowExportEntityIdRecord.EntityId"/> column
/// predates the rename). Not a cache — an assignment ledger, per entity, which is why it is a table
/// rather than a <see cref="BatchState"/> key. Export has exactly one ADT output, so it reads the
/// whole table; import scopes its read to one target map (<see cref="WowExportEntityIds.LoadForMapAsync"/>).
/// </summary>
public sealed class WowExportEntityIdRecord
{
    public long EntityId { get; set; }

    public long AllocatedId { get; set; }
}

[Subsystem(nameof(EditorStorage))]
public sealed class WowExportEntityIdTableConfiguration : ITableConfiguration, IMapScopedData
{
    private readonly EditorStorage _storage;

    // Found only through the map's entities (EntityId, not a map column) — must run before
    // MapSceneEntityFactory (priority 0) deletes them.
    public float Priority => -1.0f;

    public WowExportEntityIdTableConfiguration(EditorStorage storage)
    {
        _storage = storage;
    }

    public void Configure(ModelBuilder model)
    {
        model.Entity<WowExportEntityIdRecord>(entity =>
        {
            entity.ToTable("wow_export_entity_ids");
            entity.HasKey(record => record.EntityId);
        });
    }

    string IMapScopedData.Label => "Export id ledger";

    Task<int> IMapScopedData.CountAsync(EditorDbContext context, MapId map) =>
        _storage.CountForMapEntitiesAsync<WowExportEntityIdRecord>(context, nameof(WowExportEntityIdRecord.EntityId), map);

    Task<IReadOnlySet<int>> IMapScopedData.MapIdsAsync(EditorDbContext context) =>
        // The ledger is keyed on entity id, not a map — it holds no map id of its own to contribute,
        // and every map it could point to is already covered by MapSceneEntityFactory's own MapIdsAsync.
        Task.FromResult<IReadOnlySet<int>>(new HashSet<int>());

    Task IMapScopedData.DeleteAsync(EditorDbContext context, DbTransaction transaction, MapId map) =>
        _storage.DeleteForMapEntitiesAsync<WowExportEntityIdRecord>(context, transaction, nameof(WowExportEntityIdRecord.EntityId), map);
}

/// <summary>Reads and writes the ledger above, under the editor storage's own lock.</summary>
public static class WowExportEntityIds
{
    public static async Task<IReadOnlyDictionary<long, long>> LoadAsync(EditorStorage storage)
    {
        using IDisposable reader = await storage.Lock.ReaderAsync().ConfigureAwait(false);
        await using EditorDbContext context = storage.CreateContext();

        List<WowExportEntityIdRecord> rows = await context.Set<WowExportEntityIdRecord>().AsNoTracking()
            .ToListAsync()
            .ConfigureAwait(false);

        return rows.ToDictionary(record => record.EntityId, record => record.AllocatedId);
    }

    /// <summary>The allocated uids of placements that live in <paramref name="mapId"/>. The ADT import
    /// uses this — not the whole table — to tell an already-imported object from a new one, so
    /// importing one source map's tiles into a second target map is not blocked by the first's
    /// placements.</summary>
    public static async Task<IReadOnlyCollection<long>> LoadForMapAsync(EditorStorage storage, int mapId)
    {
        using IDisposable reader = await storage.Lock.ReaderAsync().ConfigureAwait(false);
        await using EditorDbContext context = storage.CreateContext();

        return await context.Set<WowExportEntityIdRecord>().AsNoTracking()
            .Join(
                context.MapEntities.AsNoTracking(),
                record => record.EntityId,
                sceneEntity => (long)sceneEntity.Id,
                (record, sceneEntity) => new { record.AllocatedId, sceneEntity.MapId })
            .Where(row => row.MapId == mapId)
            .Select(row => row.AllocatedId)
            .ToListAsync()
            .ConfigureAwait(false);
    }

    public static Task UpsertAsync(EditorStorage storage, IReadOnlyDictionary<long, long> ids) =>
        storage.UpsertLongPairsAsync<WowExportEntityIdRecord>(
            ids.Select(pair => (pair.Key, pair.Value)).ToList(),
            nameof(WowExportEntityIdRecord.EntityId),
            nameof(WowExportEntityIdRecord.AllocatedId));
}
