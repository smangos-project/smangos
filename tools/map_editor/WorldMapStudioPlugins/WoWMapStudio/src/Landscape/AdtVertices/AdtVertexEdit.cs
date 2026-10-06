using System.Collections.Generic;
using System.Linq;
using NVector3 = System.Numerics.Vector3;

namespace WorldMapStudio.WoW;

/// <summary>What a one-shot vertex edit did: chunks it changed, and chunks it could not touch.</summary>
public readonly record struct AdtVertexEditResult(int Changed, int Skipped)
{
    public static readonly AdtVertexEditResult None = new(0, 0);
}

/// <summary>
/// One-shot vertex edits on whole chunks, with no brush: fill a height, colour or light, and remove colour or
/// light. Each call is one undo step. They run on the same <see cref="AdtVertexSession"/> a stroke does.
/// </summary>
public sealed class AdtVertexEdit
{
    private readonly AdtMapWorld _world;
    private readonly EditSessionManager _sessions;

    public AdtVertexEdit(AdtMapWorld world, EditSessionManager sessions)
    {
        _world = world;
        _sessions = sessions;
    }

    /// <summary>Sets every vertex of the chunks to <paramref name="value"/>: <c>X</c> is the height, or R, G, B
    /// the colour multiplier or light. A chunk with vertices it cannot edit (not loaded, or outside the image)
    /// keeps those.</summary>
    public AdtVertexEditResult Fill(AdtVertexKind kind, IEnumerable<ChunkCoord> chunks, NVector3 value) =>
        Run(kind, Describe(kind, "Fill"), chunks, session =>
        {
            var changed = new HashSet<ChunkCoord>();
            foreach (ChunkCoord coord in Imported(chunks))
            {
                bool any = false;
                foreach (AdtVertex vertex in VerticesOf(coord))
                {
                    any |= session.Lattice.Set(vertex, value);
                }

                if (any)
                {
                    changed.Add(coord);
                }
            }

            return changed.Count;
        });

    /// <summary>Puts colour or light back to its neutral value in the chunks, leaving them using it.</summary>
    public AdtVertexEditResult Reset(AdtVertexKind kind, IEnumerable<ChunkCoord> chunks) =>
        Fill(kind, chunks, AdtVertexCodec.For(kind).Neutral);

    /// <summary>Takes vertex colour or light off the chunks: their data is cleared and they stop claiming it.
    /// A vertex a neighbouring chunk that keeps the layer also holds is left alone.</summary>
    public AdtVertexEditResult Remove(AdtVertexKind kind, IEnumerable<ChunkCoord> chunks)
    {
        if (kind == AdtVertexKind.Height)
        {
            return AdtVertexEditResult.None;
        }

        return Run(kind, Describe(kind, "Remove"), chunks, session =>
        {
            var removed = Imported(chunks)
                .Where(coord => Claims(session, coord))
                .ToHashSet();

            foreach (ChunkCoord coord in removed)
            {
                foreach (AdtVertex vertex in VerticesOf(coord))
                {
                    if (AdtVertexLattice.ChunksHolding(vertex).All(holder => removed.Contains(holder) || !Claims(session, holder)))
                    {
                        session.Lattice.Clear(vertex);
                    }
                }

                session.SetLayer(coord, enabled: false);
            }

            return removed.Count;
        });
    }

    /// <summary>The nine by nine corners of a chunk, in row order.</summary>
    public static IEnumerable<AdtVertex> CornersOf(ChunkCoord chunk)
    {
        for (int z = 0; z <= AdtVertex.CellsPerChunk; z++)
        {
            for (int x = 0; x <= AdtVertex.CellsPerChunk; x++)
            {
                yield return AdtVertex.Corner(chunk, x, z);
            }
        }
    }

    /// <summary>The eight by eight cell centres of a chunk, in row order.</summary>
    public static IEnumerable<AdtVertex> CentresOf(ChunkCoord chunk)
    {
        for (int z = 0; z < AdtVertex.CellsPerChunk; z++)
        {
            for (int x = 0; x < AdtVertex.CellsPerChunk; x++)
            {
                yield return AdtVertex.Centre(chunk, x, z);
            }
        }
    }

    /// <summary>Every vertex of a chunk: its corners, then its cell centres.</summary>
    public static IEnumerable<AdtVertex> VerticesOf(ChunkCoord chunk) => CornersOf(chunk).Concat(CentresOf(chunk));

    private static string Describe(AdtVertexKind kind, string verb) => kind switch
    {
        AdtVertexKind.Height => $"{verb} height",
        AdtVertexKind.Color => $"{verb} vertex colour",
        _ => $"{verb} vertex light",
    };

    private IEnumerable<ChunkCoord> Imported(IEnumerable<ChunkCoord> chunks) =>
        chunks.Distinct().Where(coord => _world.TryChunk(coord, out _));

    private bool Claims(AdtVertexSession session, ChunkCoord coord) =>
        _world.TryChunk(coord, out AdtChunkRef chunk) && session.Has(chunk.Tile.Chunks[(chunk.Col, chunk.Row)]);

    private AdtVertexEditResult Run(
        AdtVertexKind kind,
        string description,
        IEnumerable<ChunkCoord> requested,
        System.Func<AdtVertexSession, int> operation)
    {
        int total = requested.Distinct().Count();
        if (AdtVertexSession.TryOpen(_world, kind) is not { } session)
        {
            return new AdtVertexEditResult(0, total);
        }

        int changed;
        try
        {
            changed = operation(session);
            session.Settle();
        }
        catch
        {
            session.Cancel();
            throw;
        }

        if (session.End(description) is { } command)
        {
            _sessions.Record(command);
        }

        return new AdtVertexEditResult(changed, total - changed);
    }
}
