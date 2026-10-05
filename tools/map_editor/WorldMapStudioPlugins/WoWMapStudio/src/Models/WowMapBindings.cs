using System;
using System.Data.Common;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Godot;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>
/// One project map's explicit binding to a WoW map id — the many-to-one replacement for guessing the
/// WoW map id from <see cref="Map.Name"/>. No row means the project map is unbound. Several project
/// maps may bind to the same <see cref="WowMapId"/> (splitting one continent across project maps); see
/// <see cref="WowMapBindings"/>.
/// </summary>
public sealed class WowMapBindingRecord
{
    /// <summary>The project map's id — <see cref="MapId.Value"/>.</summary>
    public int ProjectMap { get; set; }

    /// <summary><c>dbc_map.ID</c>.</summary>
    public int WowMapId { get; set; }
}

/// <summary>A project map resolved to the WoW map it is bound to: the numeric id every extracted file is
/// named by, and the <c>dbc_map</c> directory its ADTs live in.</summary>
public readonly record struct WowMapBinding(MapId Editor, int WowMapId, string Directory);

[Subsystem(nameof(EditorStorage))]
public sealed class WowMapBindingTableConfiguration : ITableConfiguration, IMapScopedData
{
    private readonly EditorStorage _storage;

    // Keyed directly on the project map, not through an entity — see IMapScopedData's priority convention.
    public float Priority => 1.0f;

    public WowMapBindingTableConfiguration(EditorStorage storage)
    {
        _storage = storage;
    }

    public void Configure(ModelBuilder model)
    {
        model.Entity<WowMapBindingRecord>(entity =>
        {
            entity.ToTable("wms_wow_map_bindings");
            entity.HasKey(record => record.ProjectMap);
            entity.Property(record => record.ProjectMap).ValueGeneratedNever();
        });
    }

    string IMapScopedData.Label => "WoW map binding";

    Task<int> IMapScopedData.CountAsync(EditorDbContext context, MapId map) =>
        _storage.CountWhereMapAsync<WowMapBindingRecord>(context, nameof(WowMapBindingRecord.ProjectMap), map);

    async Task<IReadOnlySet<int>> IMapScopedData.MapIdsAsync(EditorDbContext context) =>
        (await context.Set<WowMapBindingRecord>().AsNoTracking().Select(record => record.ProjectMap).Distinct().ToListAsync().ConfigureAwait(false))
            .ToHashSet();

    Task IMapScopedData.DeleteAsync(EditorDbContext context, DbTransaction transaction, MapId map) =>
        _storage.DeleteWhereMapAsync<WowMapBindingRecord>(context, transaction, nameof(WowMapBindingRecord.ProjectMap), map);
}

/// <summary>
/// Resolves a project map's explicit WoW map id binding (<see cref="WowMapBindingRecord"/>) — the
/// replacement for guessing it from <see cref="Map.Name"/>. Loaded lazily on first use and reloaded
/// whenever <see cref="MapSystem.Version"/> changes (map list edits, world reloads); subsystems are
/// constructed once and never rebuilt for the process's lifetime, so there is nothing to hook for an
/// explicit "world unloaded" drop.
///
/// WoW-side rows (creature, gameobject, dbc_light, graveyards, …) are keyed on the WoW map id, so every
/// project map bound to that id sees the same rows. Project-side data (terrain, scene entities, paint
/// images) stays per project map. An unbound map loads no WoW rows and refuses WoW writes — the same
/// behaviour a name matching nothing had before this existed.
/// </summary>
[Subsystem(nameof(EditorStorage))]
public sealed class WowMapBindings : ISubsystem
{
    private readonly EditorContext _context;
    private readonly EditorStorage _storage;

    private const string BackfillOperationId = "wow-map-bindings";
    private const string BackfillKey = "name-backfill-v1";

    private bool _loaded;
    private bool _backfillChecked;
    private int _loadedVersion = -1;
    private Dictionary<int, int> _cache = new();

    public WowMapBindings(EditorStorage storage)
    {
        _storage = storage;
        _context = storage.Context;
    }

    /// <summary>The registered instance for <paramref name="context"/> — reachable both from a
    /// storage-backed factory and from
    /// plain <see cref="EditorContext"/> code, e.g. the properties section.</summary>
    public static WowMapBindings For(EditorContext context) =>
        context.Database.EditorStorage.WowMapBindings;

