INSERT INTO `migrations` VALUES ('20240328202439');

-- Add your query below.
 -- Hakkari Frostwing should wander and share spawns with Hakkari Sapper.
 UPDATE `creature` SET `id2` =8336, `movement_type` =1, `wander_distance` =15 WHERE `id` =5291;

-- End of migration.
;
