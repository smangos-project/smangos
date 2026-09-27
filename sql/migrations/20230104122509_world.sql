INSERT INTO `migrations` VALUES ('20230104122509');

-- Add your query below.
 -- Extend end time of some events.
 UPDATE `game_event` SET `end_time` = '2030-12-31 00:00:00' WHERE `entry` =47;

UPDATE `game_event` SET `end_time` = '2030-12-31 12:00:00' WHERE `entry` =48;

-- End of migration.
;
