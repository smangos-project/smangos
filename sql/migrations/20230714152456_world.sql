INSERT INTO `migrations` VALUES ('20230714152456');

DELETE FROM `pool_template` WHERE `entry` IN (SELECT `pool_entry` FROM `pool_gameobject` WHERE `guid` IN (35705, 35721));

DELETE FROM `gameobject` WHERE `guid` IN (35705, 35721);

DELETE FROM `pool_gameobject` WHERE `guid` IN (35705, 35721);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (353+1, 74447, 47, 2111.45, 1613.83, 81.1889, -1.98968, 0, 0, 0.838671, -0.544639, 604800, 604800, 100, 1, 0, 0, 0, 10), (353+2, 74447, 47, 2199.22, 1654.65, 86.1542, -1.37881, 0, 0, 0.636078, -0.771625, 604800, 604800, 100, 1, 0, 0, 0, 10), (353+3, 74447, 47, 2127.73, 1607.64, 81.4757, 1.309, 0, 0, 0.608761, 0.793354, 604800, 604800, 100, 1, 0, 0, 0, 10), (353+4, 74447, 47, 2202.04, 1807.37, 64.3126, 2.14675, 0, 0, 0.878817, 0.47716, 604800, 604800, 100, 1, 0, 0, 0, 10), (353+5, 74448, 47, 2111.45, 1613.83, 81.1889, -1.98968, 0, 0, 0.838671, -0.544639, 604800, 604800, 100, 1, 0, 0, 0, 10), (353+6, 74448, 47, 2199.22, 1654.65, 86.1542, -1.37881, 0, 0, 0.636078, -0.771625, 604800, 604800, 100, 1, 0, 0, 0, 10), (353+7, 74448, 47, 2127.73, 1607.64, 81.4757, 1.309, 0, 0, 0.608761, 0.793354, 604800, 604800, 100, 1, 0, 0, 0, 10), (353+8, 74448, 47, 2202.04, 1807.37, 64.3126, 2.14675, 0, 0, 0.878817, 0.47716, 604800, 604800, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `patch_min`, `patch_max`) VALUES (2761+1, 1, 'Large Iron Bound Chest / Large Solid Chest - Razorfen Kraul', 0, 10), (2761+2, 1, 'Large Iron Bound Chest / Large Solid Chest - Razorfen Kraul', 0, 10), (2761+3, 1, 'Large Iron Bound Chest / Large Solid Chest - Razorfen Kraul', 0, 10), (2761+4, 1, 'Large Iron Bound Chest / Large Solid Chest - Razorfen Kraul', 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`, `patch_min`, `patch_max`) VALUES (353+1, 2761+1, 'Large Iron Bound Chest / Large Solid Chest - Razorfen Kraul', 0, 10), (353+2, 2761+2, 'Large Iron Bound Chest / Large Solid Chest - Razorfen Kraul', 0, 10), (353+3, 2761+3, 'Large Iron Bound Chest / Large Solid Chest - Razorfen Kraul', 0, 10), (353+4, 2761+4, 'Large Iron Bound Chest / Large Solid Chest - Razorfen Kraul', 0, 10), (353+5, 2761+1, 'Large Iron Bound Chest / Large Solid Chest - Razorfen Kraul', 0, 10), (353+6, 2761+2, 'Large Iron Bound Chest / Large Solid Chest - Razorfen Kraul', 0, 10), (353+7, 2761+3, 'Large Iron Bound Chest / Large Solid Chest - Razorfen Kraul', 0, 10), (353+8, 2761+4, 'Large Iron Bound Chest / Large Solid Chest - Razorfen Kraul', 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (2761+5, 2, 'Razorfen Kraul - Large Iron Bound Chest / Large Solid Chest (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (2761+1, 2761+5, 0, 'Large Iron Bound Chest / Large Solid Chest - Razorfen Kraul', 0), (2761+2, 2761+5, 0, 'Large Iron Bound Chest / Large Solid Chest - Razorfen Kraul', 0), (2761+3, 2761+5, 0, 'Large Iron Bound Chest / Large Solid Chest - Razorfen Kraul', 0), (2761+4, 2761+5, 0, 'Large Iron Bound Chest / Large Solid Chest - Razorfen Kraul', 0);

-- End of migration.
;
