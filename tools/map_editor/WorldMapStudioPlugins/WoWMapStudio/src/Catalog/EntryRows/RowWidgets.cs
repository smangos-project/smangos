using System;
using ImGuiNET;

namespace WorldMapStudio.WoW;

/// <summary>Number boxes for one row field; a change builds and records an undoable command.</summary>
internal static class RowWidgets
{
    public static void Int(EditorContext context, string label, int current, Func<double, IEditCommand> edit)
    {
        int value = current;
        ImGui.SetNextItemWidth(140.0f);
        if (ImGui.InputInt(label, ref value) && value != current)
        {
            Record(context, edit(value));
        }
    }

    public static void Float(EditorContext context, string label, float current, Func<double, IEditCommand> edit)
    {
        float value = current;
        ImGui.SetNextItemWidth(140.0f);
        if (ImGui.DragFloat(label, ref value, 0.1f) && !value.Equals(current))
        {
            Record(context, edit(value));
        }
    }

    private static void Record(EditorContext context, IEditCommand command)
    {
        command.Apply();
        context.EditSessions.Record(command);
    }
}
