-- key: entry
SELECT "entry", "SchoolMask", "SpellFamilyName", "SpellFamilyMask0", "SpellFamilyMask1", "SpellFamilyMask2", "procFlags", "procEx", "ppmRate", "CustomChance", "Cooldown", "build_min", "build_max" FROM "spell_proc_event" WHERE "build_min" <= 5875 AND 5875 <= "build_max"
