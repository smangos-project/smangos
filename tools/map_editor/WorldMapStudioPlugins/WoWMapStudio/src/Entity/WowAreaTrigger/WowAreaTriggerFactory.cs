using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Godot;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>Maps <see cref="WowAreaTrigger"/> to `dbc_areatrigger` plus its optional `areatrigger_teleport`
/// row. The scan loads every trigger for the map once.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class WowAreaTriggerFactory : ISceneEntityFactory, ISpawnFactory
{
    private const string MarkerTypeId = "wow-areatrigger-marker";
    private static readonly Color MarkerColor = new(0.9f, 0.5f, 0.2f);

    private readonly WorldStorage _storage;

    private readonly Dictionary<WowAreaTrigger, int> _newIds = [];
    private MapId? _cachedMap;
    private List<DbcAreatriggerRecord> _cached = [];
    private Dictionary<int, AreatriggerTeleportRecord> _cachedTeleports = [];

    public WowAreaTriggerFactory(WorldStorage storage)
    {
        _storage = storage;
    }

    public string SpawnKind => "Area Trigger";

    public string PickerCatalogName => "";

    public Type EntityType => typeof(WowAreaTrigger);

    public bool Handles(IEntity entity) => entity is WowAreaTrigger;

    public void Configure(ModelBuilder model)
    {
    }

    public string? BridgeSource => "wow.areatrigger";

    public long? PersistentKey(SceneEntity entity) => ((WowAreaTrigger)entity).TriggerId;

    public async Task<SceneEntityScan> ScanAsync(MapId map, Aabb region, IReadOnlySet<long> loaded, bool publishing)
    {
        if (ResolveWowMap(map) is not int mapValue)
        {
            return SceneEntityScan.Empty;
        }

        await LoadAsync(map, mapValue).ConfigureAwait(false);

        var keys = _cached.Select(row => (long)row.ID).ToList();
        List<SceneEntity> built = _cached.Where(row => !loaded.Contains(row.ID))
            .Select(row => (SceneEntity)ToEntity(row, map)).ToList();
        return new SceneEntityScan(built, keys, []);
    }

    // `dbc_areatrigger.ID` has no autoincrement; new ids continue after the table's highest.
    public async Task PrepareBatchAsync(DbContext context, IReadOnlyList<IEntity> saves)
    {
        var fresh = saves.OfType<WowAreaTrigger>().Where(t => t.TriggerId is null).ToList();
        _newIds.Clear();
        if (fresh.Count == 0)
        {
            return;
        }

        var db = (WorldDbContext)context;
        int next = (await db.DbcAreatrigger.AsNoTracking().MaxAsync(row => (int?)row.ID).ConfigureAwait(false) ?? 0) + 1;
        foreach (WowAreaTrigger trigger in fresh)
        {
            _newIds[trigger] = next++;
        }
    }

    public Action Stage(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        var trigger = (WowAreaTrigger)entity;

        if (ResolveWowMap(trigger.Map) is not int mapValue)
        {
            throw new InvalidOperationException(
                $"map {trigger.Map.Value} has no matching dbc_map row — name the editor map after a WoW map before saving its area triggers.");
        }

        if (trigger.TriggerId is uint existingId)
        {
            DbcAreatriggerRecord record = db.DbcAreatrigger.Find((int)existingId)
                ?? throw new InvalidOperationException($"dbc_areatrigger row {existingId} was removed outside the editor.");
            WriteFields(trigger, record, mapValue);
            StageTeleport(db, (int)existingId, trigger.Teleport);
            return () => _cachedMap = null;
        }

        if (!_newIds.TryGetValue(trigger, out int newId))
        {
            throw new InvalidOperationException("dbc_areatrigger id was not allocated for this batch.");
        }

        var newRecord = new DbcAreatriggerRecord { ID = newId };
        WriteFields(trigger, newRecord, mapValue);
        db.DbcAreatrigger.Add(newRecord);
        StageTeleport(db, newId, trigger.Teleport);
        return () =>
        {
            trigger.TriggerId = (uint)newRecord.ID;
            _cachedMap = null;
        };
    }

    public void StageDelete(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        var trigger = (WowAreaTrigger)entity;
        if (trigger.TriggerId is uint id)
        {
            db.DbcAreatrigger.Remove(new DbcAreatriggerRecord { ID = (int)id });
            AreatriggerTeleportRecord? teleport = db.AreatriggerTeleport.Find((int)id);
            if (teleport is not null)
            {
                db.AreatriggerTeleport.Remove(teleport);
            }
        }
    }

    public SceneEntity Create(EditorContext context, MapId map, Transform3D transform, string key)
    {
        var trigger = new WowAreaTrigger
        {
            Map = map,
            Transform = transform,
            Radius = 5.0f,
        };
        trigger.AddIntrinsicComponent(new WowPointMarker(MarkerTypeId, "Area Trigger Marker", MarkerColor, 3.0f));

        var command = new CreateEntityCommand(context.Scene, trigger);
        command.Apply();
        context.EditSessions.Record(command);
        return trigger;
    }

    private WowAreaTrigger ToEntity(DbcAreatriggerRecord record, MapId map)
    {
        var trigger = new WowAreaTrigger
        {
            Map = map,
            TriggerId = (uint)record.ID,
            Radius = record.Radius,
            BoxLength = record.BoxLength,
            BoxWidth = record.BoxWidth,
            BoxHeight = record.BoxHeight,
            BoxYaw = record.BoxYaw,
        };
        trigger.AddIntrinsicComponent(new WowPointMarker(MarkerTypeId, "Area Trigger Marker", MarkerColor, 3.0f));

        if (_cachedTeleports.TryGetValue(record.ID, out AreatriggerTeleportRecord? teleport))
        {
            trigger.Teleport = new WowAreaTriggerTeleport
            {
                Message = teleport.Message,
                RequiredLevel = teleport.RequiredLevel,
                RequiredCondition = teleport.RequiredCondition,
                TargetMap = teleport.TargetMap,
                TargetPosition = new Vector3(teleport.TargetPositionX, teleport.TargetPositionY, teleport.TargetPositionZ),
                TargetOrientation = teleport.TargetOrientation,
            };
        }

        trigger.Transform = new Transform3D(Basis.Identity,
            CreatureCoordinates.PositionToGodot(new Vector3(record.Pos1, record.Pos2, record.Pos3)));
        return trigger;
    }

    private static void WriteFields(WowAreaTrigger trigger, DbcAreatriggerRecord record, int mapValue)
    {
        record.ContinentID = mapValue;
        Vector3 position = CreatureCoordinates.PositionFromGodot(trigger.Transform.Origin);
        record.Pos1 = position.X;
        record.Pos2 = position.Y;
        record.Pos3 = position.Z;
        record.Radius = trigger.Radius;
        record.BoxLength = trigger.BoxLength;
        record.BoxWidth = trigger.BoxWidth;
        record.BoxHeight = trigger.BoxHeight;
        record.BoxYaw = trigger.BoxYaw;
    }

    private static void StageTeleport(WorldDbContext db, int id, WowAreaTriggerTeleport? teleport)
    {
        AreatriggerTeleportRecord? existing = db.AreatriggerTeleport.Find(id);
        if (teleport is null)
        {
            if (existing is not null)
            {
                db.AreatriggerTeleport.Remove(existing);
            }

            return;
        }

        AreatriggerTeleportRecord record = existing ?? new AreatriggerTeleportRecord { Id = id };
        record.Message = teleport.Message;
        record.RequiredLevel = teleport.RequiredLevel;
        record.RequiredCondition = teleport.RequiredCondition;
        record.TargetMap = teleport.TargetMap;
        record.TargetPositionX = teleport.TargetPosition.X;
        record.TargetPositionY = teleport.TargetPosition.Y;
        record.TargetPositionZ = teleport.TargetPosition.Z;
        record.TargetOrientation = teleport.TargetOrientation;
        if (existing is null)
        {
            db.AreatriggerTeleport.Add(record);
        }
    }

    private async Task LoadAsync(MapId map, int mapValue)
    {
        if (_cachedMap == map)
        {
            return;
        }

        await using WorldDbContext db = _storage.CreateContext();
        List<DbcAreatriggerRecord> rows = await db.DbcAreatrigger.AsNoTracking()
            .Where(row => row.ContinentID == mapValue).ToListAsync().ConfigureAwait(false);
        Dictionary<int, AreatriggerTeleportRecord> teleports = await db.AreatriggerTeleport.AsNoTracking()
            .ToDictionaryAsync(row => row.Id).ConfigureAwait(false);

        _cachedMap = map;
        _cached = rows;
        _cachedTeleports = teleports;
    }

    private int? ResolveWowMap(MapId editorMap) =>
        WowMapResolution.Resolve(_storage, editorMap) is int wowMap ? wowMap : null;
}
