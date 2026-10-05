using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using ImGuiNET;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>
/// Loads <c>dbc_liquidobject</c> as a whole catalog on world load, the same bulk-load shape
/// <see cref="LiquidTypeFactory"/> uses. Read-only reference data.
/// </summary>
[Subsystem(nameof(WorldStorage))]
public sealed class LiquidObjectFactory : ICatalogEntityFactory, ICatalogBrowser
{
    private const int SearchLimit = 100;

    private readonly WorldStorage _storage;

    public LiquidObjectFactory(WorldStorage storage)
    {
        _storage = storage;
    }

    public Type EntityType => typeof(LiquidObject);

    public string CatalogName => "Liquid Object";

    public bool Handles(IEntity entity) => entity is LiquidObject;

    // dbc_liquidobject is already mapped by its generated DbcLiquidobjectTableConfiguration.
    public void Configure(ModelBuilder model)
    {
    }

    public async Task<IReadOnlyList<CatalogEntity>> LoadAllAsync()
    {
        await using WorldDbContext context = _storage.CreateContext();

        List<DbcLiquidobjectRecord> rows = await context.DbcLiquidobject.AsNoTracking()
            .OrderBy(row => row.ID).ToListAsync().ConfigureAwait(false);

        return rows.Select(row => (CatalogEntity)ToEntity(row)).ToList();
    }

    public async Task<int> MaxRecordIdAsync()
    {
        await using WorldDbContext context = _storage.CreateContext();
        return await context.DbcLiquidobject.MaxAsync(row => (int?)row.ID).ConfigureAwait(false) ?? 0;
    }

    public async Task<IReadOnlyList<CatalogSearchResult>> SearchAsync(string filter)
    {
        await using WorldDbContext context = _storage.CreateContext();

        IQueryable<DbcLiquidobjectRecord> query = context.DbcLiquidobject.AsNoTracking();
        if (!string.IsNullOrWhiteSpace(filter) && int.TryParse(filter, out int id))
        {
            query = query.Where(record => record.ID == id || record.LiquidTypeID == id);
        }

        List<DbcLiquidobjectRecord> rows = await query.OrderBy(record => record.ID).Take(SearchLimit)
            .ToListAsync().ConfigureAwait(false);

        return rows.Select(record => new CatalogSearchResult(
            record.ID.ToString(), $"type {record.LiquidTypeID}")).ToList();
    }

    // Eagerly loaded (LoadAllAsync): every row is already in the registry, so this is a plain lookup.
    public Task<IReadOnlyDictionary<string, string>> DescribeAsync(EditorContext context, IReadOnlyCollection<string> keys)
    {
        var result = new Dictionary<string, string>();
        foreach (string key in keys)
        {
            if (int.TryParse(key, out int id) &&
                context.Catalog.OfType<LiquidObject>().FirstOrDefault(e => e.LiquidObjectId == id) is { } entity)
            {
                result[key] = $"type {entity.LiquidTypeId}";
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

        if (context.Catalog.OfType<LiquidObject>().FirstOrDefault(e => e.LiquidObjectId == id) is { } open)
        {
            return open;
        }

        await using WorldDbContext db = _storage.CreateContext();
        DbcLiquidobjectRecord? record = await db.DbcLiquidobject.AsNoTracking()
            .FirstOrDefaultAsync(row => row.ID == id).ConfigureAwait(false);
        if (record is null)
        {
            return null;
        }

        LiquidObject entity = ToEntity(record);
        context.Catalog.Add(entity);
        return entity;
    }

    public void DrawFields(EditorContext context, CatalogEntity entity, FieldEditTracker tracker,
        Action<string, string> navigate, string fieldFilter)
    {
        var typed = (LiquidObject)entity;
        var f = new CatalogFieldSheet(context, entity, tracker, navigate, fieldFilter);

        f.Custom("Id", () => ImGui.TextDisabled($"Id: {typed.LiquidObjectId}"));
        f.Custom("Liquid Type Id", () => ImGui.TextDisabled($"Liquid Type Id: {typed.LiquidTypeId}"));
        f.Custom("Flow", () => ImGui.TextDisabled($"Flow: dir {typed.FlowDirection}, speed {typed.FlowSpeed}"));
        f.Custom("Fishable", () => ImGui.TextDisabled($"Fishable: {typed.Fishable}"));
        f.Custom("Reflection", () => ImGui.TextDisabled($"Reflection: {typed.Reflection}"));
    }

    public void DrawCreate(EditorContext context, Action<CatalogEntity> onCreated)
    {
    }

    public CatalogEntity Create(EditorContext context, string key) =>
        throw new InvalidOperationException("dbc_liquidobject is read-only reference data.");

    public Action Stage(DbContext context, IEntity entity) =>
        throw new InvalidOperationException("dbc_liquidobject is read-only reference data.");

    public void StageDelete(DbContext context, IEntity entity) =>
        throw new InvalidOperationException("dbc_liquidobject is read-only reference data.");

    private static LiquidObject ToEntity(DbcLiquidobjectRecord record) => new()
    {
        LiquidObjectId = record.ID,
        LiquidTypeId = record.LiquidTypeID,
        FlowDirection = record.FlowDirection,
        FlowSpeed = record.FlowSpeed,
        Fishable = record.Fishable != 0,
        Reflection = record.Reflection != 0,
    };
}
