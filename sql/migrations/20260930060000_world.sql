INSERT INTO `migrations` VALUES ('20260930060000');
-- Vanilla rows and values missing from or different in the client's game object display, lock, skill line and skill race/class tables.

INSERT INTO `dbc_gameobjectdisplayinfo` (`ID`, `ModelName`) SELECT `ID`, `ModelName` FROM `vdbc_gameobjectdisplayinfo` WHERE `ID` IN (171, 172, 173, 176, 177, 179, 180, 233, 349, 350, 351, 380, 601, 622, 623, 626, 632, 751, 756, 6615);
INSERT INTO `dbc_skillline` (`ID`, `CategoryID`, `DisplayName_lang`) SELECT `ID`, `CategoryID`, `DisplayName_lang` FROM `vdbc_skillline` WHERE `ID` IN (762);
INSERT INTO `dbc_skillraceclassinfo` (`ID`, `SkillID`, `RaceMask`, `ClassMask`, `Flags`, `MinLevel`, `SkillTierID`) SELECT `ID`, `SkillID`, `RaceMask`, `ClassMask`, `Flags`, `MinLevel`, `SkillTierID` FROM `vdbc_skillraceclassinfo` WHERE `ID` IN (890);
UPDATE `dbc_lock` SET
  `Index2` = (SELECT v.`Index2` FROM `vdbc_lock` v WHERE v.`ID` = `dbc_lock`.`ID`),
  `Type2` = (SELECT v.`Type2` FROM `vdbc_lock` v WHERE v.`ID` = `dbc_lock`.`ID`)
WHERE `ID` IN (1647);
UPDATE `dbc_skillraceclassinfo` SET
  `Flags` = (SELECT v.`Flags` FROM `vdbc_skillraceclassinfo` v WHERE v.`ID` = `dbc_skillraceclassinfo`.`ID`)
WHERE `ID` IN (5, 24, 25, 26, 31, 32, 33, 39, 54, 55, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 68, 69, 70, 71, 72, 73, 74, 75, 79, 81, 82, 87, 89, 92, 94, 95, 96, 97, 98, 101, 102, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 131, 132, 149, 301, 441, 442, 443, 445, 446, 461, 463, 464, 481, 501, 521, 522, 541, 544, 581, 781, 841, 861, 862);
