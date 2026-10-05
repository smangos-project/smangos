namespace WorldMapStudio.WoW;

/// <summary>
/// Pins <see cref="WorldMapProjection"/> and <see cref="WorldRect"/> against real rows out of the
/// imported `world` database (`dbc_worldmaparea` 30/522, `dbc_dungeonmap` 27/50,
/// `dbc_worldmaptransforms` 2, `tc_creature` 280689) plus a from-scratch port of the server's own
/// `DBCManager::Map2ZoneCoordinates` formula, so a sign or column-order mistake here fails loudly
/// instead of shipping a page or floor that draws or explores wrong.
/// </summary>
public static class WorldMapProjectionTests
{
    [EditorTest(Category = "WoW WorldMap", Thread = TestThread.Background)]
    public static void Zone_percent_matches_trinity_cores_map2zone_coordinates_formula()
    {
        // dbc_worldmaparea 30 (Elwynn), read straight off its Loc columns.
        const float locLeft = 1535.4166f;
        const float locRight = -1935.4166f;
        const float locTop = -7939.583f;
        const float locBottom = -10254.166f;
        const float worldX = -9000f;
        const float worldY = -500f;

        WorldRect page = WorldRect.FromLoc(locLeft, locRight, locTop, locBottom);
        (float pctX, float pctY) = WorldMapProjection.ZonePercent(page, worldX, worldY);

        // A from-scratch port of DBCManager::Map2ZoneCoordinates (src/server/game/DataStores/DBCStores.cpp):
        // computes then swaps x/y, so the final x comes from the Left/Right pair and final y from Top/Bottom.
        float tcX = (worldX - locTop) / ((locBottom - locTop) / 100f);
        float tcY = (worldY - locLeft) / ((locRight - locLeft) / 100f);
        (float expectedPctX, float expectedPctY) = (tcY, tcX);

        Assert.AreApproximatelyEqual(expectedPctX, pctX, 0.001, "pct X");
        Assert.AreApproximatelyEqual(expectedPctY, pctY, 0.001, "pct Y");
    }

    [EditorTest(Category = "WoW WorldMap", Thread = TestThread.Background)]
    public static void Elwynn_goldshire_hit_rect_contains_marshal_dughan()
    {
        // dbc_worldmaparea 30 (Elwynn).
        WorldRect page = WorldRect.FromLoc(1535.4166f, -1935.4166f, -7939.583f, -10254.166f);

        // tc_creature 280689, Marshal Dughan, standing in Goldshire on map 0.
        PagePoint dughan = WorldMapProjection.WorldToPage(page, -9465.52f, 74.0069f);

        // dbc_worldmapoverlay 122 (Goldshire, MapAreaID 30): HitRectTop/Left/Bottom/Right.
        Assert.IsTrue(dughan.X is >= 383f and <= 461f, $"Goldshire hit rect X 383..461, got {dughan.X}");
        Assert.IsTrue(dughan.Y is >= 402f and <= 463f, $"Goldshire hit rect Y 402..463, got {dughan.Y}");
    }

    [EditorTest(Category = "WoW WorldMap", Thread = TestThread.Background)]
    public static void Dalaran_floor_27_dungeon_map_rect_reads_its_known_world_span()
    {
        // dbc_dungeonmap 27 (Dalaran, MapID 571): MinX, MaxX, MinY, MaxY columns, in that order.
        WorldRect floor = WorldRect.FromDungeonMap(222.495f, 5513.33f, 1052.51f, 6066.67f);

        Assert.AreApproximatelyEqual(222.495, floor.MinWorldY, 0.01, "min world Y");
        Assert.AreApproximatelyEqual(1052.51, floor.MaxWorldY, 0.01, "max world Y");
        Assert.AreApproximatelyEqual(5513.33, floor.MinWorldX, 0.01, "min world X");
        Assert.AreApproximatelyEqual(6066.67, floor.MaxWorldX, 0.01, "max world X");
    }

