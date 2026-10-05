using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Golden tests over real assembler output in <c>Fixtures/vmaps</c>: a small tile of three doodads, one holding a
/// single WMO and one doodad rotated on all three axes, with their models. Expected values were decoded
/// independently of these readers.
/// </summary>
public static class WowServerCollisionTests
{
    private const string TreeModel = "FILE00031B1B.m2";
    private const string BuildingModel = "FILE0001A6A1.wmo";
    private const string TiltedModel = "FILE0002E695.m2";

    private static string Vmaps()
    {
        string plugin = PluginDirectories.Find("WoWMapStudio") ?? throw new InvalidOperationException("WoWMapStudio plugin directory not found");
        return Path.Combine(plugin, "src", "Server", "Fixtures", "vmaps");
    }

    private static IReadOnlyList<WowVmapSpawn> Tile(string name) => WowVmapTileReader.Read(File.ReadAllBytes(Path.Combine(Vmaps(), name)));

    private static WowWorldModel Model(string name) => WowWorldModelReader.Read(File.ReadAllBytes(WowServerModelCache.PathOf(Vmaps(), name)));

    [EditorTest(Category = "WoW Server Collision", Thread = TestThread.Background)]
    public static void A_vmtile_decodes_its_spawns_with_their_names()
    {
        IReadOnlyList<WowVmapSpawn> spawns = Tile("000_31_40.vmtile");

        Assert.AreEqual(3, spawns.Count);
        WowVmapSpawn first = spawns[0];
        Assert.AreEqual(TreeModel, first.Name);
        Assert.AreEqual(102913u, first.Id);
        Assert.AreEqual(102912u, first.TreeIndex);
        Assert.AreEqual(5u, first.Flags);
        Assert.IsTrue(first.IsM2);
        Assert.AreEqual(1.01171875f, first.Scale);
        Assert.AreEqual(new Vector3(21363.50390625f, 17089.916015625f, 13.068999290466309f), first.Position);
        Assert.AreEqual(new Vector3(0f, 31.696533203125f, 0f), first.Rotation);
        Assert.AreEqual(new Vector3(21360.15234375f, 17085.671875f, 11.687124252319336f), first.Bound!.Value.Low);
        Assert.AreEqual(new Vector3(21366.814453125f, 17092.048828125f, 55.601619720458984f), first.Bound!.Value.High);
        Assert.AreEqual(2, spawns.Select(spawn => spawn.Name).Distinct().Count());

        WowVmapSpawn wmo = Tile("000_29_44.vmtile").Single();
        Assert.IsFalse(wmo.IsM2);
        Assert.AreEqual(BuildingModel, wmo.Name);
        Assert.AreEqual(new Vector3(0f, 40.5f, 0f), wmo.Rotation);
    }

    [EditorTest(Category = "WoW Server Collision", Thread = TestThread.Background)]
    public static void A_vmo_decodes_its_groups_vertices_and_triangles()
    {
        WowWorldModel tree = Model(TreeModel);
        Assert.AreEqual(1, tree.Groups.Count);
        Assert.AreEqual(25, tree.VertexCount);
        Assert.AreEqual(42, tree.Groups[0].Triangles.Length / 3);
        Assert.AreApproximatelyEqual(-0.44842, tree.Groups[0].Vertices[0], 0.0001);
        Assert.AreApproximatelyEqual(-3.45043, tree.Groups[0].Vertices[1], 0.0001);
        Assert.AreApproximatelyEqual(3.53933, tree.Groups[0].Vertices[2], 0.0001);
        Assert.IsTrue(tree.Groups[0].Triangles.Take(3).SequenceEqual(new[] { 0, 2, 1 }));

        WowWorldModel building = Model(BuildingModel);
        Assert.AreEqual(729u, building.RootWmoId);
        Assert.AreEqual(1, building.Groups.Count);
        Assert.AreEqual(560, building.VertexCount);
        Assert.AreEqual(492, building.Groups[0].Triangles.Length / 3);
        Assert.AreEqual(0x8809u, building.Groups[0].MogpFlags);
        Assert.AreEqual(3445u, building.Groups[0].GroupWmoId);
    }

