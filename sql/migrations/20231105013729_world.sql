INSERT INTO `migrations` VALUES ('20231105013729');

-- Add your query below.
 -- Remove custom change to icon of Consecration.
 DELETE FROM `spell_mod` WHERE `Id` IN (20116, 20922, 20923, 20924, 26573);

-- End of migration.
;
