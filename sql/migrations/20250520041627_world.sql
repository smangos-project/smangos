INSERT INTO `migrations` VALUES ('20250520041627');

-- Add your query below.
 UPDATE `spell_template` SET `script_name` = 'spell_warlock_create_healthstone' WHERE `entry` IN (5699, 6201, 6202, 11729, 11730);

-- End of migration.
;
