INSERT INTO `migrations` VALUES ('20250217053135');

-- Add your query below.
 -- Script for Hunter's Wyvern Sting.
 UPDATE `spell_template` SET `script_name` = 'spell_hunter_wyvern_sting' WHERE `entry` IN (19386, 24132, 24133) AND `build` >= 4695;

-- End of migration.
;
