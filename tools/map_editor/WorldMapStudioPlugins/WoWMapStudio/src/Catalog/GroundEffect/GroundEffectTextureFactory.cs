using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using ImGuiNET;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>
/// Loads <c>dbc_groundeffecttexture</c> as a whole catalog on world load — a small table, the same
/// bulk-load shape <see cref="AreaTableFactory"/> uses. Read-only reference data: nothing here
/// authors ground-effect rows, so <see cref="DrawCreate"/> draws nothing and the write path refuses.
/// </summary>
[Subsystem(nameof(WorldStorage))]
public sealed class GroundEffectTextureFactory : ICatalogEntityFactory, ICatalogBrowser
{
    private const int SearchLimit = 100;

    private readonly WorldStorage _storage;

    public GroundEffectTextureFactory(WorldStorage storage)
    {
        _storage = storage;
    }

    public Type EntityType => typeof(GroundEffectTexture);

    public string CatalogName => "Ground Effect Texture";

    public bool Handles(IEntity entity) => entity is GroundEffectTexture;

    public void Configure(ModelBuilder model)
    {
    }

    public async Task<IReadOnlyList<CatalogEntity>> LoadAllAsync()
    {
        await using WorldDbContext context = _storage.CreateContext();

        List<DbcGroundeffecttextureRecord> rows = await context.DbcGroundeffecttexture.AsNoTracking()
            .OrderBy(row => row.ID).ToListAsync().ConfigureAwait(false);

        return rows.Select(row => (CatalogEntity)ToEntity(row)).ToList();
    }

    public async Task<int> MaxRecordIdAsync()
    {
        await using WorldDbContext context = _storage.CreateContext();
        return await context.DbcGroundeffecttexture.MaxAsync(row => (int?)row.ID).ConfigureAwait(false) ?? 0;
    }

    public async Task<IReadOnlyList<CatalogSearchResult>> SearchAsync(string filter)
    {
        await using WorldDbContext context = _storage.CreateContext();

        IQueryable<DbcGroundeffecttextureRecord> query = context.DbcGroundeffecttexture.AsNoTracking();
        if (int.TryParse(filter?.Trim(), out int id))
        {
            query = query.Where(record => record.ID == id);
        }

        List<DbcGroundeffecttextureRecord> rows = await query.OrderBy(record => record.ID).Take(SearchLimit)
            .ToListAsync().ConfigureAwait(false);

        return rows.Select(record => new CatalogSearchResult(
            record.ID.ToString(), $"density {record.Density}")).ToList();
    }

    // Eagerly loaded (LoadAllAsync): every row is already in the registry, so this is a plain lookup.
    public Task<IReadOnlyDictionary<string, string>> DescribeAsync(EditorContext context, IReadOnlyCollection<string> keys)
    {
        var result = new Dictionary<string, string>();
        foreach (string key in keys)
        {
            if (int.TryParse(key, out int id) &&
                context.Catalog.OfType<GroundEffectTexture>().FirstOrDefault(e => e.GroundEffectTextureId == id) is { } entity)
            {
                result[key] = $"density {entity.Density}";
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

        if (context.Catalog.OfType<GroundEffectTexture>().FirstOrDefault(e => e.GroundEffectTextureId == id) is { } open)
        {
            return open;
        }

        await using WorldDbContext db = _storage.CreateContext();
        DbcGroundeffecttextureRecord? record = await db.DbcGroundeffecttexture.AsNoTracking()
            .FirstOrDefaultAsync(row => row.ID == id).ConfigureAwait(false);
        if (record is null)
        {
            return null;
        }

        GroundEffectTexture entity = ToEntity(record);
        context.Catalog.Add(entity);
        return entity;
    }

    public void DrawFields(EditorContext context, CatalogEntity entity, FieldEditTracker tracker,
        Action<string, string> navigate, string fieldFilter)
    {
        var typed = (GroundEffectTexture)entity;
        var f = new CatalogFieldSheet(context, entity, tracker, navigate, fieldFilter);

        f.Custom("Id", () => ImGui.TextDisabled($"Id: {typed.GroundEffectTextureId}"));
        f.Custom("Doodads", () => ImGui.TextDisabled($"Doodads: {string.Join(", ", typed.DoodadIds)}"));
        f.Custom("Weights", () => ImGui.TextDisabled($"Weights: {string.Join(", ", typed.DoodadWeights)}"));
        f.Custom("Density", () => ImGui.TextDisabled($"Density: {typed.Density}"));
        f.Custom("Sound", () => ImGui.TextDisabled($"Sound: {typed.Sound}"));
    }

    public void DrawCreate(EditorContext context, Action<CatalogEntity> onCreated)
    {
    }

    public CatalogEntity Create(EditorContext context, string key) =>
        throw new InvalidOperationException("dbc_groundeffecttexture is read-only reference data.");

    public Action Stage(DbContext context, IEntity entity) =>
        throw new InvalidOperationException("dbc_groundeffecttexture is read-only reference data.");

    public void StageDelete(DbContext context, IEntity entity) =>
        throw new InvalidOperationException("dbc_groundeffecttexture is read-only reference data.");

    private static GroundEffectTexture ToEntity(DbcGroundeffecttextureRecord record) => new()
    {
        GroundEffectTextureId = record.ID,
        DoodadIds = new[] { record.DoodadID1, record.DoodadID2, record.DoodadID3, record.DoodadID4 },
        DoodadWeights = new[] { record.DoodadWeight1, record.DoodadWeight2, record.DoodadWeight3, record.DoodadWeight4 },
        Density = record.Density,
        Sound = record.Sound,
    };
}