    [EditorTest(Category = "WoW WorldMap", Thread = TestThread.Background)]
    public static void Ahnkahet_floor_50_matches_world_map_area_522()
    {
        // dbc_dungeonmap 50 (Ahnkahet, MapID 619): MinX, MaxX, MinY, MaxY columns.
        WorldRect floor = WorldRect.FromDungeonMap(-1205.72f, 200.405f, -233.302f, 848.684f);

        // dbc_worldmaparea 522 (Ahnkahet, same MapID 619): LocLeft, LocRight, LocTop, LocBottom.
        WorldRect page = WorldRect.FromLoc(-233.33333f, -1206.25f, 849.99994f, 202.08333f);

        const double tolerance = 2.0;
        Assert.AreApproximatelyEqual(page.MinWorldX, floor.MinWorldX, tolerance, "min world X");
        Assert.AreApproximatelyEqual(page.MaxWorldX, floor.MaxWorldX, tolerance, "max world X");
        Assert.AreApproximatelyEqual(page.MinWorldY, floor.MinWorldY, tolerance, "min world Y");
        Assert.AreApproximatelyEqual(page.MaxWorldY, floor.MaxWorldY, tolerance, "max world Y");
    }

    [EditorTest(Category = "WoW WorldMap", Thread = TestThread.Background)]
    public static void Eversong_transform_2_region_contains_silvermoon_and_offsets_onto_eastern_kingdoms()
    {
        // dbc_worldmaptransforms 2 (MapID 530 -> NewMapID 0): RegionMin1/2, RegionMax1/2, RegionOffset1/2.
        WorldRect region = WorldRect.FromTransformRegion(4800f, -10133.333f, 16000f, -2666.6667f);
        Assert.IsTrue(region.Contains(9700f, -7450f), "region should contain Silvermoon");

        (float worldX, float worldY) = WorldMapProjection.ApplyTransform(9700f, -7450f, -2400f, 2400f);
        Assert.AreApproximatelyEqual(7300.0, worldX, 0.01, "transformed world X");
        Assert.AreApproximatelyEqual(-5050.0, worldY, 0.01, "transformed world Y");
    }

    [EditorTest(Category = "WoW WorldMap", Thread = TestThread.Background)]
    public static void Page_to_world_inverts_world_to_page()
    {
        WorldRect page = WorldRect.FromLoc(1535.4166f, -1935.4166f, -7939.583f, -10254.166f);

        foreach ((float worldX, float worldY) in new[]
                 {
                     (-9465.52f, 74.0069f), (1535.4166f, -7939.583f), (-1935.4166f, -10254.166f), (0f, 0f),
                 })
        {
            PagePoint pagePoint = WorldMapProjection.WorldToPage(page, worldX, worldY);
            (float rtX, float rtY) = WorldMapProjection.PageToWorld(page, pagePoint);
            Assert.AreApproximatelyEqual(worldX, rtX, 0.01, $"({worldX}, {worldY}) round-trip X");
            Assert.AreApproximatelyEqual(worldY, rtY, 0.01, $"({worldX}, {worldY}) round-trip Y");
        }
    }

    [EditorTest(Category = "WoW WorldMap", Thread = TestThread.Background)]
    public static void Fit_to_aspect_grows_a_non_3_2_rect_about_its_centre()
    {
        var wide = new WorldRect(MinWorldX: 0f, MaxWorldX: 1000f, MinWorldY: 0f, MaxWorldY: 500f);
        WorldRect widened = wide.FitToAspect();
        Assert.AreApproximatelyEqual(1.5, widened.Aspect, 0.0001, "aspect");
        Assert.AreApproximatelyEqual(wide.CenterWorldX, widened.CenterWorldX, 0.01, "centre X preserved");
        Assert.AreApproximatelyEqual(wide.CenterWorldY, widened.CenterWorldY, 0.01, "centre Y preserved");
        Assert.IsTrue(widened.SpanX >= wide.SpanX - 0.001f, "never shrinks X");
        Assert.IsTrue(widened.SpanY >= wide.SpanY - 0.001f, "never shrinks Y");

        var tall = new WorldRect(MinWorldX: 0f, MaxWorldX: 100f, MinWorldY: 0f, MaxWorldY: 1000f);
        Assert.AreApproximatelyEqual(1.5, tall.FitToAspect().Aspect, 0.0001, "aspect (tall input)");
    }

