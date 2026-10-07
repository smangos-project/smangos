INSERT INTO `migrations` VALUES ('20260930160000');

UPDATE `dbc_map` SET `CorpseMapID` = -1 WHERE `ID` IN (35, 44, 269, 369, 449, 450, 451, 489, 529, 533);
UPDATE `dbc_map` SET `MaxPlayers` = 10 WHERE `ID` = 109;

ALTER TABLE `map_template` DROP COLUMN `parent`;
ALTER TABLE `map_template` DROP COLUMN `map_type`;
ALTER TABLE `map_template` DROP COLUMN `player_limit`;
ALTER TABLE `map_template` DROP COLUMN `ghost_entrance_map`;
ALTER TABLE `map_template` DROP COLUMN `map_name`;
