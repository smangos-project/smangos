using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Covers <see cref="WowMapBindings"/> against a real (freshly provisioned) project storage —
/// the binding table has no live game data dependency, unlike most WoW factories' DB-bound members, so
/// this can exercise the actual read/write path rather than stopping at pure logic.</summary>
public static class WowMapBindingsTests
{
    /// <summary>A context whose editor tables live in a throwaway database, not the shared world file.</summary>
    private static EditorContext OpenContext(string name)
    {
        string databasePath = Path.Combine(Path.GetTempPath(), $"{name}_{Guid.NewGuid():N}.doltlite");
        var context = new EditorContext(new Node3D(), new Project { Name = name });
        context.Database.EditorStorage.Location.DatabasePath = databasePath;
        context.Database.EditorStorage.EnsureSchema();
        return context;
    }

    [EditorTest(Category = "WoW Map Bindings")]
    public static void Set_then_resolve_round_trips()
    {
        var context = OpenContext("__wms_wow_map_bindings_set_test__");
        WowMapBindings bindings = WowMapBindings.For(context);
        var map = new MapId(3);

        Assert.IsNull(bindings.Resolve(map), "a fresh project has no bindings yet");

        BlockingWork.Run(() => bindings.SetAsync(map, 0));
        Assert.AreEqual(0, bindings.Resolve(map));
    }

    [EditorTest(Category = "WoW Map Bindings")]
    public static void Clear_removes_the_binding()
    {
        var context = OpenContext("__wms_wow_map_bindings_clear_test__");
        WowMapBindings bindings = WowMapBindings.For(context);
        var map = new MapId(4);

        BlockingWork.Run(() => bindings.SetAsync(map, 1));
        Assert.AreEqual(1, bindings.Resolve(map));

        BlockingWork.Run(() => bindings.SetAsync(map, null));
        Assert.IsNull(bindings.Resolve(map));
    }

    [EditorTest(Category = "WoW Map Bindings")]
    public static void Two_project_maps_can_share_one_wow_map_id()
    {
        var context = OpenContext("__wms_wow_map_bindings_shared_test__");
        WowMapBindings bindings = WowMapBindings.For(context);
        var first = new MapId(5);
        var second = new MapId(6);

        BlockingWork.Run(() => bindings.SetAsync(first, 2));
        BlockingWork.Run(() => bindings.SetAsync(second, 2));

        Assert.AreEqual(2, bindings.Resolve(first));
        Assert.AreEqual(2, bindings.Resolve(second));

        IReadOnlyList<MapId> both = bindings.ProjectMapsFor(2);
        Assert.AreEqual(2, both.Count);
        Assert.IsTrue(both.Contains(first));
        Assert.IsTrue(both.Contains(second));

        // Clearing one must leave the other's binding alone.
        BlockingWork.Run(() => bindings.SetAsync(first, null));
        IReadOnlyList<MapId> remaining = bindings.ProjectMapsFor(2);
        Assert.AreEqual(1, remaining.Count);
        Assert.IsTrue(remaining.Contains(second));
        Assert.IsNull(bindings.Resolve(first));
    }

    [EditorTest(Category = "WoW Map Bindings")]
    public static void Renaming_a_bound_map_does_not_change_its_resolution()
    {
        var context = OpenContext("__wms_wow_map_bindings_rename_test__");
        Map map = context.Maps.Create(context.Maps.NextFreeId(), "Original Name", out _)!;
        WowMapBindings bindings = WowMapBindings.For(context);

        BlockingWork.Run(() => bindings.SetAsync(map.Id, 7));
        Assert.AreEqual(7, bindings.Resolve(map.Id));

        // Rename bumps MapSystem.Version, which is what forces WowMapBindings to reload from storage,
        // but a rename must not touch the resolved id.
        context.Maps.Rename(map, "Renamed Map");
        Assert.AreEqual(7, bindings.Resolve(map.Id), "renaming must not change what the map resolves to");
    }

