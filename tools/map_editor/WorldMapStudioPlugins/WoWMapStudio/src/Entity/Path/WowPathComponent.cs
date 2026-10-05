using System;
using System.Collections.Generic;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Non-generic surface so tooling (<see cref="WowPathToolFactory"/>) and scripting
/// (<c>WowPathScriptApi</c>) can find and walk "any path component" without knowing its
/// <c>TNode</c> metadata type — everything about a path's node <em>order</em> is metadata-free, only
/// per-node data itself needs the generic <see cref="WowPathComponent{TNode}"/>.</summary>
public interface IWowPathComponent : INetworkEditable
{
    /// <summary>The authored first node. 0 when the network has fewer than two vertices.</summary>
    int StartVertexId { get; }

    /// <summary>The authored second node — the direction <see cref="StartVertexId"/> walks toward.</summary>
    int SecondVertexId { get; }

    /// <summary>Each node's vertex id and local position, in authored order. Empty when the topology
    /// isn't a single connected, unbranched chain — see <see cref="ValidateTopology"/>.</summary>
    IReadOnlyList<(int VertexId, Vector3 Local)> OrderedVertexPositions { get; }

    /// <summary>Checks this path's topology against what a path is allowed to be: one connected,
    /// unbranched, faceless graph.</summary>
    IReadOnlyList<string> ValidateTopology(string displayName);

    /// <summary>Sets the authored order directly, re-deriving <see cref="OrderedVertexPositions"/> —
    /// what a caller uses after reshaping <see cref="INetworkEditable.Network"/> by hand (an inserted
    /// or removed vertex) rather than through the viewport tool.</summary>
    void SetOrder(int startVertexId, int secondVertexId);
}

/// <summary>
/// Shared shape for every path entity — a taxi route, a creature waypoint chain — backed by a
/// <see cref="VertexNetwork"/> edited with the shared <c>NetworkEditTool</c>, plus two things a plain
/// procedural mesh's network does not need: a stable authored node <b>order</b>
/// (<see cref="StartVertexId"/>/<see cref="SecondVertexId"/>, since the graph itself is unordered) and
/// per-node <typeparamref name="TNode"/> metadata keyed by vertex id.
///
/// The metadata dictionary is never pruned — <see cref="SetNetworkCommand"/> undoes an edit by
/// swapping the whole graph back in, so a vertex id that disappears and later reappears (undo/redo,
/// or a delete-then-undo) must find its old metadata still there rather than reset to
/// <see cref="DefaultNode"/>. Growth is bounded by one edit session's worth of vertex ids.
/// </summary>
public abstract class WowPathComponent<TNode> : SceneComponent, IWowPathComponent, ISceneBoundsProvider, ISceneNodeComponent, IMeshPickable
{
    private const float TubeRadius = 0.6f;
    private const float MarkerHalfSize = 1.1f;

    private VertexNetwork _network = new();
    private readonly Dictionary<int, TNode> _nodeData = [];
    private IReadOnlyList<int>? _orderedVertexIds;
    private int _metadataVersion;

    public override int ContentVersion => HashCode.Combine(_network.Version, StartVertexId, SecondVertexId, _metadataVersion);

    public VertexNetwork Network => _network;

    /// <summary>The authored first node. 0 when the network has fewer than two vertices.</summary>
    public int StartVertexId { get; private set; }

    /// <summary>The authored second node — the direction <see cref="StartVertexId"/> walks toward.</summary>
    public int SecondVertexId { get; private set; }

    /// <summary>The node this path type hands out for a vertex that has no metadata yet.</summary>
    protected abstract TNode DefaultNode { get; }

    /// <summary>
    /// The path's nodes in authored order, each with its resolved metadata. Empty when
    /// <see cref="StartVertexId"/>/<see cref="SecondVertexId"/> no longer describe a single connected,
    /// unbranched chain — see <see cref="ValidateTopology"/> for surfacing that to the user.
    /// </summary>
    public IReadOnlyList<(int VertexId, Vector3 Local, TNode Data)> OrderedNodes
    {
        get
        {
            if (_orderedVertexIds is not { Count: > 0 } ids)
            {
                return [];
            }

            var result = new List<(int, Vector3, TNode)>(ids.Count);
            foreach (int id in ids)
            {
                if (_network.Vertex(id) is { } vertex)
                {
                    result.Add((id, vertex.Position, NodeData(id)));
                }
            }

            return result;
        }
    }

    /// <summary>The metadata-free half of <see cref="OrderedNodes"/> — see
    /// <see cref="IWowPathComponent.OrderedVertexPositions"/>.</summary>
    public IReadOnlyList<(int VertexId, Vector3 Local)> OrderedVertexPositions
    {
        get
        {
            if (_orderedVertexIds is not { Count: > 0 } ids)
            {
                return [];
            }

            var result = new List<(int, Vector3)>(ids.Count);
            foreach (int id in ids)
            {
                if (_network.Vertex(id) is { } vertex)
                {
                    result.Add((id, vertex.Position));
                }
            }

            return result;
        }
    }

    public TNode NodeData(int vertexId) => _nodeData.TryGetValue(vertexId, out TNode? data) ? data : DefaultNode;

    public void SetNodeData(int vertexId, TNode data)
    {
        _nodeData[vertexId] = data;
        _metadataVersion++;
    }

