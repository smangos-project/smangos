"""FileDataIDs for files created by the editors: the `file_data_id` table of the world database (see registry.py)."""
from pathlib import Path

from . import client, doltlite, index, registry

FIRST_ID = registry.FIRST_ID


def world_path():
    """`.world/world.doltlite` next to the Content directory, unless the `custom_listfile` preference names another database."""
    p = client.prefs()
    if p.custom_listfile:
        return Path(p.custom_listfile)
    return Path(p.content_dir).parent.parent / ".world" / "world.doltlite"


def connect():
    world = world_path()
    lib = doltlite.find_library(Path(client.prefs().content_dir).parent.parent)
    if lib is None:
        raise RuntimeError("libdoltlite.so.0 not found under build/ (build database_tool)")
    return registry.open_db(world, lib)


def with_db(fn):
    db = connect()
    try:
        return fn(db)
    finally:
        db.close()


def read():
    """{fdid: path} of the custom ids."""
    return with_db(registry.read_custom)


def path_of(fdid):
    return index.path_of(fdid) or read().get(fdid)


def allocate(client_path):
    """The fdid of `client_path`: the table row (stock or custom) or a new id."""
    fdid, created = with_db(lambda db: registry.allocate(db, client_path))
    if created:
        client.close()
    return fdid


def release(client_paths):
    """Retires rows (for new files that were uninstalled)."""
    gone = with_db(lambda db: registry.release(db, client_paths))
    if gone:
        client.close()
    return gone


def adopt_session_ids():
    """Raises when wowlib allocated a custom id while writing that the table does not have."""
    s = client.session()
    ids = {}
    for line in s.listfile.read_text(encoding="utf-8").splitlines():
        fdid, sep, name = line.partition(";")
        if sep and fdid.isdigit() and int(fdid) >= FIRST_ID:
            ids[int(fdid)] = name
    gone = with_db(lambda db: registry.missing(db, ids))
    if gone:
        raise RuntimeError(f"wowlib allocated ids that are not in file_data_id: {sorted(gone.items())[:3]}")
    export_index()


def export_index():
    """Rewrites Content/fdid.index from the table."""
    return with_db(lambda db: registry.export_index(db, client.session().content))
