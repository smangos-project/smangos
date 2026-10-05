using Godot;

namespace WorldMapStudio.WoW;

public static class PlacementMapperTests
{
    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void Invert_transform_recovers_the_editor_transform_from_the_export_formula()
    {
        Basis rotation = Basis.FromEuler(new Vector3(0.3f, 1.1f, -0.7f));
        const float scale = 1.75f;
        Basis basis = rotation * Basis.FromScale(new Vector3(scale, scale, scale));
        var centre = new Vector3(2.0f, -3.0f, 4.5f);
        var transform = new Transform3D(basis, new Vector3(-9100.0f, 42.0f, 350.0f));

        // What WowPlacementScan writes to disk for this placement.
        Vector3 onDiskPosition = WowPlacementTransform.OnDiskPosition(transform * -centre);
        Vector3 onDiskRotation = WowPlacementTransform.OnDiskRotationDegrees(transform.Basis);
        ushort onDiskScale = WowPlacementTransform.OnDiskScale(transform.Basis);

        Transform3D recovered = PlacementMapper.InvertTransform(onDiskPosition, onDiskRotation, onDiskScale, centre);

        Assert.AreApproximatelyEqual(transform.Origin.X, recovered.Origin.X, 1.0, "origin X");
        Assert.AreApproximatelyEqual(transform.Origin.Y, recovered.Origin.Y, 1.0, "origin Y");
        Assert.AreApproximatelyEqual(transform.Origin.Z, recovered.Origin.Z, 1.0, "origin Z");

        Assert.AreApproximatelyEqual(scale, recovered.Basis.Scale.X, 1e-2, "uniform scale");

        Vector3 forward = (transform.Basis * Vector3.Forward).Normalized();
        Vector3 recoveredForward = (recovered.Basis * Vector3.Forward).Normalized();
        Assert.AreApproximatelyEqual(1.0, forward.Dot(recoveredForward), 1e-2, "rotation preserved");
    }
}
