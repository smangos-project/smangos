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

    def _item(self, spec, **kw):
        lay = layout()
        db = lay.open()
        try:
            changes, display = gd.plan_item_display(db, lay, spec, **kw)
        finally:
            db.close()
        gd.apply_changes(lay, changes)
        db = lay.open()
        try:
            again, display2 = gd.plan_item_display(db, lay, spec, **kw)
        finally:
            db.close()
        self.assertEqual((again, display2), ([], display))
        return lay, changes, display

    def _row(self, lay, sql, *args):
        db = lay.open()
        try:
            return db.dicts(sql, args)
        finally:
            db.close()

    def test_sword_display(self):
        spec = gd.ItemSpec(26577, [gd.ItemModel(2000000010)], [2000000011])
        lay, changes, display = self._item(spec)
        self.assertGreaterEqual(display, gd.DISPLAY_FIRST_ID)
        d = self._row(lay, f"SELECT * FROM {gd.IDI} WHERE ID = ?", display)[0]
        self.assertGreaterEqual(d["ModelResourcesID1"], gd.MODEL_RESOURCE_FIRST)
        self.assertGreaterEqual(d["ModelMaterialResourcesID1"], gd.MATERIAL_RESOURCE_FIRST)
        self.assertEqual(self._row(lay, f"SELECT * FROM {gd.MFD} WHERE FileDataID = 2000000010")[0]["ModelResourcesID"], d["ModelResourcesID1"])
        self.assertEqual(self._row(lay, f"SELECT TextureType, ModelIndex FROM {gd.IDIMMR} WHERE ItemDisplayInfoID = ?", display), [{"TextureType": 2, "ModelIndex": 0}])
        self.assertEqual(self._row(lay, f"SELECT count(*) n FROM {gd.CMFD} WHERE ID = 2000000010")[0]["n"], 0)

    def test_retexture_stock_model(self):
        spec = gd.ItemSpec(26577, [gd.ItemModel(148132)], [2000000021])
        lay, changes, display = self._item(spec)
        self.assertEqual([c.table for c in changes if c.table == gd.MFD], [])
        d = self._row(lay, f"SELECT * FROM {gd.IDI} WHERE ID = ?", display)[0]
        self.assertEqual(d["ModelResourcesID1"], 16810)

    def test_helm_variants(self):
        spec = gd.ItemSpec(1166, [gd.ItemModel(2000000030, race=1, gender=0, variant=True), gd.ItemModel(2000000031, race=1, gender=1, variant=True)], [2000000032])
        lay, changes, display = self._item(spec)
        res = self._row(lay, f"SELECT ModelResourcesID1 r FROM {gd.IDI} WHERE ID = ?", display)[0]["r"]
        cm = self._row(lay, f"SELECT c.* FROM {gd.CMFD} c JOIN {gd.MFD} m ON m.FileDataID = c.ID WHERE m.ModelResourcesID = ? ORDER BY c.ID", res)
        self.assertEqual([(c["RaceID"], c["GenderIndex"], c["PositionIndex"]) for c in cm], [(1, 0, -1), (1, 1, -1)])
        spec.models[1].gender = 0
        spec.models[1].race = 2
        db = layout().open()
        try:
            upd, _ = gd.plan_item_display(db, layout(), spec)
        finally:
            db.close()
        self.assertEqual([(c.kind, c.table) for c in upd], [("update", gd.CMFD)])

    def test_shoulders(self):
        spec = gd.ItemSpec(1057, [gd.ItemModel(2000000040, gender=2, position=0), gd.ItemModel(2000000041, gender=2, position=1)], [2000000042])
        lay, changes, display = self._item(spec)
        d = self._row(lay, f"SELECT * FROM {gd.IDI} WHERE ID = ?", display)[0]
        self.assertEqual(d["ModelResourcesID1"], d["ModelResourcesID2"])
        self.assertEqual(d["ModelMaterialResourcesID1"], d["ModelMaterialResourcesID2"])
        self.assertEqual(len(self._row(lay, f"SELECT * FROM {gd.IDIMMR} WHERE ItemDisplayInfoID = ?", display)), 2)

    def test_body_sections(self):
        spec = gd.ItemSpec(7777, [], [], {0: [(2000000050, 0), (2000000051, 1)], 3: [(2000000052, -1)]})
        lay, changes, display = self._item(spec)
        rows = self._row(lay, f"SELECT ComponentSection s, MaterialResourcesID m FROM {gd.IDIMR} WHERE ItemDisplayInfoID = ? ORDER BY s", display)
        self.assertEqual([r["s"] for r in rows], [0, 3])
        self.assertEqual(self._row(lay, f"SELECT count(*) n FROM {gd.TFD} WHERE MaterialResourcesID = ?", rows[0]["m"])[0]["n"], 2)
        self.assertEqual(self._row(lay, f"SELECT GenderIndex g FROM {gd.CTFD} WHERE ID = 2000000051")[0]["g"], 1)
        self.assertEqual(self._row(lay, f"SELECT count(*) n FROM {gd.CTFD} WHERE ID = 2000000052")[0]["n"], 0)

    def test_test_item(self):
        spec = gd.ItemSpec(26577, [gd.ItemModel(2000000060)], [2000000061])
        lay, _, display = self._item(spec)
        db = lay.open()
        try:
            changes, item = gd.plan_item(db, lay, 727, display, "Cube Sword (test)", icon=135274)
        finally:
            db.close()
        self.assertGreaterEqual(item, gd.ITEM_FIRST)
        self.assertEqual(sorted(c.table for c in changes), sorted([gd.ITEM, gd.SPARSE, gd.IAPP, gd.IMA]))
        gd.apply_changes(lay, changes)
        db = lay.open()
        try:
            self.assertEqual(db.dicts(f"SELECT Display_lang n FROM {gd.SPARSE} WHERE ID = ?", (item,))[0]["n"], "Cube Sword (test)")
            self.assertEqual(db.dicts(f"SELECT a.ItemDisplayInfoID d, a.DisplayType t FROM {gd.IMA} m JOIN {gd.IAPP} a ON a.ID = m.ItemAppearanceID WHERE m.ItemID = ?", (item,)), [{"d": display, "t": 11}])
            self.assertEqual(gd.plan_item(db, lay, 727, display, "Cube Sword (test)", icon=135274), ([], item))
            renamed, same = gd.plan_item(db, lay, 727, display, "Renamed")
            self.assertEqual((same, [c.kind for c in renamed]), (item, ["update"]))
        finally:
            db.close()

    def test_import_stock_display(self):
        lay = layout()
        db = lay.open()
        try:
            did = db.execute(f"SELECT d.ID FROM {gd.IDI} d JOIN {gd.MFD} m ON m.ModelResourcesID = d.ModelResourcesID1 "
                             f"JOIN {gd.CMFD} c ON c.ID = m.FileDataID WHERE c.RaceID = 1 AND d.ID < {gd.DISPLAY_FIRST_ID} LIMIT 1")[0][0]
            models, textures = gd.item_display_models(db, did)
            self.assertTrue(models)
            self.assertEqual(len(textures), 2)
            pick = gd.pick_default(models)
            self.assertIn(pick.race, (0, 1))
            with self.assertRaises(gd.GameDataError):
                gd.item_display_models(db, 999999999)
        finally:
            db.close()


if __name__ == "__main__":
    unittest.main()
