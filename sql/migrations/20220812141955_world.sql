INSERT INTO `migrations` VALUES ('20220812141955');

-- Add your query below.
 -- Add check for FoV hack, FoV value is also used in MaxCameraDistance Calculation so this can also be used as Max Camera Distance hack.
 INSERT INTO `warden_scans` (`id`, `type`, `str`, `data`, `address`, `length`, `result`, `flags`, `penalty`, `comment`) VALUES (92, 0, NULL, NULL, 8423860, 4, 'DB0FC93F', 2, -1, 'FoV Hack');

-- End of migration.
;
