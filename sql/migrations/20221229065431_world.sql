INSERT INTO `migrations` VALUES ('20221229065431');

-- Add your query below.
 -- Eye of Kilrogg should not have Immune to NPC flag.
 UPDATE `creature_template` SET `unit_flags` =0 WHERE `entry` =4277;

-- End of migration.
;