    [EditorTest(Category = "WoW Map Bindings")]
    public static void Backfill_binds_by_name_once_and_a_cleared_binding_does_not_come_back()
    {
        var context = OpenContext("__wms_wow_map_bindings_backfill_test__");
        context.Catalog.Add(new DbcMap { MapId = 0, Directory = "Azeroth", MapName = "Azeroth" });
        context.Catalog.Add(new DbcMap { MapId = 1, Directory = "Kalimdor", MapName = "Kalimdor" });

        Map azeroth = context.Maps.Create(context.Maps.NextFreeId(), "Azeroth", out _)!;
        Map foo = context.Maps.Create(context.Maps.NextFreeId(), "Foo", out _)!;

        WowMapBindings bindings = WowMapBindings.For(context);

        Assert.AreEqual(0, bindings.Resolve(azeroth.Id), "the one-time backfill should bind by name on first use");
        Assert.IsNull(bindings.Resolve(foo.Id), "no dbc_map row named 'Foo', so it should stay unbound");

        BlockingWork.Run(() => bindings.SetAsync(azeroth.Id, null));
        context.Maps.Rename(foo, "Foo2"); // bumps MapSystem.Version, forcing a reload from storage
        Assert.IsNull(bindings.Resolve(azeroth.Id), "a deliberately cleared binding must not be re-created by the next load");
    }

    [EditorTest(Category = "WoW Map Bindings")]
    public static void ResolveFull_pairs_the_wow_map_id_with_its_dbc_map_directory()
    {
        var context = OpenContext("__wms_wow_map_bindings_full_test__");
        context.Catalog.Add(new DbcMap { MapId = 1, Directory = "Kalimdor", MapName = "Kalimdor" });
        Map map = context.Maps.Create(context.Maps.NextFreeId(), "My Continent", out _)!;
        WowMapBindings bindings = WowMapBindings.For(context);

        BlockingWork.Run(() => bindings.SetAsync(map.Id, 1));

        WowMapBinding? full = bindings.ResolveFull(map.Id);
        Assert.IsTrue(full.HasValue);
        Assert.AreEqual(1, full.Value.WowMapId);
        Assert.AreEqual("Kalimdor", full.Value.Directory);
        Assert.AreEqual(map.Id, full.Value.Editor);
        Assert.AreEqual("Kalimdor", bindings.DirectoryFor(map.Id));
        Assert.IsNull(bindings.BindingProblem(map.Id), "a fully resolved binding has nothing to report");
    }

    [EditorTest(Category = "WoW Map Bindings")]
    public static void Unbound_map_falls_back_to_a_sanitized_folder_and_reports_one_problem()
    {
        var context = OpenContext("__wms_wow_map_bindings_unbound_test__");
        Map map = context.Maps.Create(context.Maps.NextFreeId(), "My/Map", out _)!;
        WowMapBindings bindings = WowMapBindings.For(context);

        Assert.IsFalse(bindings.ResolveFull(map.Id).HasValue);
        Assert.AreEqual("My_Map", bindings.DirectoryFor(map.Id));

        Problem? problem = bindings.BindingProblem(map.Id);
        Assert.IsNotNull(problem);
        Assert.AreEqual("NoWowMapBinding", problem.Kind);
        Assert.AreEqual(bindings.BindingProblem(map.Id)!.Key, problem.Key, "the same map must always report the same key");
    }

    [EditorTest(Category = "WoW Map Bindings")]
    public static void Bound_to_an_id_with_no_dbc_map_row_still_falls_back_and_says_why()
    {
        var context = OpenContext("__wms_wow_map_bindings_norow_test__");
        Map map = context.Maps.Create(context.Maps.NextFreeId(), "Custom", out _)!;
        WowMapBindings bindings = WowMapBindings.For(context);

        BlockingWork.Run(() => bindings.SetAsync(map.Id, 9999));

        Assert.AreEqual(9999, bindings.Resolve(map.Id));
        Assert.IsFalse(bindings.ResolveFull(map.Id).HasValue);
        Assert.AreEqual("Custom", bindings.DirectoryFor(map.Id));
        Assert.IsTrue(bindings.BindingProblem(map.Id)!.Message.Contains("9999"));
    }
}
