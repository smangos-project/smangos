using System;
using System.Linq;
using System.Text.Json.Nodes;
using System.Threading.Tasks;
using ImGuiNET;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>
/// Creates a new, empty client map: its WDT (the donor map's, without tiles), its `dbc_map` row (the donor's, renamed),
/// a `map_template` row and a `game_tele` entry. Tiles are then added with the ADT export's new-tiles setting.
/// </summary>
[Subsystem(nameof(BatchSystem))]
public sealed class WowMapCreateBatchOperation : IBatchOperation
{
    private const int CustomMapIdBase = 3000;

    private string _donorMap = "";
    private string _directory = "";
    private string _name = "";
    private int _mapId;

    public string Id => "wow.export.map";

    public string DisplayName => "WoW New Map (Cataclysm)";

    public string Description =>
        "Creates an empty client map from a donor map: WDT, dbc_map, map_template and a game_tele entry.";

    public WowMapCreateBatchOperation(BatchSystem batch)
    {
    }

    public void DrawSettings()
    {
        ImGui.InputText("Donor client map folder", ref _donorMap, 128);
        ImGui.InputText("New map folder", ref _directory, 128);
        ImGui.InputText("Display name", ref _name, 128);
        ImGui.InputInt("Map id (0 = next free)", ref _mapId);
    }

    public JsonObject SaveSettings() => new()
    {
        ["donorMap"] = _donorMap,
        ["directory"] = _directory,
        ["name"] = _name,
        ["mapId"] = _mapId,
    };

    public void LoadSettings(JsonObject settings)
    {
        _donorMap = settings["donorMap"]?.GetValue<string>() ?? "";
        _directory = settings["directory"]?.GetValue<string>() ?? "";
        _name = settings["name"]?.GetValue<string>() ?? "";
        _mapId = settings["mapId"]?.GetValue<int>() ?? 0;
    }

    public async Task RunAsync(BatchContext context, WorkContext work)
    {
        string donor = context.Settings["donorMap"]?.GetValue<string>() ?? "";
        string directory = context.Settings["directory"]?.GetValue<string>() ?? "";
        string name = context.Settings["name"]?.GetValue<string>() ?? "";
        int mapId = context.Settings["mapId"]?.GetValue<int>() ?? 0;
        if (donor.Length == 0 || directory.Length == 0)
        {
            throw new InvalidOperationException("A donor map folder and a new map folder are required.");
        }

        if (name.Length == 0)
        {
            name = directory;
        }

        string content = context.Editor.WowClientPaths.Content
            ?? throw new InvalidOperationException(WowClientPaths.MissingContentMessage);
        WorldStorage world = context.Editor.Database.Storages.OfType<WorldStorage>().Single();

        context.Step("Writing map rows");
        await using (WorldDbContext db = world.CreateContext())
        {
            if (await db.DbcMap.AnyAsync(row => row.Directory == directory).ConfigureAwait(false))
            {
                throw new InvalidOperationException($"A map with the folder '{directory}' already exists.");
            }

            DbcMapRecord source = await db.DbcMap.AsNoTracking().FirstOrDefaultAsync(row => row.Directory == donor).ConfigureAwait(false)
                ?? throw new InvalidOperationException($"No map with the folder '{donor}' in dbc_map.");
            if (mapId == 0)
            {
                mapId = Math.Max(CustomMapIdBase, await db.DbcMap.MaxAsync(row => row.ID).ConfigureAwait(false) + 1);
            }
            else if (await db.DbcMap.AnyAsync(row => row.ID == mapId).ConfigureAwait(false))
            {
                throw new InvalidOperationException($"Map id {mapId} is already in use.");
            }

            db.DbcMap.Add(new DbcMapRecord
            {
                ID = mapId,
                Directory = directory,
                MapNameLang = name,
                MapDescription0Lang = "",
                MapDescription1Lang = "",
                PvpShortDescriptionLang = "",
                PvpLongDescriptionLang = "",
                MapType = source.MapType,
                InstanceType = 0,
                ExpansionID = source.ExpansionID,
                AreaTableID = 0,
                LoadingScreenID = source.LoadingScreenID,
                TimeOfDayOverride = -1,
                ParentMapID = -1,
                CosmeticParentMapID = -1,
                MinimapIconScale = 1,
                CorpseMapID = -1,
                MaxPlayers = 0,
                WindSettingsID = source.WindSettingsID,
                Flags1 = source.Flags1,
                Flags2 = source.Flags2,
                Flags3 = source.Flags3,
            });
            db.MapTemplate.Add(new MapTemplateRecord { Entry = mapId });
            int teleId = (await db.GameTele.MaxAsync(row => (int?)row.Id).ConfigureAwait(false) ?? 0) + 1;
            db.GameTele.Add(new GameTeleRecord { Id = teleId, Map = mapId, Name = directory });
            await db.SaveChangesAsync().ConfigureAwait(false);
        }

        WowFileDataIds ids = WowFileDataIds.For(context.Editor);
        ids.Allocate($"world/maps/{directory}/{directory}.wdt");
        var fs = WowClientFiles.OpenIsolated(content);
        await Task.Run(() => WowWdtWriter.CreateEmpty(fs, donor, directory)).ConfigureAwait(false);
        ids.ExportIndex(content);
        context.Report($"Created map {mapId} ('{directory}') from '{donor}'.");
    }
}
