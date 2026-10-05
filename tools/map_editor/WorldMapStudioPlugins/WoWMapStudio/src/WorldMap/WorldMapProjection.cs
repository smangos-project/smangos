namespace WorldMapStudio.WoW;

/// <summary>
/// A rectangle in WoW world space, named by axis (never by DBC column). The one representation every
/// worldmap-adjacent table's rect gets converted into by its <c>From*</c> factory below — nothing
/// outside those factories should read a `worldmaparea`/`dungeonmap`/`worldmapcontinent`/
/// `worldmaptransforms` row's rect columns directly.
/// </summary>
public readonly record struct WorldRect(float MinWorldX, float MaxWorldX, float MinWorldY, float MaxWorldY)
{
    public float SpanX => MaxWorldX - MinWorldX;
    public float SpanY => MaxWorldY - MinWorldY;
    public float CenterWorldX => (MinWorldX + MaxWorldX) * 0.5f;
    public float CenterWorldY => (MinWorldY + MaxWorldY) * 0.5f;

    /// <summary>Y span / X span. A page or floor rect must read 3:2 or the client stretches it.</summary>
    public float Aspect => SpanX == 0f ? 0f : SpanY / SpanX;

    public bool Contains(float worldX, float worldY) =>
        worldX >= MinWorldX && worldX <= MaxWorldX && worldY >= MinWorldY && worldY <= MaxWorldY;

    /// <summary>Whether this rect and <paramref name="other"/> share any world space (touching counts).</summary>
    public bool Overlaps(WorldRect other) =>
        MinWorldX <= other.MaxWorldX && MaxWorldX >= other.MinWorldX
        && MinWorldY <= other.MaxWorldY && MaxWorldY >= other.MinWorldY;

    /// <summary>`dbc_worldmaparea`'s `LocLeft/LocRight/LocTop/LocBottom`. `LocLeft` is the max world Y
    /// (west edge), `LocRight` the min (east); `LocTop` the max world X (north), `LocBottom` the min
    /// (south).</summary>
    public static WorldRect FromLoc(float locLeft, float locRight, float locTop, float locBottom) =>
        new(MinWorldX: locBottom, MaxWorldX: locTop, MinWorldY: locRight, MaxWorldY: locLeft);

    /// <summary>`dbc_dungeonmap`'s four columns. Generated as `MinX/MaxX/MinY/MaxY` but actually laid
    /// out `min(Y, X), max(Y, X)`: the 1st column is min world Y, 2nd is min world X, 3rd is max world
    /// Y, 4th is max world X. Confirmed against real rows — under the column names' own reading, some
    /// floors have min &gt; max. This is the only place that reinterprets them; the generated record's
    /// properties stay named as generated.</summary>
    public static WorldRect FromDungeonMap(float minXColumn, float maxXColumn, float minYColumn, float maxYColumn) =>
        new(MinWorldX: maxXColumn, MaxWorldX: maxYColumn, MinWorldY: minXColumn, MaxWorldY: minYColumn);

    /// <summary>`dbc_worldmapcontinent`'s ADT tile-index boundary columns. `Left`/`Right` index the
    /// tile file's X (world Y axis), `Top`/`Bottom` its file Y (world X axis).</summary>
    public static WorldRect FromTileBounds(int left, int right, int top, int bottom)
    {
        const float tile = WorldMapProjection.AdtTileSize;
        const int origin = WorldMapProjection.AdtOriginTileIndex;
        return new WorldRect(
            MinWorldX: (origin - 1 - bottom) * tile,
            MaxWorldX: (origin - top) * tile,
            MinWorldY: (origin - 1 - right) * tile,
            MaxWorldY: (origin - left) * tile);
    }

    /// <summary>The inverse of <see cref="FromTileBounds"/>: the ADT tile indices a world rect covers,
    /// rounded to whole tiles — what an edited continent boundary has to be written back as.</summary>
    public (int Left, int Right, int Top, int Bottom) ToTileBounds()
    {
        const float tile = WorldMapProjection.AdtTileSize;
        const int origin = WorldMapProjection.AdtOriginTileIndex;
        return (
            Left: (int)System.MathF.Round(origin - (MaxWorldY / tile)),
            Right: (int)System.MathF.Round(origin - 1 - (MinWorldY / tile)),
            Top: (int)System.MathF.Round(origin - (MaxWorldX / tile)),
            Bottom: (int)System.MathF.Round(origin - 1 - (MinWorldX / tile)));
    }

    /// <summary>`dbc_worldmapcontinent`'s `TaxiMin1/2`/`TaxiMax1/2` — already (X, Y) world, always a square.</summary>
    public static WorldRect FromTaxi(float taxiMinX, float taxiMinY, float taxiMaxX, float taxiMaxY) =>
        new(taxiMinX, taxiMaxX, taxiMinY, taxiMaxY);

    /// <summary>`dbc_worldmaptransforms`'s `RegionMin1/2`/`RegionMax1/2` — (X, Y) in the source map's
    /// world space.</summary>
    public static WorldRect FromTransformRegion(float regionMinX, float regionMinY, float regionMaxX, float regionMaxY) =>
        new(regionMinX, regionMaxX, regionMinY, regionMaxY);

    /// <summary>Grows about the centre until <see cref="Aspect"/> reads exactly 3:2. Never shrinks.</summary>
    public WorldRect FitToAspect()
    {
        if (SpanX <= 0f || SpanY <= 0f)
        {
            return this;
        }

        float targetSpanY = SpanX * WorldMapProjection.PageAspect;
        if (targetSpanY > SpanY)
        {
            float grow = (targetSpanY - SpanY) * 0.5f;
            return this with { MinWorldY = MinWorldY - grow, MaxWorldY = MaxWorldY + grow };
        }

        float targetSpanX = SpanY / WorldMapProjection.PageAspect;
        float growX = (targetSpanX - SpanX) * 0.5f;
        return this with { MinWorldX = MinWorldX - growX, MaxWorldX = MaxWorldX + growX };
    }
}

