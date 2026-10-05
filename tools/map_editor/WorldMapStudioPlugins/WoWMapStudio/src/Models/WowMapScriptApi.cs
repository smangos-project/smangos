using System.Linq;
using System.Threading.Tasks;

namespace WorldMapStudio.WoW;

/// <summary>One project-map-to-WoW-map binding, as JS sees it.</summary>
public sealed class WowMapBindingDescriptor
{
    internal WowMapBindingDescriptor(int projectMap, int wowMapId)
    {
        ProjectMap = projectMap;
        WowMapId = wowMapId;
    }

    [ScriptProperty]
    public int ProjectMap { get; }

    [ScriptProperty]
    public int WowMapId { get; }
}

/// <summary>
/// Reads and writes <see cref="WowMapBindings"/>, exposed to JS as <c>wms.wowMap</c> — the scripted
/// counterpart of the Map Properties "WoW Map" section (<see cref="WowMapIdSection"/>).
/// </summary>
[Subsystem(nameof(ScriptingSystem))]
public sealed class WowMapScriptApi : IScriptModule
{
    private readonly EditorContext _context;

    public string Name => "wowMap";

    public WowMapScriptApi(ScriptingSystem system)
    {
        _context = system.Context;
    }

    /// <summary>The WoW map id a project map is bound to, or null when it is unbound.</summary>
    [ScriptFunction]
    public int? Get(int projectMap) => WowMapBindings.For(_context).Resolve(new MapId(projectMap));

    /// <summary>Binds or clears a project map's WoW map id. Throws with the blocker text if refused —
    /// see <see cref="WowMapBindings.SetAsync"/>.</summary>
    [ScriptFunction]
    public Task Set(int projectMap, int? wowMapId) =>
        WowMapBindings.For(_context).SetAsync(new MapId(projectMap), wowMapId);

    /// <summary>Every project map bound to a WoW map id.</summary>
    [ScriptFunction]
    public int[] ProjectMapsFor(int wowMapId) =>
        WowMapBindings.For(_context).ProjectMapsFor(wowMapId).Select(id => id.Value).ToArray();

    /// <summary>Every bound project map and the WoW map id it resolves to — unbound maps are left out.</summary>
    [ScriptFunction]
    public WowMapBindingDescriptor[] List()
    {
        WowMapBindings bindings = WowMapBindings.For(_context);
        return _context.Maps.Maps
            .Select(map => (Map: map, WowMapId: bindings.Resolve(map.Id)))
            .Where(pair => pair.WowMapId is not null)
            .Select(pair => new WowMapBindingDescriptor(pair.Map.Id.Value, pair.WowMapId!.Value))
            .ToArray();
    }
}
