using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Converts a placement's Godot <see cref="Transform3D"/> into the raw floats MDDF/MODF store on
/// disk. Placement position is a <em>different</em> on-disk convention from MCNK's — confirmed
/// against real MDDF entries (see <see cref="WowTileAddressing"/>'s ground truth method; the same
/// exercise, repeated for this format): raw <c>pos[0]/pos[2]</c> sit near
/// <c>OriginTileIndex*TileSize</c> for a placement inside this tile, not near zero the way a
/// ZEROPOINT-relative MCNK field would.
///
/// <b>Rotation</b> conjugates <see cref="Transform3D.Basis"/> into client world space — the frame
/// model geometry itself is authored in, which <see cref="WowCoordinates"/> already maps — and
/// decomposes it as <c>Rz(rot[1] + 180°) · Ry(rot[0]) · Rx(rot[2])</c>. That is the relationship a
/// placement's own MODF extents encode: reconstructing them from the referenced WMO's geometry only
/// reproduces Blizzard's stored bounds under the +180° yaw offset, across every asymmetric model
/// checked (see <see cref="WowPlacementTransformTests"/>, which pins one such reconstruction).
/// </summary>
public static class WowPlacementTransform
{
    private const float ZeroPoint = WowTileAddressing.OriginTileIndex * WowTileAddressing.TileSize;

    /// <summary>Client world space (X north, Y west, Z up) expressed as the basis whose columns are
    /// where each of its axes lands in Godot space — the same mapping <see cref="WowCoordinates.ToGodot(Vector3)"/>
    /// applies to model geometry, so an identity Godot basis means an unrotated model.</summary>
    private static readonly Basis WowToGodot = new(new Vector3(1, 0, 0), new Vector3(0, 0, -1), new Vector3(0, 1, 0));

    private static readonly Basis GodotToWow = WowToGodot.Inverse();

    /// <summary>The raw <c>(pos[0], pos[1], pos[2])</c> MDDF/MODF triple for a Godot world position.</summary>
    public static Vector3 OnDiskPosition(Vector3 godotPosition) =>
        new(godotPosition.Z + ZeroPoint, godotPosition.Y, ZeroPoint - godotPosition.X);

    /// <summary>The inverse of <see cref="OnDiskPosition"/> — recovers the Godot world position a raw
    /// MDDF/MODF triple describes. Exists for its own round-trip test; a reader would also use it.</summary>
    public static Vector3 FromOnDiskPosition(Vector3 onDisk) =>
        new(ZeroPoint - onDisk.Z, onDisk.Y, onDisk.X - ZeroPoint);

    /// <summary>The raw <c>(rot[0], rot[1], rot[2])</c> MDDF/MODF triple, in degrees, for a Godot
    /// world rotation. The client rebuilds these in client world space as <c>Rz(rot[1] + 180°) ·
    /// Ry(rot[0]) · Rx(rot[2])</c>, so this decomposes in that order and takes the yaw offset back
    /// out. <see cref="Basis.Orthonormalized"/> first because a scaled placement's basis is not a
    /// rotation matrix, and an Euler read of one is meaningless.</summary>
    public static Vector3 OnDiskRotationDegrees(Basis godotBasis)
    {
        Basis wowBasis = GodotToWow * godotBasis.Orthonormalized() * WowToGodot;
        Vector3 e = wowBasis.GetEuler(EulerOrder.Zyx); // wowBasis == Rz(e.Z) · Ry(e.Y) · Rx(e.X)
        return new Vector3(
            Mathf.RadToDeg(e.Y),
            // Taking the offset back out can push an unrotated placement's yaw to -360; fold it back
            // into a single turn so it reads as 0.
            Mathf.Wrap(Mathf.RadToDeg(e.Z) - 180.0f, -180.0f, 180.0f),
            Mathf.RadToDeg(e.X));
    }

    /// <summary>The inverse of <see cref="OnDiskRotationDegrees"/>.</summary>
    public static Basis FromOnDiskRotationDegrees(Vector3 degrees)
    {
        var e = new Vector3(
            Mathf.DegToRad(degrees.Z),
            Mathf.DegToRad(degrees.X),
            Mathf.DegToRad(degrees.Y + 180.0f));
        Basis wowBasis = Basis.FromEuler(e, EulerOrder.Zyx);
        return WowToGodot * wowBasis * GodotToWow;
    }

    /// <summary>MDDF/MODF's 1024-fixed-point scale field — <see cref="Basis.Scale"/>'s X component,
    /// since every model format this exporter places (<c>M2Format</c>, WMO) restricts itself to
    /// uniform scale.</summary>
    public static ushort OnDiskScale(Basis godotBasis) =>
        (ushort)Mathf.Clamp(Mathf.RoundToInt(godotBasis.Scale.X * 1024.0f), 0, ushort.MaxValue);
}
