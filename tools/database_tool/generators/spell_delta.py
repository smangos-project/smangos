#!/usr/bin/env python3
"""Literal vmangos spell overrides over the db2 spell tables.

  spell_delta.py PRE REFERENCE OUT.sql [--expect out.json]

PRE is a world built with migrations up to 20261001000000 (db2 spell tables untouched,
spell_template/spell_mod/spell_effect_mod present). REFERENCE supplies vdbc_spell (1.12).
The legacy migrations in spell_delta/ map vmangos spell data onto the db2 tables. They run
three times on copies of the db2 spell tables: with vmangos data (M), with vanilla 1.12 data
(V) and with spell_template alone (T, only to tag where a value comes from). OUT.sql keeps:
  - cells where M differs from db2 and from V (a vmangos fix), text columns excepted;
  - cells the server loader reads (LOADED) where M differs from db2;
  - rows the client lacks, with ids from per-table reserved bases, ordered by natural key;
  - spell_extra, built by the legacy migration from the vmangos data.
Run with LD_PRELOAD=build/tools/database_tool/libdoltlite.so.0.
"""
import argparse
import json
import math
import os
import shutil
import sqlite3
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
LEGACY = [os.path.join(HERE, "spell_delta", f"legacy_{n}.sql") for n in ("20261001010000", "20261001020000")]
# Lines of the legacy SQL that are vmangos-only fixes: not applied for the vanilla run.
HARDCODED = ("WHERE `entry` = 1543;", "WHERE `entry` = 20424;", "WHERE `spell_id` = 20216 AND")

BASES = {
    "dbc_spellauraoptions": 250000, "dbc_spellaurarestrictions": 30000, "dbc_spellcastingrequirements": 230000,
    "dbc_spellcategories": 180000, "dbc_spellclassoptions": 80000, "dbc_spellcooldowns": 100000,
    "dbc_spelleffect": 1220000, "dbc_spellequippeditems": 260000, "dbc_spellinterrupts": 220000,
    "dbc_spelllevels": 150000, "dbc_spellmisc": 780000, "dbc_spellpower": 320000, "dbc_spellreagents": 40000,
    "dbc_spellshapeshift": 60000, "dbc_spelltargetrestrictions": 340000, "dbc_spelltotems": 20000,
}
TABLES = ["dbc_spellname", "dbc_spell"] + sorted(BASES)
TEXT = {"Name_lang", "NameSubtext_lang", "Description_lang", "AuraDescription_lang"}
# Columns SpellMgr::LoadSpells reads. On these a vmangos value wins even where it equals 1.12:
# Classic changed balance, mechanics and spell families there, and the server keeps vanilla behaviour.
LOADED = {
    "dbc_spellmisc": {"Attributes1", "Attributes2", "Attributes3", "Attributes4", "Attributes5", "CastingTimeIndex",
                      "DurationIndex", "RangeIndex", "SchoolMask", "Speed"},
    "dbc_spellcategories": {"Category", "DispelType", "Mechanic", "DefenseType", "PreventionType", "StartRecoveryCategory"},
    "dbc_spellclassoptions": {"SpellClassSet", "SpellClassMask1", "SpellClassMask2"},
    "dbc_spellcooldowns": {"RecoveryTime", "CategoryRecoveryTime", "StartRecoveryTime"},
    "dbc_spellinterrupts": {"InterruptFlags", "AuraInterruptFlags1", "ChannelInterruptFlags1"},
    "dbc_spelllevels": {"BaseLevel", "MaxLevel", "SpellLevel"},
    "dbc_spellauraoptions": {"CumulativeAura", "ProcChance", "ProcCharges", "ProcTypeMask1", "ProcCategoryRecovery",
                             "SpellProcsPerMinuteID"},
    "dbc_spellaurarestrictions": {"CasterAuraState", "TargetAuraState"},
    "dbc_spellcastingrequirements": {"RequiresSpellFocus"},
    "dbc_spellequippeditems": {"EquippedItemClass", "EquippedItemSubclass", "EquippedItemInvTypes"},
    "dbc_spellpower": {"PowerType", "ManaCost", "ManaCostPerLevel", "ManaPerSecond", "PowerCostPct"},
    "dbc_spellreagents": {f"Reagent{i}" for i in range(1, 9)} | {f"ReagentCount{i}" for i in range(1, 9)},
    "dbc_spellshapeshift": {"ShapeshiftMask1", "ShapeshiftExclude1"},
    "dbc_spelltargetrestrictions": {"Targets", "TargetCreatureType", "MaxTargets", "MaxTargetLevel"},
    "dbc_spelltotems": {"Totem1", "Totem2"},
    "dbc_spelleffect": {"Effect", "EffectAura", "EffectBasePoints", "EffectDieSides", "EffectRealPointsPerLevel",
                        "EffectPointsPerResource", "EffectAuraPeriod", "EffectAmplitude", "EffectChainAmplitude",
                        "EffectChainTargets", "EffectMechanic", "EffectMiscValue1", "EffectMiscValue2", "ImplicitTarget1",
                        "ImplicitTarget2", "EffectTriggerSpell", "EffectBonusCoefficient", "EffectRadiusIndex1",
                        "EffectRadiusIndex2", "EffectItemType", "EffectSpellClassMask1", "EffectSpellClassMask2"},
}
# Loader values for a spell without the row.
LOADED_DEFAULT = {"EquippedItemClass": -1, "ProcChance": 101}
NATURAL = ["SpellID", "DifficultyID", "OrderIndex", "EffectIndex"]
# Ids the Classic client reuses for another spell (S): vmangos keeps the 1.12 spell, text included.
REUSED = {379, 713, 974, 1234, 1329, 23598, 23920, 30167}