    [EditorTest(Category = "WoW WorldMap", Thread = TestThread.Background)]
    public static void Tile_bounds_round_trip_through_world_space()
    {
        // dbc_worldmapcontinent 1 (Eastern Kingdoms) and 2 (Kalimdor), Left/Right/Top/Bottom.
        AssertTileRoundTrip(17, 45, 8, 60);
        AssertTileRoundTrip(16, 48, 9, 54);

        // Eastern Kingdoms' own edges, hand-derived from (origin - index) * tile.
        WorldRect easternKingdoms = WorldRect.FromTileBounds(17, 45, 8, 60);
        Assert.AreApproximatelyEqual(8000.0, easternKingdoms.MaxWorldY, 0.01, "left edge (max world Y)");
        Assert.AreApproximatelyEqual(-7466.667, easternKingdoms.MinWorldY, 0.01, "right edge (min world Y)");
        Assert.AreApproximatelyEqual(12800.0, easternKingdoms.MaxWorldX, 0.01, "top edge (max world X)");
        Assert.AreApproximatelyEqual(-15466.667, easternKingdoms.MinWorldX, 0.01, "bottom edge (min world X)");
    }

    [EditorTest(Category = "WoW WorldMap", Thread = TestThread.Background)]
    public static void Tile_bounds_round_trip_snaps_a_rect_that_lands_mid_tile()
    {
        // Half a tile past the boundary each way still resolves to the tile it sits in.
        WorldRect exact = WorldRect.FromTileBounds(17, 45, 8, 60);
        var nudged = new WorldRect(
            exact.MinWorldX + 200.0f, exact.MaxWorldX - 200.0f,
            exact.MinWorldY + 200.0f, exact.MaxWorldY - 200.0f);

        (int left, int right, int top, int bottom) = nudged.ToTileBounds();
        Assert.AreEqual(17, left, "left");
        Assert.AreEqual(45, right, "right");
        Assert.AreEqual(8, top, "top");
        Assert.AreEqual(60, bottom, "bottom");
    }

    [EditorTest(Category = "WoW WorldMap", Thread = TestThread.Background)]
    public static void Lock_aspect_holds_a_square_when_asked_for_one()
    {
        // The continent taxi rect is always square — same anchoring, aspect 1 instead of 3:2.
        var dragged = new WorldRect(MinWorldX: 0.0f, MaxWorldX: 500.0f, MinWorldY: 0.0f, MaxWorldY: 100.0f);
        WorldRect square = WorldMapProjection.LockAspect(dragged, RectHandleKind.TopLeft, aspect: 1.0f);
        Assert.AreApproximatelyEqual(square.SpanX, square.SpanY, 0.001, "square");
        Assert.AreApproximatelyEqual(0.0, square.MinWorldY, 0.001, "anchored min Y held");
        Assert.AreApproximatelyEqual(500.0, square.MaxWorldY, 0.001, "max Y rebuilt to match X span");
    }

    private static void AssertTileRoundTrip(int left, int right, int top, int bottom)
    {
        (int roundLeft, int roundRight, int roundTop, int roundBottom) =
            WorldRect.FromTileBounds(left, right, top, bottom).ToTileBounds();
        Assert.AreEqual(left, roundLeft, "left");
        Assert.AreEqual(right, roundRight, "right");
        Assert.AreEqual(top, roundTop, "top");
        Assert.AreEqual(bottom, roundBottom, "bottom");
    }

