using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Godot;
using ImGuiNET;
using NVector2 = System.Numerics.Vector2;
using NVector4 = System.Numerics.Vector4;

namespace WorldMapStudio.WoW;

/// <summary>
/// The texture stack of the selected imported chunks (or the one under the cursor): each layer's texture,
/// how much of the chunk it covers and its ground-effect id, with reorder, remove and replace, plus clean-up
/// and duplicate removal for the selection or its whole tile. Every change is one undo step.
/// </summary>
[Subsystem(nameof(WindowManager))]
public sealed class AdtChunkTexturesWindow : Window
{
    private const float ThumbnailSize = 32.0f;

    private readonly EditorContext _context;
    private readonly Dictionary<string, Task<Texture2D?>> _thumbnails = new(StringComparer.OrdinalIgnoreCase);

    private AdtChunkEdit? _loaded;
    private AdtChunkFacts? _loadedFacts;
    private ChunkCoord _loadedCoord;
    private int _loadedRevision = -1;
    private bool _wholeTile;
    private string _status = "";

    public AdtChunkTexturesWindow(WindowManager manager)
        : base("ADT Chunk Textures", startOpen: false, defaultSize: new NVector2(460.0f, 420.0f))
    {
        _context = manager.Context;
    }

    public override string? Category => "WoW";

    private AdtTextureToolFactory? Factory => _context.Tools.Factories.OfType<AdtTextureToolFactory>().FirstOrDefault();

    protected override void DrawContent()
    {
        if (Factory is not { } factory)
        {
            ImGui.TextDisabled("The ADT texture tool is not available.");
            return;
        }

        if (factory.World() is not { } world)
        {
            ImGui.TextDisabled(factory.WorldProblem ?? "This map has no imported ADT data.");
            return;
        }

        IReadOnlyList<ChunkCoord> targets = factory.Targets()
            .Where(coord => world.TryChunk(coord, out _))
            .ToList();
        if (targets.Count == 0)
        {
            ImGui.TextDisabled("No chunk selected. Use the ADT Textures tool's Select mode, or hover a chunk.");
            return;
        }

        ImGui.TextUnformatted(factory.Selection.Count > 0
            ? $"{targets.Count} chunk(s) selected"
            : "Chunk under the cursor");

        if (targets.Count == 1)
        {
            DrawChunk(factory, world, targets[0]);
        }
        else
        {
            DrawSelection(factory, world, targets);
        }

        ImGui.Separator();
        DrawActions(factory, world, targets);
        if (_status.Length > 0)
        {
            ImGui.TextDisabled(_status);
        }
    }

