using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using Godot;
using NVector3 = System.Numerics.Vector3;

namespace WorldMapStudio.WoW;

/// <summary>An imported vertex and its height, as scripts see it. <c>Col</c> and <c>Row</c> name the imported chunk
/// that holds the vertex. <c>Vx</c>, <c>Vz</c> are its place in that chunk: a corner's index from 0 to 8, or the
/// index from 0 to 7 of the cell whose centre it is when <c>Centre</c> is true.</summary>
public sealed class AdtVertexDescriptor
{
    public AdtVertexDescriptor(AdtChunkRef chunk, int vx, int vz, bool centre, bool loaded, double height)
    {
        Centre = centre;
        TileX = chunk.Tile.FileX;
        TileY = chunk.Tile.FileY;
        Col = chunk.Col;
        Row = chunk.Row;
        Vx = vx;
        Vz = vz;
        Loaded = loaded;
        Height = height;
    }

    [ScriptProperty] public int TileX { get; }
    [ScriptProperty] public int TileY { get; }
    [ScriptProperty] public int Col { get; }
    [ScriptProperty] public int Row { get; }
    [ScriptProperty] public int Vx { get; }
    [ScriptProperty] public int Vz { get; }

    /// <summary>Whether this is the vertex at the centre of a cell rather than a corner.</summary>
    [ScriptProperty] public bool Centre { get; }

    /// <summary>False when the vertex's data is not loaded yet, in which case <c>Height</c> is 0.</summary>
    [ScriptProperty] public bool Loaded { get; }

    [ScriptProperty] public double Height { get; }
}

/// <summary>The heights of one chunk's vertices.</summary>
public sealed class AdtChunkHeightsDescriptor
{
    public AdtChunkHeightsDescriptor(double[] corners, double[] centres)
    {
        Corners = corners;
        Centres = centres;
    }

    /// <summary>The 81 corner heights.</summary>
    [ScriptProperty] public double[] Corners { get; }

    /// <summary>The 64 cell-centre heights.</summary>
    [ScriptProperty] public double[] Centres { get; }
}

/// <summary>What a vertex brush stroke did.</summary>
public sealed class AdtVertexStrokeResultDescriptor
{
    public AdtVertexStrokeResultDescriptor(AdtVertexStrokeResult result)
    {
        Changed = result.Changed;
        Loading = result.Loading;
        Frozen = result.Frozen;
    }

    [ScriptProperty] public bool Changed { get; }

    /// <summary>Chunks that could not be fully edited because their data is not loaded yet.</summary>
    [ScriptProperty] public int Loading { get; }

    /// <summary>Chunks that could not be fully edited because part of their data lies outside the image.</summary>
    [ScriptProperty] public int Frozen { get; }
}

/// <summary>What a whole-chunk vertex operation did.</summary>
public sealed class AdtVertexEditResultDescriptor
{
    public AdtVertexEditResultDescriptor(AdtVertexEditResult result)
    {
        Changed = result.Changed;
        Skipped = result.Skipped;
    }

    [ScriptProperty] public int Changed { get; }
    [ScriptProperty] public int Skipped { get; }
}

/// <summary>The height tool's settings as scripts see them.</summary>
public sealed class AdtHeightBrushDescriptor : BrushDescriptor
{
    public AdtHeightBrushDescriptor(Brush brush, AdtHeightOptions options)
        : base(brush)
    {
        Rate = options.Rate;
        Side = options.Side.ToString();
        HasTarget = options.TargetHeight != null;
        Target = options.TargetHeight ?? 0.0;
        LockTarget = options.LockTarget;
    }

    [ScriptProperty] public double Rate { get; }

    /// <summary>Which vertices flatten moves: <c>Both</c>, <c>RaiseOnly</c> or <c>LowerOnly</c>.</summary>
    [ScriptProperty] public string Side { get; }

    [ScriptProperty] public bool HasTarget { get; }
    [ScriptProperty] public double Target { get; }
    [ScriptProperty] public bool LockTarget { get; }
}