    [EditorTest(Category = "WoW WorldMap", Thread = TestThread.Background)]
    public static void Lock_aspect_restores_3_2_from_the_edge_the_handle_left_alone()
    {
        // 200 x 300 (X span x Y span) is already 3:2. Widen world X by dragging a page-pixel top edge
        // (world max X): Y must follow, and since neither Y edge moved it grows about the Y centre.
        var widened = new WorldRect(MinWorldX: 0.0f, MaxWorldX: 400.0f, MinWorldY: 0.0f, MaxWorldY: 300.0f);
        WorldRect locked = WorldMapProjection.LockAspect(widened, RectHandleKind.Top);
        Assert.AreApproximatelyEqual(1.5, locked.SpanY / locked.SpanX, 0.0001, "aspect");
        Assert.AreApproximatelyEqual(400.0, locked.SpanX, 0.001, "dragged axis untouched");
        Assert.AreApproximatelyEqual(150.0, locked.CenterWorldY, 0.001, "Y centre held");

        // Dragging a page-pixel left edge is a world max-Y edge: Y is authoritative, X follows about
        // its own centre.
        var tallened = new WorldRect(MinWorldX: 0.0f, MaxWorldX: 200.0f, MinWorldY: 0.0f, MaxWorldY: 600.0f);
        WorldRect lockedX = WorldMapProjection.LockAspect(tallened, RectHandleKind.Left);
        Assert.AreApproximatelyEqual(1.5, lockedX.SpanY / lockedX.SpanX, 0.0001, "aspect");
        Assert.AreApproximatelyEqual(600.0, lockedX.SpanY, 0.001, "dragged axis untouched");
        Assert.AreApproximatelyEqual(100.0, lockedX.CenterWorldX, 0.001, "X centre held");
    }

    [EditorTest(Category = "WoW WorldMap", Thread = TestThread.Background)]
    public static void Lock_aspect_anchors_a_corner_drag_at_the_opposite_corner()
    {
        // A page-pixel top-left corner moves world max X and world max Y, so both min edges anchor.
        var dragged = new WorldRect(MinWorldX: 0.0f, MaxWorldX: 400.0f, MinWorldY: 0.0f, MaxWorldY: 123.0f);
        WorldRect locked = WorldMapProjection.LockAspect(dragged, RectHandleKind.TopLeft);
        Assert.AreApproximatelyEqual(1.5, locked.SpanY / locked.SpanX, 0.0001, "aspect");
        Assert.AreApproximatelyEqual(0.0, locked.MinWorldY, 0.001, "anchored min Y held");
        Assert.AreApproximatelyEqual(600.0, locked.MaxWorldY, 0.001, "max Y rebuilt from the anchor");

        // Bottom-right moves world min X and min Y instead, so max Y is the anchor.
        var other = new WorldRect(MinWorldX: 0.0f, MaxWorldX: 400.0f, MinWorldY: 0.0f, MaxWorldY: 123.0f);
        WorldRect lockedOther = WorldMapProjection.LockAspect(other, RectHandleKind.BottomRight);
        Assert.AreApproximatelyEqual(1.5, lockedOther.SpanY / lockedOther.SpanX, 0.0001, "aspect");
        Assert.AreApproximatelyEqual(123.0, lockedOther.MaxWorldY, 0.001, "anchored max Y held");
        Assert.AreApproximatelyEqual(-477.0, lockedOther.MinWorldY, 0.001, "min Y rebuilt from the anchor");
    }

    [EditorTest(Category = "WoW WorldMap", Thread = TestThread.Background)]
    public static void Lock_aspect_leaves_a_body_drag_alone()
    {
        var moved = new WorldRect(MinWorldX: 10.0f, MaxWorldX: 210.0f, MinWorldY: 5.0f, MaxWorldY: 95.0f);
        Assert.AreEqual(moved, WorldMapProjection.LockAspect(moved, RectHandleKind.Body));
    }

    [EditorTest(Category = "WoW WorldMap", Thread = TestThread.Background)]
    public static void Zmp_cell_matches_the_verified_whole_map_formula()
    {
        // worldY = EXT - ((col+0.5)/128)*2EXT, worldX = EXT - ((row+0.5)/128)*2EXT, EXT = 32 *
        // 533.33333 — verified against Azeroth.zmp's own area-1 (Dun Morogh) centroid.
        const float extent = 32f * 533.33333f;

        foreach ((int row, int column) in new[] { (0, 0), (63, 64), (127, 127), (10, 100) })
        {
            float expectedWorldY = extent - (((column + 0.5f) / 128f) * 2f * extent);
            float expectedWorldX = extent - (((row + 0.5f) / 128f) * 2f * extent);

            WorldRect cell = WorldMapProjection.ZmpCellToWorld(row, column);
            Assert.AreApproximatelyEqual(expectedWorldY, cell.CenterWorldY, 0.01, $"cell ({row},{column}) centre world Y");
            Assert.AreApproximatelyEqual(expectedWorldX, cell.CenterWorldX, 0.01, $"cell ({row},{column}) centre world X");
        }
    }

