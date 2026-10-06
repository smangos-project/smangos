using System.Collections.Generic;
using System.Linq;
using Godot;
using ImGuiNET;

namespace WorldMapStudio.WoW;

/// <summary>
/// Viewport header readout of the zone and area the camera is over — the <c>wow.area</c> terrain
/// attribute of the chunk under the camera, resolved to names through the <see cref="AreaTable"/>
/// catalog. "Area" is the leaf (the specific subzone); "Zone" is its top-level ancestor. When it
/// cannot resolve it says why, rather than a bare dash, so a missing declaration / unbuilt chunk /
/// unknown id are told apart. Self-registers into <see cref="ViewportHeader"/>; a sibling of
/// <see cref="GoToAdtViewportHeaderItem"/>.
/// </summary>
[Subsystem(nameof(ViewportHeader))]
public sealed class CurrentAreaViewportHeaderItem : IViewportHeaderItem
{
    private readonly EditorContext _context;

    // After the "Go to" buttons (1, 2), so the row reads action buttons then this readout.
    public float Priority => 10f;

    // Constructed by the generated ViewportHeader.InitializeSubsystems().
    public CurrentAreaViewportHeaderItem(ViewportHeader header)
    {
        _context = header.Context;
    }

    public void Draw(in ViewportHeaderContext context)
    {
        (string text, string? tooltip) = Describe(context.Camera.GlobalTransform.Origin);
        ImGui.TextDisabled(text);
        if (tooltip is not null && ImGui.IsItemHovered())
        {
            ImGui.SetTooltip(tooltip);
        }
    }

    private (string Text, string? Tooltip) Describe(Vector3 cameraWorld)
    {
        if (_context.Landscape is not { IsEnabled: true } landscape || landscape.Grid is not { } grid)
        {
            return ("Zone: (no landscape)", null);
        }

        ChunkCoord coord = grid.CoordAt(cameraWorld);
        if (landscape.ChunkIndex.OutputAt(coord) is not { } output)
        {
            return ($"Zone: (chunk {coord.X},{coord.Y} not built)", null);
        }

        if (!output.Attributes.TryGetValue(WowAdtProfile.AreaAttributeKey, out TerrainAttributeGrid areaGrid))
        {
            string declared = landscape.Catalog.Attributes.Any(a => a.Key == WowAdtProfile.AreaAttributeKey)
                ? "no claim wrote wow.area here"
                : "map declares no wow.area attribute";
            return ($"Zone: ({declared})", null);
        }

        uint id = areaGrid.Single;
        if (id == 0)
        {
            return ("Zone: (area 0)", "wow.area evaluated to 0 — the adt_chunk channel had no value at this chunk");
        }

        List<AreaTable> allAreas = _context.Catalog.OfType<AreaTable>().ToList();
        AreaTable? area = allAreas.FirstOrDefault(row => row.AreaId == (int)id);
        if (area is null)
        {
            return ($"Zone: (unknown area {id})",
                allAreas.Count == 0 ? "dbc_areatable is not loaded" : $"no dbc_areatable row with id {id}");
        }

        AreaTable zone = TopLevel(area, allAreas);
        string text = zone.AreaId == area.AreaId
            ? $"Zone: {Name(area)}  |  Area: {Name(area)}"
            : $"Zone: {Name(zone)}  |  Area: {Name(area)}";
        return (text, $"wow.area id {area.AreaId} under the camera (zone {zone.AreaId})");
    }

    // Walks ParentAreaId to the outermost ancestor still in the catalog, with a visited guard so a
    // malformed self- or cyclic reference can't spin.
    private static AreaTable TopLevel(AreaTable area, List<AreaTable> all)
    {
        var seen = new HashSet<int> { area.AreaId };
        AreaTable current = area;
        while (current.ParentAreaId != 0
            && all.FirstOrDefault(row => row.AreaId == current.ParentAreaId) is { } parent
            && seen.Add(parent.AreaId))
        {
            current = parent;
        }

        return current;
    }

    private static string Name(AreaTable area) => area.AreaName.Length > 0 ? area.AreaName : $"Area {area.AreaId}";
}