VANILLA = {
    "dispel": "DispelType", "attributesEx2": "AttributesExB", "attributesEx3": "AttributesExC",
    "attributesEx4": "AttributesExD", "stances": "ShapeshiftMask", "stancesNot": "ShapeshiftExclude",
    "manCostPerLevel": "ManaCostPerLevel", "modelNextSpell": "ModalNextSpell", "stackAmount": "CumulativeAura",
    "equippedItemSubClassMask": "EquippedItemSubclass", "equippedItemInventoryTypeMask": "EquippedItemInvTypes",
    "name": "Name_lang", "nameSubtext": "NameSubtext_lang", "description": "Description_lang",
    "auraDescription": "AuraDescription_lang", "manaCostPercentage": "ManaCostPct", "spellFamilyName": "SpellClassSet",
    "maxAffectedTargets": "MaxTargets", "dmgClass": "DefenseType", "minFactionId": "MinFactionID",
}
VANILLA_EFFECT = {
    "effectImplicitTargetA": "ImplicitTargetA", "effectImplicitTargetB": "ImplicitTargetB",
    "effectApplyAuraName": "EffectAura", "effectAmplitude": "EffectAuraPeriod", "effectMultipleValue": "EffectAmplitude",
    "effectChainTarget": "EffectChainTargets", "effectPointsPerComboPoint": "EffectPointsPerCombo",
    "dmgMultiplier": "EffectChainAmplitude",
}
# spell_template columns that the legacy migration doesn't read or that vanilla lacks.
VANILLA_SKIP = {"entry", "minTargetLevel", "customFlags", "script_name", "nameFlags", "nameSubtextFlags",
                "descriptionFlags", "auraDescriptionFlags", "spellVisual1", "spellVisual2", "spellIconId",
                "activeIconId", "spellPriority"}


def connect(path, ro=True):
    if ro:
        return sqlite3.connect(f"file:{path}?mode=ro", uri=True)
    return sqlite3.connect(f"file:{path}", uri=True)


def columns(db, table):
    return [r[1] for r in db.execute(f"PRAGMA table_info(`{table}`)")]


def copy_tables(dst, src_alias, names):
    for t in names:
        sql = dst.execute(f"SELECT sql FROM {src_alias}.sqlite_master WHERE name = ?", (t,)).fetchone()[0]
        dst.execute(sql)
        dst.execute(f"INSERT INTO main.`{t}` SELECT * FROM {src_alias}.`{t}`")
        if "SpellID" in columns(dst, t):
            dst.execute(f"CREATE INDEX `ix_{t}` ON `{t}` (`SpellID`)")


def vanilla_template(db, ref):
    """Rewrite spell_template to the 1.12 values of the spells vanilla has; drop the rest."""
    st = columns(db, "spell_template")
    vc = {c.lower(): c for c in columns(ref, "vdbc_spell")}
    assign = []
    for c in st:
        if c in VANILLA_SKIP:
            continue
        if c == "spellFamilyFlags":
            assign.append((c, "(v.`SpellClassMask1` & 4294967295) | ((v.`SpellClassMask2` & 4294967295) << 32)"))
        elif c == "effectBonusCoefficient1" or c.startswith("effectBonusCoefficient"):
            assign.append((c, "-1"))
        elif c in VANILLA:
            assign.append((c, f"v.`{VANILLA[c]}`"))
        elif c[-1].isdigit() and c[:-1] in VANILLA_EFFECT:
            assign.append((c, f"v.`{VANILLA_EFFECT[c[:-1]]}{c[-1]}`"))
        elif c.lower() in vc:
            assign.append((c, f"v.`{vc[c.lower()]}`"))
        else:
            sys.exit(f"no vanilla column for spell_template.{c}")
    db.execute("DELETE FROM spell_template WHERE entry NOT IN (SELECT ID FROM ref.vdbc_spell)")
    sets = ", ".join(f"`{c}` = {e}" for c, e in assign)
    db.execute(f"UPDATE spell_template SET {sets} FROM ref.vdbc_spell v WHERE v.ID = spell_template.entry")
    db.execute("DELETE FROM spell_mod")
    db.execute("DELETE FROM spell_effect_mod")


