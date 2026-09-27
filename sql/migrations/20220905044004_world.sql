INSERT INTO `migrations` VALUES ('20220905044004');

-- Add your query below.
 -- Remove wrong mod of Aura of Nature.
 DELETE FROM `spell_mod` WHERE `Id` =25044;

-- End of migration.
;
