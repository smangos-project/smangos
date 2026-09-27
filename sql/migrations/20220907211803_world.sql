INSERT INTO `migrations` VALUES ('20220907211803');

-- Add your query below.
 -- Fix Elemental Weapons not affecting Flametongue Weapon.
 UPDATE `spell_affect` SET `SpellFamilyMask` = `SpellFamilyMask` | 2097152 WHERE `entry` IN (16266, 29079, 29080) AND `effectId` =1;

-- End of migration.
;
