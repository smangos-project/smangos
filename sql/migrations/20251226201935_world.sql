INSERT INTO `migrations` VALUES ('20251226201935');

-- Add your query below.
 -- Dark Iron Land Mines should not move.
 UPDATE `creature_template` SET `movement_type` = 0 WHERE `entry` = 8035;

-- End of migration.
;
