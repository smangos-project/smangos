INSERT INTO `migrations` VALUES ('20221210211120');

-- Add your query below.
 -- J'eevee should be immune to both players and creatures.
 UPDATE `creature_template` SET `unit_flags` =768 WHERE `entry` =14500;

-- End of migration.
;
