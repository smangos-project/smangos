-- key: entry, item, groupid
SELECT "entry", "item", "ChanceOrQuestChance", "groupid", "mincountOrRef", "maxcount", "condition_id", "patch_min", "patch_max" FROM "creature_loot_template" WHERE "patch_min" <= 10 AND 10 <= "patch_max"
