INSERT INTO `migrations` VALUES ('20260801035152');

-- Add your query below.
 -- Fix Obsidian Destroyer's Drain Mana targeting.
 DELETE FROM `spell_effect_mod` WHERE `Id` =25755 AND `EffectIndex` IN (0, 1);

-- End of migration.
;
