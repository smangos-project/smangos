INSERT INTO `migrations` VALUES ('20231030221913');

-- Add your query below.
 -- Fix shaman sapta quests.
 INSERT INTO `spell_script_target` (`entry`, `type`, `targetEntry`, `conditionId`, `inverseEffectMask`, `build_min`, `build_max`) VALUES (8202, 0, 100028, 0, 0, 0, 5875);

INSERT INTO `spell_script_target` (`entry`, `type`, `targetEntry`, `conditionId`, `inverseEffectMask`, `build_min`, `build_max`) VALUES (8202, 0, 101749, 0, 0, 0, 5875);

INSERT INTO `spell_script_target` (`entry`, `type`, `targetEntry`, `conditionId`, `inverseEffectMask`, `build_min`, `build_max`) VALUES (8898, 0, 101748, 0, 0, 0, 5875);

INSERT INTO `spell_script_target` (`entry`, `type`, `targetEntry`, `conditionId`, `inverseEffectMask`, `build_min`, `build_max`) VALUES (8899, 0, 101750, 0, 0, 0, 5875);

-- End of migration.
;
