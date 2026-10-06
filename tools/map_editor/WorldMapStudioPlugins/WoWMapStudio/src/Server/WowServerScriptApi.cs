using System;
using System.Linq;
using System.Threading.Tasks;

namespace WorldMapStudio.WoW;

/// <summary>
/// The server data loop in JS as <c>wms.wowServer</c>: <c>stale</c> lists tiles the extractor would redo,
/// <c>extract</c> rebuilds them into the server's data directory.
/// </summary>
[Subsystem(nameof(ScriptingSystem))]
public sealed class WowServerScriptApi : IScriptModule
{
    private readonly EditorContext _context;

    public string Name => "wowServer";

    public WowServerScriptApi(ScriptingSystem system)
    {
        _context = system.Context;
    }

    /// <summary>Tiles of <paramref name="mapId"/> whose server data is out of date, as <c>"row,col"</c>; all tiles of the map unless a tile is given.</summary>
    [ScriptFunction]
    public Task<string[]> Stale(int mapId, int row = -1, int col = -1, string stages = "map,vmap,mmap") => Task.Run(async () =>
    {
        WowServerExtractResult result = await RunAsync(Request(mapId, row, col, stages, false), dryRun: true).ConfigureAwait(false);
        return WowServerExtractor.ParseStaleTiles(result.Lines).Select(t => $"{t.Row},{t.Col}").ToArray();
    });

    /// <summary>Runs the extractor and returns its exit code; output goes to the editor log.</summary>
    [ScriptFunction]
    public Task<int> Extract(int mapId, int row = -1, int col = -1, string stages = "map,vmap,mmap", bool force = false) => Task.Run(async () =>
    {
        WowServerExtractResult result = await RunAsync(Request(mapId, row, col, stages, force), dryRun: false).ConfigureAwait(false);
        return result.ExitCode;
    });

    private static WowServerExtractRequest Request(int mapId, int row, int col, string stages, bool force) =>
        row >= 0 && col >= 0 ? new WowServerExtractRequest(mapId, row, col, stages, force) : new WowServerExtractRequest(mapId, Stages: stages, Force: force);

    private Task<WowServerExtractResult> RunAsync(WowServerExtractRequest request, bool dryRun)
    {
        string exe = _context.WowServerPaths.ExtractorExe ?? throw new InvalidOperationException(WowServerPaths.MissingExtractorMessage);
        string output = _context.WowServerPaths.Data ?? throw new InvalidOperationException(WowServerPaths.MissingDataMessage);
        string input = _context.WowClientPaths.Content ?? throw new InvalidOperationException(WowClientPaths.MissingContentMessage);
        return WowServerExtractor.RunAsync(exe, WowServerExtractor.BuildArguments(request, input, output, dryRun));
    }
}
