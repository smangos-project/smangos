"""Client/server table rows for a model (creature and game object displays), written to `.world` in one transaction."""
import os
import subprocess
from dataclasses import dataclass, field
from pathlib import Path

from . import doltlite

CREATURE_MODEL_IDS = (60000, 65535)
DISPLAY_FIRST_ID = 1_000_000
CMD, CDI, GODI, MFD, ADDON = ("dbc_creaturemodeldata", "dbc_creaturedisplayinfo", "dbc_gameobjectdisplayinfo",
                              "dbc_modelfiledata", "creature_display_info_addon")
DB2_NAMES = {CMD: "CreatureModelData", CDI: "CreatureDisplayInfo", GODI: "GameObjectDisplayInfo", MFD: "ModelFileData"}
KEYS = {CMD: "ID", CDI: "ID", GODI: "ID", MFD: "FileDataID", ADDON: "display_id"}


class GameDataError(RuntimeError):
    pass


@dataclass
class ModelInfo:
    """What the rows derive from: file id, rest-pose vertex bounds and the M2 collision box (model space)."""
    fdid: int
    path: str
    bounds_min: tuple
    bounds_max: tuple
    collision_min: tuple = (0.0, 0.0, 0.0)
    collision_max: tuple = (0.0, 0.0, 0.0)
    template_fdid: int = 0


@dataclass
class Change:
    kind: str        # "insert" | "update"
    table: str
    values: dict
    changed: dict = field(default_factory=dict)  # for updates: the columns that differ


@dataclass
class Layout:
    root: Path
    world: Path
    tool: Path
    lib: Path

    @classmethod
    def discover(cls, content, world=None, root=None):
        root = Path(root) if root else Path(content).parent.parent
        lib = doltlite.find_library(root)
        return cls(root, Path(world) if world else root / ".world" / "world.doltlite",
                   root / "build" / "tools" / "database_tool" / "database_tool", lib)

    def open(self):
        if not self.world.is_file():
            raise GameDataError(f"world database {self.world} not found (run database_tool create)")
        if self.lib is None:
            raise GameDataError("libdoltlite.so.0 not found under build/ (build database_tool)")
        return doltlite.DB(self.world, self.lib)


def next_id(db, table, lo, hi):
    top = db.execute(f"SELECT MAX(ID) FROM {table} WHERE ID >= ? AND ID <= ?", (lo, hi))[0][0]
    n = max([lo - 1, top if top is not None else lo - 1]) + 1
    if n > hi:
        raise GameDataError(f"no free id left in {table} range {lo}..{hi}")
    return n


def _by_key(db, table, key, value):
    rows = db.dicts(f"SELECT * FROM {table} WHERE {key} = ?", (value,))
    return rows[0] if rows else None


def _diff(old, new):
    return {k: v for k, v in new.items() if k in old and not _same(old[k], v)}


def _same(a, b):
    if isinstance(a, float) or isinstance(b, float):
        return a is not None and b is not None and abs(float(a) - float(b)) <= 1e-6 * max(1.0, abs(float(a)))
    return a == b


def geo_box(info):
    return [*info.bounds_min, *info.bounds_max]


def collision_size(info):
    """(width, height) from the M2 collision box, which is what stock CreatureModelData stores."""
    lo, hi = info.collision_min, info.collision_max
    return max(hi[0] - lo[0], hi[1] - lo[1]), hi[2]


def radius_and_reach(info, scale):
    """Server-side bounding radius and combat reach: half the larger horizontal extent of the model, scaled."""
    ext = max(info.bounds_max[0] - info.bounds_min[0], info.bounds_max[1] - info.bounds_min[1])
    r = max(ext / 2 * scale, 0.1)
    return round(r, 4), round(r, 4)


def _model_file_data(db, info, changes):
    if _by_key(db, MFD, "FileDataID", info.fdid) is not None:
        return
    template = _by_key(db, MFD, "FileDataID", info.template_fdid) if info.template_fdid else None
    row = {"FileDataID": info.fdid, "Flags": (template or {}).get("Flags", 0),
           "LodCount": (template or {}).get("LodCount", 0), "ModelResourcesID": 0}
    changes.append(Change("insert", MFD, row))


