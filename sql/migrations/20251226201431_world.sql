INSERT INTO `migrations` VALUES ('20251226201431');

-- Add your query below.
 -- Fix Void Bolt used by Kazzak.
 DELETE FROM `spell_effect_mod` WHERE `Id` =22709;

UPDATE `creature_spells` SET `spellId_4` =21066 WHERE `entry` =123970;

UPDATE `creature_spells` SET `spellId_4` =21066 WHERE `entry` =123971;

-- End of migration.
;
