using Godot;

namespace WorldMapStudio.WoW;

public static class WowPlacementTransformTests
{
    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Position_matches_a_real_mddf_entry_from_the_ground_truth_tile()
    {
        // Entry [0] of the real Azeroth_32_49.adt's MDDF, raw on-disk floats — see
        // WowTileAddressing's ground truth. This tile's own godot bounds are X in
        // [-9600, -9066.667], Z in [0, 533.333] (derived and pinned in WowTileAddressingTests).
        var onDisk = new Vector3(17290.86f, 57.77f, 26562.11f);

        Vector3 godot = WowPlacementTransform.FromOnDiskPosition(onDisk);

        Assert.AreApproximatelyEqual(-9495.44, godot.X, 0.5, "godot X should land inside the tile's own X range");
        Assert.AreApproximatelyEqual(57.77, godot.Y, 0.01, "height passes straight through");
        Assert.AreApproximatelyEqual(224.19, godot.Z, 0.5, "godot Z should land inside the tile's own Z range");
        Assert.IsTrue(godot.X is >= -9600f and <= -9066.667f, $"X={godot.X} outside the tile's own bounds");
        Assert.IsTrue(godot.Z is >= 0f and <= 533.333f, $"Z={godot.Z} outside the tile's own bounds");
    }

    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Position_round_trips()
    {
        var godot = new Vector3(-9200.0f, 88.5f, 300.0f);
        Vector3 onDisk = WowPlacementTransform.OnDiskPosition(godot);
        Vector3 back = WowPlacementTransform.FromOnDiskPosition(onDisk);

        Assert.AreApproximatelyEqual(godot.X, back.X, 1e-3);
        Assert.AreApproximatelyEqual(godot.Y, back.Y, 1e-3);
        Assert.AreApproximatelyEqual(godot.Z, back.Z, 1e-3);
    }

    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Rotation_round_trips_for_a_representative_set_of_angles()
    {
        // Small tilts plus a wide range of yaw — the shape real placements actually have (see the
        // ground-truth sample: rot[0]/rot[2] single-digit degrees, rot[1] spanning 0-360).
        foreach (Vector3 degrees in new[]
                 {
                     Vector3.Zero,
                     new Vector3(0, 90, 0),
                     new Vector3(0, 261.9f, 0),
                     new Vector3(3.0f, -185.0f, 11.5f),
                     new Vector3(5.5f, 0, -9.0f),
                 })
        {
            Basis basis = WowPlacementTransform.FromOnDiskRotationDegrees(degrees);
            Vector3 back = WowPlacementTransform.OnDiskRotationDegrees(basis);
            Basis basisAgain = WowPlacementTransform.FromOnDiskRotationDegrees(back);

            // Compare the reconstructed BASIS, not the raw degrees — an Euler triple is not unique
            // (multiple triples describe the same orientation), but the physical orientation is.
            Assert.AreApproximatelyEqual(basis.X.X, basisAgain.X.X, 1e-3);
            Assert.AreApproximatelyEqual(basis.X.Y, basisAgain.X.Y, 1e-3);
            Assert.AreApproximatelyEqual(basis.X.Z, basisAgain.X.Z, 1e-3);
            Assert.AreApproximatelyEqual(basis.Y.X, basisAgain.Y.X, 1e-3);
            Assert.AreApproximatelyEqual(basis.Y.Y, basisAgain.Y.Y, 1e-3);
            Assert.AreApproximatelyEqual(basis.Y.Z, basisAgain.Y.Z, 1e-3);
        }
    }

    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Zero_rotation_round_trips_through_the_yaw_offset()
    {
        Basis basis = WowPlacementTransform.FromOnDiskRotationDegrees(Vector3.Zero);
        Vector3 degrees = WowPlacementTransform.OnDiskRotationDegrees(basis);
        Assert.AreApproximatelyEqual(0.0, degrees.X, 1e-3);
        Assert.AreApproximatelyEqual(0.0, degrees.Y, 1e-3);
        Assert.AreApproximatelyEqual(0.0, degrees.Z, 1e-3);

        // rot[1] == 180 is the one that leaves the model unrotated, so an unrotated placement is
        // NOT an all-zero rot[] triple. Pins the offset itself, which a round-trip alone can't.
        Basis half = WowPlacementTransform.FromOnDiskRotationDegrees(new Vector3(0, 180, 0));
        Assert.AreApproximatelyEqual(1.0, half.X.X, 1e-4, "rot[1]=180 should be the identity orientation");
        Assert.AreApproximatelyEqual(1.0, half.Y.Y, 1e-4);
        Assert.AreApproximatelyEqual(1.0, half.Z.Z, 1e-4);
    }

    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Rotation_reproduces_the_world_extents_real_placements_store()
    {
        // A MODF record carries the placement's own world-space AABB, so a real one is a self-checking
        // sample: rotate the referenced WMO's model-space bounds by whatever rot[] means, translate by
        // pos[], and the result has to land back on the stored extents. Only this convention does —
        // every quarter-turn variant misses HUMAN_BARRACKS' box by tens of units.
        //
        // Both taken from real 4.3.4 client tiles (Azeroth_30_49 and Azeroth_31_41); the model bounds
        // are the min/max of the WMO's own MOVT vertices. ANIMALDEN carries non-zero rot[0]/rot[2],
        // so it pins the pitch/roll axes and their order as well as the yaw.
        AssertExtents(
            name: "HUMAN_BARRACKS",
            localMin: new Vector3(-67.9450f, -32.6755f, -16.0690f),
            localMax: new Vector3(8.3901f, 32.6439f, 44.4389f),
            position: new Vector3(16382.24805f, 40.55330f, 26722.69727f),
            rotation: new Vector3(0.0f, 336.50f, 0.0f),
            expectedLower: new Vector3(16348.90039f, 24.45654f, 26647.32031f),
            expectedUpper: new Vector3(16439.31250f, 85.01997f, 26743.44336f));

        AssertExtents(
            name: "ANIMALDEN",
            localMin: new Vector3(-5.2266f, -3.9732f, -0.2911f),
            localMax: new Vector3(2.3599f, 3.9874f, 2.9100f),
            position: new Vector3(16960.87109f, 392.33804f, 22305.63672f),
            rotation: new Vector3(-2.0f, 340.0f, -3.5f),
            expectedLower: new Vector3(16956.28320f, 391.59158f, 22299.29492f),
            expectedUpper: new Vector3(16966.63477f, 395.59601f, 22309.26172f));
    }