def simulate(pre_path, ref_path, workdir, kind):
    path = os.path.join(workdir, f"{kind}.db")
    db = connect(path, ro=False)
    db.execute(f"ATTACH 'file:{pre_path}?mode=ro' AS pre")
    db.execute(f"ATTACH 'file:{ref_path}?mode=ro' AS ref")
    copy_tables(db, "pre", ["spell_template", "spell_mod", "spell_effect_mod"] + TABLES)
    if kind == "vanilla":
        vanilla_template(db, db)
    elif kind == "template":
        db.execute("DELETE FROM spell_mod")
        db.execute("DELETE FROM spell_effect_mod")
    for f in LEGACY:
        for stmt in open(f).read().split(";\n"):
            s = stmt.strip()
            if not s or s.startswith("INSERT INTO `migrations`"):
                continue
            if kind != "vmangos" and any(h in s + ";" for h in HARDCODED):
                continue
            db.execute(s)
    db.commit()
    return db


def same(a, b):
    if a is None or b is None:
        return a is b
    if isinstance(a, float) or isinstance(b, float):
        try:
            a, b = float(a), float(b)
        except (TypeError, ValueError):
            return False
        return a == b or math.isclose(a, b, rel_tol=1e-6, abs_tol=1e-9)
    if isinstance(a, int) and isinstance(b, int):
        return (a & 0xFFFFFFFF) == (b & 0xFFFFFFFF) if abs(a) < 1 << 32 and abs(b) < 1 << 32 else a == b
    return a == b


def lit(v):
    if v is None:
        return "NULL"
    if isinstance(v, str):
        # Control characters are spelled out: git normalises line endings in .sql files.
        quoted = "'" + v.replace("'", "''") + "'"
        for ch in "\r\n":
            quoted = quoted.replace(ch, f"' || char({ord(ch)}) || '")
        return quoted
    if isinstance(v, float):
        return repr(v)
    return str(v)


def load(db, table, alias="main"):
    cols = columns(db, table)
    return cols, {r[0]: r for r in db.execute(f"SELECT * FROM {alias}.`{table}`")}


def loader_default(t, cols, row):
    """The loader ends up with the same values whether or not the row exists."""
    get = dict(zip(cols, row))
    if t == "dbc_spellequippeditems":
        return get["EquippedItemClass"] == -1
    return all(same(get[c] or 0, LOADED_DEFAULT.get(c, 0)) for c in LOADED.get(t, ()))


def natural(cols, row):
    return tuple(row[cols.index(c)] for c in NATURAL if c in cols)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("pre")
    ap.add_argument("reference")
    ap.add_argument("out")
    ap.add_argument("--expect")
    args = ap.parse_args()

    work = tempfile.mkdtemp(prefix="spell_delta_")
    try:
        run(args, work)
    finally:
        shutil.rmtree(work, ignore_errors=True)


