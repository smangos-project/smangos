INSERT INTO `migrations` VALUES ('20260930160000');

UPDATE dbc_map SET InstanceType = 0, MaxPlayers = 0, CorpseMapID = -1, MapName_lang = 'Testing' WHERE ID = 13;
UPDATE dbc_map SET InstanceType = 1, MaxPlayers = 0, CorpseMapID = -1, MapName_lang = 'CashTest' WHERE ID = 29;
UPDATE dbc_map SET InstanceType = 3, MaxPlayers = 40, CorpseMapID = -1, MapName_lang = 'Alterac Valley' WHERE ID = 30;
UPDATE dbc_map SET InstanceType = 0, MaxPlayers = 0, CorpseMapID = -1, MapName_lang = '<unused> StormwindPrison' WHERE ID = 35;
UPDATE dbc_map SET InstanceType = 0, MaxPlayers = 30, CorpseMapID = -1, MapName_lang = 'Azshara Crater' WHERE ID = 37;
UPDATE dbc_map SET InstanceType = 1, MaxPlayers = 0, CorpseMapID = -1, MapName_lang = '<unused> Monastery' WHERE ID = 44;
UPDATE dbc_map SET InstanceType = 1, MaxPlayers = 10, CorpseMapID = 0, MapName_lang = 'Sunken Temple' WHERE ID = 109;
UPDATE dbc_map SET InstanceType = 2, MaxPlayers = 0, CorpseMapID = -1, MapName_lang = 'Emerald Dream' WHERE ID = 169;
UPDATE dbc_map SET InstanceType = 1, MaxPlayers = 0, CorpseMapID = -1, MapName_lang = 'Caverns of Time' WHERE ID = 269;
UPDATE dbc_map SET InstanceType = 0, MaxPlayers = 0, CorpseMapID = -1, MapName_lang = 'Deeprun Tram' WHERE ID = 369;
UPDATE dbc_map SET InstanceType = 0, MaxPlayers = 0, CorpseMapID = -1, MapName_lang = 'Champions'' Hall' WHERE ID = 449;
UPDATE dbc_map SET InstanceType = 0, MaxPlayers = 0, CorpseMapID = -1, MapName_lang = 'Hall of Legends' WHERE ID = 450;
UPDATE dbc_map SET InstanceType = 0, MaxPlayers = 0, CorpseMapID = -1, MapName_lang = 'Development Land' WHERE ID = 451;
UPDATE dbc_map SET InstanceType = 3, MaxPlayers = 10, CorpseMapID = -1, MapName_lang = 'Warsong Gulch' WHERE ID = 489;
UPDATE dbc_map SET InstanceType = 3, MaxPlayers = 15, CorpseMapID = -1, MapName_lang = 'Arathi Basin' WHERE ID = 529;
UPDATE dbc_map SET InstanceType = 2, MaxPlayers = 40, CorpseMapID = -1, MapName_lang = 'Naxxramas' WHERE ID = 533;

ALTER TABLE `map_template` DROP COLUMN `parent`;
ALTER TABLE `map_template` DROP COLUMN `map_type`;
ALTER TABLE `map_template` DROP COLUMN `player_limit`;
ALTER TABLE `map_template` DROP COLUMN `ghost_entrance_map`;
ALTER TABLE `map_template` DROP COLUMN `map_name`;