    [EditorTest(Category = "WoW Server Collision", Thread = TestThread.Background)]
    public static void Unique_edges_are_counted_once_across_shared_triangle_sides()
    {
        Assert.AreEqual(66, Model(TreeModel).Wireframe.Edges.Length / 2);
        Assert.AreEqual(999, Model(BuildingModel).Wireframe.Edges.Length / 2);
        Assert.AreEqual(25 * 3, Model(TreeModel).Wireframe.Vertices.Length);
    }

    private static void AssertReproducesBound(WowVmapSpawn spawn, WowWorldModel model)
    {
        var transform = new WowVmapTransform(spawn);
        var min = new Vector3(float.MaxValue, float.MaxValue, float.MaxValue);
        var max = new Vector3(float.MinValue, float.MinValue, float.MinValue);
        foreach (WowWorldModelGroup group in model.Groups)
        {
            for (int i = 0; i < group.Vertices.Length; i += 3)
            {
                Vector3 vertex = transform.ToVmap(group.Vertices[i], group.Vertices[i + 1], group.Vertices[i + 2]);
                min = new Vector3(Math.Min(min.X, vertex.X), Math.Min(min.Y, vertex.Y), Math.Min(min.Z, vertex.Z));
                max = new Vector3(Math.Max(max.X, vertex.X), Math.Max(max.Y, vertex.Y), Math.Max(max.Z, vertex.Z));
            }
        }

        // The assembler bounds a spawn by placing every vertex of its model, so the same vertices through the
        // same transform must land on the spawn's stored bound.
        Vector3 expectedLow = spawn.Bound!.Value.Low;
        Vector3 expectedHigh = spawn.Bound!.Value.High;
        foreach ((float expected, float actual) in new[]
        {
            (expectedLow.X, min.X), (expectedLow.Y, min.Y), (expectedLow.Z, min.Z),
            (expectedHigh.X, max.X), (expectedHigh.Y, max.Y), (expectedHigh.Z, max.Z),
        })
        {
            Assert.AreApproximatelyEqual(expected, actual, 0.01, spawn.Name);
        }
    }

    [EditorTest(Category = "WoW Server Collision", Thread = TestThread.Background)]
    public static void The_transform_reproduces_every_stored_doodad_bound()
    {
        foreach (WowVmapSpawn spawn in Tile("000_31_40.vmtile"))
        {
            AssertReproducesBound(spawn, Model(spawn.Name));
        }
    }

    [EditorTest(Category = "WoW Server Collision", Thread = TestThread.Background)]
    public static void The_transform_reproduces_a_bound_under_a_rotation_on_all_three_axes()
    {
        WowVmapSpawn spawn = Tile("000_32_25.vmtile").Single();
        Assert.AreEqual(TiltedModel, spawn.Name);
        Assert.AreEqual(new Vector3(75.49996948242188f, 156.0f, -74.49995422363281f), spawn.Rotation);
        AssertReproducesBound(spawn, Model(TiltedModel));
    }

