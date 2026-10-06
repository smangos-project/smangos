using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Godot;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>Maps <see cref="WowSpellTargetPosition"/> to and from `spell_target_position`. The scan loads every row for the map once.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class WowSpellTargetPositionFactory : ISceneEntityFactory, ISpawnFactory
{
    private const string MarkerTypeId = "wow-spell-target-position-marker";
    private static readonly Color MarkerColor = new(0.75f, 0.35f, 0.85f);

    private readonly WorldStorage _storage;

    private MapId? _cachedMap;
    private List<SpellTargetPositionRecord> _cached = [];

    public WowSpellTargetPositionFactory(WorldStorage storage)
    {
        _storage = storage;
    }

    public string SpawnKind => "Spell Target Position";

    // The spell entry is supplied through the spawn key.
    public string PickerCatalogName => "";

    public Type EntityType => typeof(WowSpellTargetPosition);

    public bool Handles(IEntity entity) => entity is WowSpellTargetPosition;

    public void Configure(ModelBuilder model)
    {
    }

    public string? BridgeSource => "wow.spell-target-position";

    public long? PersistentKey(SceneEntity entity)
    {
        var target = (WowSpellTargetPosition)entity;
        return target.IsSaved && ResolveWowMap(target.Map) is int map ? PackKey(target.SpellId, map) : null;
    }

    public async Task<SceneEntityScan> ScanAsync(MapId map, Aabb region, IReadOnlySet<long> loaded, bool publishing)
    {
        if (ResolveWowMap(map) is not int mapValue)
        {
            return SceneEntityScan.Empty;
        }

        List<SpellTargetPositionRecord> rows = await RowsForAsync(map, mapValue).ConfigureAwait(false);

        var keys = rows.Select(row => PackKey((uint)row.Id, row.TargetMap)).ToList();
        List<SceneEntity> built = rows.Where(row => !loaded.Contains(PackKey((uint)row.Id, row.TargetMap)))
            .Select(row => (SceneEntity)ToEntity(row, map)).ToList();
        return new SceneEntityScan(built, keys, []);
    }

    public Action Stage(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        var target = (WowSpellTargetPosition)entity;

        if (ResolveWowMap(target.Map) is not int mapValue)
        {
            throw new InvalidOperationException(
                $"map {target.Map.Value} has no matching dbc_map row — name the editor map after a WoW map before saving its spell targets.");
        }

        if (target.IsSaved)
        {
            SpellTargetPositionRecord record = db.SpellTargetPosition.Find((int)target.SpellId, mapValue)
                ?? throw new InvalidOperationException(
                    $"spell_target_position row ({target.SpellId}, {mapValue}) was removed outside the editor.");
            WriteFields(target, record);
            return () => { };
        }

        var newRecord = new SpellTargetPositionRecord { Id = (int)target.SpellId, TargetMap = mapValue };
        WriteFields(target, newRecord);
        db.SpellTargetPosition.Add(newRecord);
        return () =>
        {
            target.IsSaved = true;
            _cachedMap = null;
        };
    }

    public void StageDelete(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        var target = (WowSpellTargetPosition)entity;
        if (target.IsSaved && ResolveWowMap(target.Map) is int mapValue)
        {
            db.SpellTargetPosition.Remove(new SpellTargetPositionRecord { Id = (int)target.SpellId, TargetMap = mapValue });
        }
    }

    /// <summary>Creates a spell target at <paramref name="transform"/> for the spell entry named by <paramref name="key"/>.</summary>
    public SceneEntity Create(EditorContext context, MapId map, Transform3D transform, string key)
    {
        if (!uint.TryParse(key, out uint spellId))
        {
            throw new InvalidOperationException($"'{key}' is not a valid spell entry.");
        }

        var target = new WowSpellTargetPosition
        {
            Map = map,
            SpellId = spellId,
            Transform = transform,
        };
        target.AddIntrinsicComponent(new WowPointMarker(MarkerTypeId, "Spell Target Marker", MarkerColor, 3.0f));

        var command = new CreateEntityCommand(context.Scene, target);
        command.Apply();
        context.EditSessions.Record(command);
        return target;
    }

    private WowSpellTargetPosition ToEntity(SpellTargetPositionRecord record, MapId map)
    {
        var target = new WowSpellTargetPosition
        {
            Map = map,
            SpellId = (uint)record.Id,
            IsSaved = true,
        };
        target.AddIntrinsicComponent(new WowPointMarker(MarkerTypeId, "Spell Target Marker", MarkerColor, 3.0f));

        Vector3 position = CreatureCoordinates.PositionToGodot(
            new Vector3(record.TargetPositionX, record.TargetPositionY, record.TargetPositionZ));
        Basis rotation = CreatureCoordinates.OrientationToGodot(record.TargetOrientation);
        target.Transform = new Transform3D(rotation, position);
        return target;
    }

    private static void WriteFields(WowSpellTargetPosition target, SpellTargetPositionRecord record)
    {
        Vector3 position = CreatureCoordinates.PositionFromGodot(target.Transform.Origin);
        record.TargetPositionX = position.X;
        record.TargetPositionY = position.Y;
        record.TargetPositionZ = position.Z;
        record.TargetOrientation = CreatureCoordinates.OrientationFromGodot(target.Transform.Basis);
    }

    private async Task<List<SpellTargetPositionRecord>> RowsForAsync(MapId map, int mapValue)
    {
        if (_cachedMap == map)
        {
            return _cached;
        }

        await using WorldDbContext db = _storage.CreateContext();
        List<SpellTargetPositionRecord> rows = await db.SpellTargetPosition.AsNoTracking()
            .Where(row => row.TargetMap == mapValue).ToListAsync().ConfigureAwait(false);

        _cachedMap = map;
        _cached = rows;
        return rows;
    }

    private static long PackKey(uint spellId, int map) => ((long)spellId << 32) | (uint)map;

    private int? ResolveWowMap(MapId editorMap) =>
        WowMapResolution.Resolve(_storage, editorMap) is int wowMap ? wowMap : null;
}
