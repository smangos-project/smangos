INSERT INTO `migrations` VALUES ('20220617120610');

-- Add your query below.
 -- Remove effect mod of Gnomish Battle Chicken to fix summon duration.
 DELETE FROM `spell_effect_mod` WHERE `Id` =23133;

-- End of migration.
;
