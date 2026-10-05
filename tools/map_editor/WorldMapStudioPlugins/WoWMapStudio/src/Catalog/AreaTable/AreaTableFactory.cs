using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using ImGuiNET;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>
/// Loads <c>dbc_areatable</c> as a whole catalog on world load — a few thousand rows, the same
/// bulk-load shape <see cref="DbcMapFactory"/> uses. Eager rather than lazy so id-to-name resolution
/// works in the terrain-attribute overlay legend and readout without a query per chunk. Read-only
/// reference data: nothing here authors area rows, so <see cref="DrawCreate"/> draws nothing and the
/// write path refuses.
///
/// <see cref="SearchAsync"/> scopes a blank or name search to the open map's continent when that
/// resolves against the eagerly-loaded <c>dbc_map</c> catalog; a leading <c>*</c> searches every
/// continent, and a bare id always looks up globally.
/// </summary>
[Subsystem(nameof(WorldStorage))]
public sealed class AreaTableFactory : ICatalogEntityFactory, ICatalogBrowser
{
    private const int SearchLimit = 100;

    private readonly WorldStorage _storage;

    public AreaTableFactory(WorldStorage storage)
    {
        _storage = storage;
    }

    public Type EntityType => typeof(AreaTable);

    public string CatalogName => "Area";

    public bool Handles(IEntity entity) => entity is AreaTable;

    // dbc_areatable is already mapped by its generated DbcAreatableTableConfiguration.
    public void Configure(ModelBuilder model)
    {
    }

    public async Task<IReadOnlyList<CatalogEntity>> LoadAllAsync()
    {
        await using WorldDbContext context = _storage.CreateContext();

        List<DbcAreatableRecord> rows = await context.DbcAreatable.AsNoTracking()
            .OrderBy(row => row.ID).ToListAsync().ConfigureAwait(false);

        return rows.Select(row => (CatalogEntity)ToEntity(row)).ToList();
    }

    public async Task<int> MaxRecordIdAsync()
    {
        await using WorldDbContext context = _storage.CreateContext();
        return await context.DbcAreatable.MaxAsync(row => (int?)row.ID).ConfigureAwait(false) ?? 0;
    }

    public async Task<IReadOnlyList<CatalogSearchResult>> SearchAsync(string filter)
    {
        filter = filter?.Trim() ?? string.Empty;

        bool everyContinent = filter.StartsWith('*');
        if (everyContinent)
        {
            filter = filter[1..].Trim();
        }

        await using WorldDbContext context = _storage.CreateContext();
        IQueryable<DbcAreatableRecord> query = context.DbcAreatable.AsNoTracking();

        if (int.TryParse(filter, out int id))
        {
            query = query.Where(record => record.ID == id);
        }
        else
        {
            if (filter.Length > 0)
            {
                query = query.Where(record => EF.Functions.Like(record.AreaNameLang, $"%{filter}%"));
            }

            if (!everyContinent && OpenContinentId() is { } continent)
            {
                query = query.Where(record => record.ContinentID == continent);
            }
        }

        List<DbcAreatableRecord> rows = await query.OrderBy(record => record.ID).Take(SearchLimit)
            .ToListAsync().ConfigureAwait(false);

        return rows.Select(record => new CatalogSearchResult(record.ID.ToString(), record.AreaNameLang)).ToList();
    }