    [EditorTest(Category = "WoW Server Collision", Thread = TestThread.Background)]
    public static void Rotation_and_the_vmap_to_server_mirror_follow_the_convention()
    {
        // A yaw of 90 degrees turns the model's +X into the frame's +Y.
        var yaw = new WowVmapTransform(Vector3.Zero, new Vector3(0f, 90f, 0f), 1f);
        Vector3 turned = yaw.ToVmap(1f, 0f, 0f);
        Assert.AreApproximatelyEqual(0.0, turned.X, 1e-5);
        Assert.AreApproximatelyEqual(1.0, turned.Y, 1e-5);

        // Scale applies before the rotation, the position after it.
        var scaled = new WowVmapTransform(new Vector3(10f, 20f, 30f), Vector3.Zero, 2f);
        Assert.AreEqual(new Vector3(12f, 24f, 36f), scaled.ToVmap(1f, 2f, 3f));

        // The vmap frame is the server's mirrored about the map's centre, each axis on its own.
        Vector3 server = WowVmapTransform.VmapToServer(new Vector3(WowVmapTransform.MapHalfSize + 8913.23f, WowVmapTransform.MapHalfSize - 554.63f, 94f));
        Assert.AreApproximatelyEqual(-8913.23, server.X, 0.01);
        Assert.AreApproximatelyEqual(554.63, server.Y, 0.01);
        Assert.AreApproximatelyEqual(94.0, server.Z, 0.001);
    }

    [EditorTest(Category = "WoW Server Collision", Thread = TestThread.Background)]
    public static void A_point_in_stormwind_lands_in_the_vmtile_the_server_names_for_it()
    {
        // Stormwind is ADT Azeroth_30_48 and the server loads its collision from 000_30_48.vmtile.
        Vector3 vmap = new(WowVmapTransform.MapHalfSize + 8913.23f, WowVmapTransform.MapHalfSize - 554.63f, 94f);
        Vector3 server = WowVmapTransform.VmapToServer(vmap);
        (int fileX, int fileY) = WowTileAddressing.TileAtServer(server.X, server.Y);
        Assert.AreEqual((30, 48), (fileX, fileY));
        Assert.AreEqual("000_30_48.vmtile", WowMapGrid.VmapTileFileName(0, fileX, fileY));
    }

    [EditorTest(Category = "WoW Server Collision", Thread = TestThread.Background)]
    public static void Every_spawn_of_a_tile_starts_within_one_tile_of_it()
    {
        // 000_31_40 is fileX 31, fileY 40. The assembler also lists spawns that sit just across a tile's edge.
        foreach (WowVmapSpawn spawn in Tile("000_31_40.vmtile"))
        {
            Vector3 server = WowVmapTransform.VmapToServer(spawn.Position);
            (int fileX, int fileY) = WowTileAddressing.TileAtServer(server.X, server.Y);
            Assert.IsTrue(Math.Abs(fileX - 31) <= 1 && Math.Abs(fileY - 40) <= 1, spawn.Name);
        }
    }

    [EditorTest(Category = "WoW Server Collision", Thread = TestThread.Background)]
    public static void The_mesh_draws_every_placed_model_and_skips_the_ones_that_are_missing()
    {
        WowServerModelCache.Clear();
        IReadOnlyList<WowVmapSpawn> spawns = Tile("000_31_40.vmtile");

        WowServerCollisionMesh.Result all = WowServerCollisionMesh.Build(spawns, name => WowServerModelCache.Get(Vmaps(), name));
        Assert.AreEqual(3, all.Drawn);
        Assert.AreEqual(0, all.MissingModels);
        Assert.AreEqual(91, all.Mesh!.Vertices.Length, "each placement carries its own copy of the model's vertices");
        Assert.AreEqual((66 + 114 + 66) * 2, all.Mesh.Indices!.Length);
        Assert.AreEqual(Mesh.PrimitiveType.Lines, all.Mesh.Primitive);
        Assert.IsTrue(all.Mesh.Colors.All(color => color == WowServerCollisionMesh.M2Color));

        WowServerCollisionMesh.Result withoutTree = WowServerCollisionMesh.Build(
            spawns, name => name == TreeModel ? null : WowServerModelCache.Get(Vmaps(), name));
        Assert.AreEqual(2, withoutTree.MissingModels);
        Assert.AreEqual(1, withoutTree.Drawn);

        WowServerCollisionMesh.Result wmo = WowServerCollisionMesh.Build(Tile("000_29_44.vmtile"), name => WowServerModelCache.Get(Vmaps(), name));
        Assert.IsTrue(wmo.Mesh!.Colors.All(color => color == WowServerCollisionMesh.WmoColor), "a world model reads apart from a doodad");
    }

