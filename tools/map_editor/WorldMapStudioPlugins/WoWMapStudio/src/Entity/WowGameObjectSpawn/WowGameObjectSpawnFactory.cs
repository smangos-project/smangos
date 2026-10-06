using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Godot;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>Maps <see cref="WowGameObjectSpawn"/> to and from the vmangos `gameobject` table. Spawns draw as
/// point markers.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class WowGameObjectSpawnFactory : ISceneEntityFactory, ISpawnFactory
{
    private const string MarkerTypeId = "wow-gameobject-spawn-marker";
    private static readonly Color MarkerColor = new(0.2f, 0.7f, 0.85f);
    private const float MarkerSize = 1.5f;

    // A spawn is a point, so the scan inflates the region by one chunk instead of testing overlap.
    private const float SpawnMargin = 33.3333f;

    private readonly WorldStorage _storage;
    private readonly SpawnAppearances _appearances;
    private readonly Dictionary<WowGameObjectSpawn, int> _newGuids = [];

    public WowGameObjectSpawnFactory(WorldStorage storage)
    {
        _storage = storage;
        _appearances = new SpawnAppearances(storage);
    }

    public string SpawnKind => "Game Object";

    public string PickerCatalogName => "Game Object Template";

    public Type EntityType => typeof(WowGameObjectSpawn);

    public bool Handles(IEntity entity) => entity is WowGameObjectSpawn;

    public string? BridgeSource => "wow.gameobject";

    public long? PersistentKey(SceneEntity entity) => ((WowGameObjectSpawn)entity).Guid;

    public async Task<SceneEntityScan> ScanAsync(MapId map, Aabb region, IReadOnlySet<long> loaded, bool publishing)
    {
        if (ResolveWowMap(map) is not int mapValue)
        {
            return SceneEntityScan.Empty;
        }

        Aabb inflated = new(region.Position - (Vector3.One * SpawnMargin), region.Size + (Vector3.One * SpawnMargin * 2.0f));
        (Vector3 min, Vector3 max) = WowCoordinates.FromGodotBounds(inflated);

        await using WorldDbContext db = _storage.CreateContext();
        List<GameobjectRecord> rows = await db.Gameobject.AsNoTracking()
            .Where(record => record.Map == mapValue
                && record.PositionX >= min.X && record.PositionX <= max.X
                && record.PositionY >= min.Y && record.PositionY <= max.Y
                && record.PositionZ >= min.Z && record.PositionZ <= max.Z)
            .ToListAsync().ConfigureAwait(false);

        var keys = rows.Select(row => (long)row.Guid).ToList();
        List<SceneEntity> built = rows.Where(row => !loaded.Contains(row.Guid))
            .Select(row => (SceneEntity)ToEntity(row, map)).ToList();
        return new SceneEntityScan(built, keys, []);
    }

    // `gameobject.guid` has no autoincrement; new guids continue after the table's highest.
    public async Task PrepareBatchAsync(DbContext context, IReadOnlyList<IEntity> saves)
    {
        var fresh = saves.OfType<WowGameObjectSpawn>().Where(spawn => spawn.Guid is null).ToList();
        _newGuids.Clear();
        if (fresh.Count == 0)
        {
            return;
        }

        var db = (WorldDbContext)context;
        int next = (await db.Gameobject.AsNoTracking().MaxAsync(row => (int?)row.Guid).ConfigureAwait(false) ?? 0) + 1;
        foreach (WowGameObjectSpawn spawn in fresh)
        {
            _newGuids[spawn] = next++;
        }
    }

    public Action Stage(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        var spawn = (WowGameObjectSpawn)entity;

        if (ResolveWowMap(spawn.Map) is not int mapValue)
        {
            throw new InvalidOperationException(
                $"map {spawn.Map.Value} is not bound to a WoW map — bind it before saving its game objects.");
        }

        if (spawn.Guid is uint existing)
        {
            GameobjectRecord record = db.Gameobject.Find((int)existing)
                ?? throw new InvalidOperationException($"gameobject row {existing} was removed outside the editor.");
            WriteFields(spawn, record, mapValue);
            return () => { };
        }

        if (!_newGuids.TryGetValue(spawn, out int guid))
        {
            throw new InvalidOperationException("gameobject guid was not allocated for this batch.");
        }

        var created = new GameobjectRecord { Guid = guid };
        WriteFields(spawn, created, mapValue);
        db.Gameobject.Add(created);
        return () => spawn.Guid = (uint)guid;
    }

    public void StageDelete(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        if (((WowGameObjectSpawn)entity).Guid is uint guid)
        {
            db.Gameobject.Remove(new GameobjectRecord { Guid = (int)guid });
        }
    }

    public SceneEntity Create(EditorContext context, MapId map, Transform3D transform, string key)
    {
        if (!uint.TryParse(key, out uint templateEntry))
        {
            throw new InvalidOperationException($"'{key}' is not a valid game object template entry.");
        }

        var spawn = new WowGameObjectSpawn { Map = map, TemplateEntry = templateEntry, Transform = transform };
        spawn.AddIntrinsicComponent(NewMarker(spawn));

        var command = new CreateEntityCommand(context.Scene, spawn);
        command.Apply();
        context.EditSessions.Record(command);
        return spawn;
    }

    private WowSpawnModelComponent NewMarker(WowGameObjectSpawn spawn) => new(
        MarkerTypeId, "Game Object Spawn Marker", MarkerColor, MarkerSize,
        _storage.Context.Assets, _storage.Context.MeshMaterials,
        () => spawn.TemplateEntry, _appearances.GameObject);

    private WowGameObjectSpawn ToEntity(GameobjectRecord record, MapId map)
    {
        var spawn = new WowGameObjectSpawn
        {
            Guid = (uint)record.Guid,
            Map = map,
            TemplateEntry = (uint)record.Id,
            SpawnTimeMin = (uint)record.Spawntimesecsmin,
            SpawnTimeMax = (uint)record.Spawntimesecsmax,
        };
        spawn.AddIntrinsicComponent(NewMarker(spawn));

        Vector3 position = GameObjectCoordinates.PositionToGodot(new Vector3(record.PositionX, record.PositionY, record.PositionZ));
        var rotation = new Quaternion(record.Rotation0, record.Rotation1, record.Rotation2, record.Rotation3);
        Basis basis = rotation.LengthSquared() > 0.0f ? GameObjectCoordinates.RotationToGodot(rotation.Normalized()) : new Basis(Vector3.Up, record.Orientation);
        spawn.Transform = new Transform3D(basis, position);
        return spawn;
    }

    private static void WriteFields(WowGameObjectSpawn spawn, GameobjectRecord record, int mapValue)
    {
        record.Id = (int)spawn.TemplateEntry;
        record.Map = mapValue;
        record.Spawntimesecsmin = (int)spawn.SpawnTimeMin;
        record.Spawntimesecsmax = (int)spawn.SpawnTimeMax;

        Vector3 position = GameObjectCoordinates.PositionFromGodot(spawn.Transform.Origin);
        record.PositionX = position.X;
        record.PositionY = position.Y;
        record.PositionZ = position.Z;
        record.Orientation = GameObjectCoordinates.OrientationFromGodot(spawn.Transform.Basis);
        Quaternion rotation = GameObjectCoordinates.RotationFromGodot(spawn.Transform.Basis);
        record.Rotation0 = rotation.X;
        record.Rotation1 = rotation.Y;
        record.Rotation2 = rotation.Z;
        record.Rotation3 = rotation.W;
    }

    private int? ResolveWowMap(MapId editorMap) => WowMapResolution.Resolve(_storage, editorMap);
}
