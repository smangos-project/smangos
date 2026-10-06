using System.Collections.Generic;
using System.Linq;
using ImGuiNET;

namespace WorldMapStudio.WoW;

/// <summary>Edits the `game_graveyard_zone` links of a <see cref="WowGraveyard"/> below the stock transform fields.</summary>
[Subsystem(nameof(InspectorWindow))]
public sealed class WowGraveyardInspector : EntityInspector<WowGraveyard>
{
    private readonly SceneEntityInspector _base;
    private int _newZone;

    public WowGraveyardInspector(InspectorWindow window)
    {
        _base = new SceneEntityInspector(window);
    }

    protected override void DrawTargets(InspectorContext context, IReadOnlyList<WowGraveyard> targets)
    {
        _base.Draw(context, targets);
        if (targets.Count != 1)
        {
            return;
        }

        WowGraveyard yard = targets[0];
        context.Fields.Separator();
        context.Fields.Field("Zones", () =>
        {
            ImGui.TextDisabled("Zones served (faction 0 = both)");
            for (int i = 0; i < yard.Zones.Count; i++)
            {
                WowGraveyardZone zone = yard.Zones[i];
                ImGui.PushID(i);
                ImGui.TextUnformatted($"Zone {zone.GhostZone}");
                ImGui.SameLine();
                int faction = zone.Faction;
                ImGui.SetNextItemWidth(100);
                bool edited = ImGui.InputInt("Faction", ref faction);
                ImGui.SameLine();
                bool remove = ImGui.SmallButton("Remove");
                ImGui.PopID();
                if (remove)
                {
                    Set(context, yard, yard.Zones.Where((_, index) => index != i).ToList());
                    break;
                }

                if (edited && faction != zone.Faction)
                {
                    List<WowGraveyardZone> zones = [.. yard.Zones];
                    zones[i] = zone with { Faction = faction };
                    Set(context, yard, zones);
                }
            }

            ImGui.SetNextItemWidth(100);
            ImGui.InputInt("Zone id", ref _newZone, 0);
            ImGui.SameLine();
            if (ImGui.SmallButton("Add") && _newZone > 0 && yard.Zones.All(z => z.GhostZone != _newZone))
            {
                Set(context, yard, [.. yard.Zones, new WowGraveyardZone(_newZone, 0)]);
            }
        });
    }

    private static void Set(InspectorContext context, WowGraveyard yard, List<WowGraveyardZone> after)
    {
        var command = new SetFieldCommand<List<WowGraveyardZone>>(yard, "zones", v => yard.Zones = v, yard.Zones, after);
        command.Apply();
        context.Sessions.Record(command);
    }
}
