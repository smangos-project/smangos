using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Godot;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>
/// Maps <see cref="WowTaxiPath"/> to and from `dbc_taxipath` plus its `dbc_taxipathnode` children,
/// splitting a path that crosses continents into one entity per contiguous continent run.
///
/// Loads and groups every path's nodes once, globally (not scoped to the open map — grouping into
/// runs needs a path's whole ordered node list regardless of which continent is open, and the whole
/// table is cheap: ~630 ms for all 38k nodes), then serves whichever runs match the requested map from
/// that cache. Invalidated in <see cref="Stage"/>, the same trigger <c>WowZoneLightFactory</c> uses.
/// </summary>
[Subsystem(nameof(WorldStorage))]
public sealed class WowTaxiPathFactory : ISceneEntityFactory, ISpawnFactory
{
    private const float DefaultHalfLength = 50.0f;

    private readonly WorldStorage _storage;

    private List<Loaded>? _allRuns;

    // Batch-scoped state, set by PrepareBatchAsync and consumed by Stage — see that method's own doc
    // for why a split path's node rows must be written in one pass per commit rather than once per run.
    private Dictionary<int, List<DbcTaxipathnodeRecord>> _existingNodesByPath = new();
    private Dictionary<int, Dictionary<int, WowTaxiPath>> _runsInCommitByPath = new();
    private HashSet<int> _pathsHandledThisCommit = [];
    private int _nextNodeId;

    public WowTaxiPathFactory(WorldStorage storage)
    {
        _storage = storage;
    }

    public string SpawnKind => "Taxi Path";

    public string PickerCatalogName => "";

    public Type EntityType => typeof(WowTaxiPath);

    public bool Handles(IEntity entity) => entity is WowTaxiPath;

    public void Configure(ModelBuilder model)
    {
    }

    // Not bridged (BridgeSource stays null): the run index is positional — it counts the runs a path's
    // nodes split into, so editing the nodes can renumber it — and tags or components attached under it
    // would land on whichever run holds that number next.
    public long? PersistentKey(SceneEntity entity)
    {
        var path = (WowTaxiPath)entity;
        return path.IsSaved ? PersistentKeyOf(path.PathId, path.RunIndex) : null;
    }

    public async Task<SceneEntityScan> ScanAsync(MapId map, Aabb region, IReadOnlySet<long> loaded, bool publishing)
    {
        if (ResolveWowMap(map) is not int wowMap)
        {
            return SceneEntityScan.Empty;
        }

        List<Loaded> runs = await AllRunsAsync().ConfigureAwait(false);

        var built = new List<SceneEntity>();
        var keys = new List<long>();
        foreach (Loaded run in runs)
        {
            if (run.ContinentId != wowMap)
            {
                continue;
            }

            long key = PersistentKeyOf(run.Path.ID, run.RunIndex);
            keys.Add(key);
            if (!loaded.Contains(key))
            {
                built.Add(ToEntity(run, map));
            }
        }

        return new SceneEntityScan(built, keys, []);
    }

    /// <summary>
    /// Loads every path affected by this commit's full existing node set — not just the runs being
    /// staged — so <see cref="Stage"/> can rewrite a split path's rows in one pass. A run whose entity
    /// isn't part of this commit (its continent wasn't even open) must still round-trip untouched, and
    /// that's only possible if its rows are in hand before any of the path's other runs are staged.
    /// Also hands out one batch-wide node id high-water mark, the same convention
    /// the vmangos id convention uses, so staging more than one path in a commit can't collide.
    /// </summary>
    public async Task PrepareBatchAsync(DbContext context, IReadOnlyList<IEntity> saves)
    {
        var db = (WorldDbContext)context;
        List<WowTaxiPath> paths = saves.OfType<WowTaxiPath>().ToList();

        // `dbc_taxipath.ID` has no autoincrement; new paths continue after the table's highest id.
        int nextPathId = (await db.DbcTaxipath.AsNoTracking().MaxAsync(row => (int?)row.ID).ConfigureAwait(false) ?? 0) + 1;
        foreach (WowTaxiPath unsaved in paths.Where(path => !path.IsSaved))
        {
            unsaved.PathId = nextPathId++;
        }

        _pathsHandledThisCommit = [];

        if (paths.Count == 0)
        {
            _existingNodesByPath = new Dictionary<int, List<DbcTaxipathnodeRecord>>();
            _runsInCommitByPath = new Dictionary<int, Dictionary<int, WowTaxiPath>>();
            return;
        }

        int[] pathIds = paths.Select(p => p.PathId).Distinct().ToArray();
        List<DbcTaxipathnodeRecord> existing = await db.DbcTaxipathnode
            .Where(node => pathIds.Contains(node.PathID)).ToListAsync().ConfigureAwait(false);
        _existingNodesByPath = existing.GroupBy(node => node.PathID)
            .ToDictionary(group => group.Key, group => group.OrderBy(node => node.NodeIndex).ToList());

        _runsInCommitByPath = paths.GroupBy(p => p.PathId)
            .ToDictionary(group => group.Key, group => group.ToDictionary(p => p.RunIndex));

        _nextNodeId = (await db.DbcTaxipathnode.AsNoTracking()
            .Select(node => (int?)node.ID).MaxAsync().ConfigureAwait(false) ?? 0) + 1;
    }

