"""Game data rows on a scratch copy of the world. Run: python3 tests/test_game_data.py [SOURCE_ROOT]
The world, client db2 files are copied to a temp root; the live data is never written."""
import shutil
import sys
import tempfile
import unittest
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "addon"))
from wow_model_editor import doltlite, game_data as gd  # noqa: E402

SOURCE = Path(sys.argv.pop(1)) if len(sys.argv) > 1 else REPO
SCRATCH = Path(tempfile.mkdtemp(prefix="gd-root-"))
INFO = gd.ModelInfo(2000000001, "world/custom/wolfy.m2", (-1.0, -0.5, 0.0), (1.0, 0.5, 1.0), (-0.347, -0.347, 0.0), (0.347, 0.347, 1.2), 0)


def setUpModule():
    (SCRATCH / ".world").mkdir()
    (SCRATCH / ".client" / "Content").mkdir(parents=True)
    shutil.copy(SOURCE / ".world" / "world.doltlite", SCRATCH / ".world")
    shutil.copytree(SOURCE / ".client" / "Content" / "dbfilesclient", SCRATCH / ".client" / "Content" / "dbfilesclient")
    shutil.copytree(SOURCE / "sql", SCRATCH / "sql")
    for n in ("dbfilesclient-stock", "cache"):
        (SCRATCH / ".world" / n).symlink_to(SOURCE / ".world" / n)


def tearDownModule():
    shutil.rmtree(SCRATCH, ignore_errors=True)


def layout():
    lib = doltlite.find_library(REPO)
    return gd.Layout(SCRATCH, SCRATCH / ".world" / "world.doltlite",
                     REPO / "build" / "tools" / "database_tool" / "database_tool", lib)


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
        db = lay.open()
        commits, migrations = db.execute("SELECT count(*) FROM dolt_log"), sorted((SCRATCH / "sql" / "migrations").iterdir())
        db.close()
        gd.apply_changes(lay, changes)
        db = lay.open()
        try:
            self.assertEqual(db.execute("SELECT count(*) FROM dolt_log"), commits)
            self.assertEqual(sorted((SCRATCH / "sql" / "migrations").iterdir()), migrations)
            self.assertEqual(db.dicts(f"SELECT FileDataID FROM {gd.CMD} WHERE FileDataID = ?", (INFO.fdid,))[0]["FileDataID"], INFO.fdid)
            again, display2 = gd.plan_creature(db, lay, INFO, tdisp, scale=1.5)
            self.assertEqual((again, display2), ([], display))
            upd, _ = gd.plan_creature(db, lay, INFO, tdisp, scale=2.0)
            self.assertEqual([(c.kind, c.table) for c in upd], [("update", gd.CDI), ("update", gd.ADDON)])
        finally:
            db.close()

    def test_game_object(self):
        lay = layout()
        db = lay.open()
        try:
            changes, display = gd.plan_game_object(db, lay, INFO)
        finally:
            db.close()
        self.assertEqual(changes[0].table, gd.GODI)
        gd.apply_changes(lay, changes)
        db = lay.open()
        try:
            row = db.dicts(f"SELECT * FROM {gd.GODI} WHERE ID = ?", (display,))[0]
            self.assertEqual(row["FileDataID"], INFO.fdid)
            self.assertEqual(gd.plan_game_object(db, lay, INFO)[0], [])
        finally:
            db.close()


if __name__ == "__main__":
    unittest.main()
