INSERT INTO `migrations` VALUES ('20240110111558');

-- Add your query below.
 -- Remove custom change to Summon Various DND.
 DELETE FROM `spell_effect_mod` WHERE `Id` =27203;

-- End of migration.
;
