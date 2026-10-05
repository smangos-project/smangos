using Godot;

namespace WorldMapStudio.WoW;

public static class WowLightCoordinatesTests
{
    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void Light_position_round_trips()
    {
        var gameCoords = new Vector3(737280.0f, 15536.4f, 807168.0f);
        Vector3 godot = WowLightCoordinates.PositionToGodot(gameCoords);
        Vector3 back = WowLightCoordinates.PositionFromGodot(godot);

        Assert.AreApproximatelyEqual(gameCoords.X, back.X, 0.5);
        Assert.AreApproximatelyEqual(gameCoords.Y, back.Y, 0.5);
        Assert.AreApproximatelyEqual(gameCoords.Z, back.Z, 0.5);
    }

    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void Falloff_radius_round_trips()
    {
        foreach (float radius in new[] { 0.0f, 744.444f, 19200.0f, 72192.0f })
        {
            float godot = WowLightCoordinates.RadiusToGodot(radius);
            Assert.AreApproximatelyEqual(radius, WowLightCoordinates.RadiusFromGodot(godot), 1e-2);
        }
    }

    // Pins the 36 scale against imported dbc_light id 26 (the row "Loch Modan" borrows). Its raw
    // GameCoords convert to a point roughly (-5355, 432, 3413) in Godot space, inside that zone;
    // an unscaled or 36x-scaled read lands three orders of magnitude away.
    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void Light_scale_is_pinned_against_a_known_row()
    {
        Vector3 godot = WowLightCoordinates.PositionToGodot(new Vector3(737280.0f, 15536.4f, 807168.0f));

        Assert.AreApproximatelyEqual(-5354.67, godot.X, 2.0);
        Assert.AreApproximatelyEqual(431.57, godot.Y, 2.0);
        Assert.AreApproximatelyEqual(3413.33, godot.Z, 2.0);
    }

    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void Zone_point_round_trips()
    {
        (float Pos1, float Pos2) point = (-6044.45f, -4379.11f);
        Vector3 godot = WowLightCoordinates.ZonePointToGodot(point.Pos1, point.Pos2);
        (float Pos1, float Pos2) back = WowLightCoordinates.ZonePointFromGodot(godot);

        Assert.AreApproximatelyEqual(point.Pos1, back.Pos1, 1e-2);
        Assert.AreApproximatelyEqual(point.Pos2, back.Pos2, 1e-2);
    }

    // Pins that a zone-light point carries no 36 scale: imported dbc_zonelightpoint for "Loch Modan"
    // stores raw yard coordinates, so Pos1/Pos2 pass through to the Godot X/-Z axes unchanged.
    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void Zone_point_is_not_scaled()
    {
        Vector3 godot = WowLightCoordinates.ZonePointToGodot(-6044.45f, -4379.11f);

        Assert.AreApproximatelyEqual(-6044.45, godot.X, 1e-2);
        Assert.AreApproximatelyEqual(0.0, godot.Y, 1e-2);
        Assert.AreApproximatelyEqual(4379.11, godot.Z, 1e-2);
    }
}
