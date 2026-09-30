INSERT INTO `migrations` VALUES ('20260930010000');

UPDATE `dbc_chatchannels` SET `Flags` = 0 WHERE `ID` = 24;
UPDATE `dbc_talenttab` SET `OrderIndex` = 0 WHERE `ID` = 41;
