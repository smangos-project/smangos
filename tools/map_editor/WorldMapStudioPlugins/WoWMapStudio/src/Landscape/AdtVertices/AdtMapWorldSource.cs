using Godot;

namespace WorldMapStudio.WoW;

/// <summary>What a tool edits on the current map, rebuilt at most twice a second. Null, with a reason in
/// <see cref="Problem"/>, when the map has no imported ADT data of the kind the tool needs.</summary>
public sealed class AdtMapWorldSource
{
    private const double RefreshSeconds = 0.5;

    private readonly EditorContext _context;
    private readonly string _channel;
    private readonly string _missing;

    private AdtMapWorld? _world;
    private double _time = double.NegativeInfinity;

    public AdtMapWorldSource(EditorContext context, string channel, string missing)
    {
        _context = context;
        _channel = channel;
        _missing = missing;
    }

    public string? Problem { get; private set; }

    public AdtMapWorld? Get()
    {
        double now = Time.GetTicksMsec() / 1000.0;
        if (now - _time >= RefreshSeconds)
        {
            _world = AdtMapWorld.FromEditor(_context, _context.Maps.CurrentMap, out string? problem);
            if (_world != null && _world.PlacementFor(_channel) == null)
            {
                _world = null;
                problem = _missing;
            }

            Problem = problem;
            _time = now;
        }

        return _world;
    }

    /// <summary>Forgets the cached world, so the next <see cref="Get"/> rebuilds it.</summary>
    public void Invalidate() => _time = double.NegativeInfinity;
}
