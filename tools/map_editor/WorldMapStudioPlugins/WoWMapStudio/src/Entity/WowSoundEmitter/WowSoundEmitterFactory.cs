using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Godot;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>Maps <see cref="WowSoundEmitter"/> to `dbc_soundemitters`. The scan loads every row for the map once.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class WowSoundEmitterFactory : ISceneEntityFactory, ISpawnFactory
{
    private const string MarkerTypeId = "wow-soundemitter-marker";
    private static readonly Color MarkerColor = new(0.5f, 0.9f, 1.0f);

    private readonly WorldStorage _storage;

    private readonly Dictionary<WowSoundEmitter, int> _newIds = [];
    private MapId? _cachedMap;
    private List<DbcSoundemittersRecord> _cached = [];

    public WowSoundEmitterFactory(WorldStorage storage)
    {
        _storage = storage;
    }

    public string SpawnKind => "Sound Emitter";

    public string PickerCatalogName => "";

    public Type EntityType => typeof(WowSoundEmitter);

    public bool Handles(IEntity entity) => entity is WowSoundEmitter;

    public void Configure(ModelBuilder model)
    {
    }

    public string? BridgeSource => "wow.soundemitter";

    public long? PersistentKey(SceneEntity entity) => ((WowSoundEmitter)entity).EmitterId;

    public async Task<SceneEntityScan> ScanAsync(MapId map, Aabb region, IReadOnlySet<long> loaded, bool publishing)
    {
        if (ResolveWowMap(map) is not int mapValue)
        {
            return SceneEntityScan.Empty;
        }

        List<DbcSoundemittersRecord> rows = await RowsForAsync(map, mapValue).ConfigureAwait(false);
        var keys = rows.Select(row => (long)row.ID).ToList();
        List<SceneEntity> built = rows.Where(row => !loaded.Contains(row.ID))
            .Select(row => (SceneEntity)ToEntity(row, map)).ToList();
        return new SceneEntityScan(built, keys, []);
    }

    public async Task PrepareBatchAsync(DbContext context, IReadOnlyList<IEntity> saves)
    {
        var fresh = saves.OfType<WowSoundEmitter>().Where(l => l.EmitterId is null).ToList();
        _newIds.Clear();
        if (fresh.Count == 0)
        {
            return;
        }

        var db = (WorldDbContext)context;
        int next = (await db.DbcSoundemitters.AsNoTracking().MaxAsync(row => (int?)row.ID).ConfigureAwait(false) ?? 0) + 1;
        foreach (WowSoundEmitter emitter in fresh)
        {
            _newIds[emitter] = next++;
        }
    }

    public Action Stage(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        var emitter = (WowSoundEmitter)entity;

        if (ResolveWowMap(emitter.Map) is not int mapValue)
        {
            throw new InvalidOperationException(
                $"map {emitter.Map.Value} has no matching dbc_map row — name the editor map after a WoW map before saving its sound emitters.");
        }

        if (emitter.EmitterId is uint existingId)
        {
            DbcSoundemittersRecord record = db.DbcSoundemitters.Find((int)existingId)
                ?? throw new InvalidOperationException($"dbc_soundemitters row {existingId} was removed outside the editor.");
            WriteFields(emitter, record, mapValue);
            return () => _cachedMap = null;
        }

        if (!_newIds.TryGetValue(emitter, out int newId))
        {
            throw new InvalidOperationException("dbc_soundemitters id was not allocated for this batch.");
        }

        var newRecord = new DbcSoundemittersRecord { ID = newId };
        WriteFields(emitter, newRecord, mapValue);
        db.DbcSoundemitters.Add(newRecord);
        return () =>
        {
            emitter.EmitterId = (uint)newRecord.ID;
            _cachedMap = null;
        };
    }

    public void StageDelete(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        if (((WowSoundEmitter)entity).EmitterId is uint id)
        {
            db.DbcSoundemitters.Remove(new DbcSoundemittersRecord { ID = (int)id });
        }
    }

    public SceneEntity Create(EditorContext context, MapId map, Transform3D transform, string key)
    {
        var emitter = new WowSoundEmitter
        {
            Map = map,
            Transform = transform,
        };
        emitter.AddIntrinsicComponent(new WowPointMarker(MarkerTypeId, "Sound Emitter Marker", MarkerColor, 3.0f));

        var command = new CreateEntityCommand(context.Scene, emitter);
        command.Apply();
        context.EditSessions.Record(command);
        return emitter;
    }

    private static WowSoundEmitter ToEntity(DbcSoundemittersRecord record, MapId map)
    {
        var emitter = new WowSoundEmitter
        {
            Map = map,
            Name = record.Name,
            EmitterId = (uint)record.ID,
            SoundEntry = record.SoundEntriesID,
        };
        Vector3 position = CreatureCoordinates.PositionToGodot(new Vector3(record.Position1, record.Position2, record.Position3));
        Basis rotation = SoundEmitterCoordinates.DirectionToGodot(new Vector3(record.Direction1, record.Direction2, record.Direction3));
        emitter.Transform = new Transform3D(rotation, position);
        emitter.AddIntrinsicComponent(new WowPointMarker(MarkerTypeId, "Sound Emitter Marker", MarkerColor, 3.0f));
        return emitter;
    }

    private static void WriteFields(WowSoundEmitter emitter, DbcSoundemittersRecord record, int mapValue)
    {
        record.MapID = mapValue;
        record.Name = emitter.Name;
        record.SoundEntriesID = emitter.SoundEntry;
        Vector3 position = CreatureCoordinates.PositionFromGodot(emitter.Transform.Origin);
        record.Position1 = position.X;
        record.Position2 = position.Y;
        record.Position3 = position.Z;
        Vector3 direction = SoundEmitterCoordinates.DirectionFromGodot(emitter.Transform.Basis);
        record.Direction1 = direction.X;
        record.Direction2 = direction.Y;
        record.Direction3 = direction.Z;
    }

    private async Task<List<DbcSoundemittersRecord>> RowsForAsync(MapId map, int mapValue)
    {
        if (_cachedMap == map)
        {
            return _cached;
        }

        await using WorldDbContext db = _storage.CreateContext();
        List<DbcSoundemittersRecord> rows = await db.DbcSoundemitters.AsNoTracking()
            .Where(row => row.MapID == mapValue).ToListAsync().ConfigureAwait(false);
        _cachedMap = map;
        _cached = rows;
        return rows;
    }

    private int? ResolveWowMap(MapId editorMap) =>
        WowMapResolution.Resolve(_storage, editorMap) is int wowMap ? wowMap : null;
}
