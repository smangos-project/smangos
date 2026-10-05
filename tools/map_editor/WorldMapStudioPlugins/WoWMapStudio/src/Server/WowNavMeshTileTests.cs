using System;
using System.Buffers.Binary;
using System.IO;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Golden tests over a real generator output, <c>Fixtures/0004731.mmtile</c> (map 0, ADT (31, 47)), with
/// expected values decoded independently of this reader. The tile's own declared size has to equal the sum
/// of the recomputed sections, which is what pins the layout.
/// </summary>
public static class WowNavMeshTileTests
{
    private const string Tile = "0004731.mmtile";

    private static byte[] Fixture(string name)
    {
        string plugin = PluginDirectories.Find("WoWMapStudio") ?? throw new InvalidOperationException("WoWMapStudio plugin directory not found");
        return File.ReadAllBytes(Path.Combine(plugin, "src", "Server", "Fixtures", name));
    }

    [EditorTest(Category = "WoW Server Data", Thread = TestThread.Background)]
    public static void Header_counts_and_bounds_decode()
    {
        WowNavMeshTile tile = WowNavMeshTileReader.Read(Fixture(Tile));

        Assert.IsTrue(tile.UsesLiquids);
        Assert.AreEqual(4350, tile.PolygonCount);
        Assert.AreEqual(6127 * 3, tile.Vertices.Length);
        Assert.AreEqual(4807 * 3, tile.DetailVertices.Length);
        Assert.AreEqual(14241, tile.DetailTriangleTotal);
        Assert.AreEqual(0, tile.OffMeshConnections.Length);

        Assert.AreApproximatelyEqual(0.0, tile.BoundsMin.X, 0.001);
        Assert.AreApproximatelyEqual(28.85, tile.BoundsMin.Y, 0.01);
        Assert.AreApproximatelyEqual(-8533.333, tile.BoundsMin.Z, 0.001);
        Assert.AreApproximatelyEqual(533.333, tile.BoundsMax.X, 0.001);
        Assert.AreApproximatelyEqual(382.12, tile.BoundsMax.Y, 0.01);
        Assert.AreApproximatelyEqual(-8000.0, tile.BoundsMax.Z, 0.001);
        Assert.AreApproximatelyEqual(1.5, tile.WalkableHeight, 0.001);
        Assert.AreApproximatelyEqual(1.8, tile.WalkableClimb, 0.001);
    }

    [EditorTest(Category = "WoW Server Data", Thread = TestThread.Background)]
    public static void Polygon_areas_are_counted_and_only_present_areas_are_listed()
    {
        WowNavMeshTile tile = WowNavMeshTileReader.Read(Fixture(Tile));

        Assert.AreEqual(3262, tile.PolyArea.Count(area => area == WowNavArea.Ground));
        Assert.AreEqual(1088, tile.PolyArea.Count(area => area == WowNavArea.SteepSlope));
        Assert.IsTrue(tile.AreasPresent().SequenceEqual(new[] { WowNavArea.Ground, WowNavArea.SteepSlope }));
        Assert.AreEqual(WowNavArea.SteepSlope, tile.PolyArea[7]);
        Assert.IsTrue(tile.PolyType.All(type => type == 0));
    }

    [EditorTest(Category = "WoW Server Data", Thread = TestThread.Background)]
    public static void Surface_resolves_detail_triangles_through_polygon_and_detail_vertices()
    {
        WowNavMeshTile tile = WowNavMeshTileReader.Read(Fixture(Tile));
        (float[] positions, int[] indices, byte[] areas) = tile.BuildSurface();

        Assert.AreEqual(14241, areas.Length);
        Assert.AreEqual(14241 * 3, indices.Length);
        Assert.IsTrue(indices.All(index => index >= 0 && index * 3 < positions.Length), "every index must address a real vertex");

        // Polygon 0's first detail triangle: two corners from its detail vertices, one from its own.
        Assert.AreEqual(WowNavArea.SteepSlope, areas[0]);
        float[][] expected =
        [
            [0.0f, 238.34599f, -8533.06641f],
            [0.0f, 246.59599f, -8529.23633f],
            [1.2f, 250.59599f, -8528.0f],
        ];
        for (int corner = 0; corner < 3; corner++)
        {
            for (int axis = 0; axis < 3; axis++)
            {
                Assert.AreApproximatelyEqual(expected[corner][axis], positions[(indices[corner] * 3) + axis], 0.001);
            }
        }
    }

    [EditorTest(Category = "WoW Server Data", Thread = TestThread.Background)]
    public static void Recast_space_is_the_server_axes_permuted_and_lands_in_the_tiles_own_file_indices()
    {
        WowNavMeshTile tile = WowNavMeshTileReader.Read(Fixture(Tile));

        var server = new Vector3(-9333f, 1300f, 12f);
        Vector3 recast = WowNavCoordinates.FromServer(server);
        Assert.AreEqual(new Vector3(1300f, 12f, -9333f), recast);
        Assert.AreEqual(server, WowNavCoordinates.ToServer(recast));
        Assert.AreEqual(new Vector3(-9333f, 12f, -1300f), WowNavCoordinates.ToGodot(recast));

        Vector3 centre = WowNavCoordinates.ToServer((tile.BoundsMin + tile.BoundsMax) * 0.5f);
        Assert.AreEqual((31, 47), WowTileAddressing.TileAtServer(centre.X, centre.Y));
    }

    [EditorTest(Category = "WoW Server Data", Thread = TestThread.Background)]
    public static void Navmesh_params_decode()
    {
        WowNavMeshParams navParams = WowNavMeshParams.Read(Fixture("000.mmap"));

        Assert.AreApproximatelyEqual(533.3333, navParams.TileWidth, 0.001);
        Assert.AreApproximatelyEqual(533.3333, navParams.TileHeight, 0.001);
        Assert.AreApproximatelyEqual(-15999.999, navParams.Origin.Z, 0.001);
        Assert.AreEqual(687, navParams.MaxTiles);
        Assert.Throws<WowServerDataException>(() => WowNavMeshParams.Read(new byte[10]));
    }

    [EditorTest(Category = "WoW Server Data", Thread = TestThread.Background)]
    public static void Wrong_versions_magic_and_sizes_are_refused()
    {
        byte[] mmapVersion = Fixture(Tile);
        BinaryPrimitives.WriteUInt32LittleEndian(mmapVersion.AsSpan(8), WowServerFormats.MmapVersion + 1);
        WowServerDataException error = Assert.Throws<WowServerDataException>(() => WowNavMeshTileReader.Read(mmapVersion));
        Assert.IsTrue(error.Message.Contains("Re-extract"), error.Message);

        byte[] dtVersion = Fixture(Tile);
        BinaryPrimitives.WriteUInt32LittleEndian(dtVersion.AsSpan(4), (uint)WowServerFormats.DetourVersion + 1);
        Assert.Throws<WowServerDataException>(() => WowNavMeshTileReader.Read(dtVersion));

        byte[] magic = Fixture(Tile);
        magic[0] ^= 0xff;
        Assert.Throws<WowServerDataException>(() => WowNavMeshTileReader.Read(magic));

        byte[] cut = Fixture(Tile).AsSpan(0, 100_000).ToArray();
        Assert.Throws<WowServerDataException>(() => WowNavMeshTileReader.Read(cut));

        byte[] wrongCount = Fixture(Tile);
        BinaryPrimitives.WriteInt32LittleEndian(wrongCount.AsSpan(20 + 24), 4351);
        Assert.Throws<WowServerDataException>(() => WowNavMeshTileReader.Read(wrongCount), "a section count that no longer sums to the declared size");
    }
}
