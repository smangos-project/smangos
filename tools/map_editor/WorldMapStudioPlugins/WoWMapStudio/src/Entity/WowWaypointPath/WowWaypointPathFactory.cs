using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Godot;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>Maps <see cref="WowWaypointPath"/> to and from `creature_movement`. A path belongs to the
/// map of the creature whose guid it is keyed by.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class WowWaypointPathFactory : ISceneEntityFactory, ISpawnFactory
{
    private const float DefaultHalfLength = 5.0f;

    private readonly WorldStorage _storage;

    private MapId? _cachedMap;
    private List<(int Guid, List<CreatureMovementRecord> Nodes)> _cached = [];

    public WowWaypointPathFactory(WorldStorage storage)
    {
        _storage = storage;
    }

    public string SpawnKind => "Waypoint Path";

    public string PickerCatalogName => "";

    public Type EntityType => typeof(WowWaypointPath);

    public bool Handles(IEntity entity) => entity is WowWaypointPath;

    public void Configure(ModelBuilder model)
    {
    }

    public string? BridgeSource => "wow.waypoint-path";

    public long? PersistentKey(SceneEntity entity) => ((WowWaypointPath)entity).IsSaved ? ((WowWaypointPath)entity).CreatureGuid : null;

    public async Task<SceneEntityScan> ScanAsync(MapId map, Aabb region, IReadOnlySet<long> loaded, bool publishing)
    {
        if (WowMapResolution.Resolve(_storage, map) is not int wowMap)
        {
            return SceneEntityScan.Empty;
        }

        var paths = await PathsForAsync(map, wowMap).ConfigureAwait(false);
        var keys = paths.Select(path => (long)path.Guid).ToList();
        List<SceneEntity> built = paths.Where(path => !loaded.Contains(path.Guid))
            .Select(path => (SceneEntity)ToEntity(path.Guid, path.Nodes, map)).ToList();
        return new SceneEntityScan(built, keys, []);
    }

    public Action Stage(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        var path = (WowWaypointPath)entity;
        if (path.CreatureGuid is not uint guid || guid == 0)
        {
            throw new InvalidOperationException("a waypoint path needs the guid of the creature it belongs to before saving.");
        }

        Dictionary<int, CreatureMovementRecord> existing = db.CreatureMovement.Where(row => row.Id == (int)guid)
            .ToList().ToDictionary(row => row.Point);

        var component = path.Component<WowWaypointPathComponent>()!;
        int point = 1;
        foreach ((int _, Vector3 local, WaypointNodeData data) in component.OrderedNodes)
        {
            Vector3 server = CreatureCoordinates.PositionFromGodot(path.Transform * local);
            if (existing.Remove(point, out CreatureMovementRecord? row))
            {
                Fill(row, server, data);
            }
            else
            {
                row = new CreatureMovementRecord { Id = (int)guid, Point = point };
                Fill(row, server, data);
                db.CreatureMovement.Add(row);
            }

            point++;
        }

        db.CreatureMovement.RemoveRange(existing.Values);

        return () =>
        {
            path.IsSaved = true;
            _cachedMap = null;
        };
    }

    private static void Fill(CreatureMovementRecord row, Vector3 server, WaypointNodeData data)
    {
        row.PositionX = server.X;
        row.PositionY = server.Y;
        row.PositionZ = server.Z;
        row.Orientation = data.Orientation;
        row.Waittime = data.WaitTime;
        row.WanderDistance = data.WanderDistance;
        row.ScriptId = data.ScriptId;
    }

    public void StageDelete(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        if (((WowWaypointPath)entity) is { IsSaved: true, CreatureGuid: uint guid })
        {
            db.CreatureMovement.RemoveRange(db.CreatureMovement.Where(row => row.Id == (int)guid).ToList());
        }
    }

    /// <summary>Creates a two-node path stub; <paramref name="key"/>, when it parses as a creature guid,
    /// names the creature the path belongs to.</summary>
    public SceneEntity Create(EditorContext context, MapId map, Transform3D transform, string key)
    {
        var path = new WowWaypointPath
        {
            Map = map,
            CreatureGuid = uint.TryParse(key, out uint guid) ? guid : null,
            Transform = transform,
        };

        var component = new WowWaypointPathComponent();
        var network = new VertexNetwork();
        int a = network.AddVertex(new Vector3(-DefaultHalfLength, 0.0f, 0.0f));
        int b = network.AddVertex(new Vector3(DefaultHalfLength, 0.0f, 0.0f));
        network.AddEdge(a, b);
        component.ReplaceNetwork(network);
        component.SetOrder(a, b);
        path.AddIntrinsicComponent(component);

        var command = new CreateEntityCommand(context.Scene, path);
        command.Apply();
        context.EditSessions.Record(command);
        return path;
    }

    private static WowWaypointPath ToEntity(int guid, List<CreatureMovementRecord> nodes, MapId map)
    {
        var path = new WowWaypointPath { Map = map, CreatureGuid = (uint)guid, IsSaved = true };
        Vector3 First(CreatureMovementRecord n) => CreatureCoordinates.PositionToGodot(new Vector3(n.PositionX, n.PositionY, n.PositionZ));
        Vector3 origin = First(nodes[0]);

        var component = new WowWaypointPathComponent();
        var network = new VertexNetwork();
        var ids = new int[nodes.Count];
        for (int i = 0; i < nodes.Count; i++)
        {
            ids[i] = network.AddVertex(First(nodes[i]) - origin);
        }

        for (int i = 0; i + 1 < ids.Length; i++)
        {
            network.AddEdge(ids[i], ids[i + 1]);
        }

        component.ReplaceNetwork(network);
        for (int i = 0; i < nodes.Count; i++)
        {
            component.SetNodeData(ids[i], new WaypointNodeData(nodes[i].Orientation, nodes[i].Waittime, nodes[i].WanderDistance, nodes[i].ScriptId));
        }

        if (ids.Length >= 2)
        {
            component.SetOrder(ids[0], ids[1]);
        }

        path.AddIntrinsicComponent(component);
        path.Transform = new Transform3D(Basis.Identity, origin);
        return path;
    }

    private async Task<List<(int Guid, List<CreatureMovementRecord> Nodes)>> PathsForAsync(MapId map, int wowMap)
    {
        if (_cachedMap == map)
        {
            return _cached;
        }

        await using WorldDbContext db = _storage.CreateContext();
        var guids = db.Creature.AsNoTracking().Where(row => row.Map == wowMap).Select(row => row.Guid);
        List<CreatureMovementRecord> rows = await db.CreatureMovement.AsNoTracking()
            .Where(row => guids.Contains(row.Id)).ToListAsync().ConfigureAwait(false);

        var paths = rows.GroupBy(row => row.Id)
            .Select(group => (group.Key, group.OrderBy(row => row.Point).ToList())).ToList();
        _cachedMap = map;
        _cached = paths;
        return paths;
    }
}
