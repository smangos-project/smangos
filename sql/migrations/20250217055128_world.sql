INSERT INTO `migrations` VALUES ('20250217055128');

-- Add your query below.
 -- Script for Hunter's Readiness.
 UPDATE `spell_template` SET `script_name` = 'spell_hunter_readiness' WHERE `entry` =23989;

-- Script for Hunter's Refocus.
 UPDATE `spell_template` SET `script_name` = 'spell_hunter_refocus' WHERE `entry` =24531;

-- End of migration.
;
