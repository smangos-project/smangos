using System.Collections.Generic;
using Godot;

namespace WorldMapStudio.WoW;

public static class M2ModelWriterTests
{
    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Single_surface_round_trips_through_the_real_readers()
    {
        M2SurfaceInput surface = Quad(new Vector3(1, 2, 3), 4242, "alpha", twoSided: true, unlit: false, depthWrite: true);

        (byte[] m2Bytes, byte[] skinBytes) = M2ModelWriter.Write("TestPanel", [surface]);

        M2Document? document = M2Reader.Read(m2Bytes, "test.m2");
        Assert.IsNotNull(document);
        Assert.AreEqual(4, document!.Vertices.Length);
        for (int i = 0; i < 4; i++)
        {
            Assert.AreApproximatelyEqual(surface.Positions[i].X, document.Vertices[i].Position.X, 1e-3);
            Assert.AreApproximatelyEqual(surface.Positions[i].Y, document.Vertices[i].Position.Y, 1e-3);
            Assert.AreApproximatelyEqual(surface.Positions[i].Z, document.Vertices[i].Position.Z, 1e-3);
            Assert.AreApproximatelyEqual(surface.Uvs[i].X, document.Vertices[i].Uv.X, 1e-3);
            Assert.AreApproximatelyEqual(surface.Uvs[i].Y, document.Vertices[i].Uv.Y, 1e-3);
        }

        Assert.AreEqual(1, document.Textures.Length);
        Assert.AreEqual(4242u, document.Textures[0].FileDataId);
        Assert.IsTrue(document.Materials[0].Flags.TwoSided);
        Assert.IsTrue(document.Materials[0].Flags.DepthWrite);
        Assert.AreEqual((ushort)2, document.Materials[0].Blend);

        M2Skin? skin = M2Reader.ReadSkin(skinBytes, "test.skin");
        Assert.IsNotNull(skin);
        Assert.AreEqual(4, skin!.VertexLookup.Length);
        Assert.AreEqual(6, skin.Indices.Length);
        Assert.AreEqual(1, skin.Submeshes.Length);
        Assert.AreEqual(4, skin.Submeshes[0].VertexCount);
        Assert.AreEqual(6, skin.Submeshes[0].IndexCount);
        Assert.AreEqual(1, skin.Batches.Length);
    }

    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Two_surfaces_get_separate_materials_textures_and_submeshes()
    {
        M2SurfaceInput a = Quad(Vector3.Zero, 1, "opaque", false, false, true);
        M2SurfaceInput b = Quad(new Vector3(5, 0, 0), 2, "add", false, true, false);

        (byte[] m2Bytes, byte[] skinBytes) = M2ModelWriter.Write("TwoMats", [a, b]);

        M2Document document = M2Reader.Read(m2Bytes, "test.m2")!;
        Assert.AreEqual(8, document.Vertices.Length);
        Assert.AreEqual(2u, document.Textures[1].FileDataId);
        Assert.IsTrue(document.Materials[1].Flags.Unlit);
        Assert.IsFalse(document.Materials[1].Flags.DepthWrite);
        Assert.AreEqual((ushort)4, document.Materials[1].Blend);

        M2Skin skin = M2Reader.ReadSkin(skinBytes, "test.skin")!;
        Assert.AreEqual(2, skin.Submeshes.Length);
        Assert.AreEqual(4, skin.Submeshes[1].VertexStart);
        Assert.AreEqual(6u, skin.Submeshes[1].IndexStart);
        Assert.AreEqual((ushort)4 + 0, (ushort)(skin.Indices[6]));
    }

    private static M2SurfaceInput Quad(Vector3 origin, uint fdid, string blend, bool twoSided, bool unlit, bool depthWrite) => new(
        new List<Vector3> { origin, origin + Vector3.Right, origin + new Vector3(1, 0, -1), origin + Vector3.Forward },
        new List<Vector3> { Vector3.Up, Vector3.Up, Vector3.Up, Vector3.Up },
        new List<Vector2> { new(0, 0), new(1, 0), new(1, 1), new(0, 1) },
        new List<int> { 0, 1, 2, 0, 2, 3 },
        fdid, blend, twoSided, unlit, depthWrite);
}