    private void DrawChunk(AdtTextureToolFactory factory, AdtTextureWorld world, ChunkCoord coord)
    {
        world.TryChunk(coord, out AdtChunkRef chunk);
        AdtChunkFacts facts = chunk.Tile.Chunks[(chunk.Col, chunk.Row)];
        ImGui.TextDisabled($"Tile {chunk.Tile.FileX},{chunk.Tile.FileY}  chunk {chunk.Col},{chunk.Row}");

        if (world.OverrideAt(chunk) is { } overridden)
        {
            ImGui.TextColored(new NVector4(1.0f, 0.6f, 0.2f, 1.0f), AdtTextureTool.OverrideText(overridden));
        }

        int revision = world.Image.ContentRevision;
        if (_loaded == null || _loadedCoord != coord || _loadedRevision != revision || !ReferenceEquals(_loadedFacts, facts))
        {
            AdtChunkEdit.TryLoad(world, coord, out _loaded);
            _loadedCoord = coord;
            _loadedRevision = revision;
            _loadedFacts = facts;
        }

        if (_loaded == null)
        {
            ImGui.TextDisabled("This chunk cannot be read yet.");
            return;
        }

        AdtChunkTextureSet set = _loaded.Set;
        var edit = new AdtTextureEdit(world, _context.EditSessions);
        if (!ImGui.BeginTable("layers", 5, ImGuiTableFlags.RowBg | ImGuiTableFlags.SizingStretchProp))
        {
            return;
        }

        ImGui.TableSetupColumn("Texture", ImGuiTableColumnFlags.WidthStretch, 3.0f);
        ImGui.TableSetupColumn("Covers", ImGuiTableColumnFlags.WidthFixed, 56.0f);
        ImGui.TableSetupColumn("Effect", ImGuiTableColumnFlags.WidthFixed, 52.0f);
        ImGui.TableSetupColumn("Order", ImGuiTableColumnFlags.WidthFixed, 56.0f);
        ImGui.TableSetupColumn("", ImGuiTableColumnFlags.WidthFixed, 120.0f);

        for (int slot = set.Count - 1; slot >= 0; slot--)
        {
            string texture = set.Textures[slot];
            ImGui.PushID(slot);
            ImGui.TableNextRow();

            ImGui.TableNextColumn();
            DrawThumbnail(texture);
            ImGui.SameLine();
            ImGui.TextUnformatted($"{slot}  {NameOf(texture)}");
            if (ImGui.IsItemHovered())
            {
                ImGui.SetTooltip(texture);
            }

            ImGui.TableNextColumn();
            ImGui.TextUnformatted($"{set.Coverage(slot) * 100.0f:0}%");

            ImGui.TableNextColumn();
            int? effect = world.Materials.EffectIdOf(facts.Slots.ElementAtOrDefault(slot)?.MaterialId);
            ImGui.TextUnformatted(effect?.ToString() ?? "-");

            ImGui.TableNextColumn();
            if (slot < set.Count - 1 && ImGui.ArrowButton("up", ImGuiDir.Up))
            {
                Report(edit.Swap(coord, slot, slot + 1), "Swapped");
            }

            ImGui.SameLine();
            if (slot > 0 && ImGui.ArrowButton("down", ImGuiDir.Down))
            {
                Report(edit.Swap(coord, slot, slot - 1), "Swapped");
            }

            ImGui.TableNextColumn();
            if (ImGui.SmallButton("Use"))
            {
                factory.Use(texture);
            }

            ImGui.SameLine();
            ImGui.BeginDisabled(!CanReplaceWith(factory, texture));
            if (ImGui.SmallButton("Replace"))
            {
                Report(edit.Replace(texture, factory.Options.Texture, [coord]), "Replaced");
            }

            ImGui.EndDisabled();
            ImGui.SameLine();
            ImGui.BeginDisabled(set.Count <= 1);
            if (ImGui.SmallButton("X"))
            {
                Report(edit.RemoveSlot(coord, slot), "Removed");
            }

            ImGui.EndDisabled();
            ImGui.PopID();
        }

        ImGui.EndTable();
    }

    private void DrawSelection(AdtTextureToolFactory factory, AdtTextureWorld world, IReadOnlyList<ChunkCoord> targets)
    {
        var counts = new Dictionary<string, (string Texture, int Chunks)>();
        int overridden = 0;
        foreach (ChunkCoord coord in targets)
        {
            world.TryChunk(coord, out AdtChunkRef chunk);
            if (world.OverrideAt(chunk) != null)
            {
                overridden++;
            }

            if (world.TexturesOf(chunk.Tile.Chunks[(chunk.Col, chunk.Row)]) is not { } textures)
            {
                continue;
            }

            foreach (string texture in textures.Distinct(StringComparer.OrdinalIgnoreCase))
            {
                string key = AdtTexturePath.Key(texture);
                counts[key] = counts.TryGetValue(key, out var known) ? (known.Texture, known.Chunks + 1) : (texture, 1);
            }
        }

        if (overridden > 0)
        {
            ImGui.TextColored(new NVector4(1.0f, 0.6f, 0.2f, 1.0f), $"{overridden} chunk(s) render differently from their slots");
        }

        var edit = new AdtTextureEdit(world, _context.EditSessions);
        IReadOnlyList<ChunkCoord> scope = Scope(world, targets);
        foreach ((string texture, int chunks) in counts.Values.OrderByDescending(entry => entry.Chunks))
        {
            ImGui.PushID(texture);
            DrawThumbnail(texture);
            ImGui.SameLine();
            ImGui.TextUnformatted($"{NameOf(texture)}  in {chunks} chunk(s)");
            ImGui.SameLine();
            if (ImGui.SmallButton("Use"))
            {
                factory.Use(texture);
            }

            ImGui.SameLine();
            ImGui.BeginDisabled(!CanReplaceWith(factory, texture));
            if (ImGui.SmallButton("Replace"))
            {
                Report(edit.Replace(texture, factory.Options.Texture, scope), "Replaced");
            }

            ImGui.EndDisabled();
            ImGui.SameLine();
            if (ImGui.SmallButton("Remove"))
            {
                Report(edit.Remove(texture, scope), "Removed");
            }

            ImGui.PopID();
        }
    }

