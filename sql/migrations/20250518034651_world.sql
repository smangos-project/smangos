INSERT INTO `migrations` VALUES ('20250518034651');

-- Add your query below.
 UPDATE `spell_template` SET `script_name` = 'spell_hunter_mongoose_bite' WHERE `entry` IN (1495, 14269, 14270, 14271);

UPDATE `spell_template` SET `script_name` = 'spell_hunter_counterattack' WHERE `entry` IN (19306, 20909, 20910);

UPDATE `spell_template` SET `script_name` = 'spell_reindeer_transformation' WHERE `entry` IN (25860);

UPDATE `spell_template` SET `script_name` = 'spell_bag_of_gold' WHERE `entry` IN (25720);

-- End of migration.
;