/// <summary>
/// The ADT height tool as <c>wms.adtHeights</c>: everything the tool does, on imported chunks. Brush options are the
/// shared vocabulary (<c>Radius</c>, <c>Strength</c>, <c>Hardness</c>, ...) plus <c>Rate</c> for raising and
/// <c>Target</c> and <c>Side</c> for flattening. A chunk is addressed as <c>{ TileX, TileY, Col, Row }</c> or
/// <c>[tileX, tileY, col, row]</c>, a stroke as <c>[x, z]</c> points in world units. Each edit is one undo step.
/// </summary>
[Subsystem(nameof(ScriptingSystem))]
public sealed class AdtHeightScriptApi : IScriptModule
{
    private readonly EditorContext _context;

    public AdtHeightScriptApi(ScriptingSystem system)
    {
        _context = system.Context;
    }

    public string Name => "adtHeights";

    /// <summary>Supplies the world in place of the current map's. Tests only.</summary>
    internal Func<AdtMapWorld?>? WorldSource { get; set; }

    private AdtHeightToolFactory Factory => _context.Tools.Factories.OfType<AdtHeightToolFactory>().Single();

    /// <summary>The tool's brush and sculpt settings.</summary>
    [ScriptProperty]
    public AdtHeightBrushDescriptor CurrentBrush => new(Factory.Brush, Factory.Options);

    /// <summary>Changes the tool's settings: any brush option, <c>Rate</c>, <c>Target</c> (a height, or null for the
    /// height under a stroke's first dab), <c>Side</c>, <c>LockTarget</c>.</summary>
    [ScriptFunction]
    public AdtHeightBrushDescriptor SetBrush(object? options)
    {
        Apply(Factory.Brush, Factory.Options, options);
        return CurrentBrush;
    }

    /// <summary>The height at the vertex nearest a world point, corner or cell centre.</summary>
    [ScriptFunction]
    public AdtVertexDescriptor At(double x, double z)
    {
        AdtMapWorld world = RequireWorld();
        AdtVertex vertex = world.NearestVertex(new Vector3((float)x, 0.0f, (float)z));
        AdtChunkRef holder = AdtVertexProbe.ImportedHolder(world, vertex);
        bool loaded = AdtVertexProbe.TryRead(world, AdtVertexKind.Height, vertex, out NVector3 height);
        (int vx, int vz) = vertex.IndexIn(holder.Coord);
        return new AdtVertexDescriptor(holder, vx, vz, vertex.IsCentre, loaded, loaded ? height.X : 0.0);
    }

    /// <summary>A chunk's vertex heights in editor order, the row along +Z the slow index: <c>Corners</c> are the 81
    /// heights of its 9 by 9 corners, <c>Centres</c> the 64 of its 8 by 8 cell centres.</summary>
    [ScriptFunction]
    public AdtChunkHeightsDescriptor Vertices(int tileX, int tileY, int col, int row)
    {
        AdtMapWorld world = RequireWorld();
        ChunkCoord coord = AdtScriptArguments.ChunkAt(tileX, tileY, col, row);
        if (!world.TryChunk(coord, out _))
        {
            throw new InvalidOperationException("That chunk is not imported.");
        }

        double[] Read(IEnumerable<AdtVertex> vertices)
        {
            var heights = new List<double>();
            foreach (AdtVertex vertex in vertices)
            {
                if (!AdtVertexProbe.TryRead(world, AdtVertexKind.Height, vertex, out NVector3 height))
                {
                    throw new InvalidOperationException("That chunk cannot be read yet: its height data is not loaded.");
                }

                heights.Add(height.X);
            }

            return heights.ToArray();
        }

        return new AdtChunkHeightsDescriptor(Read(AdtVertexEdit.CornersOf(coord)), Read(AdtVertexEdit.CentresOf(coord)));
    }

