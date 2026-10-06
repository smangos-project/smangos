using System.Collections.Generic;
using Godot;

namespace WorldMapStudio.WoW;

public static class WmoModelWriterTests
{
    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Two_surfaces_round_trip_through_the_real_readers()
    {
        ModelSurfaceInput a = Quad(Vector3.Zero, 11, "opaque", true);
        ModelSurfaceInput b = Quad(new Vector3(5, 0, 0), 22, "alpha", false);

        (byte[] rootBytes, byte[] groupBytes) = WmoModelWriter.Write([a, b], 777);

        WmoRoot? root = WmoRootReader.Read(rootBytes, "test.wmo");
        Assert.IsNotNull(root);
        Assert.AreEqual(1, root!.GroupCount);
        Assert.AreEqual(2, root.Materials.Length);
        Assert.IsTrue(root.Materials[0].Flags.TwoSided);
        Assert.IsFalse(root.Materials[1].Flags.TwoSided);

        WmoGroup? group = WmoGroupReader.Read(groupBytes, root.Lighting);
        Assert.IsNotNull(group);
        Assert.AreEqual(8, group!.Vertices.Length);
        Assert.AreEqual(12, group.Indices.Length);
        Assert.AreEqual(2, group.Batches.Length);
        Assert.AreEqual((byte)1, group.Batches[1].MaterialIndex);
        Assert.AreEqual(6u, group.Batches[1].IndexStart);
        Assert.AreEqual((ushort)4, group.Batches[1].VertexStart);
        Assert.AreApproximatelyEqual(a.Uvs[2].X, group.Uvs[2].X, 1e-3);
    }

    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Bsp_covers_every_face_and_splits_large_meshes()
    {
        var vertices = new List<Vector3>();
        var indices = new List<int>();
        for (int i = 0; i < 600; i++)
        {
            int v = vertices.Count;
            vertices.Add(new Vector3(i, 0, 0));
            vertices.Add(new Vector3(i + 1, 0, 0));
            vertices.Add(new Vector3(i, 1, 0));
            indices.AddRange([v, v + 1, v + 2]);
        }

        (List<WmoBspNode> nodes, List<ushort> faces) = WmoBspBuilder.Build(vertices, indices);

        Assert.Greater(nodes.Count, 1);
        var seen = new HashSet<ushort>(faces);
        Assert.AreEqual(600, seen.Count);
        foreach (WmoBspNode node in nodes)
        {
            if (node.Flags == WmoBspBuilder.LeafFlag)
            {
                Assert.IsTrue(node.FaceCount <= 128 || node.FaceCount > 0);
                Assert.IsTrue(node.FaceStart + node.FaceCount <= faces.Count);
            }
        }
    }

    private static ModelSurfaceInput Quad(Vector3 origin, uint fdid, string blend, bool twoSided) => new(
        new List<Vector3> { origin, origin + Vector3.Right, origin + new Vector3(1, 0, -1), origin + Vector3.Forward },
        new List<Vector3> { Vector3.Up, Vector3.Up, Vector3.Up, Vector3.Up },
        new List<Vector2> { new(0, 0), new(1, 0), new(1, 1), new(0, 1) },
        new List<int> { 0, 1, 2, 0, 2, 3 },
        fdid, blend, twoSided, false, true);
}
