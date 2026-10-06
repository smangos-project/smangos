using System;
using System.Collections;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Terrain holes of imported chunks as <c>wms.adtHoles</c>. A chunk is addressed as <c>{ TileX, TileY, Col, Row }</c>
/// or <c>[tileX, tileY, col, row]</c>; its 16 hole cells are in editor axes, the row along +Z the slow index.
/// Each edit is one undo step.
/// </summary>
[Subsystem(nameof(ScriptingSystem))]
public sealed class AdtHoleScriptApi : IScriptModule
{
    private readonly EditorContext _context;

    public AdtHoleScriptApi(ScriptingSystem system)
    {
        _context = system.Context;
    }

    public string Name => "adtHoles";

    /// <summary>Supplies the world in place of the current map's. Tests only.</summary>
    internal Func<AdtMapWorld?>? WorldSource { get; set; }

    /// <summary>A chunk's 16 hole cells, true where cut.</summary>
    [ScriptFunction]
    public bool[] Cells(int tileX, int tileY, int col, int row) =>
        Edit().Read(AdtScriptArguments.ChunkAt(tileX, tileY, col, row))
        ?? throw new InvalidOperationException("That chunk is not imported or its hole data is not loaded.");

    /// <summary>Cuts holes in the chunks: all their cells, or just <paramref name="cells"/> (indices 0 to 15).
    /// Returns how many chunks changed.</summary>
    [ScriptFunction]
    public int Cut(object? chunks, object? cells = null) =>
        Edit().Set(AdtScriptArguments.Chunks(chunks), cut: true, CellList(cells));

    /// <summary>Fills holes in the chunks: all their cells, or just <paramref name="cells"/>.</summary>
    [ScriptFunction]
    public int Fill(object? chunks, object? cells = null) =>
        Edit().Set(AdtScriptArguments.Chunks(chunks), cut: false, CellList(cells));

    /// <summary>Cuts the hole cell under each <c>[x, z]</c> world point.</summary>
    [ScriptFunction]
    public int CutAt(object? points) => Edit().SetAt(AdtScriptArguments.Points(points), cut: true);

    /// <summary>Fills the hole cell under each <c>[x, z]</c> world point.</summary>
    [ScriptFunction]
    public int FillAt(object? points) => Edit().SetAt(AdtScriptArguments.Points(points), cut: false);

    private static int[]? CellList(object? cells) =>
        cells is IEnumerable and not string ? AdtScriptArguments.Numbers(cells).Select(value => (int)value).ToArray() : null;

    private AdtHoleEdit Edit()
    {
        AdtMapWorld? world = WorldSource != null
            ? WorldSource()
            : AdtMapWorld.FromEditor(_context, _context.Maps.CurrentMap, out _);
        return world is { } found && AdtHoleEdit.HasData(found)
            ? new AdtHoleEdit(found, _context.EditSessions)
            : throw new InvalidOperationException("The current map has no imported ADT hole data.");
    }
}
