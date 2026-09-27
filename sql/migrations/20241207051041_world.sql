INSERT INTO `migrations` VALUES ('20241207051041');

-- Add your query below.
 -- AQ Door was added in patch 1.9.
 UPDATE `warden_scans` SET `build_min` =5086 WHERE `id` =77;

-- End of migration.
;
