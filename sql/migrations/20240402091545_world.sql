INSERT INTO `migrations` VALUES ('20240402091545');

-- Add your query below.
 -- Remove hacks to Zinfizzlex's Portable Shredder Unit.
 UPDATE `item_template` SET `spellid_2` =0, `spellcooldown_2` =-1 WHERE `entry` =17384;

DELETE FROM `spell_effect_mod` WHERE `Id` =21544;

DELETE FROM `spell_effect_mod` WHERE `Id` =21565;

-- End of migration.
;