def plan_creature(db, layout, info, template_display, scale=1.0, textures=(), display_index=0, update_stock_model=False,
                  model_file_data=True):
    """Changes that give `info` a creature model row, a display row (the `display_index`-th one of this model; one past the
    last adds a display) and a server addon row. `template_display` is a CreatureDisplayInfo id whose other fields are copied."""
    tdisp = _by_key(db, CDI, "ID", template_display)
    if tdisp is None:
        raise GameDataError(f"template display {template_display} not found")
    tmodel = _by_key(db, CMD, "ID", tdisp["ModelID"])
    if tmodel is None:
        raise GameDataError(f"template display {template_display} has no CreatureModelData row")
    changes = []
    width, height = collision_size(info)
    derived = {"FileDataID": info.fdid, **{f"GeoBox{i + 1}": v for i, v in enumerate(geo_box(info))},
               "CollisionWidth": width, "CollisionHeight": height}
    existing = _by_key(db, CMD, "FileDataID", info.fdid)
    if existing is None:
        row = {**tmodel, **derived, "ID": next_id(db, CMD, *CREATURE_MODEL_IDS)}
        changes.append(Change("insert", CMD, row))
        model_id = row["ID"]
    else:
        model_id = existing["ID"]
        owned = CREATURE_MODEL_IDS[0] <= model_id <= CREATURE_MODEL_IDS[1]
        if owned or update_stock_model:
            diff = _diff(existing, derived)
            if diff:
                changes.append(Change("update", CMD, {"ID": model_id, **diff}, changed=diff))
    displays = db.dicts(f"SELECT * FROM {CDI} WHERE ModelID = ? AND ID >= ? ORDER BY ID", (model_id, DISPLAY_FIRST_ID)) if existing else []
    tex = {f"TextureVariationFileDataID{i + 1}": (textures[i] if i < len(textures) else 0) for i in range(4)}
    wanted = {"ModelID": model_id, "CreatureModelScale": float(scale), **tex}
    if display_index < len(displays):
        disp = displays[display_index]
        display_id = disp["ID"]
        diff = _diff(disp, wanted)
        if diff:
            changes.append(Change("update", CDI, {"ID": display_id, **diff}, changed=diff))
    else:
        display_id = next_id(db, CDI, DISPLAY_FIRST_ID, 2**31 - 1)
        changes.append(Change("insert", CDI, {**tdisp, **wanted, "ID": display_id}))
    radius, reach = radius_and_reach(info, scale)
    taddon = _by_key(db, ADDON, "display_id", template_display) or {}
    addon = _by_key(db, ADDON, "display_id", display_id)
    if addon is None:
        row = {"display_id": display_id, "bounding_radius": radius, "combat_reach": reach, "speed_walk": taddon.get("speed_walk", 1.0),
               "speed_run": taddon.get("speed_run", 1.14286), "display_id_other_gender": 0}
        changes.append(Change("insert", ADDON, row))
    else:
        diff = _diff(addon, {"bounding_radius": radius, "combat_reach": reach})
        if diff:
            changes.append(Change("update", ADDON, {"display_id": display_id, **diff}, changed=diff))
    if model_file_data:
        _model_file_data(db, info, changes)
    return changes, display_id


def model_name(path):
    return path.replace("/", "\\")[: -len(".m2")] + ".mdx" if path.lower().endswith(".m2") else path.replace("/", "\\")


def plan_game_object(db, layout, info, update_stock=False, model_file_data=True):
    """Changes for a GameObjectDisplayInfo row of `info` (looked up by FileDataID)."""
    derived = {"FileDataID": info.fdid, "ModelName": model_name(info.path), **{f"GeoBox{i + 1}": v for i, v in enumerate(geo_box(info))}}
    existing = _by_key(db, GODI, "FileDataID", info.fdid)
    changes = []
    if existing is None:
        display_id = next_id(db, GODI, DISPLAY_FIRST_ID, 2**31 - 1)
        cols = [r[1] for r in db.execute(f"PRAGMA table_info({GODI})")]
        row = {c: 0 for c in cols}
        changes.append(Change("insert", GODI, {**row, **derived, "ID": display_id}))
    else:
        display_id = existing["ID"]
        if display_id >= DISPLAY_FIRST_ID or update_stock:
            diff = _diff(existing, derived)
            if diff:
                changes.append(Change("update", GODI, {"ID": display_id, **diff}, changed=diff))
    if model_file_data:
        _model_file_data(db, info, changes)
    return changes, display_id


def run_tool(layout, *args):
    env = {**os.environ, "DATABASE_TOOL_ROOT": str(layout.root)}
    if not layout.tool.is_file():
        raise GameDataError(f"{layout.tool} not found (build database_tool)")
    r = subprocess.run([str(layout.tool), *args], capture_output=True, text=True, env=env, timeout=600)
    return r.returncode, (r.stdout + r.stderr).strip()


def apply_changes(layout, changes, export=True):
    """Writes `changes` to the world in one transaction (no dolt commit), then re-exports the touched db2 files.
    Returns {"log": [str]}."""
    result = {"log": []}
    if changes:
        db = layout.open()
        try:
            with db.transaction():
                for c in changes:
                    key = KEYS[c.table]
                    if c.kind == "insert":
                        cols = ", ".join(f"`{k}`" for k in c.values)
                        db.execute(f"INSERT INTO `{c.table}` ({cols}) VALUES ({', '.join('?' * len(c.values))})", tuple(c.values.values()))
                    else:
                        sets = ", ".join(f"`{k}` = ?" for k in c.changed)
                        db.execute(f"UPDATE `{c.table}` SET {sets} WHERE `{key}` = ?", (*c.changed.values(), c.values[key]))
        finally:
            db.close()
    if export and changes:
        tables = sorted({DB2_NAMES[c.table] for c in changes if c.table in DB2_NAMES})
        if tables:
            code, out = run_tool(layout, "export", "--format", "db2", "--client", "--tables", ",".join(tables))
            result["log"].append(out)
            if code != 0:
                raise GameDataError(f"database_tool export failed: {out}")
    return result
