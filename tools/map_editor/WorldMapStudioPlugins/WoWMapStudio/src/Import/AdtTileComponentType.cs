using System.Linq;
using ImGuiNET;

namespace WorldMapStudio.WoW;

/// <summary>Registers <see cref="AdtTileComponent"/> so a loaded scene can show one. It is created by
/// the importer, never from the inspector's Add menu, so <see cref="CanAddTo"/> is closed and the
/// inspector is a read-only summary — editing which textures a chunk uses comes later.</summary>
[Subsystem(nameof(SceneComponentRegistry))]
public sealed class AdtTileComponentType : ISceneComponentType
{
    public AdtTileComponentType(SceneComponentRegistry registry)
    {
    }

    public float Priority => 2.0f;

    public string TypeId => AdtTileComponent.Kind;

    public string DisplayName => "WoW ADT Tile";

    public bool CanAddTo(SceneEntity entity) => false;

    public SceneComponent Create() => new AdtTileComponent();

    public void DrawInspector(InspectorContext context, SceneComponent component)
    {
        var tile = (AdtTileComponent)component;
        FieldFilter fields = context.Fields;

        fields.Chrome(() =>
        {
            ImGui.TextDisabled($"Source tile {tile.FileX}, {tile.FileY}");
            ImGui.TextDisabled($"{tile.Chunks.Count} imported chunk(s)");

            int slotted = tile.Chunks.Values.Count(facts => facts.Slots.Count > 0);
            if (slotted > 0)
            {
                ImGui.TextDisabled($"{slotted} with texture layers");
            }

            if (tile.ChunkDataLayerId is not null)
            {
                ImGui.TextDisabled("area id + flags → wow.area / wow.mcnk_flags");
            }
        });
    }
}
