#!/usr/bin/env python3
"""Extract files from a client's local CASC storage into .client/Content.

    client/extract-casc.py [--client DIR] [--listfile FILE] [--output DIR]
                           [--locale enUS] [--threads N] [--force]

DIR is the folder holding `.build.info` and `Data/` (default: .client at the
repo root). The listfile has one file per line, either `<fileDataId>;<path>`
or a bare `<path>` (resolved through the root's name hashes). Files are written
lowercased under the output directory. Also written there: `fdid.index` (merged
with any existing one) and `root.hashes` (fileDataId;nameHash;ckey;size for the
whole root). Files that cannot be extracted are listed in `_dump_failed.log`.
Existing files of the right size are skipped unless --force is given.
"""

import argparse
import hashlib
import os
import struct
import sys
import threading
import zlib
from concurrent.futures import ThreadPoolExecutor

LOCALES = {
    "enUS": 0x2, "koKR": 0x4, "frFR": 0x10, "deDE": 0x20, "zhCN": 0x40, "esES": 0x80,
    "zhTW": 0x100, "enGB": 0x200, "esMX": 0x1000, "ruRU": 0x2000, "ptBR": 0x4000,
    "itIT": 0x8000, "ptPT": 0x10000,
}
CF_LOW_VIOLENCE = 0x80
CF_DO_NOT_LOAD = 0x100
CF_NO_NAME_HASH = 0x10000000
CF_EXCLUDED = CF_LOW_VIOLENCE | CF_DO_NOT_LOAD


class CascError(Exception):
    pass


def hashlittle2(key: bytes) -> int:
    """lookup3 hashlittle2 with zero seeds, returned as (c << 32) | b."""
    m = 0xFFFFFFFF

    def rot(x, k):
        return ((x << k) | (x >> (32 - k))) & m

    a = b = c = (0xDEADBEEF + len(key)) & m
    i = 0
    n = len(key)
    while n - i > 12:
        a = (a + struct.unpack_from("<I", key, i)[0]) & m
        b = (b + struct.unpack_from("<I", key, i + 4)[0]) & m
        c = (c + struct.unpack_from("<I", key, i + 8)[0]) & m
        a = (a - c) & m; a ^= rot(c, 4); c = (c + b) & m
        b = (b - a) & m; b ^= rot(a, 6); a = (a + c) & m
        c = (c - b) & m; c ^= rot(b, 8); b = (b + a) & m
        a = (a - c) & m; a ^= rot(c, 16); c = (c + b) & m
        b = (b - a) & m; b ^= rot(a, 19); a = (a + c) & m
        c = (c - b) & m; c ^= rot(b, 4); b = (b + a) & m
        i += 12
    tail = key[i:] + b"\0" * 12
    if n - i == 0:
        return (c << 32) | b
    a = (a + struct.unpack_from("<I", tail, 0)[0]) & m
    b = (b + struct.unpack_from("<I", tail, 4)[0]) & m
    c = (c + struct.unpack_from("<I", tail, 8)[0]) & m
    c ^= b; c = (c - rot(b, 14)) & m
    a ^= c; a = (a - rot(c, 11)) & m
    b ^= a; b = (b - rot(a, 25)) & m
    c ^= b; c = (c - rot(b, 16)) & m
    a ^= c; a = (a - rot(c, 4)) & m
    b ^= a; b = (b - rot(a, 14)) & m
    c ^= b; c = (c - rot(b, 24)) & m
    return (c << 32) | b


def name_hash(path: str) -> int:
    return hashlittle2(path.upper().replace("/", "\\").encode("latin-1", "replace"))


def blte_decode(data: bytes) -> bytes:
    if data[:4] != b"BLTE":
        raise CascError("not a BLTE stream")
    header_size = struct.unpack_from(">I", data, 4)[0]
    if header_size == 0:
        return _blte_chunk(data[8:])
    count = int.from_bytes(data[9:12], "big")
    pos = header_size
    out = []
    for i in range(count):
        csize, _dsize = struct.unpack_from(">II", data, 12 + i * 24)
        out.append(_blte_chunk(data[pos:pos + csize]))
        pos += csize
    return b"".join(out)


