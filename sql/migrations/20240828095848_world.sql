INSERT INTO `migrations` VALUES ('20240828095848');

-- Add your query below.
 -- Remove duplicated waypoint from Achellios the Banished.
 DELETE FROM `creature_movement_template` wHERE `entry` =5933 AND `point` =46;

UPDATE `creature` SET `spawntimesecsmin` =18000, `spawntimesecsmax` =28800 WHERE `guid` IN (21677, 21678, 21679);

UPDATE `creature_groups` SET `flags` =15 WHERE `leader_guid` =21388;

-- End of migration.
;
