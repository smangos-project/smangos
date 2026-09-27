INSERT INTO `migrations` VALUES ('20251218190441');

-- Add your query below.
 UPDATE `spell_template` SET `script_name` = 'spell_cannibalize' WHERE `entry` = 20577;

UPDATE `spell_template` SET `script_name` = 'spell_wolfshead_helm' WHERE `entry` = 17770;

-- End of migration.
;
