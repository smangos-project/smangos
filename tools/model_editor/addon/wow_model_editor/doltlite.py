"""Minimal ctypes wrapper over libdoltlite (a SQLite fork with versioned storage), for opening `.world/world.doltlite`."""
import ctypes
from contextlib import contextmanager
from pathlib import Path

OK, ROW, DONE, BUSY = 0, 100, 101, 5
INTEGER, FLOAT, TEXT, BLOB, NULL = 1, 2, 3, 4, 5
TRANSIENT = ctypes.c_void_p(-1)
OPEN_READWRITE = 2
OPEN_CREATE = 4

_c = ctypes
_lib = None
_lib_path = None


class DoltError(RuntimeError):
    pass


def find_library(repo_root):
    for rel in ("build/tools/database_tool", "build/dep/doltlite", "build/src/server"):
        p = Path(repo_root) / rel / "libdoltlite.so.0"
        if p.is_file():
            return p
    return None


def load(lib_path):
    """Loads libdoltlite once; later calls with another path are ignored."""
    global _lib, _lib_path
    if _lib is not None:
        return _lib
    lib = _c.CDLL(str(lib_path))
    vp, cp, ip, i64 = _c.c_void_p, _c.c_char_p, _c.c_int, _c.c_int64
    sigs = {
        "sqlite3_open_v2": (ip, [cp, _c.POINTER(vp), ip, cp]),
        "sqlite3_close_v2": (ip, [vp]),
        "sqlite3_prepare_v2": (ip, [vp, cp, ip, _c.POINTER(vp), _c.POINTER(cp)]),
        "sqlite3_step": (ip, [vp]),
        "sqlite3_finalize": (ip, [vp]),
        "sqlite3_errmsg": (cp, [vp]),
        "sqlite3_busy_timeout": (ip, [vp, ip]),
        "sqlite3_column_count": (ip, [vp]),
        "sqlite3_column_type": (ip, [vp, ip]),
        "sqlite3_column_int64": (i64, [vp, ip]),
        "sqlite3_column_double": (_c.c_double, [vp, ip]),
        "sqlite3_column_text": (vp, [vp, ip]),
        "sqlite3_column_blob": (vp, [vp, ip]),
        "sqlite3_column_bytes": (ip, [vp, ip]),
        "sqlite3_column_name": (cp, [vp, ip]),
        "sqlite3_bind_null": (ip, [vp, ip]),
        "sqlite3_bind_int64": (ip, [vp, ip, i64]),
        "sqlite3_bind_double": (ip, [vp, ip, _c.c_double]),
        "sqlite3_bind_text": (ip, [vp, ip, cp, ip, vp]),
        "sqlite3_bind_blob": (ip, [vp, ip, cp, ip, vp]),
    }
    for name, (res, args) in sigs.items():
        f = getattr(lib, name)
        f.restype, f.argtypes = res, args
    _lib, _lib_path = lib, Path(lib_path)
    return lib


class DB:
    """One connection. Rows come back as tuples; `columns` holds the names of the last statement."""

    def __init__(self, path, lib_path=None, busy_ms=10000, create=False):
        lib = _lib or load(lib_path)
        self.lib = lib
        self.path = Path(path)
        if not create and not self.path.is_file():
            raise DoltError(f"{path} does not exist")
        self.h = _c.c_void_p()
        rc = lib.sqlite3_open_v2(str(path).encode(), _c.byref(self.h), OPEN_READWRITE | (OPEN_CREATE if create else 0), None)
        if rc != OK:
            raise DoltError(f"cannot open {path}: {self._msg()}")
        lib.sqlite3_busy_timeout(self.h, busy_ms)
        self.columns = []
        try:
            ver = self.execute("SELECT dolt_version()")[0][0]
        except DoltError as e:
            self.close()
            raise DoltError(f"{path} is not a DoltLite database ({e})") from None
        self.version = ver

    def _msg(self):
        return (self.lib.sqlite3_errmsg(self.h) or b"").decode(errors="replace")

    def close(self):
        if self.h:
            self.lib.sqlite3_close_v2(self.h)
            self.h = None

    def execute(self, sql, params=()):
        lib, stmt, rows = self.lib, _c.c_void_p(), []
        if lib.sqlite3_prepare_v2(self.h, sql.encode(), -1, _c.byref(stmt), None) != OK:
            raise DoltError(f"{self._msg()}: {sql}")
        try:
            for i, v in enumerate(params, 1):
                if v is None:
                    rc = lib.sqlite3_bind_null(stmt, i)
                elif isinstance(v, bool):
                    rc = lib.sqlite3_bind_int64(stmt, i, int(v))
                elif isinstance(v, int):
                    rc = lib.sqlite3_bind_int64(stmt, i, v)
                elif isinstance(v, float):
                    rc = lib.sqlite3_bind_double(stmt, i, v)
                elif isinstance(v, bytes):
                    rc = lib.sqlite3_bind_blob(stmt, i, v, len(v), TRANSIENT)
                else:
                    b = str(v).encode()
                    rc = lib.sqlite3_bind_text(stmt, i, b, len(b), TRANSIENT)
                if rc != OK:
                    raise DoltError(f"bind {i}: {self._msg()}")
            n = lib.sqlite3_column_count(stmt)
            self.columns = [lib.sqlite3_column_name(stmt, i).decode() for i in range(n)]
            while True:
                rc = lib.sqlite3_step(stmt)
                if rc == DONE:
                    return rows
                if rc != ROW:
                    raise DoltError(f"{self._msg()}: {sql}")
                row = []
                for i in range(n):
                    t = lib.sqlite3_column_type(stmt, i)
                    if t == INTEGER:
                        row.append(lib.sqlite3_column_int64(stmt, i))
                    elif t == FLOAT:
                        row.append(lib.sqlite3_column_double(stmt, i))
                    elif t == TEXT:
                        row.append(_c.string_at(lib.sqlite3_column_text(stmt, i), lib.sqlite3_column_bytes(stmt, i)).decode())
                    elif t == BLOB:
                        row.append(_c.string_at(lib.sqlite3_column_blob(stmt, i), lib.sqlite3_column_bytes(stmt, i)))
                    else:
                        row.append(None)
                rows.append(tuple(row))
        finally:
            lib.sqlite3_finalize(stmt)

    def dicts(self, sql, params=()):
        rows = self.execute(sql, params)
        return [dict(zip(self.columns, r)) for r in rows]

    @contextmanager
    def transaction(self):
        self.execute("BEGIN IMMEDIATE")
        try:
            yield self
        except BaseException:
            self.execute("ROLLBACK")
            raise
        self.execute("COMMIT")
