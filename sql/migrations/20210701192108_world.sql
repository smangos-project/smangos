INSERT INTO `migrations` VALUES ('20210701192108');

-- Add your query below.
 -- Remove not needed hack of spell Tame Ice Claw Bear.
 DELETE FROM `spell_effect_mod` WHERE `Id` =19597;

-- End of migration.
;
