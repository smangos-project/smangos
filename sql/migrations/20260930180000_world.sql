INSERT INTO `migrations` VALUES ('20260930180000');

UPDATE dbc_areatable SET ContinentID = 30, AreaBit = 0 WHERE ID = 2817;
UPDATE dbc_areatable SET ContinentID = 1, AreaBit = 988 WHERE ID = 2917;
UPDATE dbc_areatable SET ContinentID = 531, AreaBit = 1066 WHERE ID = 3428;
UPDATE dbc_areatable SET ContinentID = 509, AreaBit = 1067 WHERE ID = 3429;
UPDATE dbc_areatable SET ContinentID = 1, AreaBit = 1066 WHERE ID = 3446;
UPDATE dbc_areatable SET ContinentID = 1, AreaBit = 1067 WHERE ID = 3447;
UPDATE dbc_areatable SET ContinentID = 533, AreaBit = 1075 WHERE ID = 3456;
UPDATE dbc_areatable SET ContinentID = 0, AreaBit = 0 WHERE ID = 3459;
UPDATE dbc_areatable SET ContinentID = 1, AreaBit = 1075 WHERE ID = 3478;
UPDATE `dbc_areatable` SET `Flags1` = 64 WHERE `ID` = 206;

DROP TABLE `area_template`;
