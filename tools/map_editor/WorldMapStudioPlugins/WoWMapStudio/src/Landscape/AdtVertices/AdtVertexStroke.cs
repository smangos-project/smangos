using System;
using System.Collections.Generic;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>What a stroke laid along a path did.</summary>
public readonly record struct AdtVertexStrokeResult(bool Changed, int Loading, int Frozen);

/// <summary>Lays one whole vertex stroke along a path, for scripts: the same target the tools drive, with the
/// stroke recorded as one undo step.</summary>
public static class AdtVertexStroke
{
    public static AdtVertexStrokeResult Run(
        AdtMapWorld world,
        EditSessionManager sessions,
        AdtVertexKind kind,
        IAdtVertexOp op,
        Brush brush,
        IEnumerable<Vector3> points,
        string description)
    {
        var target = new AdtVertexStrokeTarget(() => world, kind, () => op, description);
        var stroke = new BrushStroke(brush, target, sessions);
        if (!stroke.Begin())
        {
            throw new InvalidOperationException("A stroke needs a map with imported ADT data for it.");
        }

        try
        {
            foreach (Vector3 point in points)
            {
                stroke.MoveTo(point);
            }
        }
        catch
        {
            stroke.Finish(record: false);
            throw;
        }

        bool changed = stroke.Finish(record: true);
        return new AdtVertexStrokeResult(changed, target.Loading.Count, target.Frozen.Count);
    }
}
