using System;
using System.Collections.Generic;
using Godot;
using NVector3 = System.Numerics.Vector3;

namespace WorldMapStudio.WoW;

/// <summary>
/// An <see cref="IStrokeTarget"/> that edits the vertices of the imported chunks under the brush, with whatever
/// <see cref="IAdtVertexOp"/> it is given: raising, flattening, smoothing, painting colour or light. A dab
/// finds the vertices inside the brush, computes each one's new value before changing any, and writes them
/// back straight away so the viewport stays live. One stroke is one undo step.
/// </summary>
public sealed class AdtVertexStrokeTarget : IStrokeTarget
{
    private const double WorldRefreshSeconds = 0.5;

    private readonly Func<AdtMapWorld?> _worldFactory;
    private readonly AdtVertexKind _kind;
    private readonly Func<IAdtVertexOp> _opFactory;
    private readonly string _description;
    private readonly HashSet<ChunkCoord> _loading = [];
    private readonly HashSet<ChunkCoord> _frozen = [];

    private AdtMapWorld? _world;
    private double _worldTime = double.NegativeInfinity;
    private AdtVertexSession? _session;
    private IAdtVertexOp? _op;
    private bool _begun;

    public AdtVertexStrokeTarget(Func<AdtMapWorld?> worldFactory, AdtVertexKind kind, Func<IAdtVertexOp> opFactory, string description)
    {
        _worldFactory = worldFactory;
        _kind = kind;
        _opFactory = opFactory;
        _description = description;
    }

    /// <summary>Chunks the last stroke could not fully edit because their data is not loaded yet.</summary>
    public IReadOnlyCollection<ChunkCoord> Loading => _loading;

    /// <summary>Chunks the last stroke could not fully edit because part of their data lies outside the image.</summary>
    public IReadOnlyCollection<ChunkCoord> Frozen => _frozen;

    /// <summary>The op of the stroke in progress or the last one.</summary>
    public IAdtVertexOp? Op => _op;

    public bool Covers(Vector3 world)
    {
        if (_session == null && (Time.GetTicksMsec() / 1000.0) - _worldTime >= WorldRefreshSeconds)
        {
            _world = _worldFactory();
            _worldTime = Time.GetTicksMsec() / 1000.0;
        }

        return _world != null && _world.TryChunkAt(world, out _);
    }

    public bool Begin()
    {
        _world = _worldFactory();
        _worldTime = Time.GetTicksMsec() / 1000.0;
        if (_world == null || AdtVertexSession.TryOpen(_world, _kind) is not { } session)
        {
            return false;
        }

        _session = session;
        _op = _opFactory();
        _begun = false;
        _loading.Clear();
        _frozen.Clear();
        return true;
    }

    public bool Dab(in BrushDab dab)
    {
        if (_world is not { } world || _session is not { } session || _op is not { } op)
        {
            return false;
        }

        List<(AdtVertex Vertex, ChunkCoord Chunk, float Weight)> sites = Gather(world, dab);
        if (sites.Count == 0)
        {
            return false;
        }

        AdtVertexLattice lattice = session.Lattice;
        if (!_begun)
        {
            op.Begin(world.NearestVertex(dab.Center), lattice);
            _begun = true;
        }

        var results = new List<(AdtVertex Vertex, NVector3 Value)>(sites.Count);
        foreach ((AdtVertex vertex, ChunkCoord chunk, float weight) in sites)
        {
            switch (lattice.AccessOf(vertex))
            {
                case AdtVertexAccess.Loading:
                    _loading.Add(chunk);
                    continue;
                case AdtVertexAccess.Frozen:
                    _frozen.Add(chunk);
                    continue;
            }

            NVector3 current = lattice.Get(vertex);
            Vector3 position = world.PositionOf(vertex);
            var step = new AdtVertexStep(vertex, position.X, position.Z, Math.Min(1.0f, weight * dab.Strength * dab.Pressure), dab.Invert);
            NVector3 next = lattice.Codec.Clamp(op.Apply(current, step, lattice));
            if (next != current)
            {
                results.Add((vertex, next));
            }
        }

        bool changed = false;
        foreach ((AdtVertex vertex, NVector3 value) in results)
        {
            changed |= lattice.Set(vertex, value);
        }

        if (changed)
        {
            session.Settle();
        }

        return changed;
    }

    public IEditCommand? Finish()
    {
        AdtVertexSession? session = _session;
        _session = null;
        return session?.End(_description);
    }

    public void Cancel()
    {
        _session?.Cancel();
        _session = null;
    }

    // The imported vertices a dab reaches, each once even where several chunks share a corner, with the chunk
    // that offered it and the brush weight at its position. A cell centre belongs to one chunk alone.
    private static List<(AdtVertex, ChunkCoord, float)> Gather(AdtMapWorld world, in BrushDab dab)
    {
        ChunkCoord min = world.Grid.CoordAt(new Vector3(dab.Center.X - dab.Radius, 0.0f, dab.Center.Z - dab.Radius));
        ChunkCoord max = world.Grid.CoordAt(new Vector3(dab.Center.X + dab.Radius, 0.0f, dab.Center.Z + dab.Radius));

        var seen = new HashSet<AdtVertex>();
        var sites = new List<(AdtVertex, ChunkCoord, float)>();
        for (int x = min.X; x <= max.X; x++)
        {
            for (int y = min.Y; y <= max.Y; y++)
            {
                var coord = new ChunkCoord(x, y);
                BrushGrid corners = world.VertexGridOf(coord);
                if (!BrushRaster.Overlaps(corners, dab) || !world.TryChunk(coord, out _))
                {
                    continue;
                }

                BrushRaster.Walk(corners, dab, (localX, localZ, weight) =>
                {
                    AdtVertex vertex = AdtVertex.Corner(coord, localX, localZ);
                    if (seen.Add(vertex))
                    {
                        sites.Add((vertex, coord, weight));
                    }
                });

                BrushRaster.Walk(world.CentreGridOf(coord), dab, (localX, localZ, weight) =>
                    sites.Add((AdtVertex.Centre(coord, localX, localZ), coord, weight)));
            }
        }

        return sites;
    }
}
