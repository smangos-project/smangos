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
IDI, CMFD, TFD, CTFD, IDIMR, IDIMMR, ITEM, SPARSE, IAPP, IMA = (
    "dbc_itemdisplayinfo", "dbc_componentmodelfiledata", "dbc_texturefiledata", "dbc_componenttexturefiledata",
    "dbc_itemdisplayinfomaterialres", "dbc_itemdisplayinfomodelmatres", "dbc_item", "dbc_itemsparse",
    "dbc_itemappearance", "dbc_itemmodifiedappearance")
DB2_NAMES = {CMD: "CreatureModelData", CDI: "CreatureDisplayInfo", GODI: "GameObjectDisplayInfo", MFD: "ModelFileData",
             IDI: "ItemDisplayInfo", CMFD: "ComponentModelFileData", TFD: "TextureFileData", CTFD: "ComponentTextureFileData",
             IDIMR: "ItemDisplayInfoMaterialRes", IDIMMR: "ItemDisplayInfoModelMatRes", ITEM: "Item", SPARSE: "ItemSparse",
             IAPP: "ItemAppearance", IMA: "ItemModifiedAppearance"}
KEYS = {CMD: "ID", CDI: "ID", GODI: "ID", MFD: "FileDataID", ADDON: "display_id", IDI: "ID", CMFD: "ID", TFD: "FileDataID",
        CTFD: "ID", IDIMR: "ID", IDIMMR: "ID", ITEM: "ID", SPARSE: "ID", IAPP: "ID", IMA: "ID"}
MODEL_RESOURCE_FIRST = 100_000
MATERIAL_RESOURCE_FIRST = 1_000_000
ITEM_FIRST, APPEARANCE_FIRST, MODIFIED_APPEARANCE_FIRST = 300_000, 200_000, 400_000
MODEL_MAT_RES_FIRST, MATERIAL_RES_FIRST = 500_000, 2_000_000


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


def next_value(db, table, column, lo):
    top = db.execute(f"SELECT MAX(`{column}`) FROM {table} WHERE `{column}` >= ?", (lo,))[0][0]
    return lo if top is None else max(lo, top + 1)


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
                  model_file_data=False):
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


def plan_game_object(db, layout, info, update_stock=False, model_file_data=False):
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


class _Planner:
    """Collects changes for one plan and hands out ids that stay unique across the rows it has not written yet."""

    def __init__(self, db):
        self.db, self.changes, self._next, self.materials = db, [], {}, {}

    def new_id(self, table, column, lo):
        n = self._next.get((table, column))
        if n is None:
            n = next_value(self.db, table, column, lo)
        self._next[(table, column)] = n + 1
        return n

    def upsert(self, table, values, owned=True, template=None):
        """Insert a row, or update the columns that differ in an existing one; with `owned` false an existing row is left alone."""
        key = KEYS[table]
        old = _by_key(self.db, table, key, values[key])
        if old is None:
            row = {**(template or {}), **values}
            self.changes.append(Change("insert", table, row))
            return row
        if not owned:
            return old
        diff = _diff(old, values)
        if diff:
            self.changes.append(Change("update", table, {key: values[key], **diff}, changed=diff))
        return {**old, **values}


@dataclass
class ItemModel:
    """One M2 of an item display. `race`/`gender`/`position` (helm variants, shoulder sides) add a ComponentModelFileData row."""
    fdid: int
    race: int = 0
    gender: int = 2
    position: int = -1
    variant: bool = False


@dataclass
class ItemSpec:
    """Textures are BLP fdids per model index (at most two). `sections` maps a body ComponentSection (0..7) to
    [(blp fdid, gender or -1)] for armor without models."""
    template_display: int
    models: list = field(default_factory=list)
    textures: list = field(default_factory=list)
    sections: dict = field(default_factory=dict)


def _resource_of(db, table, column, fdid):
    rows = db.execute(f"SELECT `{column}` FROM {table} WHERE FileDataID = ?", (fdid,))
    return rows[0][0] if rows else None


