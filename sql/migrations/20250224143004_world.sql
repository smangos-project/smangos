INSERT INTO `migrations` VALUES ('20250224143004');

-- Add your query below.
 UPDATE `creature` SET `wander_distance` = 1.15, `movement_type` = 1 WHERE `guid` = 38291;

-- End of migration.
;
