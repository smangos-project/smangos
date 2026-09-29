-- key: spell_id
SELECT "spell_id", "prev_spell", "first_spell", "rank", "req_spell", "build_min", "build_max" FROM "spell_chain" WHERE "build_min" <= 5875 AND 5875 <= "build_max"
