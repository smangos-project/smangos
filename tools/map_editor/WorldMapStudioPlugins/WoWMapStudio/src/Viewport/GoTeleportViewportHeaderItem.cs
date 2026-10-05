using System.Globalization;
using Godot;
using ImGuiNET;

namespace WorldMapStudio.WoW;

/// <summary>
/// Viewport header button that copies the camera's current position to the clipboard as a
/// <c>.go xyz X Y Z</c> teleport command, converting the Godot scene position to server
/// world space via <see cref="CreatureCoordinates"/>. Self-registers into <see cref="ViewportHeader"/>.
/// </summary>
[Subsystem(nameof(ViewportHeader))]
public sealed class GoTeleportViewportHeaderItem : IViewportHeaderItem
{
    private string? _lastCopied;

    // Constructed by the generated ViewportHeader.InitializeSubsystems(); nothing from the host is needed.
    public GoTeleportViewportHeaderItem(ViewportHeader header)
    {
    }

    public void Draw(in ViewportHeaderContext context)
    {
        if (ImGui.Button("Copy .go"))
        {
            _lastCopied = BuildCommand(context.Camera.GlobalTransform.Origin);
            DisplayServer.ClipboardSet(_lastCopied);
        }

        if (ImGui.IsItemHovered())
        {
            ImGui.SetTooltip(_lastCopied is null
                ? "Copy this camera position as a \".go xyz\" teleport command"
                : $"Copied to clipboard:\n{_lastCopied}");
        }
    }

    private static string BuildCommand(Vector3 cameraGodotPosition)
    {
        Vector3 server = CreatureCoordinates.PositionFromGodot(cameraGodotPosition);

        return string.Format(
            CultureInfo.InvariantCulture,
            ".go xyz {0:0.###} {1:0.###} {2:0.###}",
            server.X,
            server.Y,
            server.Z);
    }
}