/// <summary>A point in page-pixel space: origin top-left, (<see cref="WorldMapProjection.PageWidth"/>,
/// <see cref="WorldMapProjection.PageHeight"/>) at the opposite corner.</summary>
public readonly record struct PagePoint(float X, float Y);

/// <summary>
/// Every world-space &#8596; page-pixel conversion the world map editor needs, in one place — nothing
/// in the editor's UI or tools computes its own version of this math.
///
/// A worldmap page is 1002&#215;668 px, origin top-left, drawn over a page rect (see
/// <see cref="WorldRect.FromLoc"/>) whose Y span / X span reads 3:2. West is +world Y, north is
/// +world X, so page pixel u runs from the rect's max Y (west, u=0) to its min Y (east, u=1), and page
/// pixel v from its max X (north, v=0) to its min X (south, v=1) — identical to the server's own
/// `DBCManager::Map2ZoneCoordinates`/`Zone2MapCoordinates` (its coordinate swap is this same "page
/// horizontal = world Y" convention), just expressed as 0..1 here instead of a 0..100 percentage.
/// </summary>
public static class WorldMapProjection
{
    public const float PageWidth = 1002f;
    public const float PageHeight = 668f;

    /// <summary>Y span / X span every page and floor rect must read.</summary>
    public const float PageAspect = 1.5f;

    public const int PageTileSize = 256;
    public const int PageTileColumns = 4;
    public const int PageTileRows = 3;
    public const int ZmpSize = 128;

    public const float AdtTileSize = 533.33333f;
    public const int AdtTilesPerMapEdge = 64;
    public const int AdtOriginTileIndex = AdtTilesPerMapEdge / 2;

    /// <summary>A `.zmp` cell's world size — the whole 64-tile map edge divided into
    /// <see cref="ZmpSize"/> cells, not a page's own extent.</summary>
    public const float ZmpCellSize = (AdtTilesPerMapEdge * AdtTileSize) / ZmpSize;

