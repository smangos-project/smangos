INSERT INTO `migrations` VALUES ('20260109110343');

-- Add your query below.
 UPDATE `spell_template` SET `script_name` = 'spell_mage_combustion_proc' WHERE `entry` =11129 AND `build` >= 5464;

UPDATE `spell_template` SET `script_name` = 'spell_mage_combustion_buff' WHERE `entry` =28682;

-- End of migration.
;
