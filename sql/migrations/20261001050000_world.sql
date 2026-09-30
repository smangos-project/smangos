INSERT INTO `migrations` VALUES ('20261001050000');

DROP TABLE `spell_template`;
DROP TABLE `spell_mod`;
DROP TABLE `spell_effect_mod`;
DROP TABLE `tmp_spell_effective`;
DROP TABLE `tmp_spell_effect_effective`;
CREATE TABLE `spell_check__new`( `spellid` INTEGER NOT NULL DEFAULT 0, `SpellFamilyName` INTEGER NOT NULL DEFAULT -1, `SpellFamilyMask` INTEGER NOT NULL DEFAULT -1, `SpellCategory` INTEGER NOT NULL DEFAULT -1, `EffectType` INTEGER NOT NULL DEFAULT -1, `EffectAura` INTEGER NOT NULL DEFAULT -1, `EffectIdx` INTEGER NOT NULL DEFAULT -1, `Name` TEXT NOT NULL DEFAULT '' COLLATE NOCASE, `Code` TEXT NOT NULL DEFAULT '' COLLATE NOCASE, PRIMARY KEY(`spellid`, `SpellFamilyName`, `SpellFamilyMask`, `SpellCategory`, `Code`));
INSERT INTO `spell_check__new` (`spellid`, `SpellFamilyName`, `SpellFamilyMask`, `SpellCategory`, `EffectType`, `EffectAura`, `EffectIdx`, `Name`, `Code`) SELECT `spellid`, `SpellFamilyName`, `SpellFamilyMask`, `SpellCategory`, `EffectType`, `EffectAura`, `EffectIdx`, `Name`, `Code` FROM `spell_check`;
DROP TABLE `spell_check`;
ALTER TABLE `spell_check__new` RENAME TO `spell_check`;
