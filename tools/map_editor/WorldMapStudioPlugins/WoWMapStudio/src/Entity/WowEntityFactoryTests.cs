using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

public static class WowEntityFactoryTests
{
    private static T Factory<T>(EditorContext context, params object[] extra) where T : class
    {
        var storage = (WorldStorage)context.Database.Storages.Single(s => s.Name == WorldStorage.StorageName);
        return (T)System.Activator.CreateInstance(typeof(T), storage)!;
    }

    private static (EditorContext, Transform3D) Setup(string name) =>
        (new EditorContext(new Node3D(), new Project { Name = name }), new Transform3D(Basis.Identity, new Vector3(1, 2, 3)));

    [EditorTest(Category = "WoW Creature", Thread = TestThread.Background)]
    public static void Game_object_create_places_an_entity()
    {
        (EditorContext context, Transform3D transform) = Setup("__wms_go_factory_test__");
        var go = (WowGameObjectSpawn)Factory<WowGameObjectSpawnFactory>(context).Create(context, new MapId(0), transform, "176");

        Assert.AreEqual(176u, go.TemplateEntry);
        Assert.IsTrue(context.Scene.Contains(go));
    }

    [EditorTest(Category = "WoW Creature", Thread = TestThread.Background)]
    public static void Game_tele_create_places_an_entity()
    {
        (EditorContext context, Transform3D transform) = Setup("__wms_tele_factory_test__");
        var tele = (WowGameTele)Factory<WowGameTeleFactory>(context).Create(context, new MapId(0), transform, "Here");

        Assert.AreEqual("Here", tele.Name);
        Assert.IsTrue(context.Scene.Contains(tele));
    }

    [EditorTest(Category = "WoW Creature", Thread = TestThread.Background)]
    public static void Spell_target_create_places_an_entity()
    {
        (EditorContext context, Transform3D transform) = Setup("__wms_spell_factory_test__");
        var target = (WowSpellTargetPosition)Factory<WowSpellTargetPositionFactory>(context).Create(context, new MapId(0), transform, "8690");

        Assert.AreEqual(8690u, target.SpellId);
        Assert.IsTrue(context.Scene.Contains(target));
    }

    [EditorTest(Category = "WoW Creature", Thread = TestThread.Background)]
    public static void Graveyard_create_places_an_entity()
    {
        (EditorContext context, Transform3D transform) = Setup("__wms_yard_factory_test__");
        var yard = (WowGraveyard)Factory<WowGraveyardFactory>(context).Create(context, new MapId(0), transform, "");

        Assert.IsNull(yard.LocId);
        Assert.IsTrue(context.Scene.Contains(yard));
    }

    [EditorTest(Category = "WoW Creature", Thread = TestThread.Background)]
    public static void Taxi_node_create_places_an_entity()
    {
        (EditorContext context, Transform3D transform) = Setup("__wms_taxi_factory_test__");
        var node = (WowTaxiNode)Factory<WowTaxiNodeFactory>(context).Create(context, new MapId(0), transform, "Test");

        Assert.IsNull(node.NodeId);
        Assert.IsTrue(context.Scene.Contains(node));
    }

    [EditorTest(Category = "WoW Creature", Thread = TestThread.Background)]
    public static void Area_trigger_create_places_an_entity()
    {
        (EditorContext context, Transform3D transform) = Setup("__wms_trigger_factory_test__");
        var trigger = (WowAreaTrigger)Factory<WowAreaTriggerFactory>(context).Create(context, new MapId(0), transform, "");

        Assert.IsNull(trigger.TriggerId);
        Assert.IsTrue(context.Scene.Contains(trigger));
    }

    [EditorTest(Category = "WoW Creature", Thread = TestThread.Background)]
    public static void Light_create_places_an_entity()
    {
        (EditorContext context, Transform3D transform) = Setup("__wms_light_factory_test__");
        var light = (WowLight)Factory<WowLightFactory>(context).Create(context, new MapId(0), transform, "");

        Assert.IsNull(light.LightId);
        Assert.IsTrue(context.Scene.Contains(light));
    }

    [EditorTest(Category = "WoW Creature", Thread = TestThread.Background)]
    public static void Sound_emitter_create_places_an_entity()
    {
        (EditorContext context, Transform3D transform) = Setup("__wms_emitter_factory_test__");
        var emitter = (WowSoundEmitter)Factory<WowSoundEmitterFactory>(context).Create(context, new MapId(0), transform, "");

        Assert.IsNull(emitter.EmitterId);
        Assert.IsTrue(context.Scene.Contains(emitter));
    }

    [EditorTest(Category = "WoW Creature", Thread = TestThread.Background)]
    public static void Zone_light_create_places_an_entity()
    {
        (EditorContext context, Transform3D transform) = Setup("__wms_zonelight_factory_test__");
        var zone = (WowZoneLight)Factory<WowZoneLightFactory>(context).Create(context, new MapId(0), transform, "Zone");

        Assert.IsNull(zone.ZoneLightId);
        Assert.AreEqual(4, zone.Component<WowZoneLightComponent>()!.OrderedNodes.Count);
        Assert.IsTrue(context.Scene.Contains(zone));
    }
}
