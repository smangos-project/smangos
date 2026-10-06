"""Registry tests without Blender. Run: python3 tests/test_registry.py"""
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "addon" / "wow_model_editor"))
import registry  # noqa: E402

WORKER = """
import sys
sys.path.insert(0, sys.argv[1])
import registry
for i in range(int(sys.argv[4])):
    registry.allocate(sys.argv[2], f"world/custom/{sys.argv[3]}_{i}.m2")
"""


class RegistryTests(unittest.TestCase):
    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp())
        self.path = self.tmp / "custom.index"

    def test_allocate_is_stable_and_counts_up(self):
        a = registry.allocate(self.path, "World\\Custom\\A.m2")
        self.assertEqual(a, registry.FIRST_ID)
        self.assertEqual(registry.allocate(self.path, "world/custom/a.m2"), a)
        self.assertEqual(registry.allocate(self.path, "world/custom/b.m2"), a + 1)
        self.assertFalse(Path(str(self.path) + ".lock").exists())

    def test_concurrent_processes_get_distinct_ids(self):
        src = str(Path(registry.__file__).parent)
        procs = [subprocess.Popen([sys.executable, "-c", WORKER, src, str(self.path), name, "40"]) for name in "abc"]
        for p in procs:
            self.assertEqual(p.wait(), 0)
        entries = registry.read(self.path)
        self.assertEqual(len(entries), 120)
        self.assertEqual(sorted(entries), list(range(registry.FIRST_ID, registry.FIRST_ID + 120)))

    def test_stale_lock_is_taken_over(self):
        lock = Path(str(self.path) + ".lock")
        lock.write_text("1")
        with registry.locked(self.path, stale=-1):
            pass
        self.assertFalse(lock.exists())
        lock.write_text("1")
        with self.assertRaises(TimeoutError):
            with registry.locked(self.path, timeout=0.1):
                pass

    def test_legacy_merge_reports_conflicts_and_dedups(self):
        self.path.write_text(f"{registry.FIRST_ID};world/custom/keep.m2\n")
        a, b = self.tmp / "listfile.custom.txt", self.tmp / "wms.index"
        a.write_text(f"{registry.FIRST_ID};world/custom/other.m2\n{registry.FIRST_ID + 5};world/custom/x.blp\n")
        b.write_text(f"{registry.FIRST_ID + 5};World/Custom/X.blp\n{registry.FIRST_ID + 6};world/custom/x.blp\n{registry.FIRST_ID + 7};world/custom/y.blp\n")
        conflicts = registry.merge_legacy(self.path, [a, b])
        self.assertEqual(len(conflicts), 2)
        self.assertEqual(registry.read(self.path), {
            registry.FIRST_ID: "world/custom/keep.m2", registry.FIRST_ID + 5: "world/custom/x.blp",
            registry.FIRST_ID + 7: "world/custom/y.blp"})
        self.assertFalse(a.exists() or b.exists())
        self.assertEqual(registry.merge_legacy(self.path, [a, b]), [])

    def test_release_and_add(self):
        registry.allocate(self.path, "world/custom/a.m2")
        registry.allocate(self.path, "world/custom/b.m2")
        self.assertEqual(list(registry.release(self.path, ["World/Custom/A.m2"]).values()), ["world/custom/a.m2"])
        self.assertEqual(registry.add(self.path, {registry.FIRST_ID + 1: "x", registry.FIRST_ID + 9: "World/c.m2"}), {registry.FIRST_ID + 9: "world/c.m2"})
        self.assertEqual(registry.allocate(self.path, "world/custom/d.m2"), registry.FIRST_ID + 10)


if __name__ == "__main__":
    unittest.main()
