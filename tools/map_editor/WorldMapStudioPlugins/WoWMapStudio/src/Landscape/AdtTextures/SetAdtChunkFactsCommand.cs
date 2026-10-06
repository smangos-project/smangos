using System.Collections.Generic;
using System.IO;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>One chunk's facts before and after an edit.</summary>
public readonly record struct AdtChunkFactsChange(int Col, int Row, AdtChunkFacts Before, AdtChunkFacts After);

/// <summary>
/// Changes the facts of some of a tile's chunks: which materials sit in which slots, and which layers the chunk
/// claims. Reports chunk impacts bounded to just the changed chunks, so exporting after a small edit rewrites
/// those chunks and not the tile.
/// </summary>
public sealed class SetAdtChunkFactsCommand : IEditCommand, IChunkChangeCommand
{
    private readonly AdtTileComponent _tile;
    private readonly IReadOnlyList<AdtChunkFactsChange> _changes;

    public SetAdtChunkFactsCommand(
        SceneEntity entity,
        AdtTileComponent tile,
        IReadOnlyList<AdtChunkFactsChange> changes,
        string description)
    {
        _tile = tile;
        _changes = changes;
        Description = description;
        Targets = [entity];

        Aabb bounds = default;
        for (int i = 0; i < changes.Count; i++)
        {
            Aabb chunk = tile.ChunkWorldBounds(changes[i].Col, changes[i].Row);
            bounds = i == 0 ? chunk : bounds.Merge(chunk);
        }

        ChunkImpacts =
        [
            new ChunkChangeImpact(
                entity,
                new ChunkChangeSnapshot(entity.Map, bounds, Fingerprint(entity, changes, before: true)),
                new ChunkChangeSnapshot(entity.Map, bounds, Fingerprint(entity, changes, before: false))),
        ];
    }

    public IReadOnlyList<IEntity> Targets { get; }

    public IReadOnlyList<ChunkChangeImpact> ChunkImpacts { get; }

    public string Description { get; }

    public void Apply()
    {
        foreach (AdtChunkFactsChange change in _changes)
        {
            _tile.SetChunk(change.Col, change.Row, change.After);
        }
    }

    public void Revert()
    {
        foreach (AdtChunkFactsChange change in _changes)
        {
            _tile.SetChunk(change.Col, change.Row, change.Before);
        }
    }

    private static string Fingerprint(SceneEntity entity, IReadOnlyList<AdtChunkFactsChange> changes, bool before)
    {
        using var stream = new MemoryStream();
        using var writer = new BinaryWriter(stream);
        foreach (AdtChunkFactsChange change in changes.OrderBy(change => change.Row).ThenBy(change => change.Col))
        {
            AdtChunkFacts facts = before ? change.Before : change.After;
            writer.Write(change.Col);
            writer.Write(change.Row);
            writer.Write(facts.HasHoles);
            writer.Write(facts.HasVertexColor);
            writer.Write(facts.HasVertexLight);
            foreach (AdtSlotAssignment slot in facts.Slots)
            {
                writer.Write(slot.SlotIndex);
                writer.Write(slot.MaterialId ?? -1);
            }
        }

        writer.Flush();
        return ChunkChangeSnapshot.FingerprintBytes(entity, stream.ToArray(), entity.Map.Value);
    }
}
