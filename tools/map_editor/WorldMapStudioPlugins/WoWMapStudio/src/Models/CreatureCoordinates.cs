using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Converts a creature row's <c>position_x/y/z</c> and <c>orientation</c> columns to and from
/// Godot space.
///
/// <b>Position</b> needs no new mapping: The server world space is the same client world
/// space <see cref="WowCoordinates"/> already converts, so <see cref="PositionToGodot"/> is exactly
/// <see cref="WowCoordinates.ToGodot(Vector3)"/> — not a coincidence, but the reduction of the known
/// MDDF placement relationship (<c>pos[0] = ZeroPoint - serverY</c>, <c>pos[1] = serverZ</c>,
/// <c>pos[2] = ZeroPoint - serverX</c>) composed with the already-locked
/// <see cref="WowPlacementTransform.OnDiskPosition"/>: the shared <c>ZeroPoint</c> term cancels and
/// what remains is <c>godot = (serverX, serverZ, -serverY)</c>. <see cref="CreatureCoordinatesTests"/>
/// pins this composed derivation directly against <see cref="WowPlacementTransform"/>'s own
/// implementation, so a sign error in either fails loudly instead of shipping spawns in the wrong
/// place.
///
/// <b>Orientation</b> is a single radians-about-server-+Z field, zero at +X. Server +Z and Godot +Y
/// are the same up axis, and the X/Y-to-X/-Z swap <see cref="WowCoordinates.ToGodot(Vector3)"/> makes
/// is a proper rotation (no reflection), so a server yaw becomes a Godot yaw of the same magnitude
/// about +Y — <em>provided</em> the model's own forward axis is WoW +X. That proviso is not settled
/// by the data; see <see cref="FacingOffset"/>.
/// </summary>
public static class CreatureCoordinates
{
    /// <summary>
    /// Correction between WoW +X (the axis <see cref="OrientationToGodot"/> derives facing from) and
    /// whatever forward axis is actually baked into creature/character M2 models — the one piece of
    /// this conversion with no independent cross-check, exactly like
    /// <see cref="WowPlacementTransform"/> flags for MDDF rotation. Zero until confirmed by eye
    /// against one asymmetric model. A single named constant (0 or +-90 degrees, in radians) so
    /// correcting it is a one-line change, not an archaeology exercise.
    /// </summary>
    public const float FacingOffset = 0.0f;

    public static Vector3 PositionToGodot(Vector3 serverPosition) => WowCoordinates.ToGodot(serverPosition);

    public static Vector3 PositionFromGodot(Vector3 godotPosition) => WowCoordinates.FromGodot(godotPosition);

    public static Basis OrientationToGodot(float serverOrientationRadians) =>
        new(Vector3.Up, serverOrientationRadians + FacingOffset);

    /// <summary>The inverse of <see cref="OrientationToGodot"/>. Reads the yaw off the basis rather
    /// than requiring the caller to already know it, since a gizmo drag produces a <see cref="Basis"/>,
    /// not an angle.</summary>
    public static float OrientationFromGodot(Basis godotBasis) =>
        godotBasis.GetEuler().Y - FacingOffset;
}
