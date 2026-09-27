INSERT INTO `migrations` VALUES ('20220107020332');

-- Add your query below.
 -- Assign "Skip Missing Targets" flag to Shadowy Assassin's waypoints command.
 UPDATE `generic_scripts` SET `data_flags` = `data_flags` | 16 WHERE `command` =60 AND `id` BETWEEN 243517 AND 243526;

-- Remove Move Random commands from Shadowy Assassin spawn script, and assign it as default movement.
 UPDATE `creature_template` SET `movement_type` =1 WHERE `entry` =2434;

DELETE FROM `generic_scripts` WHERE `command` =20 AND `id` BETWEEN 243513 AND 243526;

UPDATE `creature_movement_scripts` SET `dataint2` =0 WHERE `command` =10 AND `id` =243501 AND `dataint2` BETWEEN 243513 AND 243516;

-- End of migration.
;
