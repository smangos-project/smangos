using System;
using System.Collections.Generic;
using ImGuiNET;

namespace WorldMapStudio.WoW;

/// <summary>Edits the `dbc_zonelight` fields of a <see cref="WowZoneLight"/> below the stock transform fields.</summary>
[Subsystem(nameof(InspectorWindow))]
public sealed class WowZoneLightInspector : EntityInspector<WowZoneLight>
{
    private readonly SceneEntityInspector _base;
    private readonly FieldEditTracker _tracker = new();

    public WowZoneLightInspector(InspectorWindow window)
    {
        _base = new SceneEntityInspector(window);
    }

    protected override void DrawTargets(InspectorContext context, IReadOnlyList<WowZoneLight> targets)
    {
        _base.Draw(context, targets);
        if (targets.Count != 1)
        {
            return;
        }

        WowZoneLight light = targets[0];
        context.Fields.Separator();
        context.Fields.Field("Zone light name", () =>
        {
            string name = light.Name;
            if (ImGui.InputText("Name##zonelight", ref name, 255))
            {
                light.Name = name;
            }

            _tracker.Track(context.Sessions, light, "name", light.Name, v => light.Name = v);
        });
        Int(context, light, "Light id", light.LightId, v => light.LightId = v);
        Int(context, light, "Flags", light.Flags, v => light.Flags = v);
        Float(context, light, "Z min", light.ZMin, v => light.ZMin = v);
        Float(context, light, "Z max", light.ZMax, v => light.ZMax = v);
    }

    private void Int(InspectorContext context, WowZoneLight light, string label, int value, Action<int> set)
    {
        context.Fields.Field(label, () =>
        {
            int edited = value;
            if (ImGui.InputInt(label, ref edited))
            {
                set(edited);
            }

            _tracker.Track(context.Sessions, light, label, edited, set);
        });
    }

    private void Float(InspectorContext context, WowZoneLight light, string label, float value, Action<float> set)
    {
        context.Fields.Field(label, () =>
        {
            float edited = value;
            if (ImGui.DragFloat(label, ref edited, 1.0f))
            {
                set(edited);
            }

            _tracker.Track(context.Sessions, light, label, edited, set);
        });
    }
}