    /// <summary>Raises the ground along <paramref name="points"/> as one stroke. <paramref name="options"/> overrides
    /// the tool's settings for this call only.</summary>
    [ScriptFunction]
    public AdtVertexStrokeResultDescriptor Raise(object? points, object? options = null) =>
        Stroke(points, options, invert: false, height => new AdtRaiseOp { Rate = height.Rate });

    /// <summary>Lowers the ground along <paramref name="points"/> as one stroke.</summary>
    [ScriptFunction]
    public AdtVertexStrokeResultDescriptor Lower(object? points, object? options = null) =>
        Stroke(points, options, invert: true, height => new AdtRaiseOp { Rate = height.Rate });

    /// <summary>Flattens the ground along <paramref name="points"/>. Without a <c>Target</c> it flattens to the height
    /// under the first dab.</summary>
    [ScriptFunction]
    public AdtVertexStrokeResultDescriptor Flatten(object? points, object? options = null) =>
        Stroke(points, options, invert: false, height => new AdtFlattenOp { Target = height.TargetHeight, Side = height.Side });

    /// <summary>Smooths the ground along <paramref name="points"/>.</summary>
    [ScriptFunction]
    public AdtVertexStrokeResultDescriptor Smooth(object? points, object? options = null) =>
        Stroke(points, options, invert: false, _ => new AdtSmoothOp());

    /// <summary>Sets every vertex of the given chunks to <paramref name="height"/>.</summary>
    [ScriptFunction]
    public AdtVertexEditResultDescriptor SetChunks(object? chunks, double height) =>
        new(new AdtVertexEdit(RequireWorld(), _context.EditSessions)
            .Fill(AdtVertexKind.Height, AdtScriptArguments.Chunks(chunks), new NVector3((float)height, 0.0f, 0.0f)));

    private AdtVertexStrokeResultDescriptor Stroke(object? points, object? options, bool invert, Func<AdtHeightOptions, IAdtVertexOp> op)
    {
        AdtMapWorld world = RequireWorld();
        Brush brush = Factory.Brush.Clone();
        AdtHeightOptions height = Factory.Options.Clone();
        height.TargetHeight = null;
        height.LockTarget = false;
        Apply(brush, height, options);
        if (invert)
        {
            brush.Invert = true;
        }

        return new AdtVertexStrokeResultDescriptor(AdtVertexStroke.Run(
            world, _context.EditSessions, AdtVertexKind.Height, op(height), brush, AdtScriptArguments.Points(points), "Sculpt terrain"));
    }

    private AdtMapWorld RequireWorld()
    {
        AdtMapWorld? world = WorldSource != null
            ? WorldSource()
            : AdtMapWorld.FromEditor(_context, _context.Maps.CurrentMap, out _);
        return world is { } found && found.PlacementFor(AdtImportCatalog.HeightChannelName) != null
            ? found
            : throw new InvalidOperationException("The current map has no imported ADT height data.");
    }

    private static void Apply(Brush brush, AdtHeightOptions height, object? options)
    {
        BrushScriptOptions.Apply(brush, options, (key, value) =>
        {
            switch (key.ToLowerInvariant())
            {
                case "rate":
                    height.Rate = (float)BrushScriptOptions.ToDouble(value);
                    return true;
                case "target":
                    height.TargetHeight = value == null ? null : (float)BrushScriptOptions.ToDouble(value);
                    return true;
                case "side":
                    height.Side = ParseSide(Convert.ToString(value, CultureInfo.InvariantCulture));
                    return true;
                case "locktarget":
                    height.LockTarget = BrushScriptOptions.ToBool(value);
                    return true;
                case "points":
                    return true;
                default:
                    return false;
            }
        });
    }

    private static AdtFlattenSide ParseSide(string? name) => name?.Replace(" ", "").ToLowerInvariant() switch
    {
        "both" => AdtFlattenSide.Both,
        "raiseonly" or "raise" => AdtFlattenSide.RaiseOnly,
        "loweronly" or "lower" => AdtFlattenSide.LowerOnly,
        _ => throw new ArgumentException("Side must be Both, RaiseOnly or LowerOnly."),
    };
}
