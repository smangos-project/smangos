using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using ImGuiNET;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>
/// Loads <c>dbc_liquidtype</c> as a whole catalog on world load — a couple hundred rows, the same bulk-load shape
/// <see cref="DbcMapFactory"/> uses. Read-only reference data: nothing here authors liquid rows, so
/// <see cref="DrawCreate"/> draws nothing and the write path refuses. The database is the DBC authority in this
/// project.
///
/// The on-disk vertex format comes from <c>dbc_liquidmaterial.LVF</c> via
/// <see cref="LiquidType.MaterialId"/> — joined here at load so consumers see a resolved
/// <see cref="LiquidBasicType"/> without a second lookup.
/// </summary>
[Subsystem(nameof(WorldStorage))]
public sealed class LiquidTypeFactory : ICatalogEntityFactory, ICatalogBrowser
{
    private const int SearchLimit = 100;

    private readonly WorldStorage _storage;

    public LiquidTypeFactory(WorldStorage storage)
    {
        _storage = storage;
    }

    public Type EntityType => typeof(LiquidType);

    public string CatalogName => "Liquid Type";

    public bool Handles(IEntity entity) => entity is LiquidType;

    // dbc_liquidtype is already mapped by its generated DbcLiquidtypeTableConfiguration.
    public void Configure(ModelBuilder model)
    {
    }

    public async Task<IReadOnlyList<CatalogEntity>> LoadAllAsync()
    {
        await using WorldDbContext context = _storage.CreateContext();

        List<DbcLiquidtypeRecord> rows = await context.DbcLiquidtype.AsNoTracking()
            .OrderBy(row => row.ID).ToListAsync().ConfigureAwait(false);

        return rows.Select(row => (CatalogEntity)ToEntity(row)).ToList();
    }

    public async Task<int> MaxRecordIdAsync()
    {
        await using WorldDbContext context = _storage.CreateContext();
        return await context.DbcLiquidtype.MaxAsync(row => (int?)row.ID).ConfigureAwait(false) ?? 0;
    }

    public async Task<IReadOnlyList<CatalogSearchResult>> SearchAsync(string filter)
    {
        await using WorldDbContext context = _storage.CreateContext();

        IQueryable<DbcLiquidtypeRecord> query = context.DbcLiquidtype.AsNoTracking();
        if (!string.IsNullOrWhiteSpace(filter))
        {
            query = int.TryParse(filter, out int id)
                ? query.Where(record => record.ID == id)
                : query.Where(record => EF.Functions.Like(record.Name, $"%{filter}%"));
        }

        List<DbcLiquidtypeRecord> rows = await query.OrderBy(record => record.ID).Take(SearchLimit)
            .ToListAsync().ConfigureAwait(false);

        return rows.Select(record => new CatalogSearchResult(
            record.ID.ToString(), $"{record.Name} ({(LiquidBasicType)record.SoundBank})")).ToList();
    }

    // Eagerly loaded (LoadAllAsync): every row is already in the registry, so this is a plain lookup.
    public Task<IReadOnlyDictionary<string, string>> DescribeAsync(EditorContext context, IReadOnlyCollection<string> keys)
    {
        var result = new Dictionary<string, string>();
        foreach (string key in keys)
        {
            if (int.TryParse(key, out int id) &&
                context.Catalog.OfType<LiquidType>().FirstOrDefault(e => e.LiquidTypeId == id) is { } entity)
            {
                result[key] = $"{entity.Name} ({entity.BasicType})";
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

        if (context.Catalog.OfType<LiquidType>().FirstOrDefault(e => e.LiquidTypeId == id) is { } open)
        {
            return open;
        }

        await using WorldDbContext db = _storage.CreateContext();
        DbcLiquidtypeRecord? record = await db.DbcLiquidtype.AsNoTracking()
            .FirstOrDefaultAsync(row => row.ID == id).ConfigureAwait(false);
        if (record is null)
        {
            return null;
        }

        LiquidType entity = ToEntity(record);
        context.Catalog.Add(entity);
        return entity;
    }

    public void DrawFields(EditorContext context, CatalogEntity entity, FieldEditTracker tracker,
        Action<string, string> navigate, string fieldFilter)
    {
        var typed = (LiquidType)entity;
        var f = new CatalogFieldSheet(context, entity, tracker, navigate, fieldFilter);

        f.Custom("Id", () => ImGui.TextDisabled($"Id: {typed.LiquidTypeId}"));
        f.Custom("Name", () => ImGui.TextDisabled($"Name: {typed.Name}"));
        f.Custom("Basic Type", () => ImGui.TextDisabled($"Basic Type: {typed.BasicType}"));
        f.Custom("Material Id", () => ImGui.TextDisabled($"Material Id: {typed.MaterialId}"));
        f.Custom("Texture Template", () => ImGui.TextDisabled($"Texture Template: {typed.TextureTemplate}"));
        f.Custom("Animation", () => ImGui.TextDisabled($"Animation: {typed.AnimationX}, {typed.AnimationY}"));
    }

    public void DrawCreate(EditorContext context, Action<CatalogEntity> onCreated)
    {
    }

    public CatalogEntity Create(EditorContext context, string key) =>
        throw new InvalidOperationException("dbc_liquidtype is read-only reference data — liquid rows can't be created here.");

    public Action Stage(DbContext context, IEntity entity) =>
        throw new InvalidOperationException("dbc_liquidtype is read-only reference data — liquid rows can't be saved.");

    public void StageDelete(DbContext context, IEntity entity) =>
        throw new InvalidOperationException("dbc_liquidtype is read-only reference data — liquid rows can't be deleted.");

    private static LiquidType ToEntity(DbcLiquidtypeRecord record) => new()
    {
        LiquidTypeId = record.ID,
        Name = record.Name,
        BasicType = (LiquidBasicType)record.SoundBank,
        MaterialId = record.MaterialID,
        TextureTemplate = record.Texture1,
        AnimationX = record.Float1,
        AnimationY = record.Float2,
    };
}
