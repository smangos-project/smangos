INSERT INTO `migrations` VALUES ('20240625223058');

-- Add your query below.
 -- Stratholme door check fails before 1.11.
 UPDATE `warden_scans` SET `build_min` =5464 WHERE `id` =74;

-- End of migration.
;
