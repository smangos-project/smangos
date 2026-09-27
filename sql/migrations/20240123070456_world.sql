INSERT INTO `migrations` VALUES ('20240123070456');

-- Add your query below.
 -- Reduce respawn time of Tablet of Will.
 UPDATE `gameobject` SET `spawntimesecsmin` =30, `spawntimesecsmax` =30 WHERE `id` =142088;

-- End of migration.
;
