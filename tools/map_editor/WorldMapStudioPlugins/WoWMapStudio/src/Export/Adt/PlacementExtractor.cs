using System;
using System.Collections.Generic;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>A model placement as the editor holds it. <see cref="LocalBounds"/> is the model's local bounding box; <see cref="DoodadSet"/> only applies to WMOs.</summary>
public sealed record EditorPlacement(long RecordId, string ModelPath, Transform3D Transform, Aabb LocalBounds, ushort DoodadSet);

/// <summary>
/// Turns the editor's model placements into a tile's MDDF/MODF records. Placements already on disk keep
/// their record (flags, name set) and are only rewritten when moved; unknown ones get a fresh uid; on-disk
/// records whose entity is gone are dropped.
/// </summary>
public static class PlacementExtractor
{
    /// <summary>First uid handed out to new placements, above anything the stock client data uses.</summary>
    public const uint CustomUidBase = 0x70000000;

    private const float PositionTolerance = 0.01f;
    private const ushort ModfHasScaleFlag = 0x4;

    /// <param name="uidOf">The uid a record id was exported under, if any.</param>
    /// <param name="newUid">Allocates a uid for a placement that has none and reports it back.</param>
    public static (List<ImportedDoodad> Doodads, List<ImportedObject> Objects) Apply(
        ImportedTile template,
        IReadOnlyList<EditorPlacement> placements,
        Func<long, uint?> uidOf,
        Func<long, uint> newUid,
        Func<string, uint> fdidOf)
    {
        var doodads = new List<ImportedDoodad>();
        var objects = new List<ImportedObject>();
        var oldDoodads = template.Doodads.ToDictionary(d => d.UniqueId);
        var oldObjects = template.Objects.ToDictionary(o => o.UniqueId);
        var id = template.TileId;

        foreach (EditorPlacement p in placements)
        {
            bool wmo = p.ModelPath.EndsWith(".wmo", StringComparison.OrdinalIgnoreCase);
            uint? uid = uidOf(p.RecordId);
            Vector3 centre = p.LocalBounds.GetCenter();
            Basis basis = p.Transform.Basis;
            Vector3 position = WowPlacementTransform.OnDiskPosition(p.Transform.Origin - (basis * centre));
            Vector3 rotation = WowPlacementTransform.OnDiskRotationDegrees(basis);
            ushort scale = WowPlacementTransform.OnDiskScale(basis);

            if (uid is { } known && (wmo ? oldObjects.ContainsKey(known) : oldDoodads.ContainsKey(known)))
            {
                if (wmo)
                {
                    ImportedObject old = oldObjects[known];
                    ushort oldScale = (old.Flags & ModfHasScaleFlag) != 0 ? old.Scale : (ushort)1024;
                    objects.Add(old.DoodadSet == p.DoodadSet && Same(old.Position, old.RotationDegrees, oldScale, position, rotation, scale)
                        ? old
                        : Object(old, position, rotation, scale, p, old.Flags | (scale != 1024 ? ModfHasScaleFlag : 0)));
                }
                else
                {
                    ImportedDoodad old = oldDoodads[known];
                    doodads.Add(Same(old.Position, old.RotationDegrees, old.Scale, position, rotation, scale)
                        ? old
                        : old with { Position = position, RotationDegrees = rotation, Scale = scale });
                }

                continue;
            }

            if (!InTile(id, position))
            {
                continue;
            }

            uint fresh = uid ?? newUid(p.RecordId);
            if (wmo)
            {
                var blank = new ImportedObject(fdidOf(p.ModelPath), fresh, position, rotation, default, default, 0, 0, 0, 1024);
                objects.Add(Object(blank, position, rotation, scale, p, scale != 1024 ? ModfHasScaleFlag : 0));
            }
            else
            {
                doodads.Add(new ImportedDoodad(fdidOf(p.ModelPath), fresh, position, rotation, scale, 0));
            }
        }

        return (doodads, objects);
    }

    private static ImportedObject Object(ImportedObject baseline, Vector3 position, Vector3 rotation, ushort scale, EditorPlacement p, int flags)
    {
        (Vector3 lower, Vector3 upper) = OnDiskBounds(p.Transform, p.LocalBounds);
        return baseline with
        {
            Position = position,
            RotationDegrees = rotation,
            Scale = scale,
            LowerBounds = lower,
            UpperBounds = upper,
            Flags = (ushort)flags,
            DoodadSet = p.DoodadSet,
        };
    }

    /// <summary>The on-disk axis-aligned box of the placed model's local bounds.</summary>
    public static (Vector3 Lower, Vector3 Upper) OnDiskBounds(Transform3D transform, Aabb local)
    {
        Vector3 lower = new(float.MaxValue, float.MaxValue, float.MaxValue);
        Vector3 upper = new(float.MinValue, float.MinValue, float.MinValue);
        for (int i = 0; i < 8; i++)
        {
            Vector3 corner = local.GetEndpoint(i);
            Vector3 world = WowPlacementTransform.OnDiskPosition(transform * corner);
            lower = new Vector3(Mathf.Min(lower.X, world.X), Mathf.Min(lower.Y, world.Y), Mathf.Min(lower.Z, world.Z));
            upper = new Vector3(Mathf.Max(upper.X, world.X), Mathf.Max(upper.Y, world.Y), Mathf.Max(upper.Z, world.Z));
        }

        return (lower, upper);
    }

    private static bool InTile(ImportedTileId id, Vector3 onDisk) =>
        (int)Mathf.Floor(onDisk.X / WowTileAddressing.TileSize) == id.FileX
        && (int)Mathf.Floor(onDisk.Z / WowTileAddressing.TileSize) == id.FileY;

    private static bool Same(Vector3 pos, Vector3 rot, ushort scale, Vector3 newPos, Vector3 newRot, ushort newScale) =>
        pos.DistanceTo(newPos) <= PositionTolerance && scale == newScale
        && WowPlacementTransform.SameRotation(rot, newRot);
}
