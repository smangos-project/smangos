using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using ImGuiNET;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>
/// Loads <c>dbc_groundeffectdoodad</c> as a whole catalog on world load, the same bulk-load shape
/// <see cref="AreaTableFactory"/> uses. Read-only reference data: the write path refuses. Search
/// covers id and doodad path.
/// </summary>
[Subsystem(nameof(WorldStorage))]
public sealed class GroundEffectDoodadFactory : ICatalogEntityFactory, ICatalogBrowser
{
    private const int SearchLimit = 100;

    private readonly WorldStorage _storage;

    public GroundEffectDoodadFactory(WorldStorage storage)
    {
        _storage = storage;
    }

    public Type EntityType => typeof(GroundEffectDoodad);

    public string CatalogName => "Ground Effect Doodad";

    public bool Handles(IEntity entity) => entity is GroundEffectDoodad;

    public void Configure(ModelBuilder model)
    {
    }

    public async Task<IReadOnlyList<CatalogEntity>> LoadAllAsync()
    {
        await using WorldDbContext context = _storage.CreateContext();

        List<DbcGroundeffectdoodadRecord> rows = await context.DbcGroundeffectdoodad.AsNoTracking()
            .OrderBy(row => row.ID).ToListAsync().ConfigureAwait(false);

        return rows.Select(row => (CatalogEntity)ToEntity(row)).ToList();
    }

    public async Task<int> MaxRecordIdAsync()
    {
        await using WorldDbContext context = _storage.CreateContext();
        return await context.DbcGroundeffectdoodad.MaxAsync(row => (int?)row.ID).ConfigureAwait(false) ?? 0;
    }

    public async Task<IReadOnlyList<CatalogSearchResult>> SearchAsync(string filter)
    {
        await using WorldDbContext context = _storage.CreateContext();

        IQueryable<DbcGroundeffectdoodadRecord> query = context.DbcGroundeffectdoodad.AsNoTracking();
        filter = filter?.Trim() ?? string.Empty;
        if (filter.Length > 0)
        {
            query = int.TryParse(filter, out int id)
                ? query.Where(record => record.ID == id || record.ModelFileID == id)
                : query.Where(record => false);
        }

        List<DbcGroundeffectdoodadRecord> rows = await query.OrderBy(record => record.ID).Take(SearchLimit)
            .ToListAsync().ConfigureAwait(false);

        return rows.Select(record => new CatalogSearchResult(record.ID.ToString(), $"Model file {record.ModelFileID}")).ToList();
    }

    // Eagerly loaded (LoadAllAsync): every row is already in the registry, so this is a plain lookup.
    public Task<IReadOnlyDictionary<string, string>> DescribeAsync(EditorContext context, IReadOnlyCollection<string> keys)
    {
        var result = new Dictionary<string, string>();
        foreach (string key in keys)
        {
            if (int.TryParse(key, out int id) &&
                context.Catalog.OfType<GroundEffectDoodad>().FirstOrDefault(e => e.GroundEffectDoodadId == id) is { } entity)
            {
                result[key] = $"Model file {entity.ModelFileID}";
            }
        }

        return Task.FromResult<IReadOnlyDictionary<string, string>>(result);
    }

    public async Task<CatalogEntity?> OpenAsync(EditorContext context, string key)
    {
        if (!int.TryParse(key, out int id))
        {
            return null;
        }

        if (context.Catalog.OfType<GroundEffectDoodad>().FirstOrDefault(e => e.GroundEffectDoodadId == id) is { } open)
        {
            return open;
        }

        await using WorldDbContext db = _storage.CreateContext();
        DbcGroundeffectdoodadRecord? record = await db.DbcGroundeffectdoodad.AsNoTracking()
            .FirstOrDefaultAsync(row => row.ID == id).ConfigureAwait(false);
        if (record is null)
        {
            return null;
        }

        GroundEffectDoodad entity = ToEntity(record);
        context.Catalog.Add(entity);
        return entity;
    }

    public void DrawFields(EditorContext context, CatalogEntity entity, FieldEditTracker tracker,
        Action<string, string> navigate, string fieldFilter)
    {
        var typed = (GroundEffectDoodad)entity;
        var f = new CatalogFieldSheet(context, entity, tracker, navigate, fieldFilter);

        f.Custom("Id", () => ImGui.TextDisabled($"Id: {typed.GroundEffectDoodadId}"));
        f.Custom("Model File ID", () => ImGui.TextDisabled($"Model File ID: {typed.ModelFileID}"));
        f.Custom("Flags", () => ImGui.TextDisabled($"Flags: 0x{typed.Flags:X}"));
    }

    public void DrawCreate(EditorContext context, Action<CatalogEntity> onCreated)
    {
    }

    public CatalogEntity Create(EditorContext context, string key) =>
        throw new InvalidOperationException("dbc_groundeffectdoodad is read-only reference data.");

    public Action Stage(DbContext context, IEntity entity) =>
        throw new InvalidOperationException("dbc_groundeffectdoodad is read-only reference data.");

    public void StageDelete(DbContext context, IEntity entity) =>
        throw new InvalidOperationException("dbc_groundeffectdoodad is read-only reference data.");

    private static GroundEffectDoodad ToEntity(DbcGroundeffectdoodadRecord record) => new()
    {
        GroundEffectDoodadId = record.ID,
        ModelFileID = record.ModelFileID,
        Flags = record.Flags,
    };
}
