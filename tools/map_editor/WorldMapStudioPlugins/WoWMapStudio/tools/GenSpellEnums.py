#!/usr/bin/env python3
"""Regenerate src/Catalog/Spell/Enums/*.cs from the TrinityCore Cataclysm Classic headers (usage: GenSpellEnums.py <TrinityCoreCataClassic root>).

Reads the C++ enum blocks named below out of the world-server sources and emits one C# enum per
block (plain or [Flags]). Rerun after a core update changes any of them; review the diff.
Handles integer/hex literals, `1 << N`, prior-member references, `|` unions and `~`.
"""
import re, sys, pathlib

ROOT = pathlib.Path(sys.argv[1])
OUT = pathlib.Path(__file__).resolve().parent.parent / "src/Catalog/Spell/Enums"
OUT.mkdir(parents=True, exist_ok=True)

SHARED = (ROOT / "src/server/game/Miscellaneous/SharedDefines.h").read_text(encoding="utf-8", errors="replace")
AURAS = (ROOT / "src/server/game/Spells/Auras/SpellAuraDefines.h").read_text(encoding="utf-8", errors="replace")
SPDEF = (ROOT / "src/server/game/Spells/SpellDefines.h").read_text(encoding="utf-8", errors="replace")
SPMGR = (ROOT / "src/server/game/Spells/SpellMgr.h").read_text(encoding="utf-8", errors="replace")
SPELLH = (ROOT / "src/server/game/Spells/Spell.h").read_text(encoding="utf-8", errors="replace")


def grab_block(text, enum_name):
    # matches "enum [class] NAME [: type] { ... };"
    m = re.search(r"enum\s+(?:class\s+)?" + re.escape(enum_name) + r"\b[^{]*\{(.*?)\n\};", text, re.S)
    if not m:
        print("MISSING", enum_name); return ""
    return m.group(1)


def parse_members(body, seed=None):
    """Return list of (raw_name, value:int). Evaluates literals, hex, <<, |, &, ~, parens, and prior members."""
    # strip // and /* */ comments
    body = re.sub(r"/\*.*?\*/", "", body, flags=re.S)
    lines = []
    for ln in body.splitlines():
        ln = re.sub(r"//.*$", "", ln).strip().rstrip(",").strip()
        if ln:
            lines.append(ln)
    # rejoin members split across lines (a member continues until it has balanced parens and next token is a new NAME =)
    joined, buf = [], ""
    for ln in lines:
        buf = (buf + " " + ln).strip() if buf else ln
        if buf.count("(") == buf.count(")") and (("=" not in buf) or not buf.rstrip().endswith(("|", "&", "+", "-", "^"))):
            joined.append(buf)
            buf = ""
    if buf:
        joined.append(buf)

    env = dict(seed) if seed else {}
    out = []
    auto = 0
    for m in joined:
        if "=" in m:
            name, expr = m.split("=", 1)
            name = name.strip()
            expr = expr.strip()
            if not re.fullmatch(r"[A-Za-z_]\w*", name):
                continue
            py = expr.replace("|", "|").replace("~", "~")
            # C++ identifiers -> their python values (must already be known)
            def repl(mo):
                tok = mo.group(0)
                if re.fullmatch(r"0[xX][0-9a-fA-F]+", tok) or re.fullmatch(r"\d+", tok):
                    return tok
                if tok in ("u", "U", "l", "L", "UL", "ul"):
                    return ""
                return str(env.get(tok, "0")) if not re.fullmatch(r"\d+[uUlL]*", tok) else re.sub(r"[uUlL]+$", "", tok)
            py = re.sub(r"[A-Za-z_]\w*|0[xX][0-9a-fA-F]+|\d+[uUlL]*", repl, py)
            py = py.replace("UL", "").replace("ul", "")
            try:
                val = eval(py, {"__builtins__": {}}, {}) & 0xFFFFFFFF
            except Exception:
                continue
            env[name] = val
            auto = val + 1
            out.append((name, val))
        else:
            name = m.strip()
            if not re.fullmatch(r"[A-Za-z_]\w*", name):
                continue
            env[name] = auto
            out.append((name, auto))
            auto += 1
    return out


