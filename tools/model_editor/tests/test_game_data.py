"""Game data rows on a scratch copy of the world. Run: python3 tests/test_game_data.py [SCRATCH_ROOT]
SCRATCH_ROOT holds .world/world.doltlite, .world/dbfilesclient-stock, .client/Content/dbfilesclient and sql/migrations (copies, never the live data)."""
import sys
import unittest
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "addon"))
from wow_model_editor import doltlite, game_data as gd  # noqa: E402

SCRATCH = Path(sys.argv.pop(1)) if len(sys.argv) > 1 else Path("/tmp/gd-root")
INFO = gd.ModelInfo(2000000001, "world/custom/wolfy.m2", (-1.0, -0.5, 0.0), (1.0, 0.5, 1.0), (-0.347, -0.347, 0.0), (0.347, 0.347, 1.2), 0)


def layout():
    lib = doltlite.find_library(REPO)
    return gd.Layout(SCRATCH, SCRATCH / ".world" / "world.doltlite", SCRATCH / "sql" / "migrations",
                     REPO / "build" / "tools" / "database_tool" / "database_tool", lib)


@unittest.skipUnless((SCRATCH / ".world" / "world.doltlite").is_file(), "no scratch root")
class GameDataTests(unittest.TestCase):
    def test_creature_flow(self):
        lay = layout()
        db = lay.open()
        try:
            tdisp = db.dicts("SELECT display_id FROM creature_display_info_addon WHERE display_id IN (SELECT ID FROM dbc_creaturedisplayinfo) LIMIT 1")[0]["display_id"]
            changes, display = gd.plan_creature(db, lay, INFO, tdisp, scale=1.5)
        finally:
            db.close()
        self.assertEqual([c.table for c in changes if c.kind == "insert"][:3], [gd.CMD, gd.CDI, gd.ADDON])
        self.assertGreaterEqual(display, gd.DISPLAY_FIRST_ID)
        res = gd.apply_changes(lay, changes, "test creature")
        self.assertTrue(res["migration"].is_file(), res)
        db = lay.open()
        try:
            self.assertEqual(db.dicts(f"SELECT FileDataID FROM {gd.CMD} WHERE FileDataID = ?", (INFO.fdid,))[0]["FileDataID"], INFO.fdid)
            again, display2 = gd.plan_creature(db, lay, INFO, tdisp, scale=1.5)
            self.assertEqual((again, display2), ([], display))
            upd, _ = gd.plan_creature(db, lay, INFO, tdisp, scale=2.0)
            self.assertEqual([(c.kind, c.table) for c in upd], [("update", gd.CDI), ("update", gd.ADDON)])
        finally:
            db.close()
        code, out = gd.run_tool(lay, "update", "--databases", "world")
        self.assertEqual(code, 0, out)

    def test_game_object(self):
        lay = layout()
        db = lay.open()
        try:
            changes, display = gd.plan_game_object(db, lay, INFO)
        finally:
            db.close()
        self.assertEqual(changes[0].table, gd.GODI)
        gd.apply_changes(lay, changes, "test go")
        db = lay.open()
        try:
            row = db.dicts(f"SELECT * FROM {gd.GODI} WHERE ID = ?", (display,))[0]
            self.assertEqual(row["FileDataID"], INFO.fdid)
            self.assertEqual(gd.plan_game_object(db, lay, INFO)[0], [])
        finally:
            db.close()


if __name__ == "__main__":
    unittest.main()
