using System.Collections.Generic;
using System.Linq;

namespace WorldMapStudio.WoW;

/// <summary>
/// Turns one ADT edit into its single undo command: the image stroke's chunk edits, plus the tiles' chunk facts
/// that changed. Every ADT tool that edits an imported image ends its edit here.
/// </summary>
public static class AdtEditCommit
{
    /// <summary>Ends <paramref name="image"/>'s stroke and returns the command for its edits and for every
    /// chunk of <paramref name="chunks"/> whose facts are no longer <c>Original</c>, or null when nothing
    /// changed.</summary>
    public static IEditCommand? Commit(
        PaintImage image,
        ImageComponent placement,
        IEnumerable<(AdtChunkRef Chunk, AdtChunkFacts Original)> chunks,
        string description)
    {
        var imageEdits = image.EndStroke();
        var commands = new List<IEditCommand>();
        if (imageEdits.Count > 0)
        {
            commands.Add(new PaintImageChunksCommand(image, imageEdits, placement.AffectedEntities, description));
        }

        foreach (var group in chunks.GroupBy(pair => pair.Chunk.Entity))
        {
            AdtChunkFactsChange[] changes = group
                .Select(pair => new AdtChunkFactsChange(
                    pair.Chunk.Col, pair.Chunk.Row, pair.Original, pair.Chunk.Tile.Chunks[(pair.Chunk.Col, pair.Chunk.Row)]))
                .Where(change => !change.Before.SameAs(change.After))
                .ToArray();
            if (changes.Length > 0)
            {
                commands.Add(new SetAdtChunkFactsCommand(group.Key, group.First().Chunk.Tile, changes, description));
            }
        }

        return commands.Count == 0 ? null : new BatchEditCommand(description, commands);
    }
}
