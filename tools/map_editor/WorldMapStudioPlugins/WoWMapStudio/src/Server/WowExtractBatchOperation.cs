using System;
using System.Linq;
using System.Text.Json.Nodes;
using System.Threading.Tasks;
using ImGuiNET;

namespace WorldMapStudio.WoW;

/// <summary>Runs map_extractor over the project's client content into the server data folder; the extractor skips what is current.</summary>
[Subsystem(nameof(BatchSystem))]
public sealed class WowExtractBatchOperation : IBatchOperation
{
    private static readonly string[] AllStages = ["map", "vmap", "mmap"];

    private readonly BatchSystem _batch;

    private int _mapId;
    private int _row = -1;
    private int _col = -1;
    private string[] _stages = AllStages;
    private bool _force;
    private bool _dryRun;

    public WowExtractBatchOperation(BatchSystem batch)
    {
        _batch = batch;
    }

    public string Id => "wow.extract";

    public string DisplayName => "WoW Server Data (extractor)";

    public string Description =>
        "Rebuilds server map, vmap and mmap data from the client data folder. Only stale tiles are redone unless forced.";

    public void DrawSettings()
    {
        ImGui.InputInt("WoW map id", ref _mapId);
        if (_batch.Context.Maps.CurrentMap is { } current
            && WowMapBindings.For(_batch.Context).Resolve(current) is int currentWow
            && currentWow != _mapId
            && ImGui.SmallButton($"Use current map ({currentWow})"))
        {
            _mapId = currentWow;
        }

        ImGui.InputInt("Tile row (-1 = whole map)", ref _row);
        ImGui.InputInt("Tile column (-1 = whole map)", ref _col);
        foreach (string stage in AllStages)
        {
            bool on = _stages.Contains(stage);
            if (ImGui.Checkbox(stage, ref on))
            {
                string[] next = AllStages.Where(s => s == stage ? on : _stages.Contains(s)).ToArray();
                _stages = next.Length == 0 ? [stage] : next;
            }

            ImGui.SameLine();
        }

        ImGui.NewLine();
        ImGui.Checkbox("Force (rebuild even if current)", ref _force);
        ImGui.Checkbox("Dry run", ref _dryRun);
    }

    public JsonObject SaveSettings() => new()
    {
        ["mapId"] = _mapId,
        ["row"] = _row,
        ["col"] = _col,
        ["stages"] = string.Join(',', _stages),
        ["force"] = _force,
        ["dryRun"] = _dryRun,
    };

    public void LoadSettings(JsonObject settings)
    {
        _mapId = settings["mapId"]?.GetValue<int>() ?? 0;
        _row = settings["row"]?.GetValue<int>() ?? -1;
        _col = settings["col"]?.GetValue<int>() ?? -1;
        string[] stages = (settings["stages"]?.GetValue<string>() ?? "map,vmap,mmap")
            .Split(',', StringSplitOptions.RemoveEmptyEntries).Where(AllStages.Contains).ToArray();
        _stages = stages.Length == 0 ? AllStages : stages;
        _force = settings["force"]?.GetValue<bool>() ?? false;
        _dryRun = settings["dryRun"]?.GetValue<bool>() ?? false;
    }

    public async Task RunAsync(BatchContext context, WorkContext work)
    {
        WowServerPaths paths = context.Editor.WowServerPaths;
        string input = context.Editor.WowClientPaths.Content ?? throw new InvalidOperationException(WowClientPaths.MissingContentMessage);
        string output = paths.Data ?? throw new InvalidOperationException(WowServerPaths.MissingDataMessage);
        string exe = paths.ExtractorExe ?? throw new InvalidOperationException(WowServerPaths.MissingExtractorMessage);

        JsonObject s = context.Settings;
        int row = s["row"]?.GetValue<int>() ?? -1;
        int col = s["col"]?.GetValue<int>() ?? -1;
        var request = new WowServerExtractRequest(
            s["mapId"]?.GetValue<int>() ?? 0,
            row >= 0 && col >= 0 ? row : null,
            row >= 0 && col >= 0 ? col : null,
            s["stages"]?.GetValue<string>() ?? "map,vmap,mmap",
            s["force"]?.GetValue<bool>() ?? false);
        bool dryRun = s["dryRun"]?.GetValue<bool>() ?? false;

        context.Step(dryRun ? "Extractor (dry run)" : "Extractor");
        WowServerExtractResult result = await WowServerExtractor.RunAsync(exe, WowServerExtractor.BuildArguments(request, input, output, dryRun)).ConfigureAwait(false);
        foreach (string line in result.Lines.Where(l => !string.IsNullOrWhiteSpace(l)))
        {
            context.Log(line);
        }

        if (dryRun)
        {
            var stale = WowServerExtractor.ParseStaleTiles(result.Lines);
            context.Log(stale.Count == 0
                ? "Server data is current."
                : $"Server data is stale: {stale.Count} tile(s) would be redone ({stale[0].Row},{stale[0].Col}{(stale.Count > 1 ? ", ..." : "")}).");
        }

        if (result.ExitCode != 0)
        {
            throw new InvalidOperationException($"The extractor exited with code {result.ExitCode}; see the batch log.");
        }
    }
}
