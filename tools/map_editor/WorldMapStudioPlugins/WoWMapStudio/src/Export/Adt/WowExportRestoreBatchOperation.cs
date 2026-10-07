using System;
using System.Text.Json.Nodes;
using System.Threading.Tasks;
using ImGuiNET;

namespace WorldMapStudio.WoW;

/// <summary>Puts the client files back as they were before the first export (see <see cref="WowExportBackup"/>).</summary>
[Subsystem(nameof(BatchSystem))]
public sealed class WowExportRestoreBatchOperation : IBatchOperation
{
    public string Id => "wow.export.restore";

    public string DisplayName => "WoW Export Restore";

    public string Description =>
        "Restores every client file overwritten by the ADT and map exports and deletes the files they created.";

    public WowExportRestoreBatchOperation(BatchSystem batch)
    {
    }

    public void DrawSettings() => ImGui.TextWrapped("Reverts all exported client files to the saved originals.");

    public JsonObject SaveSettings() => new();

    public void LoadSettings(JsonObject settings)
    {
    }

    public Task RunAsync(BatchContext context, WorkContext work)
    {
        string content = context.Editor.WowClientPaths.Content
            ?? throw new InvalidOperationException(WowClientPaths.MissingContentMessage);
        if (WowExportBackup.ClientRunning())
        {
            throw new InvalidOperationException("The WoW client is running; close it before restoring.");
        }

        int count = new WowExportBackup(content).Restore();
        context.Report($"Restored {count} file(s).");
        return Task.CompletedTask;
    }
}