def csharpify(raw, strip_prefix):
    n = raw
    for p in strip_prefix:
        if n.startswith(p):
            n = n[len(p):]
            break
    # PascalCase from SNAKE_CASE
    parts = [w for w in n.split("_") if w != ""]
    name = "".join(w.capitalize() for w in parts)
    if not name or name[0].isdigit():
        name = "_" + name
    return name


def emit(cs_name, raw_pairs, strip_prefix, flags, drop_re=None, description="", shift1=False):
    if shift1:
        raw_pairs = [(n, (1 << (v - 1)) & 0xFFFFFFFF) for n, v in raw_pairs if v >= 1]
    seen_names = {}
    seen_vals = set()
    members = []
    for raw, val in raw_pairs:
        if drop_re and re.search(drop_re, raw):
            continue
        nm = csharpify(raw, strip_prefix)
        base = nm
        i = 2
        while nm in seen_names and seen_names[nm] != val:
            nm = f"{base}{i}"; i += 1
        if nm in seen_names:
            continue
        seen_names[nm] = val
        members.append((nm, val))
    lines = []
    lines.append("namespace WorldMapStudio.WoW;")
    lines.append("")
    if description:
        lines.append(f"/// <summary>{description}</summary>")
    if flags:
        lines.append("[System.Flags]")
    lines.append(f"public enum {cs_name}" + (" : uint" if flags else ""))
    lines.append("{")
    if flags and not any(v == 0 for _, v in members):
        lines.append("    None = 0,")
    for nm, val in members:
        lit = f"0x{val:X}" if flags else str(val)
        lines.append(f"    {nm} = {lit},")
    lines.append("}")
    (OUT / f"{cs_name}.cs").write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"{cs_name}: {len(members)} members")


emit("SpellEffectType", parse_members(grab_block(SHARED, "SpellEffects")),
     ["SPELL_EFFECT_"], flags=False, drop_re=r"TOTAL_SPELL_EFFECTS",
     description="`dbc_spelleffect.Effect` — what the effect does.")

emit("SpellAuraType", parse_members(grab_block(AURAS, "AuraType")),
     ["SPELL_AURA_"], flags=False, drop_re=r"TOTAL_AURAS",
     description="`dbc_spelleffect.EffectAura` — the aura an APPLY_AURA effect applies.")

emit("SpellMechanicType", parse_members(grab_block(SHARED, "Mechanics")),
     ["MECHANIC_"], flags=False, drop_re=r"MAX_MECHANIC",
     description="`Mechanics` — the crowd-control / mechanic bucket.")

emit("SpellDispelKind", parse_members(grab_block(SHARED, "DispelType")),
     ["DISPEL_"], flags=False, drop_re=r"_MASK|MAX_",
     description="`DispelType` — the dispel school.")

_schools = dict((n, v) for n, v in parse_members(grab_block(SHARED, "SpellSchools")))
emit("SpellSchoolMaskFlags", parse_members(grab_block(SHARED, "SpellSchoolMask"), seed=_schools),
     ["SPELL_SCHOOL_MASK_"], flags=True, drop_re=r"_NONE$|_SPELL$|_MAGIC$|_ALL$",
     description="`dbc_spell.SchoolMask` — resistance schools, one bit each.")

emit("SpellPowerKind", parse_members(grab_block(SHARED, "Powers")),
     ["POWER_"], flags=False, drop_re=r"MAX_POWERS|POWER_ALL$|POWER_HEALTH|POWER_UNUSED",
     description="`dbc_spell.PowerType` — the resource the spell costs.")

emit("SpellFamilyName", parse_members(grab_block(SHARED, "SpellFamilyNames")),
     ["SPELLFAMILY_"], flags=False,
     description="`dbc_spellclassoptions.SpellClassSet` — the spell family.")

emit("SpellImplicitTarget", parse_members(grab_block(SHARED, "Targets")),
     ["TARGET_"], flags=False, drop_re=r"TOTAL_SPELL_TARGETS|_END$",
     description="`dbc_spelleffect.ImplicitTarget1/2` — how the effect picks its targets.")

emit("SpellCastTargetFlags", parse_members(grab_block(SPDEF, "SpellCastTargetFlags")),
     ["TARGET_FLAG_"], flags=True, drop_re=r"_MASK$|UNUSED",
     description="`dbc_spelltargetrestrictions.Targets` — target-type mask.")

