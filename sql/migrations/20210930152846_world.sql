INSERT INTO `migrations` VALUES ('20210930152846');

-- Add your query below.
 -- Remove gossip npc_flag from Nijel's Point Guard.
 UPDATE `creature_template` SET `npc_flags` = 0 WHERE `entry` = 8151;

-- End of migration.
;
