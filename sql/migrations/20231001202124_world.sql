INSERT INTO `migrations` VALUES ('20231001202124');

-- Add your query below.
 -- Flightmasters at Valors Rest in Silithus was added in 1.3
 UPDATE `creature` SET `patch_min` =1 WHERE `guid` =14;

UPDATE `creature` SET `patch_min` =1 WHERE `guid` =18;

UPDATE `creature_template` SET `patch` =1 WHERE `entry` =1233 AND `patch` =0;

UPDATE `creature_template` SET `patch` =1 WHERE `entry` =14242 AND `patch` =0;

-- End of migration.
;
