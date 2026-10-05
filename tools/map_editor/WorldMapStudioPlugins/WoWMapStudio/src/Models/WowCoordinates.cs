using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Converts from WoW's Z-up, right-handed client-model space into Godot's Y-up, right-handed
/// scene space. The transform is a proper rotation (determinant +1), so it does not affect
/// triangle winding by itself.
/// </summary>
public static class WowCoordinates
{
    public static Vector3 ToGodot(Vector3 wow) => new(wow.X, wow.Z, -wow.Y);

    /// <summary>The inverse of <see cref="ToGodot(Vector3)"/> — for a writer producing client-model
    /// space (M2 vertex positions/normals, WMO geometry) from this editor's own Godot-space data.</summary>
    public static Vector3 FromGodot(Vector3 godot) => new(godot.X, -godot.Z, godot.Y);

    /// <summary>Converts a raw on-disk (x, y, z, w) quaternion into Godot space, matching the position conversion.</summary>
    public static Quaternion ToGodot(Quaternion wow) => new Quaternion(wow.X, wow.Z, -wow.Y, wow.W).Normalized();

    /// <summary>The inverse of <see cref="ToGodot(Quaternion)"/> — for a writer producing a raw on-disk
    /// (x, y, z, w) quaternion (e.g. `tc_gameobject.rotation0..3`) from a Godot-space rotation.</summary>
    public static Quaternion FromGodot(Quaternion godot) => new Quaternion(godot.X, -godot.Z, godot.Y, godot.W).Normalized();

    /// <summary>Converts a WoW-space min/max corner pair into a Godot <see cref="Aabb"/>.</summary>
    public static Aabb TransformBounds(Vector3 wowMin, Vector3 wowMax)
    {
        // Position converts as (x, z, -y): a signed axis permutation, so per-axis min/max carries
        // over directly without needing to transform all 8 corners.
        Vector3 min = new(wowMin.X, wowMin.Z, -wowMax.Y);
        Vector3 max = new(wowMax.X, wowMax.Z, -wowMin.Y);
        return new Aabb(min, max - min);
    }

    /// <summary>The inverse of <see cref="TransformBounds"/>: recovers the WoW-space min/max corner
    /// pair a Godot <see cref="Aabb"/> was built from. Needed on the query side rather than the
    /// export side — a streaming scan's region is computed in Godot space but has to filter rows
    /// stored in WoW-space columns.</summary>
    public static (Vector3 Min, Vector3 Max) FromGodotBounds(Aabb godot)
    {
        Vector3 godotMin = godot.Position;
        Vector3 godotMax = godot.End;
        Vector3 min = new(godotMin.X, -godotMax.Z, godotMin.Y);
        Vector3 max = new(godotMax.X, -godotMin.Z, godotMax.Y);
        return (min, max);
    }
}