def _blte_chunk(chunk: bytes) -> bytes:
    mode, body = chunk[:1], chunk[1:]
    if mode == b"N":
        return body
    if mode == b"Z":
        return zlib.decompress(body)
    if mode == b"E":
        raise CascError("encrypted BLTE frame")
    raise CascError("unsupported BLTE mode %r" % mode)


class Storage:
    """Local CASC archives: idx lookup by encoding key, data.NNN reads."""

    def __init__(self, data_dir: str):
        self.data_dir = data_dir
        self.index = {}
        self.fds = {}
        self._load_indices()

    def _load_indices(self):
        data = os.path.join(self.data_dir, "data")
        best = {}
        for name in os.listdir(data):
            if not name.endswith(".idx") or len(name) != 14:
                continue
            bucket, version = int(name[:2], 16), int(name[2:10], 16)
            if bucket not in best or version > best[bucket][0]:
                best[bucket] = (version, name)
        if not best:
            raise CascError("no .idx files in %s" % data)
        for _version, name in best.values():
            self._load_idx(os.path.join(data, name))

    def _load_idx(self, path: str):
        with open(path, "rb") as f:
            raw = f.read()
        size_len, offs_len, key_len, offs_bits = raw[12], raw[13], raw[14], raw[15]
        entry_len = key_len + offs_len + size_len
        start = None
        for pos in range(0x18, 0x40, 8):
            if pos + 8 <= len(raw):
                entries_size = struct.unpack_from("<I", raw, pos)[0]
                if entries_size and entries_size % entry_len == 0 and pos + 8 + entries_size <= len(raw) \
                        and len(raw) - (pos + 8 + entries_size) < 0x10000:
                    start = pos + 8
                    break
        if start is None:
            raise CascError("cannot locate entries in %s" % path)
        mask = (1 << offs_bits) - 1
        for pos in range(start, start + entries_size, entry_len):
            key = raw[pos:pos + key_len]
            offs = int.from_bytes(raw[pos + key_len:pos + key_len + offs_len], "big")
            size = int.from_bytes(raw[pos + key_len + offs_len:pos + entry_len], "little")
            if key not in self.index:
                self.index[key] = (offs >> offs_bits, offs & mask, size)

    def has(self, ekey: bytes) -> bool:
        return ekey[:9] in self.index

    def read(self, ekey: bytes) -> bytes:
        entry = self.index.get(ekey[:9])
        if entry is None:
            raise CascError("not in local storage")
        archive, offset, size = entry
        fd = self.fds.get(archive)
        if fd is None:
            fd = self.fds.setdefault(
                archive, os.open(os.path.join(self.data_dir, "data", "data.%03d" % archive), os.O_RDONLY))
        blob = os.pread(fd, size, offset)
        if len(blob) != size:
            raise CascError("truncated archive entry")
        return blob[30:]


def read_key_values(path: str) -> dict:
    values = {}
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        for line in f:
            if "=" in line and not line.startswith("#"):
                k, v = line.split("=", 1)
                values[k.strip()] = v.strip()
    return values


def read_build_info(client: str):
    with open(os.path.join(client, ".build.info"), "r", encoding="utf-8") as f:
        lines = [l.rstrip("\r\n") for l in f if l.strip()]
    columns = [c.split("!")[0] for c in lines[0].split("|")]
    rows = [dict(zip(columns, l.split("|"))) for l in lines[1:]]
    for row in rows:
        if row.get("Active") == "1":
            return row
    return rows[0]


