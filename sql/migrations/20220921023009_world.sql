INSERT INTO `migrations` VALUES ('20220921023009');

-- Add your query below.
 -- Dolanaar, Teldrassil GY is neutral.
 UPDATE `game_graveyard_zone` SET `faction` = '0' WHERE `id` =91 AND `ghost_zone` =141;

-- End of migration.
;
