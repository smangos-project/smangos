using Godot;

namespace WorldMapStudio.WoW;

public static class WowCoordinatesTests
{
    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void Position_round_trips_through_godot_space()
    {
        var wow = new Vector3(120.5f, -340.25f, 88.0f);
        Vector3 godot = WowCoordinates.ToGodot(wow);
        Vector3 back = WowCoordinates.FromGodot(godot);

        Assert.AreApproximatelyEqual(wow.X, back.X, 1e-4);
        Assert.AreApproximatelyEqual(wow.Y, back.Y, 1e-4);
        Assert.AreApproximatelyEqual(wow.Z, back.Z, 1e-4);
    }

    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void Bounds_round_trip_through_godot_space()
    {
        var wowMin = new Vector3(-100.0f, -50.0f, 10.0f);
        var wowMax = new Vector3(200.0f, 75.0f, 40.0f);

        Aabb godot = WowCoordinates.TransformBounds(wowMin, wowMax);
        (Vector3 min, Vector3 max) = WowCoordinates.FromGodotBounds(godot);

        Assert.AreApproximatelyEqual(wowMin.X, min.X, 1e-4);
        Assert.AreApproximatelyEqual(wowMin.Y, min.Y, 1e-4);
        Assert.AreApproximatelyEqual(wowMin.Z, min.Z, 1e-4);
        Assert.AreApproximatelyEqual(wowMax.X, max.X, 1e-4);
        Assert.AreApproximatelyEqual(wowMax.Y, max.Y, 1e-4);
        Assert.AreApproximatelyEqual(wowMax.Z, max.Z, 1e-4);
    }

    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void Bounds_round_trip_from_godot_space()
    {
        var godot = new Aabb(new Vector3(-500.0f, 0.0f, 20.0f), new Vector3(150.0f, 60.0f, 300.0f));

        (Vector3 wowMin, Vector3 wowMax) = WowCoordinates.FromGodotBounds(godot);
        Aabb back = WowCoordinates.TransformBounds(wowMin, wowMax);

        Assert.AreApproximatelyEqual(godot.Position.X, back.Position.X, 1e-4);
        Assert.AreApproximatelyEqual(godot.Position.Y, back.Position.Y, 1e-4);
        Assert.AreApproximatelyEqual(godot.Position.Z, back.Position.Z, 1e-4);
        Assert.AreApproximatelyEqual(godot.End.X, back.End.X, 1e-4);
        Assert.AreApproximatelyEqual(godot.End.Y, back.End.Y, 1e-4);
        Assert.AreApproximatelyEqual(godot.End.Z, back.End.Z, 1e-4);
    }

    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void Quaternion_round_trips_through_godot_space()
    {
        var wow = new Quaternion(new Vector3(0.3f, 0.7f, -0.2f).Normalized(), Mathf.DegToRad(53.0f));
        Quaternion godot = WowCoordinates.ToGodot(wow);
        Quaternion back = WowCoordinates.FromGodot(godot);

        Assert.AreApproximatelyEqual(wow.X, back.X, 1e-4);
        Assert.AreApproximatelyEqual(wow.Y, back.Y, 1e-4);
        Assert.AreApproximatelyEqual(wow.Z, back.Z, 1e-4);
        Assert.AreApproximatelyEqual(wow.W, back.W, 1e-4);
    }

    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void Quaternion_transform_agrees_with_the_vector_transform()
    {
        // What actually matters beyond the round trip above is that this agrees with ToGodot(Vector3)
        // — rotating in WoW space then converting the result must equal converting first and rotating
        // by the converted quaternion in Godot space.
        var wow = new Quaternion(new Vector3(1, 0, 0), Mathf.DegToRad(37.0f));
        Quaternion godot = WowCoordinates.ToGodot(wow);

        Vector3 sample = new(1, 2, 3);
        Vector3 rotated = wow * sample;
        Vector3 godotRotated = WowCoordinates.ToGodot(rotated);
        Vector3 fromConvertedBasis = godot * WowCoordinates.ToGodot(sample);

        Assert.AreApproximatelyEqual(godotRotated.X, fromConvertedBasis.X, 1e-3);
        Assert.AreApproximatelyEqual(godotRotated.Y, fromConvertedBasis.Y, 1e-3);
        Assert.AreApproximatelyEqual(godotRotated.Z, fromConvertedBasis.Z, 1e-3);
    }
}
