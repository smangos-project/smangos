INSERT INTO `migrations` VALUES ('20260820165456');

-- Add your query below.
 -- Use default display id scales for Trick - Critter.
 UPDATE `creature_template` SET `display_scale1` =0, `display_scale2` =0, `display_scale3` =0, `display_scale4` =0 WHERE `entry` =15219;

-- End of migration.
;
