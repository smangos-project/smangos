INSERT INTO `migrations` VALUES ('20210804212019');

-- Add your query below.
 -- Update Firecrackers Respawn Times
 UPDATE `gameobject` SET `spawntimesecsmin` = 10, `spawntimesecsmax` = 10 WHERE `id` IN (180763, 180764);

-- End of migration.
;
