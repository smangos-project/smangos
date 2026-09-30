INSERT INTO `migrations` VALUES ('20261001030000');

UPDATE `dbc_spelltargetrestrictions` SET `ConeDegrees` = (SELECT c.`cone_degrees` FROM `spell_cone` c WHERE c.`entry` = `dbc_spelltargetrestrictions`.`SpellID`) WHERE `DifficultyID` = 0 AND `SpellID` IN (SELECT `entry` FROM `spell_cone`);
UPDATE `dbc_spelltargetrestrictions` SET `ConeDegrees` = 60 WHERE `DifficultyID` = 0 AND `SpellID` = 30758;
DROP TABLE `spell_cone`;
DROP TABLE `spell_enchant_charges`;
UPDATE `dbc_spellpower` SET `PowerType` = -2 WHERE `PowerType` = 4294967294;
