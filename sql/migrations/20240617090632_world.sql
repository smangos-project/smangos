INSERT INTO `migrations` VALUES ('20240617090632');

-- Add your query below.
 -- Fix Widow's Embrace not hitting Grand Widow Faerlina.
 INSERT INTO `spell_script_target` (`entry`, `type`, `targetEntry`, `conditionId`, `inverseEffectMask`, `build_min`, `build_max`) VALUES (28732, 1, 15953, 0, 0, 5464, 5875);

UPDATE `creature_template` SET `immunity_flags` =2 WHERE `entry` =15953;

-- End of migration.
;
