INSERT INTO `migrations` VALUES ('20250524153727');

-- Add your query below.
 -- Fix missing loot for Jade
 INSERT INTO creature_loot_template (entry, item, ChanceOrQuestChance, groupid, mincountOrRef, maxcount, condition_id, patch_min, patch_max) VALUES (1063, 30072, 39, 0, -30072, 1, 0, 0, 10);

-- End of migration.
;
