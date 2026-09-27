INSERT INTO `migrations` VALUES ('20230123070823');

-- Add your query below.
 -- Change faction of Ritual Candle Aura to one that is hostile to mobs but friendly to players.
 -- This is a server side trap, so it's not sniffable, and true faction is unknown.
 UPDATE `gameobject_template` SET `faction` =250 WHERE `entry` =179688;

-- End of migration.
;
