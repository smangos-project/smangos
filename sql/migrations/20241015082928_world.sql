INSERT INTO `migrations` VALUES ('20241015082928');

-- Add your query below.
 -- Add missing relations for quest Mortal Champions.
 INSERT INTO `creature_questrelation` (`id`, `quest`, `patch_min`) VALUES (15503, 8579, 7);

INSERT INTO `creature_involvedrelation` (`id`, `quest`, `patch_min`) VALUES (15503, 8579, 7);

-- End of migration.
;
