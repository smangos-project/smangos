"""Install/uninstall tests without Blender. Run: python3 tests/test_install.py"""
import hashlib
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "addon" / "wow_model_editor"))
import install  # noqa: E402


def snapshot(root):
    return {p.relative_to(root).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(Path(root).rglob("*")) if p.is_file()}


class InstallTests(unittest.TestCase):
    def setUp(self):
        tmp = Path(tempfile.mkdtemp())
        self.content, self.output = tmp / ".client" / "Content", tmp / "out"
        (self.content / "creature/wolf").mkdir(parents=True)
        (self.content / "creature/wolf/wolf.m2").write_bytes(b"stock")
        (self.output / "creature/wolf").mkdir(parents=True)
        (self.output / "creature/wolf/wolf.m2").write_bytes(b"edited")
        (self.output / "world/custom").mkdir(parents=True)
        (self.output / "world/custom/new.m2").write_bytes(b"new")

    def test_install_then_uninstall_restores_content_exactly(self):
        before = snapshot(self.content)
        rels = install.output_files(self.output)
        install.install(self.content, self.output, rels, check_online=False)
        self.assertEqual((self.content / "creature/wolf/wolf.m2").read_bytes(), b"edited")
        self.assertEqual((self.content / "world/custom/new.m2").read_bytes(), b"new")
        self.assertEqual(install.stale(self.content, self.output), [])
        (self.output / "world/custom/new.m2").write_bytes(b"newer")
        self.assertEqual(install.stale(self.content, self.output), ["world/custom/new.m2"])
        install.install(self.content, self.output, rels, check_online=False)
        install.uninstall(self.content, check_online=False)
        self.assertEqual(snapshot(self.content), before)
        self.assertFalse((self.content / "world").exists())
        self.assertFalse(install.store_dir(self.content).exists())

    def test_missing_output_file_is_an_error(self):
        with self.assertRaises(install.InstallError):
            install.install(self.content, self.output, ["nope.m2"], check_online=False)


if __name__ == "__main__":
    unittest.main()