    /// <summary>Walks a placement the whole way the exporter does — on-disk rot/pos back into Godot
    /// space, model bounds through <see cref="WowCoordinates"/>, then the corners back on disk — and
    /// checks the resulting extents against the ones the file itself stores.</summary>
    private static void AssertExtents(string name, Vector3 localMin, Vector3 localMax, Vector3 position, Vector3 rotation, Vector3 expectedLower, Vector3 expectedUpper)
    {
        Basis basis = WowPlacementTransform.FromOnDiskRotationDegrees(rotation);
        Vector3 origin = WowPlacementTransform.FromOnDiskPosition(position);

        var lower = new Vector3(float.MaxValue, float.MaxValue, float.MaxValue);
        var upper = new Vector3(float.MinValue, float.MinValue, float.MinValue);
        for (int corner = 0; corner < 8; corner++)
        {
            var local = new Vector3(
                (corner & 1) == 0 ? localMin.X : localMax.X,
                (corner & 2) == 0 ? localMin.Y : localMax.Y,
                (corner & 4) == 0 ? localMin.Z : localMax.Z);
            Vector3 onDisk = WowPlacementTransform.OnDiskPosition(origin + (basis * WowCoordinates.ToGodot(local)));
            lower = new Vector3(Mathf.Min(lower.X, onDisk.X), Mathf.Min(lower.Y, onDisk.Y), Mathf.Min(lower.Z, onDisk.Z));
            upper = new Vector3(Mathf.Max(upper.X, onDisk.X), Mathf.Max(upper.Y, onDisk.Y), Mathf.Max(upper.Z, onDisk.Z));
        }

        // Tolerance covers the model bounds being the vertex AABB rather than whatever Blizzard's own
        // tool measured; the next-best convention is off by more than a unit even on the small models.
        Assert.AreApproximatelyEqual(expectedLower.X, lower.X, 0.1, $"{name} lower[0]");
        Assert.AreApproximatelyEqual(expectedLower.Y, lower.Y, 0.1, $"{name} lower[1]");
        Assert.AreApproximatelyEqual(expectedLower.Z, lower.Z, 0.1, $"{name} lower[2]");
        Assert.AreApproximatelyEqual(expectedUpper.X, upper.X, 0.1, $"{name} upper[0]");
        Assert.AreApproximatelyEqual(expectedUpper.Y, upper.Y, 0.1, $"{name} upper[1]");
        Assert.AreApproximatelyEqual(expectedUpper.Z, upper.Z, 0.1, $"{name} upper[2]");

        Vector3 back = WowPlacementTransform.OnDiskRotationDegrees(basis);
        Assert.AreApproximatelyEqual(0.0, Mathf.Wrap(back.Y - rotation.Y, -180.0f, 180.0f), 1e-2, $"{name} rot[1] round-trip");
    }

    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Scale_encodes_as_1024_fixed_point()
    {
        var uniform = Basis.Identity.Scaled(new Vector3(2.5f, 2.5f, 2.5f));
        ushort scale = WowPlacementTransform.OnDiskScale(uniform);
        Assert.AreEqual((ushort)(2.5f * 1024.0f), scale);
    }
}
