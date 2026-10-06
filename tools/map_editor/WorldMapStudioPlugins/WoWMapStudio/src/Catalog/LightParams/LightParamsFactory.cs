using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Godot;
using ImGuiNET;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>Maps <see cref="LightParams"/> to `dbc_lightparams` plus its `dbc_lightdata` rows. Read-only catalog.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class LightParamsFactory : ICatalogEntityFactory, ICatalogBrowser
{
    private readonly WorldStorage _storage;

    public LightParamsFactory(WorldStorage storage)
    {
        _storage = storage;
    }

    public Type EntityType => typeof(LightParams);

    public string CatalogName => "Light Params";

    public bool Handles(IEntity entity) => entity is LightParams;

    public void Configure(ModelBuilder model)
    {
    }

    public async Task<IReadOnlyList<CatalogEntity>> LoadAllAsync()
    {
        await using WorldDbContext db = _storage.CreateContext();
        List<DbcLightparamsRecord> rows = await db.DbcLightparams.AsNoTracking().ToListAsync().ConfigureAwait(false);
        ILookup<int, DbcLightdataRecord> data = (await db.DbcLightdata.AsNoTracking().ToListAsync().ConfigureAwait(false))
            .ToLookup(row => row.LightParamID);
        return rows.Select(row => (CatalogEntity)ToEntity(row, data[row.ID])).ToList();
    }

    public async Task<int> MaxRecordIdAsync()
    {
        await using WorldDbContext db = _storage.CreateContext();
        return await db.DbcLightparams.MaxAsync(row => (int?)row.ID).ConfigureAwait(false) ?? 0;
    }

    public async Task<IReadOnlyList<CatalogSearchResult>> SearchAsync(string filter)
    {
        await using WorldDbContext db = _storage.CreateContext();
        IQueryable<DbcLightparamsRecord> query = db.DbcLightparams.AsNoTracking();
        if (int.TryParse(filter, out int id))
        {
            query = query.Where(row => row.ID == id);
        }

        List<DbcLightparamsRecord> rows = await query.OrderBy(row => row.ID).ToListAsync().ConfigureAwait(false);
        return rows.Select(row => new CatalogSearchResult(row.ID.ToString(), $"Light Params {row.ID}")).ToList();
    }

    public Task<IReadOnlyDictionary<string, string>> DescribeAsync(EditorContext context, IReadOnlyCollection<string> keys)
    {
        var result = new Dictionary<string, string>();
        foreach (string key in keys)
        {
            if (int.TryParse(key, out int id) && context.Catalog.OfType<LightParams>().Any(e => e.ParamsId == id))
            {
                result[key] = $"Light Params {id}";
            }
        }

        return Task.FromResult<IReadOnlyDictionary<string, string>>(result);
    }

    public Task<CatalogEntity?> OpenAsync(EditorContext context, string key) =>
        Task.FromResult<CatalogEntity?>(int.TryParse(key, out int id)
            ? context.Catalog.OfType<LightParams>().FirstOrDefault(e => e.ParamsId == id)
            : null);

    public void DrawFields(EditorContext context, CatalogEntity entity, FieldEditTracker tracker,
        Action<string, string> navigate, string fieldFilter)
    {
        var typed = (LightParams)entity;
        ImGui.TextDisabled($"Id: {typed.ParamsId}");
        ImGui.TextDisabled($"Skybox: {typed.LightSkyboxId?.ToString() ?? "(none)"}");
        ImGui.TextDisabled($"Keys: {typed.ColorBand(WowSkyColorChannel.SkyColorTop).Keys.Count}");
    }

    public void DrawCreate(EditorContext context, Action<CatalogEntity> onCreated)
    {
    }

    public CatalogEntity Create(EditorContext context, string key) =>
        throw new InvalidOperationException("Light params are read-only.");

    public Action Stage(DbContext context, IEntity entity) =>
        throw new InvalidOperationException("Light params are read-only.");

    public void StageDelete(DbContext context, IEntity entity)
    {
    }

    private static LightParams ToEntity(DbcLightparamsRecord row, IEnumerable<DbcLightdataRecord> data)
    {
        var p = new LightParams
        {
            ParamsId = row.ID,
            IsSaved = true,
            Glow = row.Glow,
            RiverShallowAlpha = row.WaterShallowAlpha,
            RiverDeepAlpha = row.WaterDeepAlpha,
            OceanShallowAlpha = row.OceanShallowAlpha,
            OceanDeepAlpha = row.OceanDeepAlpha,
            LightSkyboxId = row.LightSkyboxID > 0 ? row.LightSkyboxID : null,
        };

        foreach (DbcLightdataRecord key in data.OrderBy(d => d.Time))
        {
            AddColor(p, WowSkyColorChannel.LightGlobalDiffuse, key.Time, key.DirectColor);
            AddColor(p, WowSkyColorChannel.LightGlobalAmbient, key.Time, key.AmbientColor);
            AddColor(p, WowSkyColorChannel.SkyColorTop, key.Time, key.SkyTopColor);
            AddColor(p, WowSkyColorChannel.SkyColorMiddle, key.Time, key.SkyMiddleColor);
            AddColor(p, WowSkyColorChannel.SkyColorBand1, key.Time, key.SkyBand1Color);
            AddColor(p, WowSkyColorChannel.SkyColorBand2, key.Time, key.SkyBand2Color);
            AddColor(p, WowSkyColorChannel.SkyColorSmog, key.Time, key.SkySmogColor);
            AddColor(p, WowSkyColorChannel.SkyFogColor, key.Time, key.SkyFogColor);
            AddColor(p, WowSkyColorChannel.SunColor, key.Time, key.SunColor);
            AddColor(p, WowSkyColorChannel.SunCloudColor, key.Time, key.CloudSunColor);
            AddColor(p, WowSkyColorChannel.CloudEmissiveColor, key.Time, key.CloudEmissiveColor);
            AddColor(p, WowSkyColorChannel.CloudLayer1AmbientColor, key.Time, key.CloudLayer1AmbientColor);
            AddColor(p, WowSkyColorChannel.CloudLayer2AmbientColor, key.Time, key.CloudLayer2AmbientColor);
            AddColor(p, WowSkyColorChannel.OceanColorLight, key.Time, key.OceanCloseColor);
            AddColor(p, WowSkyColorChannel.OceanColorDark, key.Time, key.OceanFarColor);
            AddColor(p, WowSkyColorChannel.RiverColorLight, key.Time, key.RiverCloseColor);
            AddColor(p, WowSkyColorChannel.RiverColorDark, key.Time, key.RiverFarColor);
            p.FloatBand(WowSkyFloatChannel.FogDistance).Add(key.Time, key.FogEnd);
            p.FloatBand(WowSkyFloatChannel.FogMultiplier).Add(key.Time, key.FogScaler);
            p.FloatBand(WowSkyFloatChannel.CloudDensity).Add(key.Time, key.CloudDensity);
        }

        return p;
    }

    private static void AddColor(LightParams p, WowSkyColorChannel channel, int time, int packed) =>
        p.ColorBand(channel).Add(time, new Color(
            ((packed >> 16) & 0xFF) / 255.0f, ((packed >> 8) & 0xFF) / 255.0f, (packed & 0xFF) / 255.0f));
}
