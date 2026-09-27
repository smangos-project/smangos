INSERT INTO `migrations` VALUES ('20231022153238');

-- Add your query below.
 -- Improved Evasion/Endurance
 UPDATE `spell_affect` SET `build_min` = '5875' WHERE `entry` IN (13742, 13872) AND `effectId` =0;

-- Elusiveness
 UPDATE `spell_affect` SET `build_min` = '5875' WHERE `entry` IN (13981, 14066, 14067) AND `effectId` =0;

-- Improved Cheap Shot/Dirty Deeds
 UPDATE `spell_affect` SET `build_min` = '5875' WHERE `entry` IN (14082, 14083) AND `effectId` =0;

-- End of migration.
;