    /// <summary>The WoW map id <paramref name="map"/> is bound to, or null if unbound.</summary>
    public int? Resolve(MapId map)
    {
        EnsureLoaded();
        return _cache.TryGetValue(map.Value, out int wowMapId) ? wowMapId : null;
    }

    /// <summary>The distinct WoW map ids any project map is bound to, in id order.</summary>
    public IReadOnlyList<int> BoundWowMaps()
    {
        EnsureLoaded();
        return _cache.Values.Distinct().Order().ToList();
    }

    /// <summary>Every project map bound to <paramref name="wowMapId"/> — the reverse lookup export
    /// conflict checks and the properties section's "also bound to" note need.</summary>
    public IReadOnlyList<MapId> ProjectMapsFor(int wowMapId)
    {
        EnsureLoaded();
        return _cache.Where(pair => pair.Value == wowMapId).Select(pair => new MapId(pair.Key)).ToList();
    }

    /// <summary>
    /// <see cref="Resolve"/> plus the bound WoW map's <see cref="DbcMap.Directory"/>. Null when
    /// <paramref name="map"/> is unbound or its WoW map id has no <c>dbc_map</c> row with a directory —
    /// <see cref="BindingProblem"/> says which.
    /// </summary>
    public WowMapBinding? ResolveFull(MapId map)
    {
        if (Resolve(map) is not int wowMapId)
        {
            return null;
        }

        return _context.Catalog.OfType<DbcMap>().FirstOrDefault(row => row.MapId == wowMapId) is { Directory.Length: > 0 } dbcMap
            ? new WowMapBinding(map, wowMapId, dbcMap.Directory)
            : null;
    }

    /// <summary>
    /// The client folder <paramref name="map"/>'s tiles are written into: the bound WoW map's own
    /// <see cref="DbcMap.Directory"/>, so several project maps bound to one WoW map share a folder.
    /// Falls back to a sanitized <see cref="Map.DisplayName"/> when <see cref="ResolveFull"/> has no answer.
    /// </summary>
    public string DirectoryFor(MapId map) => ResolveFull(map)?.Directory ?? FallbackDirectory(map);

    /// <summary>
    /// The one "this map has no usable WoW binding" problem, for every consumer to report under its own
    /// scope. Null when <see cref="ResolveFull"/> succeeds.
    /// </summary>
    public Problem? BindingProblem(MapId map)
    {
        if (ResolveFull(map) != null)
        {
            return null;
        }

        string cause = Resolve(map) is int wowMapId
            ? $"is bound to WoW map {wowMapId}, which has no dbc_map directory"
            : "has no WoW map binding";

        return new Problem
        {
            Key = $"NoWowMapBinding:{map.Value}",
            Severity = ProblemSeverity.Warning,
            Category = "WoW Map Binding",
            Kind = "NoWowMapBinding",
            Message = $"Map '{DisplayNameOf(map)}' (id {map.Value}) {cause}; it exports to folder '{FallbackDirectory(map)}' and has no server data preview.",
        };
    }

    private string DisplayNameOf(MapId map) =>
        _context.Maps.Maps.FirstOrDefault(candidate => candidate.Id == map)?.DisplayName ?? $"Map{map.Value}";

    private string FallbackDirectory(MapId map)
    {
        string name = DisplayNameOf(map);
        foreach (char invalid in Path.GetInvalidFileNameChars())
        {
            name = name.Replace(invalid, '_');
        }

        return name.Length == 0 ? $"Map{map.Value}" : name;
    }

    /// <summary>
    /// Binds or clears <paramref name="map"/>'s WoW map id. Reloads the world when <paramref name="map"/>
    /// is the current one — every factory has already streamed rows for the old id, and invalidating the
    /// streaming system alone would not drop them. Throws with the blocker text instead of reloading
    /// while an edit session is open on the current map, the same guard <see cref="MapSystem.DeleteBlocker"/>
    /// applies to deleting a map outright.
    /// </summary>
    public async Task SetAsync(MapId map, int? wowMapId)
    {
        bool isCurrent = map == _context.Maps.CurrentMap;
        if (isCurrent && _context.Operations.Blocker is { } blocker)
        {
            throw new InvalidOperationException(blocker);
        }

        EnsureLoaded();

        if (wowMapId is { } value)
        {
            await UpsertAsync(map.Value, value).ConfigureAwait(false);
            _cache[map.Value] = value;
        }
        else
        {
            await DeleteRowAsync(map.Value).ConfigureAwait(false);
            _cache.Remove(map.Value);
        }

        if (isCurrent)
        {
            _context.RequestReload("WoW map id changed");
        }
    }

