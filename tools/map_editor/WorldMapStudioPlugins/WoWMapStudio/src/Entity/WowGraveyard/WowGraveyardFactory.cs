using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Godot;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>Maps <see cref="WowGraveyard"/> to and from `world_safe_locs`. The scan loads every row for the
/// map once.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class WowGraveyardFactory : ISceneEntityFactory, ISpawnFactory
{
    private const string MarkerTypeId = "wow-graveyard-marker";
    private static readonly Color MarkerColor = new(0.6f, 0.6f, 0.9f);

    private readonly WorldStorage _storage;

    private readonly Dictionary<WowGraveyard, int> _newIds = [];
    private MapId? _cachedMap;
    private List<WorldSafeLocsRecord> _cached = [];
    private Dictionary<int, List<WowGraveyardZone>> _cachedZones = [];

    public WowGraveyardFactory(WorldStorage storage)
    {
        _storage = storage;
    }

    public string SpawnKind => "Graveyard";

    // No catalog of origin exists for a fresh safe location — it instances nothing else, and there is no
    // spell-entry-style external identity to type in either (unlike WowSpellTargetPositionFactory).
    public string PickerCatalogName => "";

    public Type EntityType => typeof(WowGraveyard);

    public bool Handles(IEntity entity) => entity is WowGraveyard;

    public void Configure(ModelBuilder model)
    {
    }

    public string? BridgeSource => "wow.graveyard";

    public long? PersistentKey(SceneEntity entity) => ((WowGraveyard)entity).LocId;

    public async Task<SceneEntityScan> ScanAsync(MapId map, Aabb region, IReadOnlySet<long> loaded, bool publishing)
    {
        if (ResolveWowMap(map) is not int mapValue)
        {
            return SceneEntityScan.Empty;
        }

        List<WorldSafeLocsRecord> rows = await RowsForAsync(map, mapValue).ConfigureAwait(false);

        var keys = rows.Select(row => (long)row.Id).ToList();
        List<SceneEntity> built = rows.Where(row => !loaded.Contains(row.Id))
            .Select(row => (SceneEntity)ToEntity(row, map)).ToList();
        return new SceneEntityScan(built, keys, []);
    }

    // `world_safe_locs.id` has no autoincrement; new ids continue after the table's highest.
    public async Task PrepareBatchAsync(DbContext context, IReadOnlyList<IEntity> saves)
    {
        var fresh = saves.OfType<WowGraveyard>().Where(loc => loc.LocId is null).ToList();
        _newIds.Clear();
        if (fresh.Count == 0)
        {
            return;
        }

        var db = (WorldDbContext)context;
        int next = (await db.WorldSafeLocs.AsNoTracking().MaxAsync(row => (int?)row.Id).ConfigureAwait(false) ?? 0) + 1;
        foreach (WowGraveyard loc in fresh)
        {
            _newIds[loc] = next++;
        }
    }

    public Action Stage(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        var loc = (WowGraveyard)entity;

        if (ResolveWowMap(loc.Map) is not int mapValue)
        {
            throw new InvalidOperationException(
                $"map {loc.Map.Value} has no matching dbc_map row — name the editor map after a WoW map before saving its safe locations.");
        }

        if (loc.LocId is uint existingId)
        {
            WorldSafeLocsRecord record = db.WorldSafeLocs.Find((int)existingId)
                ?? throw new InvalidOperationException($"world_safe_locs row {existingId} was removed outside the editor.");
            WriteFields(loc, record, mapValue);
            StageZones(db, record.Id, loc.Zones);
            return () => _cachedMap = null;
        }

        if (!_newIds.TryGetValue(loc, out int newId))
        {
            throw new InvalidOperationException("world_safe_locs id was not allocated for this batch.");
        }

        var newRecord = new WorldSafeLocsRecord { Id = newId };
        WriteFields(loc, newRecord, mapValue);
        db.WorldSafeLocs.Add(newRecord);
        StageZones(db, newId, loc.Zones);
        return () =>
        {
            loc.LocId = (uint)newRecord.Id;
            _cachedMap = null;
        };
    }

    public void StageDelete(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        var loc = (WowGraveyard)entity;
        if (loc.LocId is uint id)
        {
            db.WorldSafeLocs.Remove(new WorldSafeLocsRecord { Id = (int)id });
            db.GameGraveyardZone.RemoveRange(db.GameGraveyardZone.Where(row => row.Id == (int)id).ToList());
        }
    }

    /// <summary>Creates a new safe location at <paramref name="transform"/>; <paramref name="key"/> is unused.</summary>
    public SceneEntity Create(EditorContext context, MapId map, Transform3D transform, string key)
    {
        var loc = new WowGraveyard
        {
            Map = map,
            Transform = transform,
        };
        loc.AddIntrinsicComponent(new WowPointMarker(MarkerTypeId, "Graveyard Marker", MarkerColor, 3.0f));

        var command = new CreateEntityCommand(context.Scene, loc);
        command.Apply();
        context.EditSessions.Record(command);
        return loc;
    }

    private WowGraveyard ToEntity(WorldSafeLocsRecord record, MapId map)
    {
        var loc = new WowGraveyard
        {
            Map = map,
            LocId = (uint)record.Id,
            Zones = _cachedZones.TryGetValue(record.Id, out List<WowGraveyardZone>? zones) ? zones : [],
        };
        loc.AddIntrinsicComponent(new WowPointMarker(MarkerTypeId, "Graveyard Marker", MarkerColor, 3.0f));

        Vector3 position = CreatureCoordinates.PositionToGodot(new Vector3(record.X, record.Y, record.Z));
        Basis rotation = CreatureCoordinates.OrientationToGodot(record.Orientation);
        loc.Transform = new Transform3D(rotation, position);
        return loc;
    }

    private static void StageZones(WorldDbContext db, int id, List<WowGraveyardZone> zones)
    {
        var wanted = zones.GroupBy(z => z.GhostZone).ToDictionary(g => g.Key, g => g.First().Faction);
        foreach (GameGraveyardZoneRecord row in db.GameGraveyardZone.Where(row => row.Id == id).ToList())
        {
            if (wanted.Remove(row.GhostZone, out int faction))
            {
                row.Faction = faction;
            }
            else
            {
                db.GameGraveyardZone.Remove(row);
            }
        }

        foreach ((int ghostZone, int faction) in wanted)
        {
            db.GameGraveyardZone.Add(new GameGraveyardZoneRecord { Id = id, GhostZone = ghostZone, Faction = faction });
        }
    }

    private static void WriteFields(WowGraveyard loc, WorldSafeLocsRecord record, int mapValue)
    {
        record.MapId = mapValue;

        Vector3 position = CreatureCoordinates.PositionFromGodot(loc.Transform.Origin);
        record.X = position.X;
        record.Y = position.Y;
        record.Z = position.Z;
        record.Orientation = CreatureCoordinates.OrientationFromGodot(loc.Transform.Basis);
    }

    private async Task<List<WorldSafeLocsRecord>> RowsForAsync(MapId map, int mapValue)
    {
        if (_cachedMap == map)
        {
            return _cached;
        }

        await using WorldDbContext db = _storage.CreateContext();
        List<WorldSafeLocsRecord> rows = await db.WorldSafeLocs.AsNoTracking()
            .Where(row => row.MapId == mapValue).ToListAsync().ConfigureAwait(false);

        var ids = rows.Select(row => row.Id).ToList();
        List<GameGraveyardZoneRecord> links = await db.GameGraveyardZone.AsNoTracking()
            .Where(row => ids.Contains(row.Id)).ToListAsync().ConfigureAwait(false);

        _cachedZones = links.GroupBy(row => row.Id)
            .ToDictionary(group => group.Key, group => group.Select(row => new WowGraveyardZone(row.GhostZone, row.Faction)).ToList());
        _cachedMap = map;
        _cached = rows;
        return rows;
    }

    private int? ResolveWowMap(MapId editorMap) =>
        WowMapResolution.Resolve(_storage, editorMap) is int wowMap ? wowMap : null;
}
