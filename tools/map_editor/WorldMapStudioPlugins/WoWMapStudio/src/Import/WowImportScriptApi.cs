using System;
using System.Linq;
using System.Text.Json.Nodes;
using System.Threading.Tasks;

namespace WorldMapStudio.WoW;

/// <summary>One imported tile of a project map, as JS sees it.</summary>
public sealed class WowImportedTileDescriptor
{
    internal WowImportedTileDescriptor(AdtImportTileRecord row)
    {
        FileX = row.FileX;
        FileY = row.FileY;
        SourceId = row.SourceId;
        ImporterVersion = row.ImporterVersion;
        ImportedAtUtc = row.ImportedAtUtc.ToString("O");
    }

    [ScriptProperty]
    public int FileX { get; }

    [ScriptProperty]
    public int FileY { get; }

    [ScriptProperty]
    public string SourceId { get; }

    [ScriptProperty]
    public int ImporterVersion { get; }

    [ScriptProperty]
    public string ImportedAtUtc { get; }
}

/// <summary>
/// The ADT import in WoW vocabulary, exposed to JS as <c>wms.wowImport</c> — a thin shell over the
/// <c>wow.import.adt</c> batch operation and the <c>wow_adt_import_tiles</c> table, so anything the
/// import window can do a script can do too. Mirrors <see cref="WowExportScriptApi"/>.
/// </summary>
[Subsystem(nameof(ScriptingSystem))]
public sealed class WowImportScriptApi : IScriptModule
{
    private const string OperationId = "wow.import.adt";

    private readonly EditorContext _context;

    public string Name => "wowImport";

    public WowImportScriptApi(ScriptingSystem system)
    {
        _context = system.Context;
    }

    /// <summary>
    /// Starts an ADT import and returns its batch run id — poll it with <c>wms.batch.status</c>.
    /// <paramref name="sourceMap"/> is the client's map folder name (e.g. <c>"Azeroth"</c>);
    /// <paramref name="targetMap"/> is the project map to create or extend; the four tile indices are
    /// an inclusive file-coordinate rectangle.
    /// </summary>
    [ScriptFunction]
    public long Run(string sourceMap, string targetMap, int minX, int minY, int maxX, int maxY, bool diskStorage = false, bool importWater = true)
    {
        if (string.IsNullOrWhiteSpace(sourceMap) || string.IsNullOrWhiteSpace(targetMap))
        {
            throw new InvalidOperationException("sourceMap and targetMap are required.");
        }

        IBatchOperation operation = _context.Batch.Find(OperationId)
            ?? throw new InvalidOperationException($"'{OperationId}' is not registered.");

        var overrides = new JsonObject
        {
            ["sourceMap"] = sourceMap,
            ["targetMap"] = targetMap,
            ["diskStorage"] = diskStorage,
            ["importWater"] = importWater,
            ["minX"] = minX,
            ["minY"] = minY,
            ["maxX"] = maxX,
            ["maxY"] = maxY,
        };

        BatchRun run = _context.Batch.TryStart(operation, overrides, out string? blocker)
            ?? throw new InvalidOperationException(blocker ?? "Could not start the ADT import.");

        return run.Id;
    }

    /// <summary>Which tiles a project map already has imported, from <c>wow_adt_import_tiles</c>.</summary>
    [ScriptFunction]
    public Task<WowImportedTileDescriptor[]> Status(int mapId) => Task.Run(async () =>
    {
        return (await AdtImportTileStore.LoadForMapAsync(_context.Database.EditorStorage, mapId).ConfigureAwait(false))
            .OrderBy(row => (row.FileY, row.FileX))
            .Select(row => new WowImportedTileDescriptor(row))
            .ToArray();
    });

    /// <summary>Starts an ADT export of a project map's imported tiles and returns its batch run id.</summary>
    [ScriptFunction]
    public long Export(string sourceMap, string targetMap)
    {
        IBatchOperation operation = _context.Batch.Find("wow.export.adt")
            ?? throw new InvalidOperationException("'wow.export.adt' is not registered.");

        var overrides = new JsonObject { ["sourceMap"] = sourceMap, ["targetMap"] = targetMap };
        BatchRun run = _context.Batch.TryStart(operation, overrides, out string? blocker)
            ?? throw new InvalidOperationException(blocker ?? "Could not start the ADT export.");

        return run.Id;
    }
}