def parse_encoding(data: bytes) -> dict:
    """ckey -> (content size, [ekeys])."""
    if data[:2] != b"EN":
        raise CascError("bad encoding file")
    ckey_len, ekey_len = data[3], data[4]
    cpage_kb, _epage_kb = struct.unpack_from(">HH", data, 5)
    cpage_count = struct.unpack_from(">I", data, 9)[0]
    espec_size = struct.unpack_from(">I", data, 18)[0]
    pos = 22 + espec_size + cpage_count * (ckey_len + 16)
    page_size = cpage_kb * 1024
    table = {}
    for _ in range(cpage_count):
        page = data[pos:pos + page_size]
        pos += page_size
        p = 0
        while p + 1 + 5 + ckey_len <= page_size:
            keys = page[p]
            if keys == 0:
                break
            size = int.from_bytes(page[p + 1:p + 6], "big")
            ckey = page[p + 6:p + 6 + ckey_len]
            q = p + 6 + ckey_len
            table[ckey] = (size, [page[q + i * ekey_len:q + (i + 1) * ekey_len] for i in range(keys)])
            p = q + keys * ekey_len
    return table


def parse_root(data: bytes):
    """-> {fileDataId: [(ckey, nameHash or None, contentFlags, localeFlags)]}."""
    version, pos = 1, 0
    if data[:4] == b"TSFM":
        first, second = struct.unpack_from("<II", data, 4)
        version, pos = (second, first) if first <= 0x20 and second in (1, 2) else (1, 12)
    entries = {}
    while pos + 12 <= len(data):
        if version >= 2:
            count, locale, cf1, cf2 = struct.unpack_from("<IIII", data, pos)
            cf3 = data[pos + 16]
            flags = cf1 | cf2 | (cf3 << 17)
            pos += 17
        else:
            count, flags, locale = struct.unpack_from("<III", data, pos)
            pos += 12
        deltas = struct.unpack_from("<%di" % count, data, pos)
        pos += 4 * count
        has_names = not flags & CF_NO_NAME_HASH
        record = 24 if has_names else 16
        fdid = -1
        for i in range(count):
            fdid += deltas[i] + 1
            at = pos + record * i
            nh = struct.unpack_from("<Q", data, at + 16)[0] if has_names else None
            entries.setdefault(fdid, []).append((data[at:at + 16], nh, flags, locale))
        pos += record * count
    return entries


def pick_entry(candidates, locale_bit: int):
    best = None
    for entry in candidates:
        _ckey, _nh, flags, locale = entry
        if flags & CF_EXCLUDED:
            continue
        score = 2 if locale & locale_bit and locale != 0xFFFFFFFF else 1 if locale & locale_bit else 0
        if best is None or score > best[0]:
            best = (score, entry)
    return best[1] if best and best[0] else None


def read_listfile(path: str):
    """-> [(fileDataId or None, path)]."""
    items = []
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            if ";" in line:
                left, right = line.split(";", 1)
                if left.isdigit():
                    items.append((int(left), right.strip()))
                    continue
            items.append((None, line))
    return items


def read_index(path: str) -> dict:
    out = {}
    if os.path.exists(path):
        with open(path, "r", encoding="utf-8", errors="replace") as f:
            for line in f:
                left, _, right = line.rstrip("\n").partition(";")
                if left.isdigit():
                    out[int(left)] = right
    return out


