using Godot;

namespace WorldMapStudio.WoW;

public static class WowAreaTriggerVolumeTests
{
    // Same test as ObjectMgr::IsPointInAreaTriggerZone for box triggers.
    private static bool ServerContains(float length, float width, float height, float yaw, Vector3 offset)
    {
        double rotation = 2 * System.Math.PI - yaw;
        double sin = System.Math.Sin(rotation);
        double cos = System.Math.Cos(rotation);
        double dx = offset.X * cos - offset.Y * sin;
        double dy = offset.Y * cos + offset.X * sin;
        return System.Math.Abs(dx) <= length / 2 && System.Math.Abs(dy) <= width / 2 && System.Math.Abs(offset.Z) <= height / 2;
    }

    [EditorTest(Category = "WoW Entities", Thread = TestThread.Background)]
    public static void Box_volume_matches_server_containment()
    {
        const float length = 12.0f, width = 4.0f, height = 6.0f;
        var size = new Vector3(length, height, width);
        int mismatches = 0;
        foreach (float yaw in new[] { 0.0f, 0.6f, 2.2f, 4.9f })
        {
            Basis inverse = WowAreaTriggerVolume.BoxBasis(yaw).Inverse();
            for (float x = -8.0f; x <= 8.0f; x += 0.5f)
                for (float y = -8.0f; y <= 8.0f; y += 0.5f)
                {
                    var offset = new Vector3(x, y, 1.0f);
                    Vector3 local = (inverse * WowCoordinates.ToGodot(offset)).Abs();
                    bool editor = local.X <= size.X / 2 + 1e-3f && local.Y <= size.Y / 2 + 1e-3f && local.Z <= size.Z / 2 + 1e-3f;
                    bool server = ServerContains(length, width, height, yaw, offset);
                    bool nearEdge = System.Math.Abs(local.X - size.X / 2) < 0.01f || System.Math.Abs(local.Z - size.Z / 2) < 0.01f;
                    if (editor != server && !nearEdge)
                        mismatches++;
                }
        }
        Assert.AreEqual(0, mismatches);
    }
}
