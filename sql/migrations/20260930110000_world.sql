INSERT INTO `migrations` VALUES ('20260930110000');

UPDATE `dbc_taxinodes` SET `MountCreatureID1` = 17660, `MountCreatureID2` = 17660 WHERE `ID` IN (85, 86, 87);
-- Endpoint of a client taxi path.
INSERT INTO `dbc_taxinodes` (`ID`, `ContinentID`, `Pos1`, `Pos2`, `Pos3`, `Name_lang`, `MountCreatureID1`, `MountCreatureID2`) VALUES (81, 131074, 0, 0, 0, 'Filming', 0, 0);

DROP TABLE `gameobject_display_info_addon`;
DROP TABLE `taxi_nodes`;
