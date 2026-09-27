INSERT INTO `migrations` VALUES ('20241027045545');

-- Add your query below.
 -- Adds 2 more threat to Seal of Justice, I missed it in prior PR
 UPDATE `spell_threat` SET `threat` =22 WHERE `entry` =20164;

-- Blessing of Kings should give 20 threat
 INSERT INTO `spell_threat` (`entry`, `Threat`, `multiplier`, `ap_bonus`, `build_min`, `build_max`) VALUES (20217, 20, 1, 0, 0, 5875);

-- End of migration.
;