def main(argv=None) -> int:
    repo = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--client", default=os.path.join(repo, ".client"))
    ap.add_argument("--listfile", required=True)
    ap.add_argument("--output")
    ap.add_argument("--locale", default="enUS", choices=sorted(LOCALES))
    ap.add_argument("--threads", type=int, default=8)
    ap.add_argument("--force", action="store_true")
    args = ap.parse_args(argv)

    client = os.path.realpath(args.client)
    output = args.output or os.path.join(client, "Content")
    locale_bit = LOCALES[args.locale]

    build = read_build_info(client)
    config_dir = os.path.join(client, "Data", "config")
    key = build["Build Key"]
    build_config = read_key_values(os.path.join(config_dir, key[:2], key[2:4], key))
    root_ckey = bytes.fromhex(build_config["root"].split()[0])
    encoding_ekey = bytes.fromhex(build_config["encoding"].split()[1])

    print("opening storage (%s)" % build.get("Version", key))
    storage = Storage(os.path.join(client, "Data"))
    encoding = parse_encoding(blte_decode(storage.read(encoding_ekey)))
    print("encoding: %d entries" % len(encoding))

    def local_ekey(ckey):
        info = encoding.get(ckey)
        if info:
            for ekey in info[1]:
                if storage.has(ekey):
                    return ekey
        return None

    root_ekey = local_ekey(root_ckey)
    if root_ekey is None:
        raise CascError("root not in local storage")
    root_raw = blte_decode(storage.read(root_ekey))
    root = parse_root(root_raw)
    print("root: %d file data ids" % len(root))

    os.makedirs(output, exist_ok=True)

    listing = read_listfile(args.listfile)
    by_hash = {}
    if any(i is None for i, _ in listing):
        for fdid, cands in root.items():
            for cand in cands:
                if cand[1] is not None:
                    by_hash.setdefault(cand[1], fdid)

    with open(os.path.join(output, "root.hashes"), "w", encoding="utf-8") as f:
        f.write("# fileDataId;nameHash;ckey;size -- from the build's root and encoding (build %s)\n"
                % build.get("Version", "").rsplit(".", 1)[-1])
        for fdid in sorted(root):
            entry = pick_entry(root[fdid], locale_bit)
            if entry and entry[1] is not None and entry[0] in encoding:
                f.write("%d;%016x;%s;%d\n" % (fdid, entry[1], entry[0].hex(), encoding[entry[0]][0]))

    jobs = []
    unresolved = []
    for fdid, path in listing:
        path = path.replace("\\", "/").lower()
        if fdid is None:
            fdid = by_hash.get(name_hash(path))
        if fdid is None or fdid not in root:
            unresolved.append((fdid, path, "not in root"))
            continue
        entry = pick_entry(root[fdid], locale_bit)
        if entry is None:
            unresolved.append((fdid, path, "no entry for locale"))
            continue
        jobs.append((fdid, path, entry[0]))
    print("%d listed, %d to extract" % (len(listing), len(jobs)))

    failed = list(unresolved)
    names = {}
    lock = threading.Lock()
    done = [0, 0]

    def extract(job):
        fdid, path, ckey = job
        dest = os.path.join(output, *path.split("/"))
        size = encoding.get(ckey, (None,))[0]
        try:
            if not args.force and size is not None and os.path.isfile(dest) and os.path.getsize(dest) == size:
                with lock:
                    names[fdid] = path
                    done[1] += 1
                return
            ekey = local_ekey(ckey)
            if ekey is None:
                raise CascError("not in local storage")
            data = blte_decode(storage.read(ekey))
            if hashlib.md5(data).digest() != ckey:
                raise CascError("content hash mismatch (got %s)" % hashlib.md5(data).hexdigest())
            os.makedirs(os.path.dirname(dest), exist_ok=True)
            with open(dest + ".part", "wb") as f:
                f.write(data)
            os.replace(dest + ".part", dest)
            with lock:
                names[fdid] = path
                done[0] += 1
        except (CascError, zlib.error, OSError) as e:
            with lock:
                failed.append((fdid, path, str(e)))
        with lock:
            if (done[0] + done[1] + len(failed)) % 2000 == 0:
                print("  %d/%d" % (done[0] + done[1] + len(failed) - len(unresolved), len(jobs)), flush=True)

    with ThreadPoolExecutor(args.threads) as pool:
        list(pool.map(extract, jobs))

    index_path = os.path.join(output, "fdid.index")
    index = read_index(index_path)
    index.update(names)
    with open(index_path, "w", encoding="utf-8") as f:
        for fdid in sorted(index):
            f.write("%d;%s\n" % (fdid, index[fdid]))

    log_path = os.path.join(output, "_dump_failed.log")
    if failed:
        with open(log_path, "w", encoding="utf-8") as f:
            for fdid, path, reason in sorted(failed, key=lambda x: (x[0] is None, x[0] or 0)):
                f.write("%s;%s;%s\n" % (fdid, path, reason))
    elif os.path.exists(log_path):
        os.remove(log_path)
    print("extracted %d, up to date %d, failed %d%s" % (
        done[0], done[1], len(failed), " (see _dump_failed.log)" if failed else ""))
    return 1 if failed and not (done[0] or done[1]) else 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (CascError, OSError, KeyError) as e:
        print("error: %s" % e, file=sys.stderr)
        sys.exit(1)
