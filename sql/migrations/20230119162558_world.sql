INSERT INTO `migrations` VALUES ('20230119162558');

-- Add your query below.
 -- Remove empty creature group.
 DELETE FROM `creature_groups` WHERE `leader_guid` =33676;

-- End of migration.
;