    public Action Stage(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        var path = (WowTaxiPath)entity;

        if (ResolveWowMap(path.Map) is not int wowMap)
        {
            throw new InvalidOperationException(
                $"map {path.Map.Value} has no matching dbc_map row — name the editor map after a WoW map before saving its taxi paths.");
        }

        if (path.IsSaved)
        {
            DbcTaxipathRecord record = db.DbcTaxipath.Find(path.PathId)
                ?? throw new InvalidOperationException($"dbc_taxipath row {path.PathId} was removed outside the editor.");
            WriteRow(path, record);
        }
        else
        {
            var record = new DbcTaxipathRecord { ID = path.PathId };
            WriteRow(path, record);
            db.DbcTaxipath.Add(record);
        }

        // Only the first run of a given path seen in this commit does the (re)write — see
        // PrepareBatchAsync's doc — so a second run's Stage call for the same path is a no-op here.
        if (_pathsHandledThisCommit.Add(path.PathId))
        {
            StageNodesForPath(db, path.PathId);
        }

        return () =>
        {
            path.IsSaved = true;
            _allRuns = null;
        };
    }

    public void StageDelete(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        var path = (WowTaxiPath)entity;
        if (!path.IsSaved)
        {
            return;
        }

        // Deletes every run's rows, not just this entity's — a taxi path is one dbc_taxipath row
        // regardless of how many continents it spans, and a partial path (one run's nodes with no
        // matching from/to node data) is not valid data to leave behind.
        List<DbcTaxipathnodeRecord> nodes = db.DbcTaxipathnode.Where(node => node.PathID == path.PathId).ToList();
        db.DbcTaxipathnode.RemoveRange(nodes);
        db.DbcTaxipath.Remove(new DbcTaxipathRecord { ID = path.PathId });
    }

    /// <summary>Creates a new two-node path stub — <paramref name="key"/>, when it parses as
    /// <c>"from-to"</c>, sets the taxi node endpoints; otherwise both default to 0 for the user to fill
    /// in via the inspector, the same tolerance <c>WowZoneLightFactory.Create</c> gives a malformed
    /// borrowed-light key.</summary>
    public SceneEntity Create(EditorContext context, MapId map, Transform3D transform, string key)
    {
        (int fromNode, int toNode) = ParseKey(key);

        var path = new WowTaxiPath
        {
            Map = map,
                        RunIndex = 0,
            FromTaxiNodeId = fromNode,
            ToTaxiNodeId = toNode,
            IsSaved = false,
            Transform = transform,
        };

        var component = new WowTaxiPathComponent();
        var network = new VertexNetwork();
        int a = network.AddVertex(new Vector3(-DefaultHalfLength, 0.0f, 0.0f));
        int b = network.AddVertex(new Vector3(DefaultHalfLength, 0.0f, 0.0f));
        network.AddEdge(a, b);
        component.ReplaceNetwork(network);
        component.SetNodeData(a, TaxiNodeData.Default);
        component.SetNodeData(b, TaxiNodeData.Default);
        component.SetOrder(a, b);
        path.AddIntrinsicComponent(component);

        var command = new CreateEntityCommand(context.Scene, path);
        command.Apply();
        context.EditSessions.Record(command);
        return path;
    }

