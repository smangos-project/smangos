INSERT INTO `migrations` VALUES ('20260930020000');

-- Vanilla transport paths: the client stores these axes rotated.
UPDATE `dbc_transportanimation` SET
  `Pos1` = (SELECT v.`Pos1` FROM `vdbc_transportanimation` v WHERE v.`ID` = `dbc_transportanimation`.`ID`),
  `Pos2` = (SELECT v.`Pos2` FROM `vdbc_transportanimation` v WHERE v.`ID` = `dbc_transportanimation`.`ID`),
  `Pos3` = (SELECT v.`Pos3` FROM `vdbc_transportanimation` v WHERE v.`ID` = `dbc_transportanimation`.`ID`)
WHERE `ID` IN (SELECT `ID` FROM `vdbc_transportanimation`);
