using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Godot;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>Maps <see cref="WowZoneLight"/> to `dbc_zonelight` and its `dbc_zonelightpoint` polygon.
/// Point positions are the server X/Y; the polygon's height comes from the entity transform.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class WowZoneLightFactory : ISceneEntityFactory, ISpawnFactory
{
    private const float DefaultHalfSize = 20.0f;

    private readonly WorldStorage _storage;

    private readonly Dictionary<WowZoneLight, int> _newIds = [];
    private int _nextPointId;
    private MapId? _cachedMap;
    private List<(DbcZonelightRecord Row, List<DbcZonelightpointRecord> Points)> _cached = [];

    public WowZoneLightFactory(WorldStorage storage)
    {
        _storage = storage;
    }

    public string SpawnKind => "Zone Light";

    public string PickerCatalogName => "";

    public Type EntityType => typeof(WowZoneLight);

    public bool Handles(IEntity entity) => entity is WowZoneLight;

    public void Configure(ModelBuilder model)
    {
    }

    public string? BridgeSource => "wow.zone-light";

    public long? PersistentKey(SceneEntity entity) => ((WowZoneLight)entity).ZoneLightId;

    public async Task<SceneEntityScan> ScanAsync(MapId map, Aabb region, IReadOnlySet<long> loaded, bool publishing)
    {
        if (WowMapResolution.Resolve(_storage, map) is not int wowMap)
        {
            return SceneEntityScan.Empty;
        }

        await LoadAsync(map, wowMap).ConfigureAwait(false);

        var keys = _cached.Select(item => (long)item.Row.ID).ToList();
        List<SceneEntity> built = _cached.Where(item => !loaded.Contains(item.Row.ID) && item.Points.Count > 0)
            .Select(item => (SceneEntity)ToEntity(item.Row, item.Points, map)).ToList();
        return new SceneEntityScan(built, keys, []);
    }

    public async Task PrepareBatchAsync(DbContext context, IReadOnlyList<IEntity> saves)
    {
        var fresh = saves.OfType<WowZoneLight>().ToList();
        _newIds.Clear();
        if (fresh.Count == 0)
        {
            return;
        }

        var db = (WorldDbContext)context;
        _nextPointId = (await db.DbcZonelightpoint.AsNoTracking().MaxAsync(row => (int?)row.ID).ConfigureAwait(false) ?? 0) + 1;
        int next = (await db.DbcZonelight.AsNoTracking().MaxAsync(row => (int?)row.ID).ConfigureAwait(false) ?? 0) + 1;
        foreach (WowZoneLight zone in fresh.Where(z => z.ZoneLightId is null))
        {
            _newIds[zone] = next++;
        }
    }

    public Action Stage(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        var zone = (WowZoneLight)entity;

        if (WowMapResolution.Resolve(_storage, zone.Map) is not int mapValue)
        {
            throw new InvalidOperationException(
                $"map {zone.Map.Value} has no matching dbc_map row — name the editor map after a WoW map before saving its zone lights.");
        }

        var component = zone.Component<WowZoneLightComponent>()!;
        if (component.OrderedNodes.Count < 3)
        {
            throw new InvalidOperationException("a zone light polygon needs at least three vertices.");
        }

        DbcZonelightRecord record;
        bool isNew = zone.ZoneLightId is null;
        if (zone.ZoneLightId is uint existingId)
        {
            record = db.DbcZonelight.Find((int)existingId)
                ?? throw new InvalidOperationException($"dbc_zonelight row {existingId} was removed outside the editor.");
            db.DbcZonelightpoint.RemoveRange(db.DbcZonelightpoint.Where(row => row.ZoneLightID == record.ID).ToList());
        }
        else
        {
            if (!_newIds.TryGetValue(zone, out int newId))
            {
                throw new InvalidOperationException("dbc_zonelight id was not allocated for this batch.");
            }

            record = new DbcZonelightRecord { ID = newId };
        }

        record.Name = zone.Name;
        record.MapID = mapValue;
        record.LightID = zone.LightId;
        record.Flags = zone.Flags;
        record.Zmin = zone.ZMin;
        record.Zmax = zone.ZMax;
        if (isNew)
        {
            db.DbcZonelight.Add(record);
        }

        int order = 0;
        foreach ((int _, Vector3 local, ZoneLightNodeData _) in component.OrderedNodes)
        {
            Vector3 server = CreatureCoordinates.PositionFromGodot(zone.Transform * local);
            db.DbcZonelightpoint.Add(new DbcZonelightpointRecord
            {
                ID = _nextPointId++,
                Pos1 = server.X,
                Pos2 = server.Y,
                PointOrder = order++,
                ZoneLightID = record.ID,
            });
        }

        return () =>
        {
            zone.ZoneLightId = (uint)record.ID;
            _cachedMap = null;
        };
    }

    public void StageDelete(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        if (((WowZoneLight)entity).ZoneLightId is uint id)
        {
            db.DbcZonelightpoint.RemoveRange(db.DbcZonelightpoint.Where(row => row.ZoneLightID == (int)id).ToList());
            db.DbcZonelight.Remove(new DbcZonelightRecord { ID = (int)id });
        }
    }

    public SceneEntity Create(EditorContext context, MapId map, Transform3D transform, string key)
    {
        var zone = new WowZoneLight { Map = map, Transform = transform, Name = key };

        var component = new WowZoneLightComponent();
        var network = new VertexNetwork();
        int a = network.AddVertex(new Vector3(-DefaultHalfSize, 0.0f, -DefaultHalfSize));
        int b = network.AddVertex(new Vector3(DefaultHalfSize, 0.0f, -DefaultHalfSize));
        int c = network.AddVertex(new Vector3(DefaultHalfSize, 0.0f, DefaultHalfSize));
        int d = network.AddVertex(new Vector3(-DefaultHalfSize, 0.0f, DefaultHalfSize));
        network.AddEdge(a, b);
        network.AddEdge(b, c);
        network.AddEdge(c, d);
        component.ReplaceNetwork(network);
        component.SetOrder(a, b);
        zone.AddIntrinsicComponent(component);

        var command = new CreateEntityCommand(context.Scene, zone);
        command.Apply();
        context.EditSessions.Record(command);
        return zone;
    }

    private static WowZoneLight ToEntity(DbcZonelightRecord row, List<DbcZonelightpointRecord> points, MapId map)
    {
        var zone = new WowZoneLight
        {
            Map = map,
            ZoneLightId = (uint)row.ID,
            Name = row.Name,
            LightId = row.LightID,
            Flags = row.Flags,
            ZMin = row.Zmin,
            ZMax = row.Zmax,
        };

        Vector3 At(DbcZonelightpointRecord p) => CreatureCoordinates.PositionToGodot(new Vector3(p.Pos1, p.Pos2, 0.0f));
        List<DbcZonelightpointRecord> ordered = points.OrderBy(p => p.PointOrder).ToList();
        Vector3 origin = At(ordered[0]);

        var component = new WowZoneLightComponent();
        var network = new VertexNetwork();
        var ids = ordered.Select(p => network.AddVertex(At(p) - origin)).ToArray();
        for (int i = 0; i + 1 < ids.Length; i++)
        {
            network.AddEdge(ids[i], ids[i + 1]);
        }

        component.ReplaceNetwork(network);
        if (ids.Length >= 2)
        {
            component.SetOrder(ids[0], ids[1]);
        }

        zone.AddIntrinsicComponent(component);
        zone.Transform = new Transform3D(Basis.Identity, origin);
        return zone;
    }

    private async Task LoadAsync(MapId map, int wowMap)
    {
        if (_cachedMap == map)
        {
            return;
        }

        await using WorldDbContext db = _storage.CreateContext();
        List<DbcZonelightRecord> rows = await db.DbcZonelight.AsNoTracking().Where(row => row.MapID == wowMap).ToListAsync().ConfigureAwait(false);
        var ids = rows.Select(row => row.ID).ToList();
        List<DbcZonelightpointRecord> points = await db.DbcZonelightpoint.AsNoTracking()
            .Where(row => ids.Contains(row.ZoneLightID)).ToListAsync().ConfigureAwait(false);
        var byZone = points.ToLookup(p => p.ZoneLightID);
        _cached = rows.Select(row => (row, byZone[row.ID].ToList())).ToList();
        _cachedMap = map;
    }
}
