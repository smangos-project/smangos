INSERT INTO `migrations` VALUES ('20231130171235');

-- Add your query below.
 -- Fix opening Sharpbeak's cage.
 INSERT INTO `spell_script_target` (`entry`, `type`, `targetEntry`, `conditionId`, `inverseEffectMask`, `build_min`, `build_max`) VALUES (11792, 0, 144070, 0, 0, 0, 5875);

-- End of migration.
;
