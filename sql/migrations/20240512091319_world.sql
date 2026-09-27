INSERT INTO `migrations` VALUES ('20240512091319');

-- Add your query below.
 -- Fix Blackwing Technician loot exploit.
 UPDATE `creature_groups` SET `flags` =6 WHERE `leader_guid` IN (59768, 59771, 84542, 84557, 84558, 84625, 84840, 300979, 300980);

-- End of migration.
;
