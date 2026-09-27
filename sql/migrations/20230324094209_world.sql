INSERT INTO `migrations` VALUES ('20230324094209');

-- Add your query below.
 -- Entry at creature_spells was already existing, just spell_list_id was not linked
 UPDATE `creature_template` SET `spell_list_id` = 18950 WHERE `entry` =1895;

-- End of migration.
;
