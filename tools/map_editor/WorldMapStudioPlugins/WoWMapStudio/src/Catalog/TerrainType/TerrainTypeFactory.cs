using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using ImGuiNET;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>
/// Loads <c>dbc_terraintype</c> as a whole catalog on world load, the same bulk-load shape
/// <see cref="AreaTableFactory"/> uses. Read-only reference data: the write path refuses. Search
/// covers id and description.
/// </summary>
[Subsystem(nameof(WorldStorage))]
public sealed class TerrainTypeFactory : ICatalogEntityFactory, ICatalogBrowser
{
    private const int SearchLimit = 100;

    private readonly WorldStorage _storage;

    public TerrainTypeFactory(WorldStorage storage)
    {
        _storage = storage;
    }

    public Type EntityType => typeof(TerrainType);

    public string CatalogName => "Terrain Type";

    public bool Handles(IEntity entity) => entity is TerrainType;

    public void Configure(ModelBuilder model)
    {
    }

    public async Task<IReadOnlyList<CatalogEntity>> LoadAllAsync()
    {
        await using WorldDbContext context = _storage.CreateContext();

        List<DbcTerraintypeRecord> rows = await context.DbcTerraintype.AsNoTracking()
            .OrderBy(row => row.ID).ToListAsync().ConfigureAwait(false);

        return rows.Select(row => (CatalogEntity)ToEntity(row)).ToList();
    }

    public async Task<int> MaxRecordIdAsync()
    {
        await using WorldDbContext context = _storage.CreateContext();
        return await context.DbcTerraintype.MaxAsync(row => (int?)row.ID).ConfigureAwait(false) ?? 0;
    }

    public async Task<IReadOnlyList<CatalogSearchResult>> SearchAsync(string filter)
    {
        await using WorldDbContext context = _storage.CreateContext();

        IQueryable<DbcTerraintypeRecord> query = context.DbcTerraintype.AsNoTracking();
        filter = filter?.Trim() ?? string.Empty;
        if (filter.Length > 0)
        {
            query = int.TryParse(filter, out int id)
                ? query.Where(record => record.ID == id)
                : query.Where(record => EF.Functions.Like(record.TerrainDesc, $"%{filter}%"));
        }

        List<DbcTerraintypeRecord> rows = await query.OrderBy(record => record.ID).Take(SearchLimit)
            .ToListAsync().ConfigureAwait(false);

        return rows.Select(record => new CatalogSearchResult(record.ID.ToString(), record.TerrainDesc)).ToList();
    }

    // Eagerly loaded (LoadAllAsync): every row is already in the registry, so this is a plain lookup.
    public Task<IReadOnlyDictionary<string, string>> DescribeAsync(EditorContext context, IReadOnlyCollection<string> keys)
    {
        var result = new Dictionary<string, string>();
        foreach (string key in keys)
        {
            if (int.TryParse(key, out int id) &&
                context.Catalog.OfType<TerrainType>().FirstOrDefault(e => e.TerrainTypeId == id) is { } entity)
            {
                result[key] = entity.Description;
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

        if (context.Catalog.OfType<TerrainType>().FirstOrDefault(e => e.TerrainTypeId == id) is { } open)
        {
            return open;
        }

        await using WorldDbContext db = _storage.CreateContext();
        DbcTerraintypeRecord? record = await db.DbcTerraintype.AsNoTracking()
            .FirstOrDefaultAsync(row => row.ID == id).ConfigureAwait(false);
        if (record is null)
        {
            return null;
        }

        TerrainType entity = ToEntity(record);
        context.Catalog.Add(entity);
        return entity;
    }

    public void DrawFields(EditorContext context, CatalogEntity entity, FieldEditTracker tracker,
        Action<string, string> navigate, string fieldFilter)
    {
        var typed = (TerrainType)entity;
        var f = new CatalogFieldSheet(context, entity, tracker, navigate, fieldFilter);

        f.Custom("Id", () => ImGui.TextDisabled($"Id: {typed.TerrainTypeId}"));
        f.Custom("Terrain Id", () => ImGui.TextDisabled($"Terrain Id: {typed.TerrainId}"));
        f.Custom("Description", () => ImGui.TextDisabled($"Description: {typed.Description}"));
        f.Custom("Footstep Spray", () => ImGui.TextDisabled(
            $"Footstep Spray: run {typed.FootstepSprayRun}, walk {typed.FootstepSprayWalk}"));
        f.Custom("Sound Id", () => ImGui.TextDisabled($"Sound Id: {typed.SoundId}"));
        f.Custom("Flags", () => ImGui.TextDisabled($"Flags: 0x{typed.Flags:X}"));
    }

    public void DrawCreate(EditorContext context, Action<CatalogEntity> onCreated)
    {
    }

    public CatalogEntity Create(EditorContext context, string key) =>
        throw new InvalidOperationException("dbc_terraintype is read-only reference data.");

    public Action Stage(DbContext context, IEntity entity) =>
        throw new InvalidOperationException("dbc_terraintype is read-only reference data.");

    public void StageDelete(DbContext context, IEntity entity) =>
        throw new InvalidOperationException("dbc_terraintype is read-only reference data.");

    private static TerrainType ToEntity(DbcTerraintypeRecord record) => new()
    {
        TerrainTypeId = record.ID,
        TerrainId = record.TerrainID,
        Description = record.TerrainDesc,
        FootstepSprayRun = record.FootstepSprayRun,
        FootstepSprayWalk = record.FootstepSprayWalk,
        SoundId = record.SoundID,
        Flags = record.Flags,
    };
}
