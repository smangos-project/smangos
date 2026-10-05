using System;
using System.Globalization;
using Godot;
using ImGuiNET;
using Vector2 = System.Numerics.Vector2;
using NVector3 = System.Numerics.Vector3;

namespace WorldMapStudio.WoW;

/// <summary>
/// Viewport header button that opens a modal to fly the camera to a set of server
/// coordinates — the same X/Y/Z a <c>.go xyz</c> command takes — converting to Godot scene space via
/// <see cref="CreatureCoordinates"/>. The inbound counterpart to <see cref="GoTeleportViewportHeaderItem"/>.
/// Self-registers into <see cref="ViewportHeader"/>.
/// </summary>
[Subsystem(nameof(ViewportHeader))]
public sealed class GoToCoordinatesViewportHeaderItem : IViewportHeaderItem
{
    private const string PopupId = "Go to coordinates";

    private readonly ViewportFocus _focus;

    private NVector3 _target;
    private bool _openRequested;
    private bool _active;

    // After GoTeleportViewportHeaderItem's "Copy .go" (0) so the pair reads copy-then-go on the row.
    public float Priority => 1f;

    // Constructed by the generated ViewportHeader.InitializeSubsystems().
    public GoToCoordinatesViewportHeaderItem(ViewportHeader header)
    {
        _focus = header.Context.Focus;
    }

    public void Draw(in ViewportHeaderContext context)
    {
        if (ImGui.Button("Go to..."))
        {
            Vector3 server = CreatureCoordinates.PositionFromGodot(_focus.Position);
            _target = new NVector3(server.X, server.Y, server.Z);
            _openRequested = true;
        }

        if (ImGui.IsItemHovered())
        {
            ImGui.SetTooltip("Fly the camera to server coordinates (.go xyz X Y Z)");
        }

        DrawPopup();
    }

    private void DrawPopup()
    {
        if (_openRequested)
        {
            ImGui.OpenPopup(PopupId);
            _openRequested = false;
            _active = true;
        }

        if (!_active)
        {
            return;
        }

        bool open = true;
        ImGuiEx.PopupModal(PopupId, true, ref open, ImGuiWindowFlags.AlwaysAutoResize, () =>
        {
            ImGui.TextDisabled("Server coordinates, as taken by .go xyz");
            ImGui.InputFloat3("X / Y / Z", ref _target);

            if (ImGui.Button("Paste .go") && TryParseGoCommand(DisplayServer.ClipboardGet(), out NVector3 pasted))
            {
                _target = pasted;
            }

            ImGui.Separator();

            if (ImGui.Button("Go", new Vector2(120f, 0f)))
            {
                _focus.MoveTo(CreatureCoordinates.PositionToGodot(new Vector3(_target.X, _target.Y, _target.Z)));
                open = false;
            }

            ImGui.SameLine();
            if (ImGui.Button("Cancel", new Vector2(120f, 0f)))
            {
                open = false;
            }
        });

        if (!open)
        {
            _active = false;
        }
    }

    // Takes the first three numbers out of a copied ".go xyz X Y Z" command, or a bare "X Y Z".
    private static bool TryParseGoCommand(string text, out NVector3 result)
    {
        result = default;

        Span<float> values = stackalloc float[3];
        int found = 0;
        foreach (string token in text.Split((char[]?)null, StringSplitOptions.RemoveEmptyEntries))
        {
            if (!float.TryParse(token, NumberStyles.Float, CultureInfo.InvariantCulture, out float value))
            {
                continue;
            }

            values[found++] = value;
            if (found == 3)
            {
                result = new NVector3(values[0], values[1], values[2]);
                return true;
            }
        }

        return false;
    }
}
