using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using ImGuiNET;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>
/// Loads `dbc_map` as a whole catalog — a few hundred rows, small enough to bulk-load on world load
/// the same way <see cref="BattlefieldTemplateFactory"/> does its own table. Read-only: nothing here
/// authors map rows, so <see cref="DrawCreate"/> draws nothing and
/// <see cref="Create"/>/<see cref="Stage"/>/<see cref="StageDelete"/> refuse. It exists so the WoW map
/// id is resolvable by name — <see cref="WowCreatureSpawnFactory"/> reads this catalog to scope a
/// `tc_creature` scan to the map actually on screen.
/// </summary>
[Subsystem(nameof(WorldStorage))]
public sealed class DbcMapFactory : ICatalogEntityFactory, ICatalogBrowser
{
    private const int SearchLimit = 100;

    private readonly WorldStorage _storage;

    public DbcMapFactory(WorldStorage storage)
    {
        _storage = storage;
    }

    public Type EntityType => typeof(DbcMap);

    public string CatalogName => "Map";

    public bool Handles(IEntity entity) => entity is DbcMap;

    // dbc_map is already mapped by its generated DbcMapTableConfiguration.
    public void Configure(ModelBuilder model)
    {
    }

    public async Task<IReadOnlyList<CatalogEntity>> LoadAllAsync()
    {
        await using WorldDbContext context = _storage.CreateContext();

        List<DbcMapRecord> rows = await context.DbcMap.AsNoTracking()
            .OrderBy(row => row.ID).ToListAsync().ConfigureAwait(false);

        return rows.Select(row => (CatalogEntity)ToEntity(row)).ToList();
    }

    public async Task<int> MaxRecordIdAsync()
    {
        await using WorldDbContext context = _storage.CreateContext();
        return await context.DbcMap.MaxAsync(row => (int?)row.ID).ConfigureAwait(false) ?? 0;
    }

    public async Task<IReadOnlyList<CatalogSearchResult>> SearchAsync(string filter)
    {
        await using WorldDbContext context = _storage.CreateContext();

        IQueryable<DbcMapRecord> query = context.DbcMap.AsNoTracking();
        if (!string.IsNullOrWhiteSpace(filter))
        {
            query = int.TryParse(filter, out int id)
                ? query.Where(record => record.ID == id)
                : query.Where(record => EF.Functions.Like(record.Directory, $"%{filter}%")
                    || EF.Functions.Like(record.MapNameLang, $"%{filter}%"));
        }

        List<DbcMapRecord> rows = await query.OrderBy(record => record.ID).Take(SearchLimit)
            .ToListAsync().ConfigureAwait(false);

        return rows.Select(record => new CatalogSearchResult(
            record.ID.ToString(), $"{record.Directory} ({record.MapNameLang})")).ToList();
    }

    // Eagerly loaded (LoadAllAsync): every row is already in the registry, so this is a plain lookup.
    public Task<IReadOnlyDictionary<string, string>> DescribeAsync(EditorContext context, IReadOnlyCollection<string> keys)
    {
        var result = new Dictionary<string, string>();
        foreach (string key in keys)
        {
            if (int.TryParse(key, out int id) &&
                context.Catalog.OfType<DbcMap>().FirstOrDefault(e => e.MapId == id) is { } entity)
            {
                result[key] = $"{entity.Directory} ({entity.MapName})";
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

        if (context.Catalog.OfType<DbcMap>().FirstOrDefault(e => e.MapId == id) is { } open)
        {
            return open;
        }

        await using WorldDbContext db = _storage.CreateContext();
        DbcMapRecord? record = await db.DbcMap.AsNoTracking()
            .FirstOrDefaultAsync(row => row.ID == id).ConfigureAwait(false);
        return record is null ? null : Register(context, ToEntity(record));
    }

    // Every field is read-only reference data — drawn disabled, nothing written back to the tracker.
    // `navigate` goes unused: nothing here links to another catalog.
    public void DrawFields(EditorContext context, CatalogEntity entity, FieldEditTracker tracker,
        Action<string, string> navigate, string fieldFilter)
    {
        var typed = (DbcMap)entity;
        var f = new CatalogFieldSheet(context, entity, tracker, navigate, fieldFilter);

        f.Custom("Map Id", () => ImGui.TextDisabled($"Map Id: {typed.MapId}"));
        f.Custom("Directory", () => ImGui.TextDisabled($"Directory: {typed.Directory}"));
        f.Custom("Name", () => ImGui.TextDisabled($"Name: {typed.MapName}"));
        f.Custom("Instance Type", () => ImGui.TextDisabled($"Instance Type: {typed.InstanceType}"));
        f.Custom("Parent Map Id", () => ImGui.TextDisabled($"Parent Map Id: {typed.ParentMapId}"));
    }

    // Read-only reference data — nothing to create.
    public void DrawCreate(EditorContext context, Action<CatalogEntity> onCreated)
    {
    }

    public CatalogEntity Create(EditorContext context, string key) =>
        throw new InvalidOperationException("dbc_map is read-only reference data — map rows can't be created here.");

    public Action Stage(DbContext context, IEntity entity) =>
        throw new InvalidOperationException("dbc_map is read-only reference data — map rows can't be saved.");

    public void StageDelete(DbContext context, IEntity entity) =>
        throw new InvalidOperationException("dbc_map is read-only reference data — map rows can't be deleted.");

    private static DbcMap Register(EditorContext context, DbcMap entity)
    {
        context.Catalog.Add(entity);
        return entity;
    }

    private static DbcMap ToEntity(DbcMapRecord record) => new()
    {
        MapId = record.ID,
        Directory = record.Directory,
        MapName = record.MapNameLang,
        InstanceType = record.InstanceType,
        ParentMapId = record.ParentMapID,
    };
}
