INSERT INTO `migrations` VALUES ('20221108221837');

-- Add your query below.
 -- Disable some warden scans on 1.12.3.
 UPDATE `warden_scans` SET `flags` = (`flags` & ~8) WHERE `id` IN (74, 75, 76, 77, 78, 85);

-- End of migration.
;
