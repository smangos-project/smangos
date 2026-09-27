INSERT INTO `migrations` VALUES ('20260629164032');

-- Add your query below.
 UPDATE `creature` SET `mana_percent` = 100 WHERE `id` IN (6379, 193, 6135, 8555);

UPDATE `creature_ai_scripts` SET `delay` = 0 WHERE `id` = 1074101;

-- End of migration.
;
