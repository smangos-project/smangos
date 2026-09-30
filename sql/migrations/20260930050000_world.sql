INSERT INTO `migrations` VALUES ('20260930050000');

-- Vanilla collision height of three models, within float rounding of the client value.
UPDATE `dbc_creaturemodeldata` SET `CollisionHeight` = 2.125999927520752 WHERE `ID` = 44;
UPDATE `dbc_creaturemodeldata` SET `CollisionHeight` = 2.125999927520752 WHERE `ID` = 2091;
UPDATE `dbc_creaturemodeldata` SET `CollisionHeight` = 2.125999927520752 WHERE `ID` = 2131;