    /// <summary>Checks this path's topology against what a path is allowed to be: one connected,
    /// unbranched, faceless graph.</summary>
    public IReadOnlyList<string> ValidateTopology(string displayName) =>
        _network.ValidateFor(displayName, new NetworkCapabilities(AllowsMultipleGraphs: false, AllowsBranching: false, AllowsFaces: false));

    public void ReplaceNetwork(VertexNetwork network)
    {
        Vector3? previousStart = _network.Vertex(StartVertexId)?.Position;
        Vector3? previousSecond = _network.Vertex(SecondVertexId)?.Position;
        _network = network.Clone();
        foreach (NetworkVertex vertex in _network.Vertices)
        {
            if (!_nodeData.ContainsKey(vertex.Id))
            {
                _nodeData[vertex.Id] = DefaultNode;
            }
        }

        Reconcile(previousStart, previousSecond);

        // Unlike WowZoneLightBoundary's ring, this component's tube mesh is real persisted geometry
        // (BuildNode), not something the edit tool redraws itself every frame — so nothing else makes
        // the viewport notice a drag until the representation is rebuilt. A no-op before the entity is
        // attached (Owner null): factories call this while still constructing the component, before
        // AddComponent, which builds the initial representation itself.
        Owner?.RefreshRepresentation();
    }

    /// <summary>Sets the authored order directly — what a factory calls once after loading or creating
    /// the network, since a fresh <see cref="ReplaceNetwork"/> has no prior anchors to reconcile from.</summary>
    public void SetOrder(int startVertexId, int secondVertexId)
    {
        StartVertexId = startVertexId;
        SecondVertexId = secondVertexId;
        _orderedVertexIds = _network.OrderedChainFrom(startVertexId, secondVertexId);
    }

    /// <summary>Re-derives <see cref="StartVertexId"/>/<see cref="SecondVertexId"/> after an edit: both
    /// anchors present just re-walks; a missing start picks the chain endpoint nearest its last known
    /// position and takes that endpoint's one neighbour as the new second; a missing second (start
    /// still present) picks whichever of start's current neighbours is nearest its own last known
    /// position. Never guesses across a branch — an invalid topology just leaves
    /// <see cref="OrderedNodes"/> empty for <see cref="ValidateTopology"/> to report.</summary>
    private void Reconcile(Vector3? previousStartPos, Vector3? previousSecondPos)
    {
        if (_network.Vertices.Count < 2)
        {
            SetOrder(0, 0);
            return;
        }

        int startId = StartVertexId;
        if (_network.Vertex(startId) == null)
        {
            IReadOnlyList<int> endpoints = _network.Endpoints();
            startId = endpoints.Count > 0
                ? NearestVertex(endpoints, previousStartPos)
                : _network.Vertices.Select(v => v.Id).OrderBy(id => id).First();
            int secondFromStart = NeighboursOf(startId).OrderBy(id => id).FirstOrDefault(startId);
            SetOrder(startId, secondFromStart);
            return;
        }

        int secondId = SecondVertexId;
        if (_network.Vertex(secondId) == null)
        {
            List<int> neighbours = NeighboursOf(startId).ToList();
            secondId = neighbours.Count > 0 ? NearestVertex(neighbours, previousSecondPos) : startId;
        }

        SetOrder(startId, secondId);
    }

    private IEnumerable<int> NeighboursOf(int vertexId) =>
        _network.Edges.Where(edge => edge.A == vertexId || edge.B == vertexId)
            .Select(edge => edge.A == vertexId ? edge.B : edge.A);

    private int NearestVertex(IReadOnlyList<int> candidates, Vector3? reference)
    {
        if (reference is not { } position)
        {
            return candidates.OrderBy(id => id).First();
        }

        return candidates
            .OrderBy(id => (_network.Vertex(id)!.Position - position).LengthSquared())
            .ThenBy(id => id)
            .First();
    }

    /// <summary>Copies this component's network, metadata and order into a freshly constructed clone —
    /// the shared tail of every subclass's <see cref="Clone"/> override.</summary>
    protected void CopyStateInto(WowPathComponent<TNode> clone)
    {
        clone._network = _network.Clone();
        foreach ((int id, TNode data) in _nodeData)
        {
            clone._nodeData[id] = data;
        }

        clone._metadataVersion = _metadataVersion;
        clone.SetOrder(StartVertexId, SecondVertexId);
    }

    public bool PlanarXZ => false;

    public bool SnapToTerrainOnPlace => true;

    public Aabb LocalBounds => _network.Bounds();

    public bool ContributesChunkOwnership => false;

    public virtual Node3D? BuildNode()
    {
        ArrayMesh mesh = WowPathMesh.Build(_network, TubeRadius, MarkerHalfSize);
        if (mesh.GetSurfaceCount() == 0)
        {
            return null;
        }

        return new MeshInstance3D
        {
            Name = "Path Geometry",
            Mesh = mesh,
            MaterialOverride = new StandardMaterial3D
            {
                AlbedoColor = MeshColor,
                ShadingMode = BaseMaterial3D.ShadingModeEnum.PerPixel,
            },
        };
    }

    /// <summary>The tube/marker colour, distinct per path type so a taxi route and a waypoint chain
    /// read apart at a glance.</summary>
    protected abstract Color MeshColor { get; }
}
