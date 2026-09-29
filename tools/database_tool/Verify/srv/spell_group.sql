-- key: group_id, group_spell_id, spell_id
SELECT "group_id", "group_spell_id", "spell_id", "build_min", "build_max" FROM "spell_group" WHERE "build_min" <= 5875 AND 5875 <= "build_max"
