INSERT INTO `migrations` VALUES ('20230706092629');

-- Add your query below.
 -- Remove hack fixes to spell damage that tried to balance some abilities damage according to the old incorrect formula
 DELETE FROM `spell_effect_mod` WHERE `Id` =26194;

-- Skeram earth shock
 DELETE FROM `spell_effect_mod` WHERE `Id` =26192;

-- Skeram arcane explosion
 DELETE FROM `spell_effect_mod` WHERE `Id` =14297;

-- Anubisath Guardian Shadow Storm
 DELETE FROM `spell_effect_mod` WHERE `Id` =26546;

-- Anubisath Sentinel Shadow Storm
 DELETE FROM `spell_effect_mod` WHERE `Id` =26555;

-- Anubisath Defender Shadow Storm
 -- End of migration.
;