    private WowTaxiPath ToEntity(Loaded run, MapId map)
    {
        var path = new WowTaxiPath
        {
            Map = map,
            PathId = run.Path.ID,
            RunIndex = run.RunIndex,
            FromTaxiNodeId = run.Path.FromTaxiNode,
            ToTaxiNodeId = run.Path.ToTaxiNode,
            Cost = run.Path.Cost,
            IsSaved = true,
        };

        var component = new WowTaxiPathComponent();
        Vector3 origin = CreatureCoordinates.PositionToGodot(new Vector3(run.Nodes[0].Loc1, run.Nodes[0].Loc2, run.Nodes[0].Loc3));

        var network = new VertexNetwork();
        var vertexIds = new int[run.Nodes.Count];
        for (int i = 0; i < run.Nodes.Count; i++)
        {
            DbcTaxipathnodeRecord node = run.Nodes[i];
            Vector3 world = CreatureCoordinates.PositionToGodot(new Vector3(node.Loc1, node.Loc2, node.Loc3));
            vertexIds[i] = network.AddVertex(world - origin);
        }

        for (int i = 0; i + 1 < vertexIds.Length; i++)
        {
            network.AddEdge(vertexIds[i], vertexIds[i + 1]);
        }

        component.ReplaceNetwork(network);
        for (int i = 0; i < run.Nodes.Count; i++)
        {
            DbcTaxipathnodeRecord node = run.Nodes[i];
            component.SetNodeData(vertexIds[i], new TaxiNodeData(node.Flags, node.Delay, node.ArrivalEventID, node.DepartureEventID, node.ContinentID));
        }

        if (vertexIds.Length >= 2)
        {
            component.SetOrder(vertexIds[0], vertexIds[1]);
        }

        path.AddIntrinsicComponent(component);
        path.Transform = new Transform3D(Basis.Identity, origin);
        return path;
    }

    /// <summary>
    /// Rewrites every `dbc_taxipathnode` row for <paramref name="pathId"/> in one pass: for each run
    /// index up to the highest known either from the database or from this commit, either the live
    /// component of whichever run entity is part of this commit, or — for a run this commit never
    /// touched — its prior rows carried forward field-for-field. <c>NodeIndex</c> (and each row's
    /// synthetic <c>ID</c>) comes out dense across the concatenation regardless, following
    /// <c>WowZoneLightFactory.StagePoints</c>.
    /// </summary>
    private void StageNodesForPath(WorldDbContext db, int pathId)
    {
        List<DbcTaxipathnodeRecord> existing = _existingNodesByPath.GetValueOrDefault(pathId, []);
        List<(int RunIndex, int ContinentId, List<DbcTaxipathnodeRecord> Nodes)> existingRuns = GroupIntoRuns(existing);
        Dictionary<int, WowTaxiPath> runsInCommit = _runsInCommitByPath.GetValueOrDefault(pathId, new Dictionary<int, WowTaxiPath>());

        int maxRun = Math.Max(
            existingRuns.Count == 0 ? -1 : existingRuns[^1].RunIndex,
            runsInCommit.Count == 0 ? -1 : runsInCommit.Keys.Max());

        db.DbcTaxipathnode.RemoveRange(existing);

        int nodeIndex = 0;
        for (int run = 0; run <= maxRun; run++)
        {
            if (runsInCommit.TryGetValue(run, out WowTaxiPath? runEntity))
            {
                if (ResolveWowMap(runEntity.Map) is not int continentId)
                {
                    throw new InvalidOperationException(
                        $"map {runEntity.Map.Value} has no matching dbc_map row — name the editor map after a WoW map before saving its taxi paths.");
                }

                var component = runEntity.Component<WowTaxiPathComponent>()!;
                foreach ((int _, Vector3 local, TaxiNodeData data) in component.OrderedNodes)
                {
                    Vector3 world = runEntity.Transform * local;
                    Vector3 server = CreatureCoordinates.PositionFromGodot(world);
                    db.DbcTaxipathnode.Add(new DbcTaxipathnodeRecord
                    {
                        ID = _nextNodeId++,
                        PathID = pathId,
                        NodeIndex = nodeIndex++,
                        ContinentID = continentId,
                        Loc1 = server.X,
                        Loc2 = server.Y,
                        Loc3 = server.Z,
                        Flags = data.Flags,
                        Delay = data.Delay,
                        ArrivalEventID = data.ArrivalEventId,
                        DepartureEventID = data.DepartureEventId,
                    });
                }

                continue;
            }

            List<DbcTaxipathnodeRecord> preserved = existingRuns.FirstOrDefault(candidate => candidate.RunIndex == run).Nodes ?? [];
            foreach (DbcTaxipathnodeRecord node in preserved)
            {
                db.DbcTaxipathnode.Add(new DbcTaxipathnodeRecord
                {
                    ID = _nextNodeId++,
                    PathID = pathId,
                    NodeIndex = nodeIndex++,
                    ContinentID = node.ContinentID,
                    Loc1 = node.Loc1,
                    Loc2 = node.Loc2,
                    Loc3 = node.Loc3,
                    Flags = node.Flags,
                    Delay = node.Delay,
                    ArrivalEventID = node.ArrivalEventID,
                    DepartureEventID = node.DepartureEventID,
                });
            }
        }
    }

