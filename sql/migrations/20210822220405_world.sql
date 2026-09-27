INSERT INTO `migrations` VALUES ('20210822220405');

-- Add your query below.
 -- Reduce respawn of Fifth Mosh'aru Tablet to match that of Sixth Mosh'aru Tablet.
 UPDATE `gameobject` SET `spawntimesecsmin` =25, `spawntimesecsmax` =25 WHERE `id` =175949;

-- End of migration.
;