    private const float ZmpMapExtent = AdtOriginTileIndex * AdtTileSize;

    public static PagePoint WorldToPage(WorldRect page, float worldX, float worldY)
    {
        (float u, float v) = WorldToUv(page, worldX, worldY);
        return new PagePoint(u * PageWidth, v * PageHeight);
    }

    public static (float WorldX, float WorldY) PageToWorld(WorldRect page, PagePoint point) =>
        UvToWorld(page, point.X / PageWidth, point.Y / PageHeight);

    /// <summary>The `/way`-style zone percentage (0..100) a client shows for a world point on this page.</summary>
    public static (float PctX, float PctY) ZonePercent(WorldRect page, float worldX, float worldY)
    {
        (float u, float v) = WorldToUv(page, worldX, worldY);
        return (u * 100f, v * 100f);
    }

    /// <summary>`dbc_worldmaptransforms`'s `RegionOffset1/2`, applied to a point already known to be
    /// inside the transform's region (test with <see cref="WorldRect.Contains"/> first).</summary>
    public static (float WorldX, float WorldY) ApplyTransform(float worldX, float worldY, float offsetX, float offsetY) =>
        (worldX + offsetX, worldY + offsetY);

    /// <summary>An art pixel (the page's own 1002x668 space, drawn over <paramref name="artRect"/>) to a
    /// canvas pixel (the same space over <paramref name="viewRect"/>, the canvas's frozen basis) — the
    /// composition of <see cref="PageToWorld"/> then <see cref="WorldToPage"/>. Identity when
    /// <paramref name="artRect"/> equals <paramref name="viewRect"/>.</summary>
    public static PagePoint ArtToCanvas(WorldRect artRect, WorldRect viewRect, PagePoint artPixel)
    {
        (float worldX, float worldY) = PageToWorld(artRect, artPixel);
        return WorldToPage(viewRect, worldX, worldY);
    }

    /// <summary>The inverse of <see cref="ArtToCanvas"/>.</summary>
    public static PagePoint CanvasToArt(WorldRect artRect, WorldRect viewRect, PagePoint canvasPixel)
    {
        (float worldX, float worldY) = PageToWorld(viewRect, canvasPixel);
        return WorldToPage(artRect, worldX, worldY);
    }

    /// <summary>Where a child rect's four corners land on a page, as a page-pixel bounding box — takes
    /// the bounding box of all four projected corners rather than just two, so it stays correct
    /// regardless of which world axis a given page maps to which screen axis.</summary>
    public static (float MinX, float MinY, float MaxX, float MaxY) ProjectOntoPage(WorldRect child, WorldRect page)
    {
        PagePoint a = WorldToPage(page, child.MinWorldX, child.MinWorldY);
        PagePoint b = WorldToPage(page, child.MinWorldX, child.MaxWorldY);
        PagePoint c = WorldToPage(page, child.MaxWorldX, child.MinWorldY);
        PagePoint d = WorldToPage(page, child.MaxWorldX, child.MaxWorldY);
        float minX = System.MathF.Min(System.MathF.Min(a.X, b.X), System.MathF.Min(c.X, d.X));
        float maxX = System.MathF.Max(System.MathF.Max(a.X, b.X), System.MathF.Max(c.X, d.X));
        float minY = System.MathF.Min(System.MathF.Min(a.Y, b.Y), System.MathF.Min(c.Y, d.Y));
        float maxY = System.MathF.Max(System.MathF.Max(a.Y, b.Y), System.MathF.Max(c.Y, d.Y));
        return (minX, minY, maxX, maxY);
    }

