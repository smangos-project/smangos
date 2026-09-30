INSERT INTO `migrations` VALUES ('20260930090000');

UPDATE `dbc_factiontemplate` SET `Flags` = 4129 WHERE `ID` = 1254;
UPDATE `dbc_factiontemplate` SET `FactionGroup` = 0, `FriendGroup` = 0 WHERE `ID` = 1014;
UPDATE `dbc_factiontemplate` SET `FriendGroup` = 0, `Friend2` = 0 WHERE `ID` = 54;
UPDATE `dbc_factiontemplate` SET `Friend2` = 0 WHERE `ID` = 50;
UPDATE `dbc_factiontemplate` SET `Enemies1` = 0 WHERE `ID` IN (21, 1475);

DROP TABLE `faction`;
DROP TABLE `faction_template`;
DROP TABLE `pet_spell_data`;
