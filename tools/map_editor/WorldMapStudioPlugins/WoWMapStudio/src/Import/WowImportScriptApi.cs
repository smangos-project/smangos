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

    /// <summary>
    /// Creates and persists a procedural model that outputs an M2: one square panel (side <paramref name="size"/>)
    /// textured with <paramref name="texturePath"/>, and places one instance at editor position (x, y, z) in the
    /// current map's open edit session. Returns the catalog id.
    /// </summary>
    [ScriptFunction]
    public Task<int> CreatePanelModel(string name, double size, string texturePath, double x, double y, double z)
        => CreatePanel(name, size, texturePath, x, y, z, M2Format.FormatId, M2Material.Texture, M2Material.TwoSided);

    /// <summary>Like <see cref="CreatePanelModel"/> but the model outputs a WMO.</summary>
    [ScriptFunction]
    public Task<int> CreatePanelWmoModel(string name, double size, string texturePath, double x, double y, double z)
        => CreatePanel(name, size, texturePath, x, y, z, WmoFormat.FormatId, WmoMaterial.Texture, WmoMaterial.TwoSided);

    /// <summary>
    /// Creates and persists a square water surface (side <paramref name="size"/>, liquid type
    /// <paramref name="liquidTypeId"/>) with its corners snapped to the MH2O lattice, and places it at editor
    /// position (x, y, z) in the current map's open edit session. Returns the catalog id.
    /// </summary>
    [ScriptFunction]
    public async Task<int> CreatePondModel(string name, double size, int liquidTypeId, double x, double y, double z)
    {
        var origin = new Godot.Vector3((float)x, (float)y, (float)z);
        float h = (float)size / 2.0f;
        var network = new VertexNetwork();
        int[] ids = new int[4];
        Godot.Vector3[] corners = [new(-h, 0, -h), new(h, 0, -h), new(h, 0, h), new(-h, 0, h)];
        for (int i = 0; i < 4; i++)
        {
            ids[i] = network.AddVertex(WaterLatticeSnap.ToSubchunkCorner(_context, origin + corners[i]) - origin);
        }

        network.AddFace(ids);

        var values = new MeshParameterValues();
        values.Set(WowLiquidParameters.LiquidTypeIdLookup, liquidTypeId);
        values.Set(WowLiquidParameters.FrameDuration, 0.15f);
        values.Set(WaterSurfaceMeshFunction.UvScale, 0.05f);
        values.Set(WaterAttributeParameters.Fishable, false);
        values.Set(WaterAttributeParameters.Fatigue, false);

        var model = new ProceduralModel { Name = name, FunctionId = WaterSurfaceMeshFunction.FunctionId, Parameters = values.Serialize() };
        model.LoadNetwork(network);
        _context.Catalog.AssignId(model);
        _context.Catalog.Add(model);
        await Task.Run(() => _context.Database.EditorStorage.CommitAsync([model], [])).ConfigureAwait(false);

        var entity = new MapSceneEntity { Name = name, Map = _context.Maps.CurrentMap, Transform = new Godot.Transform3D(Godot.Basis.Identity, origin) };
        entity.AddComponent(new ProceduralComponent(_context.Procedural) { ModelId = model.RecordId });
        _context.Scene.Add(entity);
        _context.EditSessions.Record(new CreateEntityCommand(_context.Scene, entity));
        return model.RecordId ?? 0;
    }

    private async Task<int> CreatePanel(string name, double size, string texturePath, double x, double y, double z, string formatId, MeshParameter textureParameter, MeshParameter twoSided)
    {
        var network = new VertexNetwork();
        float h = (float)size / 2.0f;
        int a = network.AddVertex(new Godot.Vector3(-h, 0, -h));
        int b = network.AddVertex(new Godot.Vector3(h, 0, -h));
        int c = network.AddVertex(new Godot.Vector3(h, 0, h));
        int d = network.AddVertex(new Godot.Vector3(-h, 0, h));
        network.AddFace([a, b, c, d]);

        var formats = new ProceduralFormats();
        formats.Set(PanelNetworkMeshFunction.Output, formatId);

        var materials = new ProceduralBindings();
        var textureValues = new MeshParameterValues();
        textureValues.Set(textureParameter, texturePath);
        textureValues.Set(twoSided, true);
        materials.BindInline(PanelNetworkMeshFunction.Output, PanelNetworkMeshFunction.Surface, textureValues);

        var model = new ProceduralModel
        {
            Name = name,
            FunctionId = "builtin.mesh.panel_network",
            Formats = formats.Serialize(),
            Materials = materials.Serialize(),
        };
        model.LoadNetwork(network);
        _context.Catalog.AssignId(model);
        _context.Catalog.Add(model);
        await Task.Run(() => _context.Database.EditorStorage.CommitAsync([model], [])).ConfigureAwait(false);

        var entity = new MapSceneEntity
        {
            Name = name,
            Map = _context.Maps.CurrentMap,
            Transform = new Godot.Transform3D(Godot.Basis.Identity, new Godot.Vector3((float)x, (float)y, (float)z)),
        };
        entity.AddComponent(new ProceduralComponent(_context.Procedural) { ModelId = model.RecordId });
        _context.Scene.Add(entity);
        _context.EditSessions.Record(new CreateEntityCommand(_context.Scene, entity));
        return model.RecordId ?? 0;
    }
}
