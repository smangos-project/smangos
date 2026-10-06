using System;
using System.Collections.Generic;
using System.Linq;
using ImGuiNET;

namespace WorldMapStudio.WoW;

/// <summary>Edits a <see cref="WowAreaTrigger"/>'s volume and teleport destination below the stock
/// transform fields. The volume is rebuilt after each edit.</summary>
[Subsystem(nameof(InspectorWindow))]
public sealed class WowAreaTriggerInspector : EntityInspector<WowAreaTrigger>
{
    private readonly SceneEntityInspector _base;
    private readonly FieldEditTracker _tracker = new();

    public WowAreaTriggerInspector(InspectorWindow window)
    {
        _base = new SceneEntityInspector(window);
    }

    protected override void DrawTargets(InspectorContext context, IReadOnlyList<WowAreaTrigger> targets)
    {
        _base.Draw(context, targets);
        if (targets.Count != 1)
        {
            return;
        }

        WowAreaTrigger trigger = targets[0];
        FieldFilter fields = context.Fields;
        fields.Separator();
        ImGui.TextDisabled("Volume (sphere when radius > 0)");
        Float(context, trigger, "Radius", trigger.Radius, v => trigger.Radius = v);
        Float(context, trigger, "Box length", trigger.BoxLength, v => trigger.BoxLength = v);
        Float(context, trigger, "Box width", trigger.BoxWidth, v => trigger.BoxWidth = v);
        Float(context, trigger, "Box height", trigger.BoxHeight, v => trigger.BoxHeight = v);
        Float(context, trigger, "Box yaw", trigger.BoxYaw, v => trigger.BoxYaw = v);

        fields.Separator();
        bool teleport = trigger.Teleport != null;
        fields.Field("Teleport", () =>
        {
            if (ImGui.Checkbox("Teleport", ref teleport))
            {
                bool enable = teleport;
                var command = new SetFieldCommand<WowAreaTriggerTeleport?>(trigger, "teleport", v => trigger.Teleport = v,
                    trigger.Teleport, enable ? new WowAreaTriggerTeleport() : null);
                command.Apply();
                context.Sessions.Record(command);
            }
        });

        if (trigger.Teleport is not { } t)
        {
            return;
        }

        fields.Field("Message", () =>
        {
            string message = t.Message;
            if (ImGui.InputText("Message", ref message, 255))
            {
                t.Message = message;
            }

            _tracker.Track(context.Sessions, trigger, "teleport message", t.Message, v => t.Message = v);
        });
        Int(context, trigger, "Required level", t.RequiredLevel, v => t.RequiredLevel = v);
        Int(context, trigger, "Required condition", t.RequiredCondition, v => t.RequiredCondition = v);
        Int(context, trigger, "Target map", t.TargetMap, v => t.TargetMap = v);
        Float(context, trigger, "Target X", t.TargetPosition.X, v => t.TargetPosition = new Godot.Vector3(v, t.TargetPosition.Y, t.TargetPosition.Z), refresh: false);
        Float(context, trigger, "Target Y", t.TargetPosition.Y, v => t.TargetPosition = new Godot.Vector3(t.TargetPosition.X, v, t.TargetPosition.Z), refresh: false);
        Float(context, trigger, "Target Z", t.TargetPosition.Z, v => t.TargetPosition = new Godot.Vector3(t.TargetPosition.X, t.TargetPosition.Y, v), refresh: false);
        Float(context, trigger, "Target orientation", t.TargetOrientation, v => t.TargetOrientation = v, refresh: false);
    }

    private void Float(InspectorContext context, WowAreaTrigger trigger, string label, float value, Action<float> set, bool refresh = true)
    {
        context.Fields.Field(label, () =>
        {
            float edited = value;
            if (ImGui.DragFloat(label, ref edited, 0.1f))
            {
                set(edited);
                if (refresh)
                {
                    RefreshVolume(trigger);
                }
            }

            _tracker.Track(context.Sessions, trigger, label, edited, v =>
            {
                set(v);
                if (refresh)
                {
                    RefreshVolume(trigger);
                }
            });
        });
    }

    private void Int(InspectorContext context, WowAreaTrigger trigger, string label, int value, Action<int> set)
    {
        context.Fields.Field(label, () =>
        {
            int edited = value;
            if (ImGui.InputInt(label, ref edited))
            {
                set(edited);
            }

            _tracker.Track(context.Sessions, trigger, label, edited, set);
        });
    }

    private static void RefreshVolume(WowAreaTrigger trigger)
    {
        foreach (SceneComponent old in trigger.Components.OfType<WowAreaTriggerVolume>().ToList())
        {
            trigger.RemoveComponent(old);
        }

        trigger.AddIntrinsicComponent(WowAreaTriggerVolume.For(trigger));
    }
}
