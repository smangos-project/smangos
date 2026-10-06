using System;
using System.Collections.Generic;
using ImGuiNET;

namespace WorldMapStudio.WoW;

/// <summary>Base for inspectors that show the stock transform fields followed by a few typed scalar fields.</summary>
public abstract class WowFieldInspector<T> : EntityInspector<T> where T : SceneEntity
{
    private readonly SceneEntityInspector _base;
    private readonly FieldEditTracker _tracker = new();

    protected WowFieldInspector(InspectorWindow window)
    {
        _base = new SceneEntityInspector(window);
    }

    protected abstract void DrawFields(InspectorContext context, T target);

    protected override void DrawTargets(InspectorContext context, IReadOnlyList<T> targets)
    {
        _base.Draw(context, targets);
        if (targets.Count != 1)
        {
            return;
        }

        context.Fields.Separator();
        DrawFields(context, targets[0]);
    }

    protected void Text(InspectorContext context, T target, string label, string value, Action<string> set)
    {
        context.Fields.Field(label, () =>
        {
            string edited = value;
            if (ImGui.InputText(label, ref edited, 255))
            {
                set(edited);
            }

            _tracker.Track(context.Sessions, target, label, edited, set);
        });
    }

    protected void Int(InspectorContext context, T target, string label, int value, Action<int> set)
    {
        context.Fields.Field(label, () =>
        {
            int edited = value;
            if (ImGui.InputInt(label, ref edited))
            {
                set(edited);
            }

            _tracker.Track(context.Sessions, target, label, edited, set);
        });
    }

    protected void UInt(InspectorContext context, T target, string label, uint value, Action<uint> set)
    {
        Int(context, target, label, (int)value, v => set((uint)Math.Max(v, 0)));
    }

    protected void Float(InspectorContext context, T target, string label, float value, Action<float> set, float speed = 0.1f)
    {
        context.Fields.Field(label, () =>
        {
            float edited = value;
            if (ImGui.DragFloat(label, ref edited, speed))
            {
                set(edited);
            }

            _tracker.Track(context.Sessions, target, label, edited, set);
        });
    }
}
