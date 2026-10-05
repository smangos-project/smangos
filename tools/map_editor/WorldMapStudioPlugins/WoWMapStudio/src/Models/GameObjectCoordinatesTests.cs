using Godot;

namespace WorldMapStudio.WoW;

public static class GameObjectCoordinatesTests
{
    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void Rotation_round_trips_through_godot_space()
    {
        var wow = new Quaternion(new Vector3(0.2f, 0.9f, 0.1f).Normalized(), Mathf.DegToRad(64.0f));
        Basis godot = GameObjectCoordinates.RotationToGodot(wow);
        Quaternion back = GameObjectCoordinates.RotationFromGodot(godot);

        Assert.AreApproximatelyEqual(wow.X, back.X, 1e-3);
        Assert.AreApproximatelyEqual(wow.Y, back.Y, 1e-3);
        Assert.AreApproximatelyEqual(wow.Z, back.Z, 1e-3);
        Assert.AreApproximatelyEqual(wow.W, back.W, 1e-3);
    }

    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void Identity_rotation_round_trips_to_identity()
    {
        Basis godot = GameObjectCoordinates.RotationToGodot(Quaternion.Identity);
        Quaternion back = GameObjectCoordinates.RotationFromGodot(godot);

        Assert.AreApproximatelyEqual(0.0, back.X, 1e-4);
        Assert.AreApproximatelyEqual(0.0, back.Y, 1e-4);
        Assert.AreApproximatelyEqual(0.0, back.Z, 1e-4);
        Assert.AreApproximatelyEqual(1.0, back.W, 1e-4);
    }
}