    [EditorTest(Category = "WoW Server Collision", Thread = TestThread.Background)]
    public static void Placed_vertices_land_near_the_spawns_in_godot_space()
    {
        WowServerModelCache.Clear();
        WowServerMeshData mesh = WowServerCollisionMesh.Build(Tile("000_31_40.vmtile"), name => WowServerModelCache.Get(Vmaps(), name)).Mesh!;

        // Spawns sit at server X about -4300 and server Y between -23 and 57; Godot X is server X, Z is minus server Y.
        Assert.IsTrue(mesh.Vertices.All(v => v.X > -4400f && v.X < -4200f), "Godot X is server X");
        Assert.IsTrue(mesh.Vertices.All(v => v.Z > -100f && v.Z < 50f), "Godot Z is minus server Y");
    }

    [EditorTest(Category = "WoW Server Collision", Thread = TestThread.Background)]
    public static void The_model_cache_shares_a_model_and_rereads_it_when_the_file_changes()
    {
        WowServerModelCache.Clear();
        string directory = Path.Combine(Path.GetTempPath(), $"wms-model-cache-{Guid.NewGuid():N}");
        try
        {
            string path = WowServerModelCache.PathOf(directory, "a.m2");
            Directory.CreateDirectory(directory);

            Assert.IsNull(WowServerModelCache.Get(directory, "a.m2"), "a model with no file is absent");

            File.Copy(WowServerModelCache.PathOf(Vmaps(), TreeModel), path);
            WowWorldModel? first = WowServerModelCache.Get(directory, "a.m2");
            Assert.IsNotNull(first, "a model that appears is picked up");
            Assert.IsTrue(ReferenceEquals(first, WowServerModelCache.Get(directory, "a.m2")), "an unchanged model is read once");

            File.Copy(WowServerModelCache.PathOf(Vmaps(), TiltedModel), path, true);
            WowWorldModel? second = WowServerModelCache.Get(directory, "a.m2");
            Assert.IsFalse(ReferenceEquals(first, second), "a changed model is read again");
            Assert.AreEqual(Model(TiltedModel).VertexCount, second!.VertexCount);
        }
        finally
        {
            Directory.Delete(directory, true);
            WowServerModelCache.Clear();
        }
    }

    [EditorTest(Category = "WoW Server Collision", Thread = TestThread.Background)]
    public static void Other_versions_truncation_and_bad_chunks_are_refused()
    {
        byte[] tile = File.ReadAllBytes(Path.Combine(Vmaps(), "000_31_40.vmtile"));
        byte[] old = (byte[])tile.Clone();
        old[7] = (byte)'6';
        WowServerDataException error = Assert.Throws<WowServerDataException>(() => WowVmapTileReader.Read(old));
        Assert.IsTrue(error.Message.Contains("VMAP_7.6") && error.Message.Contains("Re-extract"), error.Message);

        Assert.Throws<WowServerDataException>(() => WowVmapTileReader.Read(tile.AsSpan(0, tile.Length - 5).ToArray()));
        Assert.Throws<WowServerDataException>(() => WowVmapTileReader.Read(new byte[] { 1, 2, 3 }));

        byte[] model = File.ReadAllBytes(WowServerModelCache.PathOf(Vmaps(), TreeModel));
        byte[] oldModel = (byte[])model.Clone();
        oldModel[7] = (byte)'6';
        Assert.Throws<WowServerDataException>(() => WowWorldModelReader.Read(oldModel));
        Assert.Throws<WowServerDataException>(() => WowWorldModelReader.Read(model.AsSpan(0, model.Length / 2).ToArray()));

        byte[] badChunk = (byte[])model.Clone();
        badChunk[8] = (byte)'X';
        Assert.Throws<WowServerDataException>(() => WowWorldModelReader.Read(badChunk));
    }
}
