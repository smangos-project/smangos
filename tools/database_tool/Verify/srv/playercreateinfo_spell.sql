-- key: race, class, spell
SELECT "race", "class", "spell", "build_min", "build_max", "note" FROM "playercreateinfo_spell" WHERE "build_min" <= 5875 AND 5875 <= "build_max"
