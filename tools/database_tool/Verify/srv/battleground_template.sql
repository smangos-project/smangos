-- key: id
SELECT b."id" AS "id",
       d."MinPlayers" AS "min_players_per_team",
       d."MaxPlayers" AS "max_players_per_team",
       d."MinLevel" AS "min_level",
       d."MaxLevel" AS "max_level",
       b."alliance_win_spell" AS "alliance_win_spell",
       b."alliance_lose_spell" AS "alliance_lose_spell",
       b."horde_win_spell" AS "horde_win_spell",
       b."horde_lose_spell" AS "horde_lose_spell",
       b."alliance_start_location" AS "alliance_start_location",
       b."horde_start_location" AS "horde_start_location",
       b."player_loot_id" AS "player_loot_id"
FROM "battleground_template" b
JOIN "dbc_battlemasterlist" d ON d."ID" = b."id"
