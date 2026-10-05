using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Converts <c>dbc_light</c> and <c>dbc_zonelightpoint</c> coordinates to and from Godot space. The
/// two tables use different conventions, kept side by side here so the asymmetry is stated once.
///
/// <b><c>dbc_light.GameCoords</c> and the two <c>GameFalloff</c> radii</b> are the MDDF/MODF on-disk
/// placement convention (see <see cref="WowPlacementTransform"/>) pre-multiplied by
/// <see cref="GameCoordScale"/>. The raw triple divided by 36 is exactly what
/// <see cref="WowPlacementTransform.FromOnDiskPosition"/> maps to a Godot position, and a raw radius
/// divided by 36 is a Godot-unit distance. Confirmed against imported data: <c>dbc_light</c> id 26,
/// the row the "Loch Modan" <c>dbc_zonelight</c> borrows, converts to a point inside that zone's
/// own polygon.
///
/// <b><c>dbc_zonelightpoint.Pos1</c> / <c>Pos2</c></b> are raw world X and Y in yards, with no
/// scale and no height. They convert straight through <see cref="WowCoordinates.ToGodot(Vector3)"/>
/// with Z at zero — the same path a server position takes for a creature spawn.
/// </summary>
public static class WowLightCoordinates
{
    /// <summary><c>dbc_light</c> stores position and both falloff radii as yards times this. One
    /// named constant so the scale is a one-line change if a future re-import pre-scales the column.</summary>
    public const float GameCoordScale = 36.0f;

    public static Vector3 PositionToGodot(Vector3 gameCoords) =>
        WowPlacementTransform.FromOnDiskPosition(gameCoords / GameCoordScale);

    public static Vector3 PositionFromGodot(Vector3 godotPosition) =>
        WowPlacementTransform.OnDiskPosition(godotPosition) * GameCoordScale;

    public static float RadiusToGodot(float gameRadius) => gameRadius / GameCoordScale;

    public static float RadiusFromGodot(float godotRadius) => godotRadius * GameCoordScale;

    /// <summary>A <c>dbc_zonelightpoint</c> (Pos1, Pos2) pair as a Godot ground position, Y at zero.</summary>
    public static Vector3 ZonePointToGodot(float pos1, float pos2) =>
        WowCoordinates.ToGodot(new Vector3(pos1, pos2, 0.0f));

    /// <summary>The (Pos1, Pos2) pair for a Godot position; height is discarded.</summary>
    public static (float Pos1, float Pos2) ZonePointFromGodot(Vector3 godotPosition)
    {
        Vector3 wow = WowCoordinates.FromGodot(godotPosition);
        return (wow.X, wow.Y);
    }
}
