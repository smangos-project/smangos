-- key: entry, SpellID
SELECT "entry", "SpellID", "Active", "build_min", "build_max" FROM "spell_learn_spell" WHERE "build_min" <= 5875 AND 5875 <= "build_max"
