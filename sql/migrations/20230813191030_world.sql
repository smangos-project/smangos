INSERT INTO `migrations` VALUES ('20230813191030');

-- Add your query below.
 UPDATE `warden_scans` SET `build_max` = 6141 WHERE `id` IN (74, 75, 76, 77, 78);

-- End of migration.
;
