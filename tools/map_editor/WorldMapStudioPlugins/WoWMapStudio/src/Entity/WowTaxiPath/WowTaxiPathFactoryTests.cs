namespace WorldMapStudio.WoW;

/// <summary>Covers the DB-free parts of <see cref="WowTaxiPathFactory"/>. <c>ScanAsync</c>/<c>Stage</c>
/// need live <c>dbc_taxipath</c> tables and follow <c>WowCreatureSpawnFactory</c>'s no-dedicated-test
/// precedent.</summary>
public static class WowTaxiPathFactoryTests
{
    [EditorTest(Category = "WoW Taxi", Thread = TestThread.Background)]
    public static void Spawn_kind_matches_what_wms_spawn_would_list()
    {
        Assert.AreEqual("Taxi Path", new WowTaxiPathFactory(null!).SpawnKind);
    }

    [EditorTest(Category = "WoW Taxi", Thread = TestThread.Background)]
    public static void Persistent_key_folds_path_id_and_run_index_without_colliding()
    {
        var factory = new WowTaxiPathFactory(null!);

        long? run0 = factory.PersistentKey(new WowTaxiPath { PathId = 285, RunIndex = 0, IsSaved = true });
        long? run1 = factory.PersistentKey(new WowTaxiPath { PathId = 285, RunIndex = 1, IsSaved = true });
        long? otherPath = factory.PersistentKey(new WowTaxiPath { PathId = 286, RunIndex = 0, IsSaved = true });

        Assert.AreNotEqual(run0, run1, "two runs of the same path must not collide");
        Assert.AreNotEqual(run0, otherPath, "a different path's first run must not collide either");
        Assert.IsNull(factory.PersistentKey(new WowTaxiPath { PathId = 285, RunIndex = 0 }), "an unsaved path has no key yet");
    }
}
