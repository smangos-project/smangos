INSERT INTO `migrations` VALUES ('20250523160438');

-- Add your query below.
 -- Slim's Friend should be immune to Mind Control.
 UPDATE `creature_template` SET `mechanic_immune_mask` =1 WHERE `entry` =4971;

-- End of migration.
;
