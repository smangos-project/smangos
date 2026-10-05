using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using ImGuiNET;
using Microsoft.EntityFrameworkCore;
using Vector2 = System.Numerics.Vector2;
using Vector4 = System.Numerics.Vector4;

namespace WorldMapStudio.WoW;

/// <summary>
/// Maps <see cref="LightSkybox"/> to and from cata's `dbc_lightskybox` table — the role
/// <see cref="BattlefieldTemplateFactory"/> plays for `tc_battlefield_template`, plus an
/// <see cref="ModelAssetPicker"/> for browsing to a model path since this table's `Name` column is one.
/// </summary>
[Subsystem(nameof(WorldStorage))]
public sealed class LightSkyboxFactory : ICatalogEntityFactory, ICatalogBrowser
{
    private const string CreatePopupId = "Create Light Skybox";

    private readonly WorldStorage _storage;
    private readonly Lazy<ModelAssetPicker> _skyboxPicker;

    private bool _createOpenRequested;
    private bool _createActive;
    private int _createId;

    public LightSkyboxFactory(WorldStorage storage)
    {
        _storage = storage;
        _skyboxPicker = new Lazy<ModelAssetPicker>(() =>
            new ModelAssetPicker(storage.Context.Assets, storage.Context.MeshMaterials, storage.Context.Root));
    }

    public Type EntityType => typeof(LightSkybox);

    public string CatalogName => "Light Skybox";

    public bool Handles(IEntity entity) => entity is LightSkybox;

    // dbc_lightskybox is already mapped by its generated DbcLightskyboxTableConfiguration.
    public void Configure(ModelBuilder model)
    {
    }

    public async Task<IReadOnlyList<CatalogEntity>> LoadAllAsync()
    {
        await using WorldDbContext context = _storage.CreateContext();

        List<DbcLightskyboxRecord> rows = await context.DbcLightskybox.AsNoTracking()
            .ToListAsync().ConfigureAwait(false);

        return rows.Select(row => (CatalogEntity)ToEntity(row)).ToList();
    }

    public async Task<int> MaxRecordIdAsync()
    {
        await using WorldDbContext context = _storage.CreateContext();
        return await context.DbcLightskybox.MaxAsync(row => (int?)row.ID).ConfigureAwait(false) ?? 0;
    }

    public async Task<IReadOnlyList<CatalogSearchResult>> SearchAsync(string filter)
    {
        await using WorldDbContext context = _storage.CreateContext();

        IQueryable<DbcLightskyboxRecord> query = context.DbcLightskybox.AsNoTracking();
        if (!string.IsNullOrWhiteSpace(filter))
        {
            query = int.TryParse(filter, out int id)
                ? query.Where(record => record.ID == id)
                : query.Where(record => EF.Functions.Like(record.Name, $"%{filter}%"));
        }

        List<DbcLightskyboxRecord> rows = await query.OrderBy(record => record.ID)
            .ToListAsync().ConfigureAwait(false);

        return rows.Select(record => new CatalogSearchResult(
            record.ID.ToString(), NormalizeModelPath(record.Name))).ToList();
    }

    // Eagerly loaded (LoadAllAsync): every row is already in the registry, so this is a plain lookup.
    public Task<IReadOnlyDictionary<string, string>> DescribeAsync(EditorContext context, IReadOnlyCollection<string> keys)
    {
        var result = new Dictionary<string, string>();
        foreach (string key in keys)
        {
            if (int.TryParse(key, out int id) &&
                context.Catalog.OfType<LightSkybox>().FirstOrDefault(e => e.SkyboxId == id) is { } entity)
            {
                result[key] = entity.ModelPath;
            }
        }

        return Task.FromResult<IReadOnlyDictionary<string, string>>(result);
    }

    public async Task<CatalogEntity?> OpenAsync(EditorContext context, string key)
    {
        if (!int.TryParse(key, out int id))
        {
            return null;
        }

        if (context.Catalog.OfType<LightSkybox>().FirstOrDefault(e => e.SkyboxId == id) is { } open)
        {
            return open;
        }

        await using WorldDbContext db = _storage.CreateContext();
        DbcLightskyboxRecord? record = await db.DbcLightskybox.AsNoTracking()
            .FirstOrDefaultAsync(row => row.ID == id).ConfigureAwait(false);
        if (record is null)
        {
            return null;
        }

        LightSkybox entity = ToEntity(record);
        context.Catalog.Add(entity);
        return entity;
    }

    public void DrawFields(EditorContext context, CatalogEntity entity, FieldEditTracker tracker,
        Action<string, string> navigate, string fieldFilter)
    {
        var typed = (LightSkybox)entity;
        var f = new CatalogFieldSheet(context, entity, tracker, navigate, fieldFilter);

        f.Custom("Id", () => ImGui.TextDisabled($"Id: {typed.SkyboxId}"));

        f.Custom("Model", () =>
        {
            ImGui.AlignTextToFramePadding();
            ImGui.Text("Model:");
            ImGui.SameLine();
            ImGui.TextDisabled(typed.ModelPath.Length == 0 ? "(none)" : typed.ModelPath);
            ImGui.SameLine();
            if (ImGui.Button("Browse##skybox"))
            {
                _skyboxPicker.Value.Browse(typed.ModelPath, selected =>
                    RecordNow(context, typed, "model path", typed.ModelPath, selected, v => typed.ModelPath = v));
            }

            if (typed.ModelPath.Length > 0)
            {
                ImGui.SameLine();
                if (ImGui.SmallButton("Clear##skybox"))
                {
                    RecordNow(context, typed, "model path", typed.ModelPath, "", v => typed.ModelPath = v);
                }
            }
        });

        f.Custom("Full Day Animation", () => DrawFlag(context, typed, "Full Day Animation", WowSkyboxFlags.FullDay));
        f.Custom("Combine With Procedural Sky", () => DrawFlag(context, typed, "Combine With Procedural Sky", WowSkyboxFlags.Combine));

        _skyboxPicker.Value.Draw();
    }

