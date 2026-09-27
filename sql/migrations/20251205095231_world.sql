INSERT INTO `migrations` VALUES ('20251205095231');

-- Add your query below.
 -- Add missing spawn points for Jailor Borhuin.
 UPDATE `creature` SET `wander_distance` =1.5, `movement_type` =1 WHERE `guid` =17020;

INSERT INTO `creature` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `wander_distance`, `movement_type`, `spawntimesecsmin`, `spawntimesecsmax`) VALUES (17115, 2431, 0, 680.027, -912.535, 171.72, 5.74213, 1.5, 1, 300, 300);

INSERT INTO `creature` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `wander_distance`, `movement_type`, `spawntimesecsmin`, `spawntimesecsmax`) VALUES (17171, 2431, 0, 750.198, -972.267, 167.587, 4.56236, 1.5, 1, 300, 300);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (1999, 1, 'Jailor Borhuin - Strahnbrad', 0, 0, 0, 10);

INSERT INTO `pool_creature_template` (`id`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (2431, 1999, 0, 'Jailor Borhuin', 0, 0, 10);

-- End of migration.
;
