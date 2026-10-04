#!/usr/bin/env python3
"""Builds a tiny synthetic CASC install and checks extract-casc.py against it."""

import hashlib
import importlib.util
import os
import struct
import tempfile
import unittest
import zlib

spec = importlib.util.spec_from_file_location("extract_casc", os.path.join(os.path.dirname(__file__), "extract-casc.py"))
ec = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ec)


def blte(data, mode=b"Z", chunks=1):
    parts = [data] if chunks == 1 else [data[: len(data) // 2], data[len(data) // 2:]]
    enc = [mode + (zlib.compress(p) if mode == b"Z" else p) for p in parts]
    header = b"\x0f" + len(enc).to_bytes(3, "big") + b"".join(
        struct.pack(">II", len(e), len(p)) + b"\0" * 16 for e, p in zip(enc, parts))
    return b"BLTE" + struct.pack(">I", 8 + len(header)) + header + b"".join(enc)


def build_install(path, files, root_version):
    """files: {fdid: (name, bytes, encrypted)}"""
    data_dir = os.path.join(path, "Data")
    os.makedirs(os.path.join(data_dir, "data"))
    archive = bytearray()
    idx = {}

    def store(payload):
        ekey = hashlib.md5(payload).digest()
        entry = ekey[::-1] + struct.pack("<I", len(payload) + 30) + b"\0" * 10
        idx[ekey[:9]] = (len(archive), len(payload) + 30)
        archive.extend(entry + payload)
        return ekey

    enc_entries, root_entries = [], []
    for fdid, (name, content, encrypted) in sorted(files.items()):
        ckey = hashlib.md5(content).digest()
        ekey = store(blte(content, b"E" if encrypted else b"Z", 2 if fdid % 2 else 1))
        enc_entries.append((ckey, len(content), ekey))
        root_entries.append((fdid, ckey, ec.name_hash(name)))

    if root_version == 2:
        root = b"TSFM" + struct.pack("<IIII", 20, 2, len(root_entries), len(root_entries))
        block = struct.pack("<IIIIB", len(root_entries), 0xFFFFFFFF, 0, 0, 0)
    else:
        root = b"TSFM" + struct.pack("<II", len(root_entries), len(root_entries))
        block = struct.pack("<III", len(root_entries), 0, 0xFFFFFFFF)
    prev, deltas = -1, b""
    for fdid, _, _ in root_entries:
        deltas += struct.pack("<i", fdid - prev - 1)
        prev = fdid
    root += block + deltas + b"".join(e[1] + struct.pack("<Q", e[2]) for e in root_entries)
    root_ckey = hashlib.md5(root).digest()
    root_ekey = store(blte(root))
    enc_entries.append((root_ckey, len(root), root_ekey))

    page = b"".join(bytes([1]) + size.to_bytes(5, "big") + ck + ek
                    for ck, size, ek in sorted(enc_entries))
    page += b"\0" * (4096 - len(page))
    enc = b"EN" + bytes([1, 16, 16]) + struct.pack(">HHII", 4, 4, 1, 0) + b"\0" + struct.pack(">I", 1) + b"z" \
        + sorted(enc_entries)[0][0] + b"\0" * 16 + page
    enc_ckey = hashlib.md5(enc).digest()
    enc_ekey = store(blte(enc))

    with open(os.path.join(data_dir, "data", "data.000"), "wb") as f:
        f.write(archive)
    entries = b"".join(k + (0 << 30 | off).to_bytes(5, "big") + struct.pack("<I", size)
                       for k, (off, size) in sorted(idx.items()))
    header = struct.pack("<HBBBBBBQ", 7, 0, 0, 4, 5, 9, 30, 0x40000000)
    body = struct.pack("<II", 16, 0) + header + b"\0" * 16 + struct.pack("<II", len(entries), 0) + entries
    with open(os.path.join(data_dir, "data", "0000000001.idx"), "wb") as f:
        f.write(body)

    build_key = "ab" * 16
    cfg = os.path.join(data_dir, "config", "ab", "ab")
    os.makedirs(cfg)
    with open(os.path.join(cfg, build_key), "w") as f:
        f.write("root = %s\nencoding = %s %s\n" % (root_ckey.hex(), enc_ckey.hex(), enc_ekey.hex()))
    with open(os.path.join(path, ".build.info"), "w") as f:
        f.write("Branch!STRING:0|Active!DEC:1|Build Key!HEX:16|Version!STRING:0\nus|1|%s|1.15.6.58797\n" % build_key)


class ExtractTest(unittest.TestCase):
    def run_case(self, root_version, listing):
        files = {
            10: ("world/maps/a.blp", b"hello" * 1000, False),
            11: ("interface/b.txt", b"x" * 5000, False),
            12: ("secret/c.bin", b"classified", True),
        }
        with tempfile.TemporaryDirectory() as tmp:
            build_install(tmp, files, root_version)
            lf = os.path.join(tmp, "list.csv")
            with open(lf, "w") as f:
                f.write(listing)
            self.assertEqual(ec.main(["--client", tmp, "--listfile", lf, "--threads", "2"]), 0)
            out = os.path.join(tmp, "Content")
            read = lambda p: open(os.path.join(out, p), "rb").read()
            self.assertEqual(read("world/maps/a.blp"), files[10][1])
            self.assertEqual(read("interface/b.txt"), files[11][1])
            self.assertFalse(os.path.exists(os.path.join(out, "secret/c.bin")))
            self.assertIn("encrypted", read("_dump_failed.log").decode())
            self.assertIn("10;world/maps/a.blp", read("fdid.index").decode())
            self.assertIn("10;%016x;" % ec.name_hash("world/maps/a.blp"), read("root.hashes").decode())
            self.assertEqual(ec.main(["--client", tmp, "--listfile", lf]), 0)

    def test_ids_v1(self):
        self.run_case(1, "10;World/Maps/A.blp\n11;interface/b.txt\n12;secret/c.bin\n")

    def test_ids_v2(self):
        self.run_case(2, "10;world/maps/a.blp\n11;interface/b.txt\n12;secret/c.bin\n")

    def test_names_only(self):
        self.run_case(1, "world/maps/a.blp\ninterface/b.txt\nsecret/c.bin\n")


if __name__ == "__main__":
    unittest.main()
