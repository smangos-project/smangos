using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Converts a `dbc_soundemitters` row's direction vector to and from a Godot basis — 532 of 1258 real
/// emitters have a nonzero direction, so the rotate gizmo edits direction directly rather than an
/// angle, with an identity basis meaning "no direction" so the 726 undirected emitters keep writing
/// zeros.
///
/// Which local axis actually reads as "forward" for the rendered indicator (see
/// <c>WowSoundEmitterMarker</c>) has no independent cross-check in this data, the same open-ended
/// piece <see cref="CreatureCoordinates.FacingOffset"/> flags for its own model-forward-axis question —
/// this uses Godot's own camera-style convention (<c>-Basis.Z</c>) rather than guessing something
/// WoW-specific.
/// </summary>
public static class SoundEmitterCoordinates
{
    public static Basis DirectionToGodot(Vector3 serverDirection)
    {
        if (serverDirection.LengthSquared() < 1e-8f)
        {
            return Basis.Identity;
        }

        Vector3 godotDirection = WowCoordinates.ToGodot(serverDirection).Normalized();
        return Basis.LookingAt(godotDirection, Vector3.Up);
    }

    /// <summary>The inverse of <see cref="DirectionToGodot"/>: an untouched (identity) basis reads back
    /// as the zero vector, guarding the case <see cref="Basis.LookingAt"/> itself would throw on rather
    /// than ever calling it with a zero target.</summary>
    public static Vector3 DirectionFromGodot(Basis godotBasis)
    {
        if (godotBasis.IsEqualApprox(Basis.Identity))
        {
            return Vector3.Zero;
        }

        Vector3 forward = -godotBasis.Z;
        return WowCoordinates.FromGodot(forward);
    }
}