emit("SpellInterruptFlags", parse_members(grab_block(SPDEF, "SpellInterruptFlags")),
     [], flags=True,
     description="`dbc_spellinterrupts.InterruptFlags`.")

emit("SpellAuraInterruptFlags", parse_members(grab_block(SPDEF, "SpellAuraInterruptFlags")),
     [], flags=True, drop_re=r"NOT_VICTIM",
     description="`dbc_spellinterrupts.AuraInterruptFlags1` / `ChannelInterruptFlags1`.")

emit("SpellAuraInterruptFlags2", parse_members(grab_block(SPDEF, "SpellAuraInterruptFlags2")),
     [], flags=True,
     description="`dbc_spellinterrupts.AuraInterruptFlags2` / `ChannelInterruptFlags2`.")

emit("SpellPreventionKind", parse_members(grab_block(SHARED, "SpellPreventionType")),
     ["SPELL_PREVENTION_TYPE_"], flags=False,
     description="`dbc_spellcategories.PreventionType`.")

emit("SpellProcFlags", parse_members(grab_block(SPMGR, "ProcFlags")),
     ["PROC_FLAG_"], flags=True, drop_re=r"_MASK$|^AUTO_ATTACK|^MELEE_PROC|^RANGED_PROC|^SPELL_PROC|^PERIODIC_PROC|^DONE_HIT|^TAKEN_HIT|^REQ_",
     description="`dbc_spellauraoptions.ProcTypeMask`.")

emit("SpellDamageClass", parse_members(grab_block(SHARED, "SpellDmgClass")),
     ["SPELL_DAMAGE_CLASS_"], flags=False,
     description="`dbc_spellcategories.DefenseType`.")

ITEMT = (ROOT / "src/server/game/Entities/Item/ItemTemplate.h").read_text(encoding="utf-8", errors="replace")
emit("SpellEquippedItemClass", parse_members(grab_block(ITEMT, "ItemClass")),
     ["ITEM_CLASS_"], flags=False, drop_re=r"MAX_ITEM_CLASS",
     description="`dbc_spellequippeditems.EquippedItemClass`.")

emit("SpellTargetCreatureTypeMask", parse_members(grab_block(SHARED, "CreatureType")),
     ["CREATURE_TYPE_"], flags=True, shift1=True,
     description="`dbc_spelltargetrestrictions.TargetCreatureType` — mask of creature types (bit N = type N+1).")

emit("SpellRangeFlags", parse_members(grab_block(SPELLH, "SpellRangeFlag")),
     ["SPELL_RANGE_"], flags=True, drop_re=r"_DEFAULT$",
     description="`dbc_spellrange.Flags`.")

emit("SpellFacingFlags", parse_members(grab_block(SPDEF, "SpellFacingCasterFlags")),
     [], flags=True,
     description="`dbc_spellcastingrequirements.FacingCasterFlags`.")

emit("SpellShapeshiftFormMask", parse_members(grab_block(AURAS, "ShapeshiftForm")),
     ["FORM_"], flags=True, shift1=True,
     description="`dbc_spellshapeshift.ShapeshiftMask1/2` — mask of stances/forms (bit N = form N+1).")

emit("SpellAuraStateType", parse_members(grab_block(SHARED, "AuraStateType")),
     ["AURA_STATE_"], flags=False, drop_re=r"UNKNOWN\d+$",
     description="`dbc_spellaurarestrictions.*AuraState` — caster/target aura-state gate.")

DBCE = (ROOT / "src/server/game/DataStores/DBCEnums.h").read_text(encoding="utf-8", errors="replace")
emit("SpellCategoryFlags", parse_members(grab_block(DBCE, "SpellCategoryFlags")),
     [], flags=True,
     description="`dbc_spellcategory.Flags`.")

for n in range(16):
    suffix = "" if n == 0 else chr(ord("A") + n - 1)
    emit(f"SpellAttr{n}Flags", parse_members(grab_block(SHARED, f"SpellAttr{n}")),
         [f"SPELL_ATTR{n}_"], flags=True,
         description=f"`dbc_spell.Attributes{('Ex' + suffix) if n else ''}` — spell attribute flags word {n}.")

print("done")