    /// <summary>
    /// Restores the 3:2 (world Y span / world X span) a page or floor rect must hold, after a free
    /// drag of one <paramref name="handle"/>, anchored the way that handle implies: the axis the
    /// handle moved is the one the user set, and the other is rebuilt from whichever edge stayed put
    /// (or about its centre, when the handle moved neither of that axis's edges).
    ///
    /// This is deliberately not <see cref="Canvas2DRectHandles"/>'s own aspect lock, which holds a
    /// ratio in page pixels: page pixels only carry world aspect faithfully when the page being drawn
    /// on is itself exactly 3:2, and a rect being fixed is precisely where that can't be assumed.
    /// A page-pixel left/right edge is a world Y edge; top/bottom is a world X edge.
    /// </summary>
    public static WorldRect LockAspect(WorldRect dragged, RectHandleKind handle, float aspect = PageAspect)
    {
        if (handle == RectHandleKind.Body || dragged.SpanX <= 0.0f || dragged.SpanY <= 0.0f || aspect <= 0.0f)
        {
            return dragged;
        }

        if (handle is RectHandleKind.Left or RectHandleKind.Right)
        {
            float spanX = dragged.SpanY / aspect;
            float centreX = dragged.CenterWorldX;
            return dragged with { MinWorldX = centreX - (spanX * 0.5f), MaxWorldX = centreX + (spanX * 0.5f) };
        }

        float spanY = dragged.SpanX * aspect;

        if (handle is RectHandleKind.TopLeft or RectHandleKind.BottomLeft)
        {
            return dragged with { MaxWorldY = dragged.MinWorldY + spanY };
        }

        if (handle is RectHandleKind.TopRight or RectHandleKind.BottomRight)
        {
            return dragged with { MinWorldY = dragged.MaxWorldY - spanY };
        }

        float centreY = dragged.CenterWorldY;
        return dragged with { MinWorldY = centreY - (spanY * 0.5f), MaxWorldY = centreY + (spanY * 0.5f) };
    }

    /// <summary>A rect covering the given world-space bounds plus a flat padding, grown to 3:2 — what
    /// "fit page/floor bounds to area" tools build their result from.</summary>
    public static WorldRect FitToTiles(float minWorldX, float minWorldY, float maxWorldX, float maxWorldY, float paddingYards) =>
        new WorldRect(minWorldX - paddingYards, maxWorldX + paddingYards, minWorldY - paddingYards, maxWorldY + paddingYards)
            .FitToAspect();

    /// <summary>A `.zmp` cell's world rect. The grid rasterises the whole 64x64 ADT map, not the page
    /// it is read next to — column tracks world Y, row tracks world X, both descending, same as a
    /// page's own u/v. This is the one place that reinterprets a cell index as world space;
    /// <see cref="WorldToZmpCell"/> is its inverse.</summary>
    public static WorldRect ZmpCellToWorld(int row, int column)
    {
        float maxWorldY = ZmpMapExtent - (column * ZmpCellSize);
        float maxWorldX = ZmpMapExtent - (row * ZmpCellSize);
        return new WorldRect(maxWorldX - ZmpCellSize, maxWorldX, maxWorldY - ZmpCellSize, maxWorldY);
    }

    /// <summary>The `.zmp` cell a world point falls in, clamped to the grid — the inverse of
    /// <see cref="ZmpCellToWorld"/>.</summary>
    public static (int Row, int Column) WorldToZmpCell(float worldX, float worldY)
    {
        int row = (int)System.MathF.Floor((ZmpMapExtent - worldX) / ZmpCellSize);
        int column = (int)System.MathF.Floor((ZmpMapExtent - worldY) / ZmpCellSize);
        return (System.Math.Clamp(row, 0, ZmpSize - 1), System.Math.Clamp(column, 0, ZmpSize - 1));
    }

    private static (float U, float V) WorldToUv(WorldRect page, float worldX, float worldY)
    {
        float u = page.SpanY == 0f ? 0f : (page.MaxWorldY - worldY) / page.SpanY;
        float v = page.SpanX == 0f ? 0f : (page.MaxWorldX - worldX) / page.SpanX;
        return (u, v);
    }

    private static (float WorldX, float WorldY) UvToWorld(WorldRect page, float u, float v) =>
        (page.MaxWorldX - (v * page.SpanX), page.MaxWorldY - (u * page.SpanY));
}
