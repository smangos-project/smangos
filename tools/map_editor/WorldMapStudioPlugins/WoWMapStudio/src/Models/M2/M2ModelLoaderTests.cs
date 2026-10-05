namespace WorldMapStudio.WoW;

public static class M2ModelLoaderTests
{
    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void Batches_sharing_a_geoset_id_group_together()
    {
        // Two submeshes both tagged geoset 0 (a face split across two material batches, say), one
        // tagged geoset 100 (a hairstyle) — the shape a real character model actually has.
        M2Skin skin = Skin(
            submeshes: [Submesh(id: 0), Submesh(id: 0), Submesh(id: 100)],
            batches: [Batch(section: 0), Batch(section: 1), Batch(section: 2)]);

        var groups = M2ModelLoader.GroupBatchesByGeoset(skin);

        Assert.AreEqual(2, groups.Count);
        Assert.AreEqual((ushort)0, groups[0].GeosetId);
        Assert.AreEqual(2, groups[0].BatchIndices.Count, "both geoset-0 batches should land in one group");
        Assert.AreEqual((ushort)100, groups[1].GeosetId);
        Assert.AreEqual(1, groups[1].BatchIndices.Count);
    }

    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void Groups_preserve_first_seen_order_not_numeric_order()
    {
        M2Skin skin = Skin(
            submeshes: [Submesh(id: 300), Submesh(id: 0), Submesh(id: 100)],
            batches: [Batch(section: 0), Batch(section: 1), Batch(section: 2)]);

        var groups = M2ModelLoader.GroupBatchesByGeoset(skin);

        Assert.AreEqual((ushort)300, groups[0].GeosetId);
        Assert.AreEqual((ushort)0, groups[1].GeosetId);
        Assert.AreEqual((ushort)100, groups[2].GeosetId);
    }

    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void A_single_geoset_id_produces_a_single_group_with_every_batch_index()
    {
        // The common case for a plain creature: everything is geoset 0.
        M2Skin skin = Skin(
            submeshes: [Submesh(id: 0), Submesh(id: 0), Submesh(id: 0)],
            batches: [Batch(section: 0), Batch(section: 1), Batch(section: 2)]);

        var groups = M2ModelLoader.GroupBatchesByGeoset(skin);

        Assert.AreEqual(1, groups.Count);
        Assert.AreEqual(3, groups[0].BatchIndices.Count);
        Assert.AreEqual(0, groups[0].BatchIndices[0]);
        Assert.AreEqual(1, groups[0].BatchIndices[1]);
        Assert.AreEqual(2, groups[0].BatchIndices[2]);
    }

    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void A_batch_pointing_past_the_submesh_list_is_skipped()
    {
        M2Skin skin = Skin(
            submeshes: [Submesh(id: 0)],
            batches: [Batch(section: 0), Batch(section: 5)]);

        var groups = M2ModelLoader.GroupBatchesByGeoset(skin);

        Assert.AreEqual(1, groups.Count);
        Assert.AreEqual(1, groups[0].BatchIndices.Count);
        Assert.AreEqual(0, groups[0].BatchIndices[0]);
    }

    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void A_batch_bound_to_a_rest_transparent_colour_is_hidden()
    {
        // colour block 1's rest opacity is 0 — the blink eyelid mesh. A batch with no colour block
        // (index -1) or one pointing at an opaque block stays visible.
        float[] restAlpha = [1f, 0f];

        Assert.IsTrue(M2ModelLoader.BatchIsRestHidden(restAlpha, Batch(section: 0, colorIndex: 1)));
        Assert.IsFalse(M2ModelLoader.BatchIsRestHidden(restAlpha, Batch(section: 0, colorIndex: 0)));
        Assert.IsFalse(M2ModelLoader.BatchIsRestHidden(restAlpha, Batch(section: 0)), "no colour block");
    }

    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void A_colour_index_past_the_rest_alpha_list_leaves_the_batch_visible()
    {
        Assert.IsFalse(M2ModelLoader.BatchIsRestHidden([1f], Batch(section: 0, colorIndex: 7)));
        Assert.IsFalse(M2ModelLoader.BatchIsRestHidden([], Batch(section: 0, colorIndex: 0)));
    }

    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void Reads_a_client_creature_model_and_its_skin()
    {
        string file = WowTestContent.RequireFile("creature/boar/boar.m2");
        M2Document? document = M2Reader.Read(System.IO.File.ReadAllBytes(file), file);

        Assert.IsNotNull(document);
        Assert.Greater(document!.Vertices.Length, 0);
        Assert.Greater(document.Textures.Length, 0);
        Assert.Greater(document.SkinFileDataIds.Length, 0);

        string skinFile = System.IO.Path.ChangeExtension(file, null) + "00.skin";
        M2Skin? skin = M2Reader.ReadSkin(System.IO.File.ReadAllBytes(skinFile), skinFile);
        Assert.IsNotNull(skin);
        Assert.Greater(skin!.Indices.Length, 0);
        Assert.Greater(skin.Batches.Length, 0);
        foreach (ushort local in skin.VertexLookup)
        {
            Assert.IsTrue(local < document.Vertices.Length);
        }
    }

    private static M2SkinSubmesh Submesh(ushort id) => new(id, VertexStart: 0, VertexCount: 0, IndexStart: 0, IndexCount: 0);

    private static M2SkinBatch Batch(ushort section, short colorIndex = -1) =>
        new(section, MaterialIndex: 0, TextureComboIndex: 0, ColorIndex: colorIndex);

    private static M2Skin Skin(M2SkinSubmesh[] submeshes, M2SkinBatch[] batches) => new()
    {
        VertexLookup = [],
        Indices = [],
        Submeshes = submeshes,
        Batches = batches,
    };
}