def _item_models(pl, spec):
    """ModelResourcesID of the display's models: an existing resource of the first model, or a new one with a row per model."""
    if not spec.models:
        return 0
    first = _resource_of(pl.db, MFD, "ModelResourcesID", spec.models[0].fdid)
    res = first if first else pl.new_id(MFD, "ModelResourcesID", MODEL_RESOURCE_FIRST)
    owned = res >= MODEL_RESOURCE_FIRST
    for m in spec.models:
        if first and not owned:
            break
        pl.upsert(MFD, {"FileDataID": m.fdid, "Flags": 0, "LodCount": 0, "ModelResourcesID": res}, owned=owned)
        if m.variant or m.position >= 0:
            pl.upsert(CMFD, {"ID": m.fdid, "GenderIndex": m.gender, "ClassID": 0, "RaceID": m.race, "PositionIndex": m.position})
    return res


def _material(pl, fdid):
    """MaterialResourcesID of texture `fdid` (TextureFileData is keyed by file, so one file has one resource)."""
    res = _resource_of(pl.db, TFD, "MaterialResourcesID", fdid) or pl.materials.get(fdid)
    if res is None:
        res = pl.materials[fdid] = pl.new_id(TFD, "MaterialResourcesID", MATERIAL_RESOURCE_FIRST)
        pl.upsert(TFD, {"FileDataID": fdid, "UsageType": 0, "MaterialResourcesID": res})
    return res


def _component_texture(pl, fdid, gender, material):
    """A body texture under `material`: gender -1 means the only texture, else it is added to ComponentTextureFileData."""
    pl.upsert(TFD, {"FileDataID": fdid, "UsageType": 0, "MaterialResourcesID": material})
    if gender >= 0:
        pl.upsert(CTFD, {"ID": fdid, "GenderIndex": gender, "ClassID": 0, "RaceID": 0})


def plan_item_display(db, layout, spec):
    """Changes for an ItemDisplayInfo built from `spec` (copied from `spec.template_display`); returns (changes, display id).
    The display is found again through its first model, or through the first body texture for model-less armor."""
    tdisp = _by_key(db, IDI, "ID", spec.template_display)
    if tdisp is None:
        raise GameDataError(f"template display {spec.template_display} not found")
    if not spec.models and not spec.sections:
        raise GameDataError("an item display needs a model or body textures")
    pl = _Planner(db)
    res = _item_models(pl, spec)
    paired = len(spec.models) > 1 and all(m.position >= 0 for m in spec.models)
    textures = list(spec.textures[:2])
    if paired and len(textures) == 1:
        textures *= 2
    mats = [_material(pl, t) for t in textures]
    wanted = {"ModelResourcesID1": res, "ModelResourcesID2": res if paired else 0,
              "ModelMaterialResourcesID1": 0, "ModelMaterialResourcesID2": 0}
    for i, m in enumerate(mats):
        wanted[f"ModelMaterialResourcesID{i + 1}"] = m
    if res:
        found = db.dicts(f"SELECT * FROM {IDI} WHERE ModelResourcesID1 = ? AND ID >= ? ORDER BY ID", (res, DISPLAY_FIRST_ID))
    else:
        first = next(iter(spec.sections.values()))[0][0]
        found = db.dicts(f"SELECT d.* FROM {IDI} d JOIN {IDIMR} r ON r.ItemDisplayInfoID = d.ID JOIN {TFD} t ON t.MaterialResourcesID = r.MaterialResourcesID "
                         f"WHERE t.FileDataID = ? AND d.ID >= ? ORDER BY d.ID", (first, DISPLAY_FIRST_ID))
    if found:
        display_id = found[0]["ID"]
        diff = _diff(found[0], wanted)
        if diff:
            pl.changes.append(Change("update", IDI, {"ID": display_id, **diff}, changed=diff))
    else:
        display_id = pl.new_id(IDI, "ID", DISPLAY_FIRST_ID)
        pl.changes.append(Change("insert", IDI, {**tdisp, **wanted, "ID": display_id}))
    for i, m in enumerate(mats):
        row = db.dicts(f"SELECT * FROM {IDIMMR} WHERE ItemDisplayInfoID = ? AND ModelIndex = ?", (display_id, i)) if found else []
        if row:
            diff = _diff(row[0], {"MaterialResourcesID": m, "TextureType": 2})
            if diff:
                pl.changes.append(Change("update", IDIMMR, {"ID": row[0]["ID"], **diff}, changed=diff))
        else:
            pl.changes.append(Change("insert", IDIMMR, {"ID": pl.new_id(IDIMMR, "ID", MODEL_MAT_RES_FIRST), "MaterialResourcesID": m,
                                                         "TextureType": 2, "ModelIndex": i, "ItemDisplayInfoID": display_id}))
    for section, variants in sorted(spec.sections.items()):
        rows = db.dicts(f"SELECT * FROM {IDIMR} WHERE ItemDisplayInfoID = ? AND ComponentSection = ?", (display_id, section)) if found else []
        material = rows[0]["MaterialResourcesID"] if rows else (_resource_of(db, TFD, "MaterialResourcesID", variants[0][0])
                                                                or pl.new_id(TFD, "MaterialResourcesID", MATERIAL_RESOURCE_FIRST))
        for fdid, gender in variants:
            _component_texture(pl, fdid, gender, material)
        if not rows:
            pl.changes.append(Change("insert", IDIMR, {"ID": pl.new_id(IDIMR, "ID", MATERIAL_RES_FIRST), "ComponentSection": section,
                                                        "MaterialResourcesID": material, "ItemDisplayInfoID": display_id}))
    return pl.changes, display_id


