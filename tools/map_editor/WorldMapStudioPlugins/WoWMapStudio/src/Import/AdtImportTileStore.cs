using System;
using System.Collections.Generic;
using System.Data.Common;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>
/// One row per ADT tile the importer has written into a project map. Its presence is the whole record
/// that a tile is done: a run skips any tile already at the current <see cref="ImporterVersion"/> from
/// the same <see cref="SourceId"/> (resume), re-importing a tile rewrites in place keyed on the same
/// three coordinates (idempotency), the picker marks tiles that have a row, and bumping the importer's
/// version invalidates every row so the next run redoes the map.
///
/// A <see cref="ITableConfiguration"/> rather than a <c>BatchState</c> key because the importer needs
/// per-tile bookkeeping across a multi-hour run — see the note on <c>BatchState</c> itself.
/// </summary>
public sealed class AdtImportTileRecord
{
    /// <summary>The project map's id — the map the importer created or extended, unrelated to the
    /// source's own map id.</summary>
    public int ProjectMap { get; set; }

    public int FileX { get; set; }

    public int FileY { get; set; }

    /// <summary>The asset source the tile was read from (<see cref="AssetSourceSettings"/> id), so a
    /// re-run against a different source is not mistaken for a resume.</summary>
    public string SourceId { get; set; } = "";

    /// <summary>A hash of the source tile's bytes, so a re-run can notice a tile whose source file
    /// changed even at the same importer version.</summary>
    public string SourceTileHash { get; set; } = "";

    public DateTime ImportedAtUtc { get; set; }

    /// <summary>The importer version that wrote this row. Bumping the importer's current version makes
    /// every existing row stale, exactly as the exporter's writer version re-exports everything.</summary>
    public int ImporterVersion { get; set; }

    /// <summary>Whether the importer turned the tile's MH2O into water entities, so the exporter knows
    /// that a tile with no water left should have its MH2O cleared rather than left alone.</summary>
    public bool WaterImported { get; set; }
}

[Subsystem(nameof(EditorStorage))]
public sealed class AdtImportTileTableConfiguration : ITableConfiguration, IMapScopedData
{
    private readonly EditorStorage _storage;

    // Keyed directly on the project map, not through an entity — see IMapScopedData's priority convention.
    public float Priority => 1.0f;

    public AdtImportTileTableConfiguration(EditorStorage storage)
    {
        _storage = storage;
    }

    public void Configure(ModelBuilder model)
    {
        model.Entity<AdtImportTileRecord>(entity =>
        {
            entity.ToTable("wow_adt_import_tiles");
            entity.HasKey(record => new { record.ProjectMap, record.FileX, record.FileY });
        });
    }

    string IMapScopedData.Label => "ADT import bookkeeping";

    Task<int> IMapScopedData.CountAsync(EditorDbContext context, MapId map) =>
        _storage.CountWhereMapAsync<AdtImportTileRecord>(context, nameof(AdtImportTileRecord.ProjectMap), map);

    async Task<IReadOnlySet<int>> IMapScopedData.MapIdsAsync(EditorDbContext context) =>
        (await context.Set<AdtImportTileRecord>().AsNoTracking().Select(record => record.ProjectMap).Distinct().ToListAsync().ConfigureAwait(false))
            .ToHashSet();

    Task IMapScopedData.DeleteAsync(EditorDbContext context, DbTransaction transaction, MapId map) =>
        _storage.DeleteWhereMapAsync<AdtImportTileRecord>(context, transaction, nameof(AdtImportTileRecord.ProjectMap), map);
}

/// <summary>Reads and writes <see cref="AdtImportTileRecord"/> under the editor storage's own lock.
/// The upsert is one tile's own transaction — the row lands if and only if that tile's import
/// committed.</summary>
public static class AdtImportTileStore
{
    public static async Task<IReadOnlyList<AdtImportTileRecord>> LoadForMapAsync(EditorStorage storage, int projectMap)
    {
        using IDisposable reader = await storage.Lock.ReaderAsync().ConfigureAwait(false);
        await using EditorDbContext context = storage.CreateContext();

        return await context.Set<AdtImportTileRecord>().AsNoTracking()
            .Where(row => row.ProjectMap == projectMap)
            .ToListAsync()
            .ConfigureAwait(false);
    }

    public static async Task<AdtImportTileRecord?> FindAsync(EditorStorage storage, int projectMap, int fileX, int fileY)
    {
        using IDisposable reader = await storage.Lock.ReaderAsync().ConfigureAwait(false);
        await using EditorDbContext context = storage.CreateContext();

        return await context.Set<AdtImportTileRecord>().AsNoTracking()
            .FirstOrDefaultAsync(row => row.ProjectMap == projectMap && row.FileX == fileX && row.FileY == fileY)
            .ConfigureAwait(false);
    }

    public static async Task UpsertAsync(EditorStorage storage, AdtImportTileRecord row)
    {
        using IDisposable write = await storage.Lock.WriterAsync().ConfigureAwait(false);
        await using EditorDbContext context = storage.CreateContext();
        await StageAsync(context, row).ConfigureAwait(false);
        await context.SaveChangesAsync().ConfigureAwait(false);
    }

    /// <summary>Runs within an already-open context and transaction — see
    /// <see cref="EditorStorage.CommitTransactionAsync"/>. Does not take the write lock.</summary>
    public static async Task UpsertWithinTransactionAsync(EditorDbContext context, AdtImportTileRecord row)
    {
        await StageAsync(context, row).ConfigureAwait(false);
        await context.SaveChangesAsync().ConfigureAwait(false);
    }

    private static async Task StageAsync(EditorDbContext context, AdtImportTileRecord row)
    {
        AdtImportTileRecord? existing = await context.Set<AdtImportTileRecord>()
            .FindAsync(row.ProjectMap, row.FileX, row.FileY)
            .ConfigureAwait(false);

        if (existing == null)
        {
            context.Set<AdtImportTileRecord>().Add(row);
        }
        else
        {
            existing.SourceId = row.SourceId;
            existing.SourceTileHash = row.SourceTileHash;
            existing.ImportedAtUtc = row.ImportedAtUtc;
            existing.ImporterVersion = row.ImporterVersion;
            existing.WaterImported = row.WaterImported;
        }
    }

    public static async Task DeleteAsync(EditorStorage storage, int projectMap, int fileX, int fileY)
    {
        using IDisposable write = await storage.Lock.WriterAsync().ConfigureAwait(false);
        await using EditorDbContext context = storage.CreateContext();

        AdtImportTileRecord? existing = await context.Set<AdtImportTileRecord>()
            .FindAsync(projectMap, fileX, fileY)
            .ConfigureAwait(false);
        if (existing == null)
        {
            return;
        }

        context.Set<AdtImportTileRecord>().Remove(existing);
        await context.SaveChangesAsync().ConfigureAwait(false);
    }
}