    [EditorTest(Category = "WoW WorldMap", Thread = TestThread.Background)]
    public static void World_to_zmp_cell_inverts_zmp_cell_to_world()
    {
        foreach ((int row, int column) in new[] { (0, 0), (1, 5), (63, 64), (127, 127) })
        {
            WorldRect cell = WorldMapProjection.ZmpCellToWorld(row, column);
            (int roundRow, int roundColumn) = WorldMapProjection.WorldToZmpCell(cell.CenterWorldX, cell.CenterWorldY);
            Assert.AreEqual(row, roundRow, $"row for cell ({row},{column})");
            Assert.AreEqual(column, roundColumn, $"column for cell ({row},{column})");
        }
    }

    [EditorTest(Category = "WoW WorldMap", Thread = TestThread.Background)]
    public static void World_to_zmp_cell_clamps_points_outside_the_map()
    {
        const float wellOutside = 100000f;
        (int row, int column) = WorldMapProjection.WorldToZmpCell(wellOutside, wellOutside);
        Assert.AreEqual(0, row, "row clamps to the grid's top");
        Assert.AreEqual(0, column, "column clamps to the grid's top");

        (int row2, int column2) = WorldMapProjection.WorldToZmpCell(-wellOutside, -wellOutside);
        Assert.AreEqual(WorldMapProjection.ZmpSize - 1, row2, "row clamps to the grid's bottom");
        Assert.AreEqual(WorldMapProjection.ZmpSize - 1, column2, "column clamps to the grid's bottom");
    }

    [EditorTest(Category = "WoW WorldMap", Thread = TestThread.Background)]
    public static void Overlaps_is_true_for_touching_rects_and_false_once_separated()
    {
        var a = new WorldRect(MinWorldX: 0f, MaxWorldX: 100f, MinWorldY: 0f, MaxWorldY: 100f);
        var touching = new WorldRect(MinWorldX: 100f, MaxWorldX: 200f, MinWorldY: 0f, MaxWorldY: 100f);
        var separated = new WorldRect(MinWorldX: 100.1f, MaxWorldX: 200f, MinWorldY: 0f, MaxWorldY: 100f);

        Assert.IsTrue(a.Overlaps(touching), "touching rects overlap");
        Assert.IsTrue(touching.Overlaps(a), "overlap is symmetric");
        Assert.IsFalse(a.Overlaps(separated), "separated rects don't overlap");
    }

    [EditorTest(Category = "WoW WorldMap", Thread = TestThread.Background)]
    public static void Art_to_canvas_is_identity_when_art_rect_equals_view_rect()
    {
        WorldRect rect = WorldRect.FromLoc(1535.4166f, -1935.4166f, -7939.583f, -10254.166f);

        foreach (PagePoint art in new[] { new PagePoint(0f, 0f), new PagePoint(1002f, 668f), new PagePoint(383f, 402f) })
        {
            PagePoint canvas = WorldMapProjection.ArtToCanvas(rect, rect, art);
            Assert.AreApproximatelyEqual(art.X, canvas.X, 0.01, "identity X");
            Assert.AreApproximatelyEqual(art.Y, canvas.Y, 0.01, "identity Y");
        }
    }

    [EditorTest(Category = "WoW WorldMap", Thread = TestThread.Background)]
    public static void Canvas_to_art_inverts_art_to_canvas_for_a_non_identity_rect_pair()
    {
        var viewRect = new WorldRect(MinWorldX: 0f, MaxWorldX: 1000f, MinWorldY: 0f, MaxWorldY: 1000f);
        var artRect = new WorldRect(MinWorldX: 250f, MaxWorldX: 750f, MinWorldY: 250f, MaxWorldY: 750f);

        foreach (PagePoint art in new[] { new PagePoint(0f, 0f), new PagePoint(1002f, 668f), new PagePoint(200f, 555f) })
        {
            PagePoint canvas = WorldMapProjection.ArtToCanvas(artRect, viewRect, art);
            PagePoint roundTrip = WorldMapProjection.CanvasToArt(artRect, viewRect, canvas);
            Assert.AreApproximatelyEqual(art.X, roundTrip.X, 0.01, "round-trip X");
            Assert.AreApproximatelyEqual(art.Y, roundTrip.Y, 0.01, "round-trip Y");
        }
    }