def plan_item(db, layout, template_item, display_id, name, icon=0):
    """Changes for a test item showing `display_id`: Item and ItemSparse copied from `template_item` (new id from the
    reserved range), plus ItemAppearance and ItemModifiedAppearance. Found again through the display; returns (changes, item id)."""
    titem, tsparse = _by_key(db, ITEM, "ID", template_item), _by_key(db, SPARSE, "ID", template_item)
    tima = db.dicts(f"SELECT * FROM {IMA} WHERE ItemID = ? ORDER BY ItemAppearanceModifierID, ID", (template_item,))
    if titem is None or tsparse is None or not tima:
        raise GameDataError(f"template item {template_item} needs Item, ItemSparse and ItemModifiedAppearance rows")
    tapp = _by_key(db, IAPP, "ID", tima[0]["ItemAppearanceID"])
    pl = _Planner(db)
    icon = icon or titem["IconFileDataID"]
    found = db.dicts(f"SELECT m.ItemID FROM {IMA} m JOIN {IAPP} a ON a.ID = m.ItemAppearanceID WHERE a.ItemDisplayInfoID = ? AND a.ID >= ? AND m.ID >= ? ORDER BY m.ID",
                     (display_id, APPEARANCE_FIRST, MODIFIED_APPEARANCE_FIRST))
    if found:
        item_id = found[0]["ItemID"]
        pl.upsert(ITEM, {"ID": item_id, "IconFileDataID": icon})
        pl.upsert(SPARSE, {"ID": item_id, "Display_lang": name})
        app = db.dicts(f"SELECT a.* FROM {IAPP} a JOIN {IMA} m ON m.ItemAppearanceID = a.ID WHERE m.ItemID = ?", (item_id,))[0]
        pl.upsert(IAPP, {"ID": app["ID"], "DefaultIconFileDataID": icon})
        return pl.changes, item_id
    item_id = pl.new_id(ITEM, "ID", ITEM_FIRST)
    app_id = pl.new_id(IAPP, "ID", APPEARANCE_FIRST)
    pl.changes.append(Change("insert", ITEM, {**titem, "ID": item_id, "IconFileDataID": icon}))
    pl.changes.append(Change("insert", SPARSE, {**tsparse, "ID": item_id, "Display_lang": name}))
    pl.changes.append(Change("insert", IAPP, {**tapp, "ID": app_id, "ItemDisplayInfoID": display_id, "DefaultIconFileDataID": icon}))
    pl.changes.append(Change("insert", IMA, {**tima[0], "ID": pl.new_id(IMA, "ID", MODIFIED_APPEARANCE_FIRST), "ItemID": item_id,
                                              "ItemAppearanceModifierID": 0, "ItemAppearanceID": app_id}))
    return pl.changes, item_id


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