def run(args, work):
    M = simulate(args.pre, args.reference, work, "vmangos")
    V = simulate(args.pre, args.reference, work, "vanilla")
    T = simulate(args.pre, args.reference, work, "template")

    client_spells = {r[0] for r in M.execute("SELECT ID FROM pre.dbc_spellname")}
    world_spells = {r[0] for r in M.execute("SELECT ID FROM main.dbc_spellname")}

    def dangling(t, cols, c, m):
        """db2 effect triggers a spell neither the client nor vmangos has: keep the vmangos effect."""
        if t != "dbc_spelleffect":
            return False
        i = cols.index("EffectTriggerSpell")
        return c[i] != 0 and c[i] not in world_spells and m[i] != c[i]
    # Client spells whose db2 row drops effects that trigger a spell only the server has (riding 33388):
    # vmangos keeps all of that spell's missing effects.
    c_effects = {r for r in M.execute("SELECT SpellID, DifficultyID, EffectIndex FROM pre.dbc_spelleffect")}
    server_effect_spells = {s for s, d, e, trig in M.execute(
        "SELECT SpellID, DifficultyID, EffectIndex, EffectTriggerSpell FROM main.dbc_spelleffect")
        if (s, d, e) not in c_effects and trig not in client_spells and trig in world_spells}
    updates = {"reused": [], "vanilla": [], "template": [], "mod": []}
    inserts = []
    expect = {}
    stats = {}
    for t in TABLES:
        cols, c_rows = load(M, t, "pre")
        _, m_rows = load(M, t)
        _, v_rows = load(V, t)
        _, t_rows = load(T, t)
        v_nat = {natural(cols, r): r for r in v_rows.values()}
        nu = ni = 0
        exp = expect.setdefault(t, {"added": [], "removed": [], "changed": []})
        for rid, m in m_rows.items():
            c = c_rows.get(rid)
            key = natural(cols, m)
            spell = key[0] if key else rid
            if c is not None:
                v = None if spell in REUSED or dangling(t, cols, c, m) else v_rows.get(rid)
                tr = t_rows.get(rid)
                sets = []
                for i, col in enumerate(cols):
                    if col == "ID" or (col in TEXT and spell not in REUSED) or same(m[i], c[i]):
                        continue
                    if v is not None and same(m[i], v[i]) and col not in LOADED.get(t, ()):
                        continue
                    val = m[i]
                    kind = ("reused" if spell in REUSED else "vanilla" if v is not None and same(m[i], v[i])
                            else "template" if tr is not None and same(tr[i], m[i]) else "mod")
                    sets.append((col, val, kind))
                    exp["changed"].append([[rid], col, c[i], val])
                for src in updates:
                    s = [(col, val) for col, val, k in sets if k == src]
                    if s:
                        updates[src].append((t, rid, s))
                        nu += 1
                continue
            if spell in client_spells and spell not in REUSED and not (t == "dbc_spelleffect" and spell in server_effect_spells):
                v = v_nat.get(key)
                if v is not None and all(same(m[i], v[i]) for i, col in enumerate(cols) if col != "ID") and loader_default(t, cols, m):
                    continue
            inserts.append((t, key, cols, m))
            ni += 1
        stats[t] = (nu, ni)

    by_table = {}
    for t, key, cols, m in inserts:
        by_table.setdefault(t, []).append((key, cols, m))
    ins_sql = []
    for t in TABLES:
        rows = sorted(by_table.get(t, []), key=lambda x: x[0])
        if not rows:
            continue
        cols = rows[0][1]
        exp = expect[t]
        vals = []
        for n, (key, _, m) in enumerate(rows):
            row = list(m)
            if t in BASES:
                row[cols.index("ID")] = BASES[t] + n
            vals.append("(" + ", ".join(lit(x) for x in row) + ")")
            exp["added"].append(list(key) if t in BASES else [row[0]])
        collist = ", ".join(f"`{c}`" for c in cols)
        ins_sql.append(f"INSERT INTO `{t}` ({collist}) VALUES\n" + ",\n".join(vals) + ";")

    ecols = columns(M, "spell_extra")
    extra = list(M.execute("SELECT * FROM spell_extra ORDER BY spell_id"))
    schema = M.execute("SELECT sql FROM sqlite_master WHERE name = 'spell_extra'").fetchone()[0]

    out = ["INSERT INTO `migrations` VALUES ('20261001010000');", ""]
    out.append("-- Generated by tools/database_tool/generators/spell_delta.py: vmangos spell data over db2.")
    out.append("")
    out.append("-- Spells and spell sub-rows the client lacks (vmangos content references them).")
    out += ins_sql
    out.append("")
    labels = {"reused": "-- Ids the client reuses for another spell: vmangos keeps the 1.12 spell (SpellMgr, scripts).",
              "vanilla": "-- Cells the server reads where the client changed the 1.12 value: vmangos keeps vanilla behaviour.",
              "template": "-- Cells where vmangos spell_template differs from 1.12 and the client.",
              "mod": "-- Cells from vmangos spell_mod, spell_effect_mod and core spell fixes."}
    for src in updates:
        out.append(labels[src])
        for t, rid, s in sorted(updates[src]):
            sets = ", ".join(f"`{c}` = {lit(v)}" for c, v in s)
            out.append(f"UPDATE `{t}` SET {sets} WHERE `ID` = {rid};")
        out.append("")
    out.append("-- Server-side spell data from vmangos spell_template: flags, scripts, vanilla dice, coefficient markers.")
    out.append(schema + ";")
    for i in range(0, len(extra), 500):
        chunk = extra[i:i + 500]
        out.append(f"INSERT INTO `spell_extra` ({', '.join(f'`{c}`' for c in ecols)}) VALUES\n"
                   + ",\n".join("(" + ", ".join(lit(x) for x in r) + ")" for r in chunk) + ";")
    with open(args.out, "w") as f:
        f.write("\n".join(out) + "\n")
    if args.expect:
        with open(args.expect, "w") as f:
            json.dump(expect, f)
    for t, (nu, ni) in stats.items():
        print(f"{t}: {nu} updated rows, {ni} inserted rows")
    print(f"spell_extra: {len(extra)} rows")


if __name__ == "__main__":
    main()
