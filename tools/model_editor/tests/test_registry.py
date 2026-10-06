"""Registry tests without Blender, on a scratch DoltLite file. Run: python3 tests/test_registry.py"""
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
ADDON = str(REPO / "tools" / "model_editor" / "addon" / "wow_model_editor")
sys.path.insert(0, ADDON)
import doltlite  # noqa: E402
import registry  # noqa: E402

LIB = doltlite.find_library(REPO)
SCHEMA = """CREATE TABLE file_data_id (id INTEGER PRIMARY KEY, path TEXT NOT NULL UNIQUE, source TEXT NOT NULL,
    retired INTEGER NOT NULL DEFAULT 0)"""

WORKER = """
import sys
sys.path.insert(0, sys.argv[1])
import doltlite, registry
db = registry.open_db(sys.argv[2], sys.argv[5])
for i in range(int(sys.argv[4])):
    registry.allocate(db, f"world/custom/{sys.argv[3]}_{i}.m2")
db.close()
"""


def scratch(tmp):
    path = tmp / "world.doltlite"
    db = doltlite.DB(path, LIB, create=True)
    db.execute(SCHEMA)
    db.close()
    return path


class RegistryTests(unittest.TestCase):
    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp())
        self.path = scratch(self.tmp)
        self.db = registry.open_db(self.path, LIB)

    def tearDown(self):
        self.db.close()

    def test_allocate_is_stable_and_counts_up(self):
        a, created = registry.allocate(self.db, "World\\Custom\\A.m2")
        self.assertEqual((a, created), (registry.FIRST_ID, True))
        self.assertEqual(registry.allocate(self.db, "world/custom/a.m2"), (a, False))
        self.assertEqual(registry.allocate(self.db, "world/custom/b.m2"), (a + 1, True))

    def test_stock_rows_are_reused(self):
        self.db.execute("INSERT INTO file_data_id (id, path, source) VALUES (5, 'a/b.blp', 'listfile')")
        self.assertEqual(registry.allocate(self.db, "A\\B.blp"), (5, False))
        self.assertEqual(registry.allocate(self.db, "x.blp"), (registry.FIRST_ID, True))

    def test_concurrent_processes_get_distinct_ids(self):
        procs = [subprocess.Popen([sys.executable, "-c", WORKER, ADDON, str(self.path), name, "40", str(LIB)]) for name in "abc"]
        for p in procs:
            self.assertEqual(p.wait(), 0)
        entries = registry.read_custom(self.db)
        self.assertEqual(sorted(entries), list(range(registry.FIRST_ID, registry.FIRST_ID + 120)))

    def test_release_retires_and_never_reuses(self):
        registry.allocate(self.db, "world/custom/a.m2")
        registry.allocate(self.db, "world/custom/b.m2")
        self.assertEqual(list(registry.release(self.db, ["World/Custom/A.m2"]).values()), ["world/custom/a.m2"])
        self.assertEqual(registry.allocate(self.db, "world/custom/d.m2"), (registry.FIRST_ID + 2, True))
        self.assertEqual(registry.allocate(self.db, "world/custom/a.m2"), (registry.FIRST_ID, True))

    def test_missing_and_export(self):
        a, _ = registry.allocate(self.db, "world/custom/a.m2")
        self.assertEqual(registry.missing(self.db, {a: "x", a + 7: "y", 5: "z"}), {a + 7: "y"})
        content = self.tmp / "Content"
        content.mkdir()
        (content / "custom.index").write_text("")
        self.assertTrue(registry.export_index(self.db, content))
        self.assertEqual((content / "fdid.index").read_text(), f"{a};world/custom/a.m2\n")
        self.assertFalse(registry.export_index(self.db, content))
        self.assertFalse((content / "custom.index").exists())

    def test_no_dolt_commit_is_made(self):
        before = self.db.execute("SELECT count(*) FROM dolt_log")
        registry.allocate(self.db, "world/custom/a.m2")
        self.assertEqual(self.db.execute("SELECT count(*) FROM dolt_log"), before)


if __name__ == "__main__":
    unittest.main()
