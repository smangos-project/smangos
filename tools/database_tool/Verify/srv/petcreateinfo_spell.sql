-- key: entry
SELECT "entry", "spell1", "spell2", "spell3", "spell4", "patch_min", "patch_max" FROM "petcreateinfo_spell" WHERE "patch_min" <= 10 AND 10 <= "patch_max"
