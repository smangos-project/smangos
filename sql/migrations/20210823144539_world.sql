INSERT INTO `migrations` VALUES ('20210823144539');

-- Add your query below.
 -- Elementals to the left of Gehennas should not be linked together, supposedly.
 DELETE FROM `creature_groups` WHERE `leader_guid` =91282;

-- Add missing target definitions for Majordomo's Magic Reflection.
 INSERT INTO `spell_script_target` (`entry`, `type`, `targetEntry`, `inverseEffectMask`) VALUES (20619, 1, 11663, 0);

INSERT INTO `spell_script_target` (`entry`, `type`, `targetEntry`, `inverseEffectMask`) VALUES (20619, 1, 11664, 0);

-- End of migration.
;