    private static void WriteRow(WowTaxiPath path, DbcTaxipathRecord record)
    {
        record.FromTaxiNode = path.FromTaxiNodeId;
        record.ToTaxiNode = path.ToTaxiNodeId;
        record.Cost = path.Cost;
    }

    private async Task<List<Loaded>> AllRunsAsync()
    {
        if (_allRuns is { } cached)
        {
            return cached;
        }

        await using WorldDbContext db = _storage.CreateContext();
        List<DbcTaxipathRecord> paths = await db.DbcTaxipath.AsNoTracking().ToListAsync().ConfigureAwait(false);
        List<DbcTaxipathnodeRecord> nodes = await db.DbcTaxipathnode.AsNoTracking().ToListAsync().ConfigureAwait(false);

        Dictionary<int, List<DbcTaxipathnodeRecord>> byPath = nodes.GroupBy(node => node.PathID)
            .ToDictionary(group => group.Key, group => group.OrderBy(node => node.NodeIndex).ToList());

        var result = new List<Loaded>();
        foreach (DbcTaxipathRecord path in paths)
        {
            if (!byPath.TryGetValue(path.ID, out List<DbcTaxipathnodeRecord>? ordered) || ordered.Count == 0)
            {
                continue;
            }

            foreach ((int runIndex, int continentId, List<DbcTaxipathnodeRecord> runNodes) in GroupIntoRuns(ordered))
            {
                result.Add(new Loaded(path, runIndex, continentId, runNodes));
            }
        }

        _allRuns = result;
        return result;
    }

    /// <summary>Splits one path's nodes (already ordered by <c>NodeIndex</c>) into contiguous runs of
    /// matching <c>ContinentID</c> — shared by loading (<see cref="AllRunsAsync"/>) and staging
    /// (<see cref="StageNodesForPath"/>) so both agree on where a path's continent boundaries fall.:
    /// every cross-continent path has exactly two such runs in practice, but this makes no assumption
    /// beyond "contiguous".</summary>
    private static List<(int RunIndex, int ContinentId, List<DbcTaxipathnodeRecord> Nodes)> GroupIntoRuns(List<DbcTaxipathnodeRecord> orderedNodes)
    {
        var result = new List<(int, int, List<DbcTaxipathnodeRecord>)>();
        if (orderedNodes.Count == 0)
        {
            return result;
        }

        int runIndex = 0;
        int continentId = orderedNodes[0].ContinentID;
        var current = new List<DbcTaxipathnodeRecord> { orderedNodes[0] };
        for (int i = 1; i < orderedNodes.Count; i++)
        {
            if (orderedNodes[i].ContinentID != continentId)
            {
                result.Add((runIndex, continentId, current));
                runIndex++;
                current = [];
                continentId = orderedNodes[i].ContinentID;
            }

            current.Add(orderedNodes[i]);
        }

        result.Add((runIndex, continentId, current));
        return result;
    }

    private static (int From, int To) ParseKey(string key)
    {
        string[] parts = key.Split('-', 2);
        if (parts.Length == 2 && int.TryParse(parts[0], out int from) && int.TryParse(parts[1], out int to))
        {
            return (from, to);
        }

        return (0, 0);
    }

    // Two runs is the observed maximum (dbc_taxipath); the factor of 4 leaves headroom without
    // meaningfully shrinking the id space PathID actually uses.
    private static long PersistentKeyOf(int pathId, int runIndex) => ((long)pathId * 4) + runIndex;

    private int? ResolveWowMap(MapId editorMap) => WowMapResolution.Resolve(_storage, editorMap);

    private readonly record struct Loaded(DbcTaxipathRecord Path, int RunIndex, int ContinentId, List<DbcTaxipathnodeRecord> Nodes);
}
