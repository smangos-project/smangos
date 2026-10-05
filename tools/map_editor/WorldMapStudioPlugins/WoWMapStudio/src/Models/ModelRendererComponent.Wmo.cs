using System;
using System.Collections.Generic;
using System.Linq;
using ImGuiNET;
using WorldMapStudio.WoW;

namespace WorldMapStudio;

/// <summary>
/// WMO doodad-set selection for <see cref="ModelRendererComponent"/>, injected via partial-class
/// extension so the core component stays free of WoW-specific concepts. <see cref="WmoModelLoader"/>
/// emits one <see cref="ModelPart"/> per doodad set, named "Doodads:&lt;set name&gt;", with only set 0
/// marked default-visible; this file picks among them.
/// </summary>
public sealed partial class ModelRendererComponent
{
    private string _selectedDoodadSet = "";

    /// <summary>The chosen WMO doodad set's display name, or empty to use the model's own default (set 0).</summary>
    public string SelectedDoodadSet
    {
        get => _selectedDoodadSet;
        set
        {
            if (_selectedDoodadSet == value)
            {
                return;
            }

            _selectedDoodadSet = value;
            Owner?.RefreshRepresentation();
        }
    }

    partial void ConfigurePartFilter(ModelAsset model, ref Func<ModelPart, bool>? filter)
    {
        if (_selectedDoodadSet.Length == 0)
        {
            return;
        }

        string wanted = $"{WmoModelLoader.DoodadSetPartPrefix}{_selectedDoodadSet}";
        filter = part => IsDoodadSetPart(part) ? part.Name == wanted : part.DefaultVisible;
    }

    partial void AddContentVersion(ref int hash)
    {
        hash = HashCode.Combine(hash, _selectedDoodadSet);
    }

    partial void CopyExtraTo(ModelRendererComponent clone)
    {
        clone.SelectedDoodadSet = _selectedDoodadSet;
    }

    partial void SaveFormatState(ref string? state)
    {
        if (_selectedDoodadSet.Length > 0)
        {
            state = _selectedDoodadSet;
        }
    }

    partial void LoadFormatState(string? state)
    {
        _selectedDoodadSet = state ?? "";
    }

    partial void DrawInspectorExtraHook(InspectorContext context)
    {
        if (!TryGetModel(out ModelAsset? model) || model == null)
        {
            return;
        }

        List<string> setNames = model.Parts
            .Where(IsDoodadSetPart)
            .Select(part => part.Name[WmoModelLoader.DoodadSetPartPrefix.Length..])
            .ToList();
        if (setNames.Count <= 1)
        {
            return;
        }

        context.Fields.Field("Doodad Set", () =>
        {
            string current = _selectedDoodadSet.Length > 0 ? _selectedDoodadSet : setNames[0];
            if (!ImGui.BeginCombo("Doodad Set", current))
            {
                return;
            }

            foreach (string name in setNames)
            {
                if (!ImGui.Selectable(name, name == current))
                {
                    continue;
                }

                string before = _selectedDoodadSet;
                string after = name == setNames[0] ? "" : name;
                if (before != after)
                {
                    var command = new SetComponentFieldCommand<string>(this, "doodad set", value => SelectedDoodadSet = value, before, after);
                    command.Apply();
                    context.Sessions.Record(command);
                }
            }

            ImGui.EndCombo();
        });
    }

    private static bool IsDoodadSetPart(ModelPart part) =>
        part.Name.StartsWith(WmoModelLoader.DoodadSetPartPrefix, StringComparison.Ordinal);
}