    private void EnsureLoaded()
    {
        int version = _context.Maps.Version;
        if (_loaded && _loadedVersion == version)
        {
            return;
        }

        _cache = BlockingWork.Run(LoadWithBackfillAsync);
        _loaded = true;
        _loadedVersion = version;
    }

    private async Task<Dictionary<int, int>> LoadWithBackfillAsync()
    {
        if (!_backfillChecked)
        {
            await BackfillAsync().ConfigureAwait(false);
            _backfillChecked = true;
        }

        return await LoadAllAsync().ConfigureAwait(false);
    }

    // Existing projects only ever resolved a project map's WoW map id by matching Map.Name against
    // dbc_map. Without this, upgrading a project with no explicit bindings would silently lose every
    // WoW row (creatures, lights, area table, …) on every one of its maps. Runs once ever, guarded by
    // a BatchState marker rather than "no rows yet" — so a binding the user deliberately cleared is
    // never re-created by the next load.
    private async Task BackfillAsync()
    {
        if (await _storage.LoadBatchStateAsync(BackfillOperationId, BackfillKey).ConfigureAwait(false) != null)
        {
            return;
        }

        Dictionary<int, int> existing = await LoadAllAsync().ConfigureAwait(false);
        List<DbcMap> catalog = _context.Catalog.OfType<DbcMap>().ToList();

        foreach (Map map in _context.Maps.Maps)
        {
            if (existing.ContainsKey(map.Id.Value) || LegacyNameMatch(map.Name, catalog) is not { } wowMapId)
            {
                continue;
            }

            await UpsertAsync(map.Id.Value, wowMapId).ConfigureAwait(false);
            GD.Print($"[WoW] Bound map '{map.Name}' (id {map.Id.Value}) to WoW map {wowMapId} by name (one-time migration)");
        }

        await _storage.UpsertBatchStateAsync(BackfillOperationId, BackfillKey, "true").ConfigureAwait(false);
    }

    // Matches the open map's name against dbc_map's ADT folder name or localized name,
    // case-insensitive — the guess this binding replaces, kept only for the one-time backfill above.
    private static int? LegacyNameMatch(string name, List<DbcMap> catalog)
    {
        if (string.IsNullOrWhiteSpace(name))
        {
            return null;
        }

        DbcMap? match = catalog.FirstOrDefault(map =>
            string.Equals(map.Directory, name, StringComparison.OrdinalIgnoreCase)
            || string.Equals(map.MapName, name, StringComparison.OrdinalIgnoreCase));
        return match?.MapId;
    }

    private async Task<Dictionary<int, int>> LoadAllAsync()
    {
        using IDisposable read = await _storage.Lock.ReaderAsync().ConfigureAwait(false);
        await using EditorDbContext context = _storage.CreateContext();
        return await context.Set<WowMapBindingRecord>().AsNoTracking()
            .ToDictionaryAsync(record => record.ProjectMap, record => record.WowMapId).ConfigureAwait(false);
    }

    private async Task UpsertAsync(int projectMap, int wowMapId)
    {
        using IDisposable write = await _storage.Lock.WriterAsync().ConfigureAwait(false);
        await using EditorDbContext context = _storage.CreateContext();

        WowMapBindingRecord? existing = await context.Set<WowMapBindingRecord>().FindAsync(projectMap).ConfigureAwait(false);
        if (existing == null)
        {
            context.Set<WowMapBindingRecord>().Add(new WowMapBindingRecord { ProjectMap = projectMap, WowMapId = wowMapId });
        }
        else
        {
            existing.WowMapId = wowMapId;
        }

        await context.SaveChangesAsync().ConfigureAwait(false);
    }

    private async Task DeleteRowAsync(int projectMap)
    {
        using IDisposable write = await _storage.Lock.WriterAsync().ConfigureAwait(false);
        await using EditorDbContext context = _storage.CreateContext();

        if (await context.Set<WowMapBindingRecord>().FindAsync(projectMap).ConfigureAwait(false) is { } existing)
        {
            context.Set<WowMapBindingRecord>().Remove(existing);
            await context.SaveChangesAsync().ConfigureAwait(false);
        }
    }
}