    // Eagerly loaded (LoadAllAsync): every row is already in the registry, so this is a plain lookup.
    public Task<IReadOnlyDictionary<string, string>> DescribeAsync(EditorContext context, IReadOnlyCollection<string> keys)
    {
        var result = new Dictionary<string, string>();
        foreach (string key in keys)
        {
            if (int.TryParse(key, out int id) &&
                context.Catalog.OfType<AreaTable>().FirstOrDefault(e => e.AreaId == id) is { } entity)
            {
                result[key] = entity.AreaName;
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

        if (context.Catalog.OfType<AreaTable>().FirstOrDefault(e => e.AreaId == id) is { } open)
        {
            return open;
        }

        await using WorldDbContext db = _storage.CreateContext();
        DbcAreatableRecord? record = await db.DbcAreatable.AsNoTracking()
            .FirstOrDefaultAsync(row => row.ID == id).ConfigureAwait(false);
        if (record is null)
        {
            return null;
        }

        AreaTable entity = ToEntity(record);
        context.Catalog.Add(entity);
        return entity;
    }

    // Every field is read-only reference data — drawn disabled, nothing written back to the tracker.
    public void DrawFields(EditorContext context, CatalogEntity entity, FieldEditTracker tracker,
        Action<string, string> navigate, string fieldFilter)
    {
        var typed = (AreaTable)entity;
        var f = new CatalogFieldSheet(context, entity, tracker, navigate, fieldFilter);

        f.Custom("Area Id", () => ImGui.TextDisabled($"Area Id: {typed.AreaId}"));
        f.Custom("Name", () => ImGui.TextDisabled($"Name: {typed.AreaName}"));
        f.Custom("Continent Id", () => ImGui.TextDisabled($"Continent Id: {typed.ContinentId}"));
        f.Custom("Parent Chain", () => ImGui.TextDisabled($"Parent Chain: {ParentChain(context, typed)}"));
        f.Custom("Area Bit", () => ImGui.TextDisabled($"Area Bit: {typed.AreaBit}"));
        f.Custom("Flags", () => ImGui.TextDisabled($"Flags: 0x{typed.Flags:X}"));
        f.Custom("Flags 2", () => ImGui.TextDisabled($"Flags 2: 0x{typed.Flags2:X}"));
        f.Custom("Exploration Level", () => ImGui.TextDisabled($"Exploration Level: {typed.ExplorationLevel}"));
        f.Custom("Zone Music", () => ImGui.TextDisabled($"Zone Music: {typed.ZoneMusic}"));
        f.Custom("Ambience Id", () => ImGui.TextDisabled($"Ambience Id: {typed.AmbienceId}"));
        f.Custom("Liquid Type Ids", () => ImGui.TextDisabled(
            $"Liquid Type Ids: {string.Join(", ", typed.LiquidTypeIds)}"));
    }

    // Walks ParentAreaId up through the eagerly-loaded siblings: "1519: Stormwind City -> 0: (root)".
    private static string ParentChain(EditorContext context, AreaTable area)
    {
        List<AreaTable> all = context.Catalog.OfType<AreaTable>().ToList();
        var parts = new List<string>();
        AreaTable? current = area;
        var seen = new HashSet<int>();
        while (current is not null && current.ParentAreaId != 0 && seen.Add(current.AreaId))
        {
            AreaTable? parent = all.FirstOrDefault(a => a.AreaId == current.ParentAreaId);
            parts.Add(parent is null
                ? $"{current.ParentAreaId}: (not loaded)"
                : $"{parent.AreaId}: {parent.AreaName}");
            current = parent;
        }

        return parts.Count == 0 ? "(top-level zone)" : string.Join(" -> ", parts);
    }

    // The open map's WoW map id, which is what dbc_areatable.ContinentID references, resolved through
    // the map's explicit WowMapBindings binding. Null when the map is unbound — search then spans
    // every continent rather than hiding everything.
    private int? OpenContinentId() => WowMapResolution.Resolve(_storage, _storage.Context.Maps.CurrentMap);

    public void DrawCreate(EditorContext context, Action<CatalogEntity> onCreated)
    {
    }

    public CatalogEntity Create(EditorContext context, string key) =>
        throw new InvalidOperationException("dbc_areatable is read-only reference data — area rows can't be created here.");

    public Action Stage(DbContext context, IEntity entity) =>
        throw new InvalidOperationException("dbc_areatable is read-only reference data — area rows can't be saved.");

    public void StageDelete(DbContext context, IEntity entity) =>
        throw new InvalidOperationException("dbc_areatable is read-only reference data — area rows can't be deleted.");

    private static AreaTable ToEntity(DbcAreatableRecord record) => new()
    {
        AreaId = record.ID,
        ContinentId = record.ContinentID,
        ParentAreaId = record.ParentAreaID,
        AreaBit = record.AreaBit,
        Flags = record.Flags1,
        Flags2 = record.Flags2,
        AreaName = record.AreaNameLang,
        ExplorationLevel = record.ExplorationLevel,
        ZoneMusic = record.ZoneMusic,
        AmbienceId = record.AmbienceID,
        LiquidTypeIds = new[]
        {
            record.LiquidTypeID1,
            record.LiquidTypeID2,
            record.LiquidTypeID3,
            record.LiquidTypeID4,
        },
    };
}
