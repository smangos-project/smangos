INSERT INTO `migrations` VALUES ('20250801071813');

-- Add your query below.
 -- Digging Claws should not summon mobs every time they're used.
 UPDATE `spell_template` SET `script_name` = 'spell_digging_claw' WHERE `entry` =5166;

-- End of migration.
;
