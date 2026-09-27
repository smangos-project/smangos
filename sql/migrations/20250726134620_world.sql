INSERT INTO `migrations` VALUES ('20250726134620');

-- Add your query below.
 -- Fix Westfall Woodworker script.
 UPDATE `creature_movement_scripts` SET `data_flags` =2 WHERE `delay` =5 AND `command` =41 AND `id` IN (4552103, 4552104, 4552105, 4552106);

-- End of migration.
;
