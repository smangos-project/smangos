INSERT INTO `migrations` VALUES ('20251227013205');

-- Add your query below.
 -- Remove obsolete hacks to Furious Howl.
 DELETE FROM `spell_mod` WHERE `Id` IN (24597, 24603, 24604, 24605);

-- End of migration.
;
