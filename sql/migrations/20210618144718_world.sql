INSERT INTO `migrations` VALUES ('20210618144718');

-- Add your query below.
 -- Fix middle deeprun tram subway car.
 UPDATE `gameobject_template` SET `data5` =4294967295 WHERE `entry` =176084;

-- End of migration.
;
