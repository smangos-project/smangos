"""The custom FileDataID registry (`Content/custom.index`, `id;path` lines sorted by id), shared by the addon, WMS and the client."""
import os
import time
from contextlib import contextmanager
from pathlib import Path

FIRST_ID = 2_000_000_000
LOCK_TIMEOUT = 30.0
LOCK_STALE = 60.0


def normalize(path):
    return path.replace("\\", "/").lower()


def read(path):
    """{fdid: path} of a registry (or legacy listfile) file."""
    out = {}
    path = Path(path)
    if path.is_file():
        for line in path.read_text(encoding="utf-8").splitlines():
            fdid, sep, name = line.partition(";")
            if sep and name and fdid.isdigit():
                out.setdefault(int(fdid), name)
    return out


def _write(entries, path):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    tmp = path.with_name(path.name + ".tmp")
    tmp.write_text("".join(f"{k};{v}\n" for k, v in sorted(entries.items())), encoding="utf-8")
    os.replace(tmp, path)


@contextmanager
def locked(path, timeout=LOCK_TIMEOUT, stale=LOCK_STALE):
    """Exclusive `<path>.lock` (created with O_EXCL, retried); a lock older than `stale` seconds is taken over."""
    lock = Path(str(path) + ".lock")
    lock.parent.mkdir(parents=True, exist_ok=True)
    end = time.monotonic() + timeout
    while True:
        try:
            fd = os.open(lock, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
            break
        except FileExistsError:
            try:
                if time.time() - lock.stat().st_mtime > stale:
                    lock.unlink()
                    continue
            except FileNotFoundError:
                continue
            if time.monotonic() > end:
                raise TimeoutError(f"{lock} is held by another process")
            time.sleep(0.02)
    try:
        os.write(fd, str(os.getpid()).encode())
        os.close(fd)
        yield
    finally:
        lock.unlink(missing_ok=True)


def merge_legacy(path, legacy_files):
    """Folds old allocation files into the registry once and deletes them. Returns conflict descriptions
    (an id or path that the registry already has under a different path or id; the registry's entry wins)."""
    legacy_files = [Path(f) for f in legacy_files if Path(f).is_file()]
    if not legacy_files:
        return []
    conflicts = []
    with locked(path):
        entries = read(path)
        by_path = {v.lower(): k for k, v in entries.items()}
        for f in legacy_files:
            for fdid, name in read(f).items():
                key = normalize(name)
                if entries.get(fdid) == key or by_path.get(key) == fdid:
                    continue
                if fdid in entries or key in by_path:
                    conflicts.append(f"{f.name}: {fdid};{key} conflicts with {fdid};{entries.get(fdid)} / {by_path.get(key)};{key}")
                    continue
                entries[fdid] = key
                by_path[key] = fdid
        _write(entries, path)
        for f in legacy_files:
            f.unlink()
    return conflicts


def allocate(path, client_path):
    """The registry id of `client_path`, adding it with the next free id (re-reads the file under the lock)."""
    key = normalize(client_path)
    with locked(path):
        entries = read(path)
        for k, v in entries.items():
            if v == key:
                return k
        fdid = max([FIRST_ID - 1, *entries]) + 1
        entries[fdid] = key
        _write(entries, path)
        return fdid


def add(path, new):
    """Adds {fdid: path} entries that are not registered yet (ids another tool allocated)."""
    with locked(path):
        entries = read(path)
        todo = {k: normalize(v) for k, v in new.items() if k not in entries}
        if todo:
            _write({**entries, **todo}, path)
    return todo


def release(path, client_paths):
    """Removes the entries of `client_paths`; returns the removed {fdid: path}."""
    keys = {normalize(p) for p in client_paths}
    with locked(path):
        entries = read(path)
        gone = {k: v for k, v in entries.items() if v in keys}
        if gone:
            _write({k: v for k, v in entries.items() if k not in gone}, path)
    return gone
