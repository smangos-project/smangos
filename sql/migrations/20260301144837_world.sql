INSERT INTO `migrations` VALUES ('20260301144837');

-- Add your query below.
 -- Correct Pools
 UPDATE `pool_template` SET `max_limit` =13 WHERE `entry` =1239 AND `patch_min` =0 AND `patch_max` =10;

UPDATE `pool_template` SET `max_limit` =11 WHERE `entry` =1241 AND `patch_min` =0 AND `patch_max` =10;

UPDATE `pool_template` SET `max_limit` =11 WHERE `entry` =1240 AND `patch_min` =0 AND `patch_max` =10;

UPDATE `pool_template` SET `max_limit` =11 WHERE `entry` =1243 AND `patch_min` =0 AND `patch_max` =10;

UPDATE `pool_template` SET `max_limit` =4 WHERE `entry` =1055 AND `patch_min` =0 AND `patch_max` =10;

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Bitter Reaches Pool 1
 INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `patch_min`) VALUES (6315+1, 176584, 1, 4713.92, -7084.78, 92.2154, 1.309, 0, 0, 0.608761, 0.793354, 300, 300, 100, 1, 0), (6315+2, 176584, 1, 4702.16, -7436.1, 74.7979, 1.29154, 0, 0, 0.601814, 0.798636, 300, 300, 100, 1, 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (48424, 18323, 68506);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3941+1, 1, 'Azshara - Bitter Reaches Pool 1 - Dreamfoil', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (48424, 3941+1, 0, 'Dreamfoil - Azshara - Bitter Reaches Pool 1', 0, 0, 10), (18323, 3941+1, 0, 'Dreamfoil - Azshara - Bitter Reaches Pool 1', 0, 0, 10), (68506, 3941+1, 0, 'Dreamfoil - Azshara - Bitter Reaches Pool 1', 0, 0, 10), (6315+1, 3941+1, 0, 'Dreamfoil - Azshara - Bitter Reaches Pool 1', 0, 0, 10), (6315+2, 3941+1, 0, 'Dreamfoil - Azshara - Bitter Reaches Pool 1', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Bitter Reaches Pool 2
 INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `patch_min`) VALUES (6315+3, 176584, 1, 4540.86, -6872.87, 123.385, 2.07694, 0, 0, 0.861629, 0.507539, 300, 300, 100, 1, 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (68511, 18306, 48427, 18324);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3941+2, 1, 'Azshara - Bitter Reaches Pool 2 - Dreamfoil', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (68511, 3941+2, 0, 'Dreamfoil - Azshara - Bitter Reaches Pool 2', 0, 0, 10), (18306, 3941+2, 0, 'Dreamfoil - Azshara - Bitter Reaches Pool 2', 0, 0, 10), (48427, 3941+2, 0, 'Dreamfoil - Azshara - Bitter Reaches Pool 2', 0, 0, 10), (18324, 3941+2, 0, 'Dreamfoil - Azshara - Bitter Reaches Pool 2', 0, 0, 10), (6315+3, 3941+2, 0, 'Dreamfoil - Azshara - Bitter Reaches Pool 2', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Thalassian Base Camp
 INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `patch_min`) VALUES (6315+4, 176584, 1, 4539.28, -6204.98, 95.9491, 3.14159, 0, 0, 1, -0.0000000437114, 300, 300, 100, 1, 0), (6315+5, 176584, 1, 4682.51, -6256, 114.437, 0.541051, 0, 0, 0.267238, 0.963631, 300, 300, 100, 1, 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (48422, 68512, 18321);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3941+3, 1, 'Azshara - Thalassian Base Camp - Dreamfoil', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (48422, 3941+3, 0, 'Dreamfoil - Azshara - Thalassian Base Camp', 0, 0, 10), (68512, 3941+3, 0, 'Dreamfoil - Azshara - Thalassian Base Camp', 0, 0, 10), (18321, 3941+3, 0, 'Dreamfoil - Azshara - Thalassian Base Camp', 0, 0, 10), (6315+4, 3941+3, 0, 'Dreamfoil - Azshara - Thalassian Base Camp', 0, 0, 10), (6315+5, 3941+3, 0, 'Dreamfoil - Azshara - Thalassian Base Camp', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Ursolan
 INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `patch_min`) VALUES (6315+7, 176584, 1, 4287.9, -5356.2, 124.548, 1.78023, 0, 0, 0.777145, 0.629321, 300, 300, 100, 1, 0), (6315+8, 176584, 1, 4698.31, -5699.55, 97.5191, 6.02139, 0, 0, -0.130526, 0.991445, 300, 300, 100, 1, 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (18322, 48425, 18311);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3941+4, 1, 'Azshara - Ursolan - Dreamfoil', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (18322, 3941+4, 0, 'Dreamfoil - Azshara - Ursolan', 0, 0, 10), (48425, 3941+4, 0, 'Dreamfoil - Azshara - Ursolan', 0, 0, 10), (18311, 3941+4, 0, 'Dreamfoil - Azshara - Ursolan', 0, 0, 10), (6315+7, 3941+4, 0, 'Dreamfoil - Azshara - Ursolan', 0, 0, 10), (6315+8, 3941+4, 0, 'Dreamfoil - Azshara - Ursolan', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Timbermaw Hold
 DELETE FROM `pool_gameobject` WHERE `guid` IN (48418, 18320, 68509, 68505, 18307);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3941+5, 1, 'Azshara - Timbermaw Hold - Dreamfoil', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (48418, 3941+5, 0, 'Dreamfoil - Azshara - Timbermaw Hold', 0, 0, 10), (18320, 3941+5, 0, 'Dreamfoil - Azshara - Timbermaw Hold', 0, 0, 10), (68509, 3941+5, 0, 'Dreamfoil - Azshara - Timbermaw Hold', 0, 0, 10), (68505, 3941+5, 0, 'Dreamfoil - Azshara - Timbermaw Hold', 0, 0, 10), (18307, 3941+5, 0, 'Dreamfoil - Azshara - Timbermaw Hold', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Valormok
 DELETE FROM `pool_gameobject` WHERE `guid` IN (18319, 18308, 48423, 18310, 16406, 68504);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3941+6, 1, 'Azshara - Valormok - Dreamfoil', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (18319, 3941+6, 0, 'Dreamfoil - Azshara - Valormok', 0, 0, 10), (18308, 3941+6, 0, 'Dreamfoil - Azshara - Valormok', 0, 0, 10), (48423, 3941+6, 0, 'Dreamfoil - Azshara - Valormok', 0, 0, 10), (18310, 3941+6, 0, 'Dreamfoil - Azshara - Valormok', 0, 0, 10), (16406, 3941+6, 0, 'Dreamfoil - Azshara - Valormok', 0, 0, 10), (68504, 3941+6, 0, 'Dreamfoil - Azshara - Valormok', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Haldarr Encampment
 DELETE FROM `pool_gameobject` WHERE `guid` IN (18318, 18313, 68510, 19268, 39930);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3941+7, 1, 'Azshara - Haldarr Encampment - Dreamfoil', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (18318, 3941+7, 0, 'Dreamfoil - Azshara - Haldarr Encampment', 0, 0, 10), (18313, 3941+7, 0, 'Dreamfoil - Azshara - Haldarr Encampment', 0, 0, 10), (68510, 3941+7, 0, 'Dreamfoil - Azshara - Haldarr Encampment', 0, 0, 10), (19268, 3941+7, 0, 'Dreamfoil - Azshara - Haldarr Encampment', 0, 0, 10), (39930, 3941+7, 0, 'Dreamfoil - Azshara - Haldarr Encampment', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Forlorn Ridge
 INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `patch_min`) VALUES (6315+9, 176584, 1, 2531.94, -5146.42, 112.009, 1.16937, 0, 0, 0.551936, 0.833886, 300, 300, 100, 1, 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (48421, 68508, 68507, 18315);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3941+8, 1, 'Azshara - Forlorn Ridge - Dreamfoil', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (48421, 3941+8, 0, 'Dreamfoil - Azshara - Forlorn Ridge', 0, 0, 10), (68508, 3941+8, 0, 'Dreamfoil - Azshara - Forlorn Ridge', 0, 0, 10), (68507, 3941+8, 0, 'Dreamfoil - Azshara - Forlorn Ridge', 0, 0, 10), (18315, 3941+8, 0, 'Dreamfoil - Azshara - Forlorn Ridge', 0, 0, 10), (6315+9, 3941+8, 0, 'Dreamfoil - Azshara - Forlorn Ridge', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Lake Mennar Pool 1
 INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `patch_min`) VALUES (6315+10, 176584, 1, 2903.76, -5402.82, 113.29, 2.35619, 0, 0, 0.923879, 0.382686, 300, 300, 100, 1, 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (48417, 18316, 18317, 48561);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3941+9, 1, 'Azshara - Lake Mennar Pool 1 - Dreamfoil', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (48417, 3941+9, 0, 'Dreamfoil - Azshara - Lake Mennar Pool 1', 0, 0, 10), (18316, 3941+9, 0, 'Dreamfoil - Azshara - Lake Mennar Pool 1', 0, 0, 10), (18317, 3941+9, 0, 'Dreamfoil - Azshara - Lake Mennar Pool 1', 0, 0, 10), (48561, 3941+9, 0, 'Dreamfoil - Azshara - Lake Mennar Pool 1', 0, 0, 10), (6315+10, 3941+9, 0, 'Dreamfoil - Azshara - Lake Mennar Pool 1', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Lake Mennar Pool 2
 INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `patch_min`) VALUES (6315+11, 176584, 1, 2908.85, -5531.92, 147.083, -1.09956, 0, 0, -0.522498, 0.85264, 300, 300, 100, 1, 0), (6315+12, 176584, 1, 2760.86, -5590.14, 126.904, 5.74214, 0, 0, -0.267238, 0.963631, 300, 300, 100, 1, 0), (6315+13, 176584, 1, 2521.02, -5468.87, 121.34, 6.10865, 0, 0, -0.0871553, 0.996195, 300, 300, 100, 1, 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (48419, 48617);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3941+10, 1, 'Azshara - Lake Mennar Pool 2 - Dreamfoil', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (48419, 3941+10, 0, 'Dreamfoil - Azshara - Lake Mennar Pool 2', 0, 0, 10), (48617, 3941+10, 0, 'Dreamfoil - Azshara - Lake Mennar Pool 2', 0, 0, 10), (6315+11, 3941+10, 0, 'Dreamfoil - Azshara - Lake Mennar Pool 2', 0, 0, 10), (6315+12, 3941+10, 0, 'Dreamfoil - Azshara - Lake Mennar Pool 2', 0, 0, 10), (6315+13, 3941+10, 0, 'Dreamfoil - Azshara - Lake Mennar Pool 2', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- South Azshara Pool 1
 INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `patch_min`) VALUES (6315+14, 176584, 1, 2519.15, -6037.05, 100.571, 1.85005, 0, 0, 0.798635, 0.601815, 300, 300, 100, 1, 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (68513, 48420, 18314, 18405, 18309);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3941+11, 1, 'Azshara - South Azshara Pool 1 - Dreamfoil', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (68513, 3941+11, 0, 'Dreamfoil - Azshara - South Azshara Pool 1', 0, 0, 10), (48420, 3941+11, 0, 'Dreamfoil - Azshara - South Azshara Pool 1', 0, 0, 10), (18314, 3941+11, 0, 'Dreamfoil - Azshara - South Azshara Pool 1', 0, 0, 10), (18405, 3941+11, 0, 'Dreamfoil - Azshara - South Azshara Pool 1', 0, 0, 10), (18309, 3941+11, 0, 'Dreamfoil - Azshara - South Azshara Pool 1', 0, 0, 10), (6315+14, 3941+11, 0, 'Dreamfoil - Azshara - South Azshara Pool 1', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- South Azshara Pool 2
 INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `patch_min`) VALUES (6315+15, 176584, 1, 2402.95, -6139.04, 98.8376, -2.9845, 0, 0, -0.996917, 0.0784664, 300, 300, 100, 1, 0), (6315+16, 176584, 1, 2458.87, -6244.39, 100.365, 3.08918, 0, 0, 0.999657, 0.0262016, 300, 300, 100, 1, 0), (6315+17, 176584, 1, 2733.36, -6213.72, 101.873, 5.98648, 0, 0, -0.147809, 0.989016, 300, 300, 100, 1, 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (18312, 48426);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3941+12, 1, 'Azshara - South Azshara Pool 2 - Dreamfoil', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (18312, 3941+12, 0, 'Dreamfoil - Azshara - South Azshara Pool 2', 0, 0, 10), (48426, 3941+12, 0, 'Dreamfoil - Azshara - South Azshara Pool 2', 0, 0, 10), (6315+15, 3941+12, 0, 'Dreamfoil - Azshara - South Azshara Pool 2', 0, 0, 10), (6315+16, 3941+12, 0, 'Dreamfoil - Azshara - South Azshara Pool 2', 0, 0, 10), (6315+17, 3941+12, 0, 'Dreamfoil - Azshara - South Azshara Pool 2', 0, 0, 10);

DELETE FROM `pool_template` WHERE `entry` =1242 AND `patch_min` =0 AND `patch_max` =10;

-- End of migration.
;