    private void DrawActions(AdtTextureToolFactory factory, AdtTextureWorld world, IReadOnlyList<ChunkCoord> targets)
    {
        ImGui.Checkbox("Apply to the whole tile", ref _wholeTile);
        IReadOnlyList<ChunkCoord> scope = Scope(world, targets);
        var edit = new AdtTextureEdit(world, _context.EditSessions);

        if (ImGui.Button("Clean up unused"))
        {
            Report(edit.Cleanup(scope), "Cleaned up");
        }

        ImGui.SameLine();
        if (ImGui.Button("Remove duplicates"))
        {
            Report(edit.RemoveDuplicates(scope), "Merged duplicates in");
        }

        ImGui.SameLine();
        if (ImGui.Button("Select tile") && world.TryChunk(targets[0], out AdtChunkRef chunk))
        {
            factory.SetSelection(AdtTextureEdit.ChunksOfTile(chunk.Tile));
        }

        ImGui.SameLine();
        if (ImGui.Button("Clear selection"))
        {
            factory.ClearSelection();
        }
    }

    // The chunks an action applies to: the targets, or every imported chunk of the tiles they sit in.
    private IReadOnlyList<ChunkCoord> Scope(AdtTextureWorld world, IReadOnlyList<ChunkCoord> targets)
    {
        if (!_wholeTile)
        {
            return targets;
        }

        var tiles = new HashSet<AdtTileComponent>();
        foreach (ChunkCoord coord in targets)
        {
            if (world.TryChunk(coord, out AdtChunkRef chunk))
            {
                tiles.Add(chunk.Tile);
            }
        }

        return tiles.SelectMany(AdtTextureEdit.ChunksOfTile).ToList();
    }

    private void Report(AdtTextureEditResult result, string verb)
    {
        _status = result.Changed == 0
            ? "Nothing to change."
            : $"{verb} {result.Changed} chunk(s).{(result.Skipped > 0 ? $" {result.Skipped} could not be read." : "")}";
    }

    private static bool CanReplaceWith(AdtTextureToolFactory factory, string texture) =>
        factory.Options.Texture.Length > 0 && !AdtTexturePath.Same(factory.Options.Texture, texture);

    private void DrawThumbnail(string texture)
    {
        var box = new NVector2(ThumbnailSize, ThumbnailSize);
        if (!_thumbnails.TryGetValue(texture, out Task<Texture2D?>? task))
        {
            task = _context.Assets.LoadTextureAssetAsync(texture);
            _thumbnails[texture] = task;
        }

        if (task is { IsCompletedSuccessfully: true, Result: { } image })
        {
            ImGui.Image((IntPtr)image.GetRid().Id, box);
        }
        else
        {
            ImGui.Dummy(box);
        }
    }

    private static string NameOf(string texture)
    {
        string normal = texture.Replace('\\', '/');
        return normal[(normal.LastIndexOf('/') + 1)..];
    }
}
