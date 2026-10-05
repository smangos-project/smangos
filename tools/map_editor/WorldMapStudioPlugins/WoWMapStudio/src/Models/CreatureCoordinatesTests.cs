using Godot;

namespace WorldMapStudio.WoW;

public static class CreatureCoordinatesTests
{
    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void Server_position_round_trips()
    {
        var server = new Vector3(1234.5f, -678.25f, 45.0f);
        Vector3 godot = CreatureCoordinates.PositionToGodot(server);
        Vector3 back = CreatureCoordinates.PositionFromGodot(godot);

        Assert.AreApproximatelyEqual(server.X, back.X, 1e-4);
        Assert.AreApproximatelyEqual(server.Y, back.Y, 1e-4);
        Assert.AreApproximatelyEqual(server.Z, back.Z, 1e-4);
    }

    // Pins the composed derivation in CreatureCoordinates' class doc against
    // WowPlacementTransform's own (already-locked) implementation, rather than trusting the algebra
    // on paper: converting a server position to Godot space and then to an MDDF on-disk triple must
    // reproduce the known raw relationship (pos[0] = ZeroPoint - serverY, pos[1] = serverZ,
    // pos[2] = ZeroPoint - serverX) exactly, since both describe the same physical placement.
    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void Position_conversion_agrees_with_the_known_mddf_relationship()
    {
        const float zeroPoint = WowTileAddressing.OriginTileIndex * WowTileAddressing.TileSize;
        var server = new Vector3(1234.5f, -678.25f, 45.0f);

        Vector3 godot = CreatureCoordinates.PositionToGodot(server);
        Vector3 onDisk = WowPlacementTransform.OnDiskPosition(godot);

        Assert.AreApproximatelyEqual(zeroPoint - server.Y, onDisk.X, 0.01, "pos[0] should be ZeroPoint - serverY");
        Assert.AreApproximatelyEqual(server.Z, onDisk.Y, 1e-4, "pos[1] should be serverZ");
        Assert.AreApproximatelyEqual(zeroPoint - server.X, onDisk.Z, 0.01, "pos[2] should be ZeroPoint - serverX");
    }

    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void Orientation_round_trips_for_a_representative_set_of_angles()
    {
        foreach (float radians in new[] { 0.0f, 0.5f, 1.5708f, -1.5708f, 2.5f, -2.5f, 3.0f, -3.0f })
        {
            Basis basis = CreatureCoordinates.OrientationToGodot(radians);
            float back = CreatureCoordinates.OrientationFromGodot(basis);
            Assert.AreApproximatelyEqual(radians, back, 1e-3, $"orientation {radians} did not round-trip");
        }
    }

    // Isolated from CreatureCoordinates.FacingOffset (subtracting it back out before it's re-added
    // inside OrientationToGodot) so this keeps testing the underlying yaw derivation regardless of
    // what that still-unresolved constant is set to — see FacingOffset's own doc comment.
    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void Orientation_matches_the_position_conversion_of_the_facing_vector()
    {
        foreach (float radians in new[] { 0.0f, 0.7f, 1.9f, -1.2f, 2.9f, -2.9f })
        {
            Basis basis = CreatureCoordinates.OrientationToGodot(radians - CreatureCoordinates.FacingOffset);
            Vector3 godotFacing = basis * Vector3.Right;

            var serverFacing = new Vector3(Mathf.Cos(radians), Mathf.Sin(radians), 0.0f);
            Vector3 expected = WowCoordinates.ToGodot(serverFacing);

            Assert.AreApproximatelyEqual(expected.X, godotFacing.X, 1e-3);
            Assert.AreApproximatelyEqual(expected.Y, godotFacing.Y, 1e-3);
            Assert.AreApproximatelyEqual(expected.Z, godotFacing.Z, 1e-3);
        }
    }
}
