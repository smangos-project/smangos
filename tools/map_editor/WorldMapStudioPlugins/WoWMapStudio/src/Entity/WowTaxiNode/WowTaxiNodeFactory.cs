using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Godot;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>Maps <see cref="WowTaxiNode"/> to and from `dbc_taxinodes`. The scan loads every node for the
/// map once.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class WowTaxiNodeFactory : ISceneEntityFactory, ISpawnFactory
{
    private const string MarkerTypeId = "wow-taxi-node-marker";
    private static readonly Color MarkerColor = new(0.95f, 0.55f, 0.15f);

    private readonly WorldStorage _storage;

    private readonly Dictionary<WowTaxiNode, int> _newIds = [];
    private MapId? _cachedMap;
    private List<DbcTaxinodesRecord> _cached = [];

    public WowTaxiNodeFactory(WorldStorage storage)
    {
        _storage = storage;
    }

    public string SpawnKind => "Taxi Node";

    // No catalog of origin exists for a fresh taxi node — unlike a creature spawn or a zone light, it
    // instances nothing else. SpawnMenu.Browse creates directly with no key when it finds none.
    public string PickerCatalogName => "";

    public Type EntityType => typeof(WowTaxiNode);

    public bool Handles(IEntity entity) => entity is WowTaxiNode;

    public void Configure(ModelBuilder model)
    {
    }

    public string? BridgeSource => "wow.taxi-node";

    public long? PersistentKey(SceneEntity entity)
    {
        return ((WowTaxiNode)entity).NodeId;
    }

    public async Task<SceneEntityScan> ScanAsync(MapId map, Aabb region, IReadOnlySet<long> loaded, bool publishing)
    {
        if (ResolveWowMap(map) is not int wowMap)
        {
            return SceneEntityScan.Empty;
        }

        List<DbcTaxinodesRecord> rows = await RowsForAsync(map, wowMap).ConfigureAwait(false);

        var built = new List<SceneEntity>();
        var keys = new List<long>();
        foreach (DbcTaxinodesRecord row in rows)
        {
            keys.Add(row.ID);
            if (!loaded.Contains(row.ID))
            {
                built.Add(ToEntity(row, map));
            }
        }

        return new SceneEntityScan(built, keys, []);
    }

    // `dbc_taxinodes.ID` has no autoincrement; new ids continue after the table's highest.
    public async Task PrepareBatchAsync(DbContext context, IReadOnlyList<IEntity> saves)
    {
        var fresh = saves.OfType<WowTaxiNode>().Where(node => node.NodeId is null).ToList();
        _newIds.Clear();
        if (fresh.Count == 0)
        {
            return;
        }

        var db = (WorldDbContext)context;
        int next = (await db.DbcTaxinodes.AsNoTracking().MaxAsync(row => (int?)row.ID).ConfigureAwait(false) ?? 0) + 1;
        foreach (WowTaxiNode node in fresh)
        {
            _newIds[node] = next++;
        }
    }

    public Action Stage(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        var node = (WowTaxiNode)entity;

        if (ResolveWowMap(node.Map) is not int wowMap)
        {
            throw new InvalidOperationException(
                $"map {node.Map.Value} has no matching dbc_map row — name the editor map after a WoW map before saving its taxi nodes.");
        }

        if (node.NodeId is int existingId)
        {
            DbcTaxinodesRecord record = db.DbcTaxinodes.Find(existingId)
                ?? throw new InvalidOperationException($"dbc_taxinodes row {node.NodeId} was removed outside the editor.");
            WriteRow(node, record, wowMap);
            return () => { };
        }

        if (!_newIds.TryGetValue(node, out int newId))
        {
            throw new InvalidOperationException("dbc_taxinodes id was not allocated for this batch.");
        }

        var newRecord = new DbcTaxinodesRecord { ID = newId };
        WriteRow(node, newRecord, wowMap);
        db.DbcTaxinodes.Add(newRecord);
        return () =>
        {
            node.NodeId = newId;
            _cachedMap = null;
        };
    }

    public void StageDelete(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        var node = (WowTaxiNode)entity;
        if (node.NodeId is int id)
        {
            db.DbcTaxinodes.Remove(new DbcTaxinodesRecord { ID = id });
        }
    }

    /// <summary>Creates a new taxi node named <paramref name="key"/> (may be blank — see
    /// <see cref="WowTaxiNode.DisplayName"/>) at <paramref name="transform"/>.</summary>
    public SceneEntity Create(EditorContext context, MapId map, Transform3D transform, string key)
    {
        var node = new WowTaxiNode
        {
            Map = map,
            Name = key,
            Transform = transform,
        };
        node.AddIntrinsicComponent(new WowPointMarker(MarkerTypeId, "Taxi Node Marker", MarkerColor, 3.0f));

        var command = new CreateEntityCommand(context.Scene, node);
        command.Apply();
        context.EditSessions.Record(command);
        return node;
    }

    private WowTaxiNode ToEntity(DbcTaxinodesRecord row, MapId map)
    {
        var node = new WowTaxiNode
        {
            Map = map,
            NodeId = row.ID,
            Name = row.NameLang,
            MountCreatureId1 = row.MountCreatureID1,
            MountCreatureId2 = row.MountCreatureID2,
            Flags = row.Flags,
            MapOffset1 = row.MapOffset1,
            MapOffset2 = row.MapOffset2,
        };
        node.AddIntrinsicComponent(new WowPointMarker(MarkerTypeId, "Taxi Node Marker", MarkerColor, 3.0f));

        Vector3 position = CreatureCoordinates.PositionToGodot(new Vector3(row.Pos1, row.Pos2, row.Pos3));
        node.Transform = new Transform3D(Basis.Identity, position);
        return node;
    }

    private static void WriteRow(WowTaxiNode node, DbcTaxinodesRecord record, int wowMap)
    {
        record.ContinentID = wowMap;
        record.NameLang = node.Name;
        record.MountCreatureID1 = node.MountCreatureId1;
        record.MountCreatureID2 = node.MountCreatureId2;
        record.Flags = node.Flags;
        record.MapOffset1 = node.MapOffset1;
        record.MapOffset2 = node.MapOffset2;

        Vector3 position = CreatureCoordinates.PositionFromGodot(node.Transform.Origin);
        record.Pos1 = position.X;
        record.Pos2 = position.Y;
        record.Pos3 = position.Z;
    }

    private async Task<List<DbcTaxinodesRecord>> RowsForAsync(MapId map, int wowMap)
    {
        if (_cachedMap == map)
        {
            return _cached;
        }

        await using WorldDbContext db = _storage.CreateContext();
        List<DbcTaxinodesRecord> nodes = await db.DbcTaxinodes.AsNoTracking()
            .Where(row => row.ContinentID == wowMap).ToListAsync().ConfigureAwait(false);

                _cachedMap = map;
        _cached = nodes;
        return nodes;
    }

    private int? ResolveWowMap(MapId editorMap) => WowMapResolution.Resolve(_storage, editorMap);
}
