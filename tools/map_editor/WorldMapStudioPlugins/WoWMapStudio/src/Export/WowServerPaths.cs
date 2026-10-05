using System;
using System.IO;

namespace WorldMapStudio.WoW;

/// <summary>
/// Where the project keeps the data the extractor built for the server, read from the project's named
/// paths. The mirror image of <see cref="WowClientPaths"/>.
/// </summary>
[Subsystem(nameof(EditorContext))]
public sealed class WowServerPaths : ISubsystem
{
    /// <summary>The project path that holds <see cref="Data"/>.</summary>
    public const string DataKey = "wow.serverData";

    /// <summary>The project path that holds <see cref="ExtractorExe"/>.</summary>
    public const string ExtractorExeKey = "wow.extractorExe";

    private readonly EditorContext _context;

    public WowServerPaths(EditorContext context)
    {
        _context = context;
    }

    /// <summary>
    /// The worldserver's own <c>DataDir</c> (<c>.server/data</c>), so the editor reads the exact bytes
    /// the running server loads. Null when the project does not set <see cref="DataKey"/>.
    /// </summary>
    public string? Data =>
        _context.Project.Paths.TryGetValue(DataKey, out string? path) && path.Length > 0 ? path : null;

    /// <summary><c>.map</c> files.</summary>
    public string? Maps => Sub("maps");

    /// <summary><c>.vmtree</c>, <c>.vmtile</c> and <c>.vmo</c> files.</summary>
    public string? Vmaps => Sub("vmaps");

    /// <summary><c>.mmap</c> and <c>.mmtile</c> files.</summary>
    public string? Mmaps => Sub("mmaps");

    /// <summary>The three directories previews read, resolved so they can be handed to a worker thread.</summary>
    public WowServerRoots Roots => new(Maps, Vmaps, Mmaps);

    /// <summary>
    /// The extractor binary: the project's <see cref="ExtractorExeKey"/> when set, otherwise the build
    /// output of this checkout. Null when neither exists.
    /// </summary>
    public string? ExtractorExe
    {
        get
        {
            if (_context.Project.Paths.TryGetValue(ExtractorExeKey, out string? configured) && configured.Length > 0)
            {
                return File.Exists(configured) ? configured : null;
            }

            return DefaultExtractorExe();
        }
    }

    /// <summary>Why <see cref="ExtractorExe"/> is unavailable.</summary>
    public static string MissingExtractorMessage =>
        $"Build the extractor, or set paths.{ExtractorExeKey} in the project config";

    // The checkout is found from the plugin's own directory, the only place a fixed relative layout is known.
    private static string? DefaultExtractorExe()
    {
        string? plugin = PluginDirectories.Find("WoWMapStudio");
        if (plugin == null)
        {
            return null;
        }

        string repo = Path.GetFullPath(Path.Combine(plugin, "..", "..", "..", ".."));
        string name = OperatingSystem.IsWindows() ? "map_extractor.exe" : "map_extractor";
        string path = Path.Combine(repo, "build", "tools", "map_extractor", name);
        return File.Exists(path) ? path : null;
    }

    /// <summary>Why <see cref="Data"/> is unavailable.</summary>
    public static string MissingDataMessage => $"Set paths.{DataKey} in the project config";

    private string? Sub(string name) => Data is { } data ? Path.Combine(data, name) : null;
}