    [EditorTest(Category = "WoW WorldMap", Thread = TestThread.Background)]
    public static void Art_to_canvas_pins_an_art_rect_centred_at_half_the_view_rects_span()
    {
        // An art rect centred inside the view rect at half its span on both axes: a documented pinned
        // case, worth catching a sign/order regression on its own rather than only via round-tripping.
        var viewRect = new WorldRect(MinWorldX: 0f, MaxWorldX: 1000f, MinWorldY: 0f, MaxWorldY: 1000f);
        var artRect = new WorldRect(MinWorldX: 250f, MaxWorldX: 750f, MinWorldY: 250f, MaxWorldY: 750f);

        PagePoint origin = WorldMapProjection.ArtToCanvas(artRect, viewRect, new PagePoint(0f, 0f));
        Assert.AreApproximatelyEqual(250.5, origin.X, 0.01, "origin canvas X");
        Assert.AreApproximatelyEqual(167.0, origin.Y, 0.01, "origin canvas Y");

        PagePoint opposite = WorldMapProjection.ArtToCanvas(artRect, viewRect, new PagePoint(1002f, 668f));
        Assert.AreApproximatelyEqual(751.5, opposite.X, 0.01, "opposite canvas X");
        Assert.AreApproximatelyEqual(501.0, opposite.Y, 0.01, "opposite canvas Y");
    }

    [EditorTest(Category = "WoW WorldMap", Thread = TestThread.Background)]
    public static void Art_to_canvas_does_not_throw_for_a_degenerate_art_rect()
    {
        // A zero-span art rect collapses every art pixel to its single world point (0, 0) here, which
        // is the view rect's own min corner (page pixel u=1, v=1) — (PageWidth, PageHeight) on canvas.
        var degenerate = new WorldRect(MinWorldX: 0f, MaxWorldX: 0f, MinWorldY: 0f, MaxWorldY: 0f);
        var viewRect = new WorldRect(MinWorldX: 0f, MaxWorldX: 1000f, MinWorldY: 0f, MaxWorldY: 1000f);

        PagePoint canvas = WorldMapProjection.ArtToCanvas(degenerate, viewRect, new PagePoint(500f, 300f));
        Assert.AreApproximatelyEqual(1002.0, canvas.X, 0.01, "degenerate art rect collapses to the view's min-corner canvas pixel");
        Assert.AreApproximatelyEqual(668.0, canvas.Y, 0.01, "degenerate art rect collapses to the view's min-corner canvas pixel");
    }

    [EditorTest(Category = "WoW WorldMap", Thread = TestThread.Background)]
    public static void Project_onto_page_bounds_a_zone_rect_inside_its_continent_page()
    {
        // dbc_worldmaparea 14 (Azeroth continent page, MapID 0, AreaID 0).
        WorldRect continentPage = WorldRect.FromLoc(18171.97f, -22569.21f, 11176.344f, -15973.344f);

        // dbc_worldmaparea 30 (Elwynn), a zone on that same continent.
        WorldRect elwynn = WorldRect.FromLoc(1535.4166f, -1935.4166f, -7939.583f, -10254.166f);

        (float minX, float minY, float maxX, float maxY) =
            WorldMapProjection.ProjectOntoPage(elwynn, continentPage);

        Assert.IsTrue(minX >= 0f && maxX <= WorldMapProjection.PageWidth, $"X in page bounds, got {minX}..{maxX}");
        Assert.IsTrue(minY >= 0f && maxY <= WorldMapProjection.PageHeight, $"Y in page bounds, got {minY}..{maxY}");
        Assert.IsTrue(maxX > minX, "non-empty width");
        Assert.IsTrue(maxY > minY, "non-empty height");
    }
}
