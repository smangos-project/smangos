INSERT INTO `migrations` VALUES ('20230322080012');

-- Add your query below.
 -- Fix respawn time of Gromsblood.
 UPDATE `gameobject` SET `spawntimesecsmin` =2700, `spawntimesecsmax` =2700 WHERE `id` =142145 AND `map` !=429;

UPDATE `gameobject` SET `spawntimesecsmin` =604800, `spawntimesecsmax` =604800 WHERE `id` =142145 AND `map` =429;

-- End of migration.
;
