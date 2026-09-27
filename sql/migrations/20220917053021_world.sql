INSERT INTO `migrations` VALUES ('20220917053021');

-- Add your query below.
 -- Remove wrong spell from Entropic Horror.
 UPDATE `creature_template` SET `spell_id1` =0, `script_name` = '' WHERE `entry` =9879;

-- End of migration.
;
