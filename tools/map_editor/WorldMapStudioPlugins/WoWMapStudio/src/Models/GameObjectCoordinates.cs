using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Converts a `tc_gameobject` row's `position_x/y/z` and `rotation0..3` columns to and from Godot space.
/// Position is the same conversion <see cref="CreatureCoordinates"/> already uses — server world space
/// needs no gameobject-specific mapping. Rotation is a genuine quaternion rather than a single yaw, since
/// 770 real spawns are tilted out of the vertical — <see cref="WowCoordinates.ToGodot(Quaternion)"/>
/// already exists for this (used by <c>WmoRootReader</c>'s doodad placements), so this only wraps it under
/// a gameobject-specific name, matching <see cref="CreatureCoordinates"/>'s own shape.
/// </summary>
public static class GameObjectCoordinates
{
    public static Vector3 PositionToGodot(Vector3 serverPosition) => WowCoordinates.ToGodot(serverPosition);

    public static Vector3 PositionFromGodot(Vector3 godotPosition) => WowCoordinates.FromGodot(godotPosition);

    public static Basis RotationToGodot(Quaternion serverRotation) => new(WowCoordinates.ToGodot(serverRotation));

    /// <summary>The inverse of <see cref="RotationToGodot"/>. Reads the quaternion off the basis rather
    /// than requiring the caller to already have one, since a gizmo drag produces a <see cref="Basis"/>.</summary>
    public static Quaternion RotationFromGodot(Basis godotBasis) => WowCoordinates.FromGodot(godotBasis.GetRotationQuaternion());

    /// <summary>The basis's yaw, for `tc_gameobject.orientation` — informational only (the quaternion
    /// drives the actual rendered rotation; see `WowGameObjectSpawn.Orientation`'s own doc), but reads
    /// off the Godot basis directly rather than decomposing the WoW-space quaternion
    /// <see cref="RotationFromGodot"/> already produced: that quaternion's own Euler decomposition
    /// assumes Godot's Y-up axis order, which does not hold for a WoW Z-up rotation. Same shape as
    /// <see cref="CreatureCoordinates.OrientationFromGodot"/>, with no <c>FacingOffset</c> term — that
    /// constant corrects for a model's own baked-in forward axis, which does not apply to a column nothing
    /// renders from.</summary>
    public static float OrientationFromGodot(Basis godotBasis) => godotBasis.GetEuler().Y;
}
