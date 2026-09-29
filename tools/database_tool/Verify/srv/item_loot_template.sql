-- key: entry, item
SELECT "entry", "item", "ChanceOrQuestChance", "groupid", "mincountOrRef", "maxcount", "condition_id", "patch_min", "patch_max" FROM "item_loot_template" WHERE "patch_min" <= 10 AND 10 <= "patch_max"
