INSERT INTO `migrations` VALUES ('20260930050000');
-- Vanilla collision height of three models, within float rounding of the client value.

UPDATE `dbc_creaturemodeldata` SET
  `CollisionHeight` = (SELECT v.`CollisionHeight` FROM `vdbc_creaturemodeldata` v WHERE v.`ID` = `dbc_creaturemodeldata`.`ID`)
WHERE `ID` IN (44, 2091, 2131);
