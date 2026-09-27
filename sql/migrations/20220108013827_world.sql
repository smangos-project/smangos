INSERT INTO `migrations` VALUES ('20220108013827');

-- Add your query below.
 -- Fix nonsensical target and flags combination on 2 mobs.
 UPDATE `creature_ai_scripts` SET `target_type` = 0 WHERE `data_flags` = 4 AND `target_type` = 1;

-- End of migration.
;
