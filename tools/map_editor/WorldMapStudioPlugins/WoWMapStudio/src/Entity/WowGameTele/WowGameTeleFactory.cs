using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Godot;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>Maps <see cref="WowGameTele"/> to and from `game_tele`. The scan loads every row for the map once.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class WowGameTeleFactory : ISceneEntityFactory, ISpawnFactory
{
    private const string MarkerTypeId = "wow-game-tele-marker";
    private static readonly Color MarkerColor = new(0.9f, 0.75f, 0.2f);

    private readonly WorldStorage _storage;

    private readonly Dictionary<WowGameTele, int> _newIds = [];
    private MapId? _cachedMap;
    private List<GameTeleRecord> _cached = [];

    public WowGameTeleFactory(WorldStorage storage)
    {
        _storage = storage;
    }

    public string SpawnKind => "Game Tele";

    // No catalog of origin exists for a fresh teleport — it instances nothing else, and there is no
    // spell-entry-style external identity to type in either (unlike WowSpellTargetPositionFactory).
    public string PickerCatalogName => "";

    public Type EntityType => typeof(WowGameTele);

    public bool Handles(IEntity entity) => entity is WowGameTele;

    public void Configure(ModelBuilder model)
    {
    }

    public string? BridgeSource => "wow.game-tele";

    public long? PersistentKey(SceneEntity entity) => ((WowGameTele)entity).TeleId;

    public async Task<SceneEntityScan> ScanAsync(MapId map, Aabb region, IReadOnlySet<long> loaded, bool publishing)
    {
        if (ResolveWowMap(map) is not int mapValue)
        {
            return SceneEntityScan.Empty;
        }

        List<GameTeleRecord> rows = await RowsForAsync(map, mapValue).ConfigureAwait(false);

        var keys = rows.Select(row => (long)row.Id).ToList();
        List<SceneEntity> built = rows.Where(row => !loaded.Contains(row.Id))
            .Select(row => (SceneEntity)ToEntity(row, map)).ToList();
        return new SceneEntityScan(built, keys, []);
    }

    // `game_tele.id` has no autoincrement; new ids continue after the table's highest.
    public async Task PrepareBatchAsync(DbContext context, IReadOnlyList<IEntity> saves)
    {
        var fresh = saves.OfType<WowGameTele>().Where(tele => tele.TeleId is null).ToList();
        _newIds.Clear();
        if (fresh.Count == 0)
        {
            return;
        }

        var db = (WorldDbContext)context;
        int next = (await db.GameTele.AsNoTracking().MaxAsync(row => (int?)row.Id).ConfigureAwait(false) ?? 0) + 1;
        foreach (WowGameTele tele in fresh)
        {
            _newIds[tele] = next++;
        }
    }

    public Action Stage(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        var tele = (WowGameTele)entity;

        if (ResolveWowMap(tele.Map) is not int mapValue)
        {
            throw new InvalidOperationException(
                $"map {tele.Map.Value} has no matching dbc_map row — name the editor map after a WoW map before saving its teleports.");
        }

        if (tele.TeleId is uint existingId)
        {
            GameTeleRecord record = db.GameTele.Find((int)existingId)
                ?? throw new InvalidOperationException($"game_tele row {existingId} was removed outside the editor.");
            WriteFields(tele, record, mapValue);
            return () => { };
        }

        if (!_newIds.TryGetValue(tele, out int newId))
        {
            throw new InvalidOperationException("game_tele id was not allocated for this batch.");
        }

        var newRecord = new GameTeleRecord { Id = newId };
        WriteFields(tele, newRecord, mapValue);
        db.GameTele.Add(newRecord);
        return () =>
        {
            tele.TeleId = (uint)newRecord.Id;
            _cachedMap = null;
        };
    }

    public void StageDelete(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        var tele = (WowGameTele)entity;
        if (tele.TeleId is uint id)
        {
            db.GameTele.Remove(new GameTeleRecord { Id = (int)id });
        }
    }

    /// <summary>Creates a new teleport named <paramref name="key"/> (may be blank — see
    /// <see cref="WowGameTele.DisplayName"/>) at <paramref name="transform"/>.</summary>
    public SceneEntity Create(EditorContext context, MapId map, Transform3D transform, string key)
    {
        var tele = new WowGameTele
        {
            Map = map,
            Name = key,
            Transform = transform,
        };
        tele.AddIntrinsicComponent(new WowPointMarker(MarkerTypeId, "Game Tele Marker", MarkerColor, 3.0f));

        var command = new CreateEntityCommand(context.Scene, tele);
        command.Apply();
        context.EditSessions.Record(command);
        return tele;
    }

    private WowGameTele ToEntity(GameTeleRecord record, MapId map)
    {
        var tele = new WowGameTele
        {
            Map = map,
            TeleId = (uint)record.Id,
            Name = record.Name,
        };
        tele.AddIntrinsicComponent(new WowPointMarker(MarkerTypeId, "Game Tele Marker", MarkerColor, 3.0f));

        Vector3 position = CreatureCoordinates.PositionToGodot(new Vector3(record.PositionX, record.PositionY, record.PositionZ));
        Basis rotation = CreatureCoordinates.OrientationToGodot(record.Orientation);
        tele.Transform = new Transform3D(rotation, position);
        return tele;
    }

    private static void WriteFields(WowGameTele tele, GameTeleRecord record, int mapValue)
    {
        record.Map = mapValue;
        record.Name = tele.Name;

        Vector3 position = CreatureCoordinates.PositionFromGodot(tele.Transform.Origin);
        record.PositionX = position.X;
        record.PositionY = position.Y;
        record.PositionZ = position.Z;
        record.Orientation = CreatureCoordinates.OrientationFromGodot(tele.Transform.Basis);
    }

    private async Task<List<GameTeleRecord>> RowsForAsync(MapId map, int mapValue)
    {
        if (_cachedMap == map)
        {
            return _cached;
        }

        await using WorldDbContext db = _storage.CreateContext();
        List<GameTeleRecord> rows = await db.GameTele.AsNoTracking()
            .Where(row => row.Map == mapValue).ToListAsync().ConfigureAwait(false);

        _cachedMap = map;
        _cached = rows;
        return rows;
    }

    private int? ResolveWowMap(MapId editorMap) =>
        WowMapResolution.Resolve(_storage, editorMap) is int wowMap ? wowMap : null;
}
