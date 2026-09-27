INSERT INTO `migrations` VALUES ('20240201124400');

-- Add your query below.
 -- Dispatch Commander Metz should finish Writ of Safe Passage.
 INSERT INTO `creature_involvedrelation` (`id`, `quest`, `patch_min`, `patch_max`) VALUES (16212, 9165, 9, 10);

-- End of migration.
;
