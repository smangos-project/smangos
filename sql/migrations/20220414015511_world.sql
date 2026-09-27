INSERT INTO `migrations` VALUES ('20220414015511');

-- Add your query below.
 -- Naxxramas should be spawned as a transport, not a normal gameobject.
 DELETE FROM `gameobject` WHERE `id` =181056;

INSERT INTO `transports` (`entry`, `name`, `period`) VALUES (181056, 'Naxxramas', 1208095);

-- End of migration.
;
