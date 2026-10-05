using System;
using System.Collections.Generic;
using System.Linq;
using ImGuiNET;
using Vector4 = System.Numerics.Vector4;

namespace WorldMapStudio.WoW;

/// <summary>
/// The Map Properties "WoW Map" section: shows and edits a project map's explicit
/// <see cref="WowMapBindings"/> binding — the id every WoW-side factory (creatures, lights, area
/// table, …) resolves through instead of guessing one from the map's name. Priority below
/// <see cref="WowMapDefaultLightSection"/> (which itself resolves through this binding) so it draws
/// first.
/// </summary>
[Subsystem(nameof(MapPropertiesRegistry))]
public sealed class WowMapIdSection : IMapPropertiesSection
{
    private const string MapCatalogName = "Map";
    private static readonly Vector4 ErrorColor = new(0.95f, 0.5f, 0.4f, 1.0f);

    private readonly CatalogReferenceField _field = new();
    private string? _error;

    public string Label => "WoW Map";

    public float Priority => -1.0f;

    public WowMapIdSection(MapPropertiesRegistry registry)
    {
    }

    public void Draw(EditorContext context, Map map, bool isCurrent)
    {
        WowMapBindings bindings = WowMapBindings.For(context);
        int? current = bindings.Resolve(map.Id);

        // Only the current map's binding change forces a reload — see WowMapBindings.SetAsync.
        string? blocker = isCurrent ? context.Operations.Blocker : null;
        bool canEdit = blocker is null;

        ImGui.BeginDisabled(!canEdit);

        int value = current ?? 0;
        ImGui.SetNextItemWidth(120.0f);
        if (ImGui.InputInt("WoW Map Id", ref value))
        {
            Apply(bindings, map, Math.Max(0, value));
        }

        var reference = new CatalogReference("WoW Map Id", MapCatalogName, current?.ToString(),
            key => Apply(bindings, map, key is null ? null : int.Parse(key)));
        _field.Draw(context, reference, navigate: null);

        if (current is not null)
        {
            ImGui.SameLine();
            if (ImGui.SmallButton("Clear##wowMapId"))
            {
                Apply(bindings, map, null);
            }
        }

        ImGui.EndDisabled();

        if (!canEdit && ImGui.IsItemHovered(ImGuiHoveredFlags.AllowWhenDisabled))
        {
            ImGui.SetTooltip(blocker!);
        }
        else if (isCurrent && canEdit)
        {
            ImGui.TextDisabled("Changing this reloads the world.");
        }

        if (current is not { } bound)
        {
            ImGui.TextWrapped("Not bound — WoW data (spawns, lights, area table…) won't load for this map.");
        }
        else
        {
            if (context.Catalog.OfType<DbcMap>().All(row => row.MapId != bound))
            {
                ImGui.TextColored(ErrorColor, $"No dbc_map row {bound}.");
            }

            DrawAlsoBound(context, bindings, map, bound);
        }

        if (_error is not null)
        {
            ImGui.TextColored(ErrorColor, _error);
        }
    }

    private static void DrawAlsoBound(EditorContext context, WowMapBindings bindings, Map map, int wowMapId)
    {
        IReadOnlyList<MapId> others = bindings.ProjectMapsFor(wowMapId).Where(id => id != map.Id).ToList();
        if (others.Count == 0)
        {
            return;
        }

        string names = string.Join(", ", others.Select(id =>
        {
            Map? other = context.Maps.Maps.FirstOrDefault(candidate => candidate.Id == id);
            return other is null ? $"map {id.Value}" : $"{other.DisplayName} ({id.Value})";
        }));

        ImGui.TextDisabled($"Also bound to this WoW map: {names}");
    }

    private void Apply(WowMapBindings bindings, Map map, int? wowMapId)
    {
        try
        {
            BlockingWork.Run(() => bindings.SetAsync(map.Id, wowMapId));
            _error = null;
        }
        catch (Exception e)
        {
            _error = e.Message;
        }
    }
}
