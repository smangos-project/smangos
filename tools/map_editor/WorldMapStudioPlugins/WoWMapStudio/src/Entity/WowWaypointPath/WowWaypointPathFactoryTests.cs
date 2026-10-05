namespace WorldMapStudio.WoW;

public static class WowWaypointPathFactoryTests
{
    [EditorTest(Category = "WoW Creature", Thread = TestThread.Background)]
    public static void Persistent_key_is_the_creature_guid_once_saved()
    {
        var factory = new WowWaypointPathFactory(null!);

        Assert.AreEqual(77L, factory.PersistentKey(new WowWaypointPath { CreatureGuid = 77, IsSaved = true }));
        Assert.IsNull(factory.PersistentKey(new WowWaypointPath { CreatureGuid = 77 }));
    }
}
