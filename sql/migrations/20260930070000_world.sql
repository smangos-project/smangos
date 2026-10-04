INSERT INTO `migrations` VALUES ('20260930070000');
-- Vanilla spell durations, shapeshift forms and item enchantments where the client differs.

INSERT INTO `dbc_spellduration` (`ID`, `Duration`, `DurationPerLevel`, `MaxDuration`) SELECT `ID`, `Duration`, `DurationPerLevel`, `MaxDuration` FROM `vdbc_spellduration` WHERE `ID` IN (507, 508);
UPDATE `dbc_spellshapeshiftform` SET
  `Name_lang` = (SELECT v.`Name_lang` FROM `vdbc_spellshapeshiftform` v WHERE v.`ID` = `dbc_spellshapeshiftform`.`ID`),
  `Flags` = (SELECT v.`Flags` FROM `vdbc_spellshapeshiftform` v WHERE v.`ID` = `dbc_spellshapeshiftform`.`ID`),
  `CreatureType` = (SELECT v.`CreatureType` FROM `vdbc_spellshapeshiftform` v WHERE v.`ID` = `dbc_spellshapeshiftform`.`ID`)
WHERE `ID` IN (2, 17, 22, 24, 28, 31, 32);
UPDATE `dbc_spellitemenchantment` SET
  `Effect1` = (SELECT v.`Effect1` FROM `vdbc_spellitemenchantment` v WHERE v.`ID` = `dbc_spellitemenchantment`.`ID`),
  `Effect2` = (SELECT v.`Effect2` FROM `vdbc_spellitemenchantment` v WHERE v.`ID` = `dbc_spellitemenchantment`.`ID`),
  `Effect3` = (SELECT v.`Effect3` FROM `vdbc_spellitemenchantment` v WHERE v.`ID` = `dbc_spellitemenchantment`.`ID`),
  `EffectPointsMin1` = (SELECT v.`EffectPointsMin1` FROM `vdbc_spellitemenchantment` v WHERE v.`ID` = `dbc_spellitemenchantment`.`ID`),
  `EffectPointsMin2` = (SELECT v.`EffectPointsMin2` FROM `vdbc_spellitemenchantment` v WHERE v.`ID` = `dbc_spellitemenchantment`.`ID`),
  `EffectPointsMin3` = (SELECT v.`EffectPointsMin3` FROM `vdbc_spellitemenchantment` v WHERE v.`ID` = `dbc_spellitemenchantment`.`ID`),
  `EffectArg1` = (SELECT v.`EffectArg1` FROM `vdbc_spellitemenchantment` v WHERE v.`ID` = `dbc_spellitemenchantment`.`ID`),
  `EffectArg2` = (SELECT v.`EffectArg2` FROM `vdbc_spellitemenchantment` v WHERE v.`ID` = `dbc_spellitemenchantment`.`ID`),
  `EffectArg3` = (SELECT v.`EffectArg3` FROM `vdbc_spellitemenchantment` v WHERE v.`ID` = `dbc_spellitemenchantment`.`ID`),
  `ItemVisual` = (SELECT v.`ItemVisual` FROM `vdbc_spellitemenchantment` v WHERE v.`ID` = `dbc_spellitemenchantment`.`ID`),
  `Flags` = (SELECT v.`Flags` FROM `vdbc_spellitemenchantment` v WHERE v.`ID` = `dbc_spellitemenchantment`.`ID`)
WHERE `ID` IN (3, 4, 5, 25, 26, 283, 284, 523, 525, 1665, 1666, 1669, 1900, 2605);
