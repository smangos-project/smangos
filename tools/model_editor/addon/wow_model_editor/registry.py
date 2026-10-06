"""The FileDataID registry: the `file_data_id` table of `.world/world.doltlite`, shared by the addon, WMS and database_tool."""
import os
from pathlib import Path

try:
    from . import doltlite
except ImportError:  # run standalone by tests/test_registry.py
    import doltlite

FIRST_ID = 2_000_000_000


def normalize(path):
    return path.replace("\\", "/").lower()


def open_db(world, lib):
    return doltlite.DB(world, lib, busy_ms=30000)


def read_all(db):
    """{fdid: path} of every active row."""
    return {i: p for i, p in db.execute("SELECT id, path FROM file_data_id WHERE retired = 0")}


def read_custom(db):
    return {i: p for i, p in db.execute("SELECT id, path FROM file_data_id WHERE retired = 0 AND id >= ?", (FIRST_ID,))}


def stamp(db):
    """Cheap change key of the active rows."""
    return list(db.execute("SELECT count(*), coalesce(sum(retired), 0), coalesce(max(id), 0) FROM file_data_id")[0])


def allocate(db, client_path, source="model_editor"):
    """The id of `client_path`: an existing row (revived when retired) or a new custom id. Returns (id, created)."""
    key = normalize(client_path)
    with db.transaction():
        row = db.execute("SELECT id, retired FROM file_data_id WHERE path = ?", (key,))
        if row:
            if row[0][1]:
                db.execute("UPDATE file_data_id SET retired = 0 WHERE id = ?", (row[0][0],))
            return row[0][0], bool(row[0][1])
        db.execute("INSERT INTO file_data_id (id, path, source) SELECT max(coalesce(max(id), 0) + 1, ?), ?, ? "
                   "FROM file_data_id WHERE id >= ?", (FIRST_ID, key, source, FIRST_ID))
        return db.execute("SELECT id FROM file_data_id WHERE path = ?", (key,))[0][0], True


def release(db, client_paths):
    """Retires the rows of `client_paths`; returns the retired {fdid: path}."""
    keys = [normalize(p) for p in client_paths]
    gone = {}
    with db.transaction():
        for k in keys:
            for i, p in db.execute("SELECT id, path FROM file_data_id WHERE path = ? AND retired = 0", (k,)):
                gone[i] = p
                db.execute("UPDATE file_data_id SET retired = 1 WHERE id = ?", (i,))
    return gone


def missing(db, session_ids):
    """The {fdid: path} entries of `session_ids` (custom range) that the table lacks."""
    have = read_custom(db)
    return {i: p for i, p in session_ids.items() if i >= FIRST_ID and i not in have}


def export_index(db, content):
    """Rewrites `Content/fdid.index` from the table (same format as `database_tool fdid export`); True when it changed."""
    content = Path(content)
    text = "".join(f"{i};{p}\n" for i, p in db.execute("SELECT id, path FROM file_data_id WHERE retired = 0 ORDER BY id"))
    dst = content / "fdid.index"
    if dst.is_file() and dst.read_text(encoding="utf-8") == text:
        return False
    tmp = dst.with_name(dst.name + ".tmp")
    tmp.write_text(text, encoding="utf-8")
    os.replace(tmp, dst)
    (content / "custom.index").unlink(missing_ok=True)
    return True