    public void DrawCreate(EditorContext context, Action<CatalogEntity> onCreated)
    {
        if (ImGui.Button("New Light Skybox"))
        {
            List<LightSkybox> existing = context.Catalog.OfType<LightSkybox>().ToList();
            _createId = existing.Count == 0 ? 1 : existing.Max(e => e.SkyboxId) + 1;
            _createOpenRequested = true;
        }

        DrawCreatePopup(context, onCreated);
    }

    private void DrawCreatePopup(EditorContext context, Action<CatalogEntity> onCreated)
    {
        if (_createOpenRequested)
        {
            ImGui.OpenPopup(CreatePopupId);
            _createOpenRequested = false;
            _createActive = true;
        }

        if (!_createActive)
        {
            return;
        }

        bool open = true;
        ImGuiEx.PopupModal(CreatePopupId, true, ref open, ImGuiWindowFlags.AlwaysAutoResize, () =>
        {
            ImGui.InputInt("Id", ref _createId);

            string? error = CreateValidationError(context);
            if (error is not null)
            {
                ImGuiEx.TextColored(CommonColors.Error, error);
            }

            ImGui.BeginDisabled(error is not null);
            if (ImGui.Button("Create", new Vector2(120.0f, 0.0f)))
            {
                CommitCreate(context, onCreated);
                open = false;
            }

            ImGui.EndDisabled();

            ImGui.SameLine();
            if (ImGui.Button("Cancel", new Vector2(120.0f, 0.0f)))
            {
                open = false;
            }
        });

        if (!open)
        {
            _createActive = false;
        }
    }

    private string? CreateValidationError(EditorContext context)
    {
        if (context.Catalog.OfType<LightSkybox>().Any(e => e.SkyboxId == _createId))
        {
            return $"Id {_createId} is already open.";
        }

        return null;
    }

    private void CommitCreate(EditorContext context, Action<CatalogEntity> onCreated) =>
        onCreated(Create(context, _createId.ToString()));

    public CatalogEntity Create(EditorContext context, string key)
    {
        if (!int.TryParse(key, out int id))
        {
            throw new InvalidOperationException($"'{key}' is not a valid light skybox id.");
        }

        if (context.Catalog.OfType<LightSkybox>().Any(e => e.SkyboxId == id))
        {
            throw new InvalidOperationException($"Light Skybox {id} is already open.");
        }

        var entity = new LightSkybox { SkyboxId = id };
        var command = new CreateCatalogEntityCommand(context.Catalog, entity);
        command.Apply();
        context.EditSessions.Record(command);
        return entity;
    }

    public Action Stage(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        var typed = (LightSkybox)entity;

        var record = new DbcLightskyboxRecord
        {
            ID = typed.SkyboxId,
            Name = typed.ModelPath,
            Flags = (int)typed.Flags,
        };

        return SavedEntityStaging.Stage(typed,
            insert: () => db.DbcLightskybox.Add(record),
            update: () => db.DbcLightskybox.Update(record));
    }

    public void StageDelete(DbContext context, IEntity entity)
    {
        var db = (WorldDbContext)context;
        var typed = (LightSkybox)entity;

        SavedEntityStaging.StageDelete(typed,
            delete: () => db.DbcLightskybox.Remove(new DbcLightskyboxRecord { ID = typed.SkyboxId }));
    }

    private static LightSkybox ToEntity(DbcLightskyboxRecord row) => new()
    {
        SkyboxId = row.ID,
        IsSaved = true,
        ModelPath = NormalizeModelPath(row.Name),
        Flags = (WowSkyboxFlags)row.Flags,
    };

    // The DBC's own Name column still uses the legacy .mdx/.mdl extension; this project's own model
    // paths (asset browsing, other DBC-sourced model refs like WmoRootReader's doodad paths) all use
    // .m2, and there's no DBC-export writer yet to round-trip the legacy extension back out.
    private static string NormalizeModelPath(string path)
    {
        if (path.EndsWith(".mdx", StringComparison.OrdinalIgnoreCase) || path.EndsWith(".mdl", StringComparison.OrdinalIgnoreCase))
        {
            return $"{path[..^4]}.m2";
        }

        return path;
    }

    private void DrawFlag(EditorContext context, LightSkybox typed, string label, WowSkyboxFlags flag)
    {
        bool set = (typed.Flags & flag) != 0;
        if (!ImGui.Checkbox(label, ref set))
        {
            return;
        }

        WowSkyboxFlags after = set ? typed.Flags | flag : typed.Flags & ~flag;
        RecordNow(context, typed, "flags", typed.Flags, after, v => typed.Flags = v);
    }

    // A checkbox/browse result has no activate/deactivate pair to bracket, so its edit is recorded on
    // the spot — same pattern WowLightParamsWindow used before this catalog moved into the browser.
    private static void RecordNow<T>(EditorContext context, CatalogEntity entity, string field, T before, T after, Action<T> set)
    {
        if (EqualityComparer<T>.Default.Equals(before, after))
        {
            return;
        }

        var command = new SetFieldCommand<T>(entity, field, set, before, after);
        command.Apply();
        context.EditSessions.Record(command);
    }
}
