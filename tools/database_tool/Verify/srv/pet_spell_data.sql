-- key: entry
SELECT "entry", "build", "spell_id1", "spell_id2", "spell_id3", "spell_id4" FROM (SELECT "entry", "build", "spell_id1", "spell_id2", "spell_id3", "spell_id4", ROW_NUMBER() OVER (PARTITION BY "entry" ORDER BY "build" DESC, rowid DESC) AS __rn FROM "pet_spell_data" WHERE "build" <= 5875) WHERE __rn = 1
