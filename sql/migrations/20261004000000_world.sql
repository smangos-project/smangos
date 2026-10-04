INSERT INTO `migrations` VALUES ('20261004000000');

UPDATE `dbc_battlemasterlist` SET `MinPlayers` = (SELECT b.`min_players_per_team` FROM `battleground_template` b WHERE b.`id` = `dbc_battlemasterlist`.`ID`) WHERE `ID` IN (SELECT `id` FROM `battleground_template`);

CREATE TABLE `battleground_template__new`( `id` INTEGER NOT NULL DEFAULT 0, `alliance_win_spell` INTEGER NOT NULL DEFAULT 0, `alliance_lose_spell` INTEGER NOT NULL DEFAULT 0, `horde_win_spell` INTEGER NOT NULL DEFAULT 0, `horde_lose_spell` INTEGER NOT NULL DEFAULT 0, `alliance_start_location` INTEGER NOT NULL DEFAULT 0, `horde_start_location` INTEGER NOT NULL DEFAULT 0, `player_loot_id` INTEGER NOT NULL DEFAULT 0, PRIMARY KEY(`id`));
INSERT INTO `battleground_template__new` (`id`, `alliance_win_spell`, `alliance_lose_spell`, `horde_win_spell`, `horde_lose_spell`, `alliance_start_location`, `horde_start_location`, `player_loot_id`) SELECT `id`, `alliance_win_spell`, `alliance_lose_spell`, `horde_win_spell`, `horde_lose_spell`, `alliance_start_location`, `horde_start_location`, `player_loot_id` FROM `battleground_template`;
DROP TABLE `battleground_template`;
ALTER TABLE `battleground_template__new` RENAME TO `battleground_template`;
