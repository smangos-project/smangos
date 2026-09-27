INSERT INTO `migrations` VALUES ('20230714180456');

-- Add your query below.
 -- Dire Maul / 153464 / 153469 / Large Solid Chest / Large Mithril Bound Chest
 DELETE FROM `pool_template` WHERE `entry` IN (SELECT `pool_entry` FROM `pool_gameobject` WHERE `guid` IN (429031, 429030, 429032, 429033));

DELETE FROM `gameobject` WHERE `guid` IN (429031, 429030, 429032, 429033);

DELETE FROM `pool_gameobject` WHERE `guid` IN (429031, 429030, 429032, 429033);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (2380+1, 153464, 429, 476.613, 570.668, 27.9231, 4.88692, 0, 0, -0.642787, 0.766045, 604800, 604800, 100, 1, 0, 0, 0, 10), (2380+2, 153464, 429, 335.605, 366.155, 2.85293, 3.6652, 0, 0, -0.965925, 0.258821, 604800, 604800, 100, 1, 0, 0, 0, 10), (2380+3, 153464, 429, 582.929, 608.082, -25.4027, 3.71755, 0, 0, -0.958819, 0.284016, 604800, 604800, 100, 1, 0, 0, 0, 10), (2380+4, 153469, 429, 476.613, 570.668, 27.9231, 4.88692, 0, 0, -0.642787, 0.766045, 604800, 604800, 100, 1, 0, 0, 0, 10), (2380+5, 153469, 429, 335.605, 366.155, 2.85293, 3.6652, 0, 0, -0.965925, 0.258821, 604800, 604800, 100, 1, 0, 0, 0, 10), (2380+6, 153469, 429, 582.929, 608.082, -25.4027, 3.71755, 0, 0, -0.958819, 0.284016, 604800, 604800, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `patch_min`, `patch_max`) VALUES (4800+1, 1, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul North', 0, 10), (4800+2, 1, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul North', 0, 10), (4800+3, 1, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul North', 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`, `patch_min`, `patch_max`) VALUES (2380+1, 4800+1, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul North', 0, 10), (2380+2, 4800+2, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul North', 0, 10), (2380+3, 4800+3, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul North', 0, 10), (2380+4, 4800+1, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul North', 0, 10), (2380+5, 4800+2, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul North', 0, 10), (2380+6, 4800+3, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul North', 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4800+4, 2, 'Dire Maul North - Large Solid Chest / Large Mithril Bound Chest (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (4800+1, 4800+4, 0, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul North', 0), (4800+2, 4800+4, 0, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul North', 0), (4800+3, 4800+4, 0, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul North', 0);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (2570+1, 153464, 429, 43.2012, -383.143, -58.6196, 4.55531, 0, 0, -0.760406, 0.649449, 604800, 604800, 100, 1, 0, 0, 0, 10), (2570+2, 153464, 429, -16.1474, -383.786, -58.62, 4.95674, 0, 0, -0.615661, 0.788011, 604800, 604800, 100, 1, 0, 0, 0, 10), (2570+3, 153464, 429, 68.3917, -397.177, -4.22106, 3.927, 0, 0, -0.923879, 0.382686, 604800, 604800, 100, 1, 0, 0, 0, 10), (2570+4, 153469, 429, 43.2012, -383.143, -58.6196, 4.55531, 0, 0, -0.760406, 0.649449, 604800, 604800, 100, 1, 0, 0, 0, 10), (2570+5, 153469, 429, -16.1474, -383.786, -58.62, 4.95674, 0, 0, -0.615661, 0.788011, 604800, 604800, 100, 1, 0, 0, 0, 10), (2570+6, 153469, 429, 68.3917, -397.177, -4.22106, 3.927, 0, 0, -0.923879, 0.382686, 604800, 604800, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `patch_min`, `patch_max`) VALUES (4810+1, 1, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul East', 0, 10), (4810+2, 1, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul East', 0, 10), (4810+3, 1, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul East', 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`, `patch_min`, `patch_max`) VALUES (2570+1, 4810+1, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul East', 0, 10), (2570+2, 4810+2, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul East', 0, 10), (2570+3, 4810+3, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul East', 0, 10), (2570+4, 4810+1, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul East', 0, 10), (2570+5, 4810+2, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul East', 0, 10), (2570+6, 4810+3, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul East', 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4810+4, 2, 'Dire Maul East - Large Solid Chest / Large Mithril Bound Chest (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (4810+1, 4810+4, 0, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul East', 0), (4810+2, 4810+4, 0, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul East', 0), (4810+3, 4810+4, 0, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul East', 0);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (2918+1, 153464, 429, 122.562, 392.1, 28.601, 3.07177, 0, 0, 0.999391, 0.0349061, 604800, 604800, 100, 1, 0, 0, 0, 10), (2918+2, 153464, 429, 122.574, 366.941, -3.48382, 2.89725, 0, 0, 0.992546, 0.12187, 604800, 604800, 100, 1, 0, 0, 0, 10), (2918+3, 153464, 429, -95.0448, 367.08, -3.48702, 4.95674, 0, 0, -0.615661, 0.788011, 604800, 604800, 100, 1, 0, 0, 0, 10), (2918+4, 153469, 429, 122.562, 392.1, 28.601, 3.07177, 0, 0, 0.999391, 0.0349061, 604800, 604800, 100, 1, 0, 0, 0, 10), (2918+5, 153469, 429, 122.574, 366.941, -3.48382, 2.89725, 0, 0, 0.992546, 0.12187, 604800, 604800, 100, 1, 0, 0, 0, 10), (2918+6, 153469, 429, -95.0448, 367.08, -3.48702, 4.95674, 0, 0, -0.615661, 0.788011, 604800, 604800, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `patch_min`, `patch_max`) VALUES (4820+1, 1, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul West', 0, 10), (4820+2, 1, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul West', 0, 10), (4820+3, 1, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul West', 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`, `patch_min`, `patch_max`) VALUES (2918+1, 4820+1, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul West', 0, 10), (2918+2, 4820+2, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul West', 0, 10), (2918+3, 4820+3, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul West', 0, 10), (2918+4, 4820+1, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul West', 0, 10), (2918+5, 4820+2, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul West', 0, 10), (2918+6, 4820+3, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul West', 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4820+4, 2, 'Dire Maul West - Large Solid Chest / Large Mithril Bound Chest (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (4820+1, 4820+4, 0, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul West', 0), (4820+2, 4820+4, 0, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul West', 0), (4820+3, 4820+4, 0, 'Large Solid Chest / Large Mithril Bound Chest - Dire Maul West', 0);

-- End of migration.
;
