using System.Linq;

namespace WorldMapStudio.WoW;

public static class WowServerExtractorTests
{
    [EditorTest(Category = "WoW Server", Thread = TestThread.Background)]
    public static void Arguments_select_map_tile_and_dry_run()
    {
        var request = new WowServerExtractRequest(0, 48, 32, "map", Force: true);
        string joined = string.Join(' ', WowServerExtractor.BuildArguments(request, "in", "out", dryRun: true));

        Assert.AreEqual("--input in --output out --stages map --map 0 --tile 48,32 --force --dry-run", joined);
    }

    [EditorTest(Category = "WoW Server", Thread = TestThread.Background)]
    public static void Whole_map_request_has_no_tile_argument()
    {
        string joined = string.Join(' ', WowServerExtractor.BuildArguments(new WowServerExtractRequest(1), "in", "out", dryRun: false));

        Assert.IsFalse(joined.Contains("--tile"));
        Assert.IsFalse(joined.Contains("--dry-run"));
    }

    [EditorTest(Category = "WoW Server", Thread = TestThread.Background)]
    public static void Dry_run_report_yields_stale_tiles()
    {
        string[] lines =
        [
            "56 known maps",
            "  map 0 tile 48,32: would redo (input changed)",
            "  map 0 tile 48,33: would redo (missing)",
            "  model FILE0001A242.wmo: would redo",
        ];

        Assert.AreEqual("48,32 48,33", string.Join(' ', WowServerExtractor.ParseStaleTiles(lines).Select(t => $"{t.Row},{t.Col}")));
    }
}
