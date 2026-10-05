using System;
using System.Collections.Generic;
using System.Globalization;
using Godot;
using ImGuiNET;
using Vector2 = System.Numerics.Vector2;

namespace WorldMapStudio.WoW;

/// <summary>
/// Viewport header button that flies the camera to the centre of a WoW ADT tile, addressed by its
/// filename indices (the <c>32_49</c> in <c>Azeroth_32_49.adt</c>). A tile-grid counterpart to
/// <see cref="GoToCoordinatesViewportHeaderItem"/>, sharing its <see cref="CreatureCoordinates"/>
/// server-space round-trip. Self-registers into <see cref="ViewportHeader"/>.
/// </summary>
[Subsystem(nameof(ViewportHeader))]
public sealed class GoToAdtViewportHeaderItem : IViewportHeaderItem
{
    private const string PopupId = "Go to ADT";

    private readonly ViewportFocus _focus;

    private int _fileX;
    private int _fileY;
    private float _serverZ;
    private bool _openRequested;
    private bool _active;

    // After GoToCoordinatesViewportHeaderItem (1) so the row reads coords-then-tile.
    public float Priority => 2f;

    // Constructed by the generated ViewportHeader.InitializeSubsystems().
    public GoToAdtViewportHeaderItem(ViewportHeader header)
    {
        _focus = header.Context.Focus;
    }

    public void Draw(in ViewportHeaderContext context)
    {
        if (ImGui.Button("Go to ADT..."))
        {
            Vector3 server = CreatureCoordinates.PositionFromGodot(_focus.Position);
            (_fileX, _fileY) = WowTileAddressing.TileAtServer(server.X, server.Y);
            _serverZ = server.Z;
            _openRequested = true;
        }

        if (ImGui.IsItemHovered())
        {
            ImGui.SetTooltip("Fly the camera to the centre of a WoW ADT tile (the X_Y in Map_X_Y.adt)");
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
            ImGui.TextDisabled("Tile filename indices, as in Map_X_Y.adt");
            ImGui.InputInt("Tile X", ref _fileX);
            ImGui.InputInt("Tile Y", ref _fileY);

            if (ImGui.Button("Paste name") && TryParseTileName(DisplayServer.ClipboardGet(), out int px, out int py))
            {
                _fileX = px;
                _fileY = py;
            }

            if (ImGui.IsItemHovered())
            {
                ImGui.SetTooltip("Read the two indices from a copied tile name like \"Azeroth_32_49\"");
            }

            (float serverX, float serverY) = WowTileAddressing.TileCentreServer(_fileX, _fileY);
            ImGui.TextDisabled(string.Format(CultureInfo.InvariantCulture, "Centre: {0:0.#}, {1:0.#}", serverX, serverY));

            ImGui.Separator();

            if (ImGui.Button("Go", new Vector2(120f, 0f)))
            {
                _focus.MoveTo(CreatureCoordinates.PositionToGodot(new Vector3(serverX, serverY, _serverZ)));
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

    // The last two integers in a copied tile name or path, e.g. "Azeroth_32_49" or a full ".adt" path.
    private static bool TryParseTileName(string text, out int fileX, out int fileY)
    {
        fileX = 0;
        fileY = 0;

        if (string.IsNullOrEmpty(text))
        {
            return false;
        }

        var numbers = new List<int>();
        foreach (string token in text.Split(new[] { ' ', '_', '\\', '/', '\t', '\r', '\n', '.' }, StringSplitOptions.RemoveEmptyEntries))
        {
            if (int.TryParse(token, NumberStyles.Integer, CultureInfo.InvariantCulture, out int value))
            {
                numbers.Add(value);
            }
        }

        if (numbers.Count < 2)
        {
            return false;
        }

        fileX = numbers[^2];
        fileY = numbers[^1];
        return true;
    }
}
