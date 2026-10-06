using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Godot;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>Maps <see cref="WowLight"/> to `dbc_light`. The scan loads every row for the map once.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class WowLightFactory : ISceneEntityFactory, ISpawnFactory
{
    private const string MarkerTypeId = "wow-light-marker";
    private static readonly Color MarkerColor = new(1.0f, 0.95f, 0.5f);

    private readonly WorldStorage _storage;

    private readonly Dictionary<WowLight, int> _newIds = [];
    private MapId? _cachedMap;
    private List<DbcLightRecord> _cached = [];

    public WowLightFactory(WorldStorage storage)
    {
        _storage = storage;
    }

    public string SpawnKind => "Light";

    public string PickerCatalogName => "";

    public Type EntityType => typeof(WowLight);

    public bool Handles(IEntity entity) => entity is WowLight;

    public void Configure(ModelBuilder model)
    {
    }

    public string? BridgeSource => "wow.light";

    public long? PersistentKey(SceneEntity entity) => ((WowLight)entity).LightId;

    public async Task<SceneEntityScan> ScanAsync(MapId map, Aabb region, IReadOnlySet<long> loaded, bool publishing)
    {
        if (ResolveWowMap(map) is not int mapValue)
        {
            return SceneEntityScan.Empty;
        }

        List<DbcLightRecord> rows = await RowsForAsync(map, mapValue).ConfigureAwait(false);
        var keys = rows.Select(row => (long)row.ID).ToList();
        List<SceneEntity> built = rows.Where(row => !loaded.Contains(row.ID))
            .Select(row => (SceneEntity)ToEntity(row, map, _storage.Context.Catalog)).ToList();
        return new SceneEntityScan(built, keys, []);
    }

    public async Task PrepareBatchAsync(DbContext context, IReadOnlyList<IEntity> saves)
    {
        var fresh = saves.OfType<WowLight>().Where(l => l.LightId is null).ToList();
        _newIds.Clear();
        if (fresh.Count == 0)
        {
            return;
        }

        var db = (WorldDbContext)context;
        int next = (await db.DbcLight.AsNoTracking().MaxAsync(row => (int?)row.ID).ConfigureAwait(false) ?? 0) + 1;
        foreach (WowLight light in fresh)
        {
            _newIds[light] = next++;
        }
    }

    public Action Stage(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        var light = (WowLight)entity;

        if (ResolveWowMap(light.Map) is not int mapValue)
        {
            throw new InvalidOperationException(
                $"map {light.Map.Value} has no matching dbc_map row — name the editor map after a WoW map before saving its lights.");
        }

        if (light.LightId is uint existingId)
        {
            DbcLightRecord record = db.DbcLight.Find((int)existingId)
                ?? throw new InvalidOperationException($"dbc_light row {existingId} was removed outside the editor.");
            WriteFields(light, record, mapValue);
            return () => _cachedMap = null;
        }

        if (!_newIds.TryGetValue(light, out int newId))
        {
            throw new InvalidOperationException("dbc_light id was not allocated for this batch.");
        }

        var newRecord = new DbcLightRecord { ID = newId };
        WriteFields(light, newRecord, mapValue);
        db.DbcLight.Add(newRecord);
        return () =>
        {
            light.LightId = (uint)newRecord.ID;
            _cachedMap = null;
        };
    }

    public void StageDelete(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        if (((WowLight)entity).LightId is uint id)
        {
            db.DbcLight.Remove(new DbcLightRecord { ID = (int)id });
        }
    }

    public SceneEntity Create(EditorContext context, MapId map, Transform3D transform, string key)
    {
        var light = new WowLight
        {
            Map = map,
            Transform = transform,
            FalloffStart = 100.0f,
            FalloffEnd = 200.0f,
        };
        light.AddIntrinsicComponent(new WowPointMarker(MarkerTypeId, "Light Marker", MarkerColor, 3.0f));
        light.AddIntrinsicComponent(new WowLightComponent(context.Catalog));

        var command = new CreateEntityCommand(context.Scene, light);
        command.Apply();
        context.EditSessions.Record(command);
        return light;
    }

    private static WowLight ToEntity(DbcLightRecord record, MapId map, CatalogEntityRegistry catalog)
    {
        var light = new WowLight
        {
            Map = map,
            LightId = (uint)record.ID,
            FalloffStart = WowLightCoordinates.RadiusToGodot(record.GameFalloffStart),
            FalloffEnd = WowLightCoordinates.RadiusToGodot(record.GameFalloffEnd),
            LightParams = record.LightParamsID1,
            Transform = new Transform3D(Basis.Identity,
                WowLightCoordinates.PositionToGodot(new Vector3(record.GameCoords1, record.GameCoords2, record.GameCoords3))),
        };
        light.AddIntrinsicComponent(new WowPointMarker(MarkerTypeId, "Light Marker", MarkerColor, 3.0f));
        light.AddIntrinsicComponent(new WowLightComponent(catalog));
        return light;
    }

    private static void WriteFields(WowLight light, DbcLightRecord record, int mapValue)
    {
        record.ContinentID = mapValue;
        Vector3 position = WowLightCoordinates.PositionFromGodot(light.Transform.Origin);
        record.GameCoords1 = position.X;
        record.GameCoords2 = position.Y;
        record.GameCoords3 = position.Z;
        record.GameFalloffStart = WowLightCoordinates.RadiusFromGodot(light.FalloffStart);
        record.GameFalloffEnd = WowLightCoordinates.RadiusFromGodot(light.FalloffEnd);
        record.LightParamsID1 = light.LightParams;
    }

    private async Task<List<DbcLightRecord>> RowsForAsync(MapId map, int mapValue)
    {
        if (_cachedMap == map)
        {
            return _cached;
        }

        await using WorldDbContext db = _storage.CreateContext();
        List<DbcLightRecord> rows = await db.DbcLight.AsNoTracking()
            .Where(row => row.ContinentID == mapValue).ToListAsync().ConfigureAwait(false);
        _cachedMap = map;
        _cached = rows;
        return rows;
    }

    private int? ResolveWowMap(MapId editorMap) =>
        WowMapResolution.Resolve(_storage, editorMap) is int wowMap ? wowMap : null;
}
