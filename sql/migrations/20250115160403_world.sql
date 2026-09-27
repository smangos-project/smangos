INSERT INTO `migrations` VALUES ('20250115160403');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (9903, 3, 'Dustwallow Marsh - Gold Vein / Mithril Deposit / Truesilver Deposit (Master Pool)', 0, 0, 0, 10);

DELETE FROM `pool_gameobject` WHERE `guid` IN (55949);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+1, 1734, 1, -2814.9, -2589.18, 33.667, 2.02458, 0, 0, 0.848047, 0.529921, 300, 300, 100, 1, 0, 0, 0, 10), (90600+2, 2047, 1, -2814.9, -2589.18, 33.667, 2.02458, 0, 0, 0.848047, 0.529921, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+1, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (55949, 6000+1, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+1, 6000+1, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+2, 6000+1, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+1, 9903, 0, 'Dustwallow Marsh - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (56059);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+3, 1734, 1, -4378.36, -2958.63, 27.1648, 3.38594, 0, 0, -0.992546, 0.12187, 300, 300, 100, 1, 0, 0, 0, 10), (90600+4, 2047, 1, -4378.36, -2958.63, 27.1648, 3.38594, 0, 0, -0.992546, 0.12187, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+2, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (56059, 6000+2, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+3, 6000+2, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+4, 6000+2, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+2, 9903, 0, 'Dustwallow Marsh - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (56061);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+5, 1734, 1, -4678.96, -4025.91, 50.7891, 2.51327, 0, 0, 0.951056, 0.309017, 300, 300, 100, 1, 0, 0, 0, 10), (90600+6, 2047, 1, -4678.96, -4025.91, 50.7891, 2.51327, 0, 0, 0.951056, 0.309017, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+3, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (56061, 6000+3, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+5, 6000+3, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+6, 6000+3, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+3, 9903, 0, 'Dustwallow Marsh - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (56063);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+7, 1734, 1, -4399.6, -3014.34, 34.0548, 0.802851, 0, 0, 0.390731, 0.920505, 300, 300, 100, 1, 0, 0, 0, 10), (90600+8, 2047, 1, -4399.6, -3014.34, 34.0548, 0.802851, 0, 0, 0.390731, 0.920505, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+4, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (56063, 6000+4, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+7, 6000+4, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+8, 6000+4, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+4, 9903, 0, 'Dustwallow Marsh - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220824);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+9, 1734, 1, -5025.09, -3735.54, 52.0642, 5.044, 0, 0, -0.580703, 0.814116, 300, 300, 100, 1, 0, 0, 0, 10), (90600+10, 2047, 1, -5025.09, -3735.54, 52.0642, 5.044, 0, 0, -0.580703, 0.814116, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+5, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220824, 6000+5, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+9, 6000+5, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+10, 6000+5, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+5, 9903, 0, 'Dustwallow Marsh - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220825);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+11, 1734, 1, -2784.04, -2644.28, 32.2065, 2.53072, 0, 0, 0.953716, 0.300708, 300, 300, 100, 1, 0, 0, 0, 10), (90600+12, 2047, 1, -2784.04, -2644.28, 32.2065, 2.53072, 0, 0, 0.953716, 0.300708, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+6, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220825, 6000+6, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+11, 6000+6, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+12, 6000+6, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+6, 9903, 0, 'Dustwallow Marsh - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220826);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+13, 1734, 1, -2742.7, -2574.46, 33.3991, 0.226892, 0, 0, 0.113203, 0.993572, 300, 300, 100, 1, 0, 0, 0, 10), (90600+14, 2047, 1, -2742.7, -2574.46, 33.3991, 0.226892, 0, 0, 0.113203, 0.993572, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+7, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220826, 6000+7, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+13, 6000+7, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+14, 6000+7, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+7, 9903, 0, 'Dustwallow Marsh - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220827);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+15, 1734, 1, -4470.82, -2971.23, 30.2181, 5.39307, 0, 0, -0.430511, 0.902586, 300, 300, 100, 1, 0, 0, 0, 10), (90600+16, 2047, 1, -4470.82, -2971.23, 30.2181, 5.39307, 0, 0, -0.430511, 0.902586, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+8, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220827, 6000+8, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+15, 6000+8, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+16, 6000+8, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+8, 9903, 0, 'Dustwallow Marsh - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427056);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+17, 2040, 1, -4434.57, -2908.51, 27.2165, 4.03171, 0, 0, -0.902585, 0.430512, 300, 300, 100, 1, 0, 0, 0, 10), (90600+18, 2047, 1, -4434.57, -2908.51, 27.2165, 4.03171, 0, 0, -0.902585, 0.430512, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+9, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (427056, 6000+9, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+17, 6000+9, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+18, 6000+9, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+9, 9903, 0, 'Dustwallow Marsh - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (2222, 220840);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+19, 1734, 1, -4464.97, -2882.58, 22.3767, 3.71755, 0, 0, -0.958819, 0.284016, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+10, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220840, 6000+10, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (2222, 6000+10, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+19, 6000+10, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+10, 9903, 0, 'Dustwallow Marsh - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

-- Pool Silver Vein / Gold Vein / Iron Deposit Spawns in Dustwallow Marsh.
 INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (9904, 9, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit (Master Pool)', 0, 0, 0, 10);

DELETE FROM `pool_gameobject` WHERE `guid` IN (56027, 40773);

DELETE FROM `pool_template` WHERE `entry` IN (2872);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+20, 1733, 1, -4337.58, -2598.09, 41.2869, 5.39307, 0, 0, -0.430511, 0.902586, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+11, 1, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (56027, 6000+11, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (40773, 6000+11, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+20, 6000+11, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+11, 9904, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (40775, 13674);

DELETE FROM `pool_template` WHERE `entry` IN (2873);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+21, 1733, 1, -3279.71, -3840.21, 52.5267, 2.51327, 0, 0, 0.951056, 0.309017, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+12, 1, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (40775, 6000+12, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (13674, 6000+12, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+21, 6000+12, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+12, 9904, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (40774, 55969);

DELETE FROM `pool_template` WHERE `entry` IN (2874);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+22, 1733, 1, -2694.8, -4296.36, 8.4852, 2.46091, 0, 0, 0.942641, 0.333808, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+13, 1, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (40774, 6000+13, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (55969, 6000+13, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+22, 6000+13, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+13, 9904, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (40776, 427162);

DELETE FROM `pool_template` WHERE `entry` IN (2875);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+23, 1733, 1, -2525.41, -3510.83, 50.4315, 1.72787, 0, 0, 0.760406, 0.649449, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+14, 1, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (40776, 6000+14, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (427162, 6000+14, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+23, 6000+14, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+14, 9904, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (11761);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+24, 1733, 1, -2273.89, -4297.95, 9.49099, 1.36136, 0, 0, 0.62932, 0.777146, 300, 300, 100, 1, 0, 0, 0, 10), (90600+25, 1734, 1, -2273.89, -4297.95, 9.49099, 1.36136, 0, 0, 0.62932, 0.777146, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+15, 1, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (11761, 6000+15, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+24, 6000+15, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+25, 6000+15, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+15, 9904, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (12108);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+26, 1733, 1, -2327.41, -4118.17, 16.9944, 5.68977, 0, 0, -0.292372, 0.956305, 300, 300, 100, 1, 0, 0, 0, 10), (90600+27, 1734, 1, -2327.41, -4118.17, 16.9944, 5.68977, 0, 0, -0.292372, 0.956305, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+16, 1, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (12108, 6000+16, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+26, 6000+16, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+27, 6000+16, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+16, 9904, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (13669);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+28, 1733, 1, -3408.88, -3554.15, 15.8101, 0.296706, 0, 0, 0.147809, 0.989016, 300, 300, 100, 1, 0, 0, 0, 10), (90600+29, 1734, 1, -3408.88, -3554.15, 15.8101, 0.296706, 0, 0, 0.147809, 0.989016, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+17, 1, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (13669, 6000+17, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+28, 6000+17, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+29, 6000+17, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+17, 9904, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (13673);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+30, 1733, 1, -3240.65, -3501.07, 19.8705, 4.41568, 0, 0, -0.803857, 0.594823, 300, 300, 100, 1, 0, 0, 0, 10), (90600+31, 1734, 1, -3240.65, -3501.07, 19.8705, 4.41568, 0, 0, -0.803857, 0.594823, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+18, 1, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (13673, 6000+18, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+30, 6000+18, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+31, 6000+18, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+18, 9904, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (13675);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+32, 1733, 1, -3661.02, -3836.26, 6.66474, 3.87463, 0, 0, -0.93358, 0.358368, 300, 300, 100, 1, 0, 0, 0, 10), (90600+33, 1734, 1, -3661.02, -3836.26, 6.66474, 3.87463, 0, 0, -0.93358, 0.358368, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+19, 1, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (13675, 6000+19, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+32, 6000+19, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+33, 6000+19, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+19, 9904, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (14611);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+34, 1733, 1, -2816.17, -2718.79, 37.3436, 1.90241, 0, 0, 0.814116, 0.580703, 300, 300, 100, 1, 0, 0, 0, 10), (90600+35, 1734, 1, -2816.17, -2718.79, 37.3436, 1.90241, 0, 0, 0.814116, 0.580703, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+20, 1, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (14611, 6000+20, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+34, 6000+20, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+35, 6000+20, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+20, 9904, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (14956);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+36, 1733, 1, -2989.55, -3071.86, 41.602, 0.733038, 0, 0, 0.358368, 0.93358, 300, 300, 100, 1, 0, 0, 0, 10), (90600+37, 1734, 1, -2989.55, -3071.86, 41.602, 0.733038, 0, 0, 0.358368, 0.93358, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+21, 1, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (14956, 6000+21, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+36, 6000+21, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+37, 6000+21, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+21, 9904, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (55938);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+38, 1733, 1, -2972.48, -3509.33, 63.2665, 4.5204, 0, 0, -0.771625, 0.636078, 300, 300, 100, 1, 0, 0, 0, 10), (90600+39, 1734, 1, -2972.48, -3509.33, 63.2665, 4.5204, 0, 0, -0.771625, 0.636078, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+22, 1, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (55938, 6000+22, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+38, 6000+22, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+39, 6000+22, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+22, 9904, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (55940);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+40, 1733, 1, -3192.54, -3406.31, 14.8458, 5.21854, 0, 0, -0.507538, 0.861629, 300, 300, 100, 1, 0, 0, 0, 10), (90600+41, 1734, 1, -3192.54, -3406.31, 14.8458, 5.21854, 0, 0, -0.507538, 0.861629, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+23, 1, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (55940, 6000+23, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+40, 6000+23, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+41, 6000+23, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+23, 9904, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (55954);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+42, 1733, 1, -2467.5, -2932.2, 43.5339, 5.72468, 0, 0, -0.275637, 0.961262, 300, 300, 100, 1, 0, 0, 0, 10), (90600+43, 1734, 1, -2467.5, -2932.2, 43.5339, 5.72468, 0, 0, -0.275637, 0.961262, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+24, 1, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (55954, 6000+24, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+42, 6000+24, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+43, 6000+24, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+24, 9904, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (55962);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+44, 1733, 1, -2666.44, -4233.33, 13.1624, 3.38594, 0, 0, -0.992546, 0.12187, 300, 300, 100, 1, 0, 0, 0, 10), (90600+45, 1734, 1, -2666.44, -4233.33, 13.1624, 3.38594, 0, 0, -0.992546, 0.12187, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+25, 1, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (55962, 6000+25, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+44, 6000+25, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+45, 6000+25, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+25, 9904, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (55963);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+46, 1733, 1, -2819.24, -4116.35, 45.4726, 4.17134, 0, 0, -0.870356, 0.492424, 300, 300, 100, 1, 0, 0, 0, 10), (90600+47, 1734, 1, -2819.24, -4116.35, 45.4726, 4.17134, 0, 0, -0.870356, 0.492424, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+26, 1, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (55963, 6000+26, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+46, 6000+26, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+47, 6000+26, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+26, 9904, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (55966);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+48, 1733, 1, -2715.27, -3905.9, 66.2966, 4.66003, 0, 0, -0.725374, 0.688355, 300, 300, 100, 1, 0, 0, 0, 10), (90600+49, 1734, 1, -2715.27, -3905.9, 66.2966, 4.66003, 0, 0, -0.725374, 0.688355, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+27, 1, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (55966, 6000+27, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+48, 6000+27, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+49, 6000+27, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+27, 9904, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (55970);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+50, 1733, 1, -2557.49, -3790, 13.52, 0.750491, 0, 0, 0.366501, 0.930418, 300, 300, 100, 1, 0, 0, 0, 10), (90600+51, 1734, 1, -2557.49, -3790, 13.52, 0.750491, 0, 0, 0.366501, 0.930418, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+28, 1, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (55970, 6000+28, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+50, 6000+28, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+51, 6000+28, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+28, 9904, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (56021);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+52, 1733, 1, -3609.48, -3458.61, 46.5524, 2.51327, 0, 0, 0.951056, 0.309017, 300, 300, 100, 1, 0, 0, 0, 10), (90600+53, 1734, 1, -3609.48, -3458.61, 46.5524, 2.51327, 0, 0, 0.951056, 0.309017, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+29, 1, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (56021, 6000+29, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+52, 6000+29, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+53, 6000+29, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+29, 9904, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (56026);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+54, 1733, 1, -4344.76, -2621.94, 37.7756, 2.35619, 0, 0, 0.92388, 0.382683, 300, 300, 100, 1, 0, 0, 0, 10), (90600+55, 1734, 1, -4344.76, -2621.94, 37.7756, 2.35619, 0, 0, 0.92388, 0.382683, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+30, 1, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (56026, 6000+30, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+54, 6000+30, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+55, 6000+30, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+30, 9904, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (56032);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+56, 1733, 1, -3782.45, -2723.36, 63.6209, 4.53786, 0, 0, -0.766044, 0.642789, 300, 300, 100, 1, 0, 0, 0, 10), (90600+57, 1734, 1, -3782.45, -2723.36, 63.6209, 4.53786, 0, 0, -0.766044, 0.642789, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+31, 1, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (56032, 6000+31, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+56, 6000+31, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+57, 6000+31, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+31, 9904, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (389471);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+58, 1733, 1, -3456.27, -3365.72, 52.7804, 0.191985, 0, 0, 0.0958452, 0.995396, 300, 300, 100, 1, 0, 0, 0, 10), (90600+59, 1734, 1, -3456.27, -3365.72, 52.7804, 0.191985, 0, 0, 0.0958452, 0.995396, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+32, 1, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (389471, 6000+32, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+58, 6000+32, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+59, 6000+32, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+32, 9904, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (389472);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+60, 1733, 1, -3051.64, -3319.91, 52.5062, 2.09439, 0, 0, 0.866025, 0.500001, 300, 300, 100, 1, 0, 0, 0, 10), (90600+61, 1734, 1, -3051.64, -3319.91, 52.5062, 2.09439, 0, 0, 0.866025, 0.500001, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+33, 1, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (389472, 6000+33, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+60, 6000+33, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+61, 6000+33, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+33, 9904, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427136);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+62, 1733, 1, -4332.39, -2608.17, 40.8422, 5.89921, 0, 0, -0.190808, 0.981627, 300, 300, 100, 1, 0, 0, 0, 10), (90600+63, 1734, 1, -4332.39, -2608.17, 40.8422, 5.89921, 0, 0, -0.190808, 0.981627, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+34, 1, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (427136, 6000+34, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+62, 6000+34, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+63, 6000+34, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+34, 9904, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427140);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+64, 1733, 1, -2587.26, -3420.52, 51.0784, 3.26377, 0, 0, -0.998135, 0.0610518, 300, 300, 100, 1, 0, 0, 0, 10), (90600+65, 1734, 1, -2587.26, -3420.52, 51.0784, 3.26377, 0, 0, -0.998135, 0.0610518, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+35, 1, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (427140, 6000+35, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+64, 6000+35, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10), (90600+65, 6000+35, 0, 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+35, 9904, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_template` WHERE `entry` IN (43520, 1335, 43524, 9005);

-- Pool Tin Vein / Silver Vein Spawns in Dustwallow Marsh.
 INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (9905, 6, 'Dustwallow Marsh - Tin Vein / Silver Vein (Master Pool)', 0, 0, 0, 10);

DELETE FROM `pool_gameobject` WHERE `guid` IN (55955);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+66, 1733, 1, -2512.52, -2842.59, 63.2461, 2.61799, 0, 0, 0.965925, 0.258821, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+36, 1, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (55955, 6000+36, 0, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 10), (90600+66, 6000+36, 0, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+36, 9905, 0, 'Dustwallow Marsh - Tin Vein / Silver Vein', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (56024);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+67, 1733, 1, -4203.84, -3725.7, 56.8925, 6.02139, 0, 0, -0.130526, 0.991445, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+37, 1, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (56024, 6000+37, 0, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 10), (90600+67, 6000+37, 0, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+37, 9905, 0, 'Dustwallow Marsh - Tin Vein / Silver Vein', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (55957);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+68, 1733, 1, -2356.37, -3166.03, 49.9149, 4.41568, 0, 0, -0.803857, 0.594823, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+38, 1, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (55957, 6000+38, 0, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 10), (90600+68, 6000+38, 0, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+38, 9905, 0, 'Dustwallow Marsh - Tin Vein / Silver Vein', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (55960);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+69, 1733, 1, -2748.49, -4020.92, 43.7121, 3.64774, 0, 0, -0.968147, 0.250381, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+39, 1, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (55960, 6000+39, 0, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 10), (90600+69, 6000+39, 0, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+39, 9905, 0, 'Dustwallow Marsh - Tin Vein / Silver Vein', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (56025);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+70, 1733, 1, -3770.91, -3715.19, 48.8534, 3.12412, 0, 0, 0.999962, 0.00873464, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+40, 1, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (56025, 6000+40, 0, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 10), (90600+70, 6000+40, 0, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+40, 9905, 0, 'Dustwallow Marsh - Tin Vein / Silver Vein', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (43247);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+71, 1733, 1, -2632.81, -3647.33, 41.4555, 1.25664, 0, 0, 0.587785, 0.809017, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+41, 1, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (43247, 6000+41, 0, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 10), (90600+71, 6000+41, 0, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+41, 9905, 0, 'Dustwallow Marsh - Tin Vein / Silver Vein', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (43246);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+72, 1733, 1, -3876.55, -3889.24, 65.1546, 0.0523589, 0, 0, 0.0261765, 0.999657, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+42, 1, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (43246, 6000+42, 0, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 10), (90600+72, 6000+42, 0, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+42, 9905, 0, 'Dustwallow Marsh - Tin Vein / Silver Vein', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (251533);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+73, 1733, 1, -4117.82, -3831.94, 56.7862, 3.08918, 0, 0, 0.999657, 0.0262016, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+43, 1, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (251533, 6000+43, 0, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 10), (90600+73, 6000+43, 0, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+43, 9905, 0, 'Dustwallow Marsh - Tin Vein / Silver Vein', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (251534);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+74, 1733, 1, -3608.11, -3567.09, 44.7214, 0.506145, 0, 0, 0.25038, 0.968148, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+44, 1, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (251534, 6000+44, 0, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 10), (90600+74, 6000+44, 0, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+44, 9905, 0, 'Dustwallow Marsh - Tin Vein / Silver Vein', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (251535);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+75, 1733, 1, -2737.97, -2789.71, 50.6377, 4.81711, 0, 0, -0.66913, 0.743145, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+45, 1, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (251535, 6000+45, 0, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 10), (90600+75, 6000+45, 0, 'Tin Vein / Silver Vein - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+45, 9905, 0, 'Dustwallow Marsh - Tin Vein / Silver Vein', 0);

DELETE FROM `pool_template` WHERE `entry` IN (43521);

-- Pool Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit Spawns in Dustwallow Marsh.
 INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (9906, 2, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit (Master Pool)', 0, 0, 0, 10);

DELETE FROM `pool_gameobject` WHERE `guid` IN (14612);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+76, 1733, 1, -2788.74, -2610.23, 37.3722, 2.94961, 0, 0, 0.995396, 0.095846, 300, 300, 100, 1, 0, 0, 0, 10), (90600+77, 1734, 1, -2788.74, -2610.23, 37.3722, 2.94961, 0, 0, 0.995396, 0.095846, 300, 300, 100, 1, 0, 0, 0, 10), (90600+78, 2040, 1, -2788.74, -2610.23, 37.3722, 2.94961, 0, 0, 0.995396, 0.095846, 300, 300, 100, 1, 0, 0, 0, 10), (90600+79, 2047, 1, -2788.74, -2610.23, 37.3722, 2.94961, 0, 0, 0.995396, 0.095846, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+46, 1, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (14612, 6000+46, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+76, 6000+46, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+77, 6000+46, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+78, 6000+46, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+79, 6000+46, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+46, 9906, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (55948);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+80, 1733, 1, -2849.43, -2682.88, 33.2633, 5.88176, 0, 0, -0.199367, 0.979925, 300, 300, 100, 1, 0, 0, 0, 10), (90600+81, 1734, 1, -2849.43, -2682.88, 33.2633, 5.88176, 0, 0, -0.199367, 0.979925, 300, 300, 100, 1, 0, 0, 0, 10), (90600+82, 2040, 1, -2849.43, -2682.88, 33.2633, 5.88176, 0, 0, -0.199367, 0.979925, 300, 300, 100, 1, 0, 0, 0, 10), (90600+83, 2047, 1, -2849.43, -2682.88, 33.2633, 5.88176, 0, 0, -0.199367, 0.979925, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+47, 1, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (55948, 6000+47, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+80, 6000+47, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+81, 6000+47, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+82, 6000+47, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+83, 6000+47, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+47, 9906, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (55950);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+84, 1733, 1, -2823.51, -2610.95, 37.3061, 6.21337, 0, 0, -0.0348988, 0.999391, 300, 300, 100, 1, 0, 0, 0, 10), (90600+85, 1734, 1, -2823.51, -2610.95, 37.3061, 6.21337, 0, 0, -0.0348988, 0.999391, 300, 300, 100, 1, 0, 0, 0, 10), (90600+86, 2040, 1, -2823.51, -2610.95, 37.3061, 6.21337, 0, 0, -0.0348988, 0.999391, 300, 300, 100, 1, 0, 0, 0, 10), (90600+87, 2047, 1, -2823.51, -2610.95, 37.3061, 6.21337, 0, 0, -0.0348988, 0.999391, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+48, 1, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (55950, 6000+48, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+84, 6000+48, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+85, 6000+48, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+86, 6000+48, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+87, 6000+48, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+48, 9906, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (56058);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+88, 1733, 1, -4427.89, -2987.49, 30.9241, 4.46804, 0, 0, -0.788011, 0.615662, 300, 300, 100, 1, 0, 0, 0, 10), (90600+89, 1734, 1, -4427.89, -2987.49, 30.9241, 4.46804, 0, 0, -0.788011, 0.615662, 300, 300, 100, 1, 0, 0, 0, 10), (90600+90, 2040, 1, -4427.89, -2987.49, 30.9241, 4.46804, 0, 0, -0.788011, 0.615662, 300, 300, 100, 1, 0, 0, 0, 10), (90600+91, 2047, 1, -4427.89, -2987.49, 30.9241, 4.46804, 0, 0, -0.788011, 0.615662, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+49, 1, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (56058, 6000+49, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+88, 6000+49, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+89, 6000+49, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+90, 6000+49, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+91, 6000+49, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+49, 9906, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427135);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+92, 1733, 1, -4420.88, -2947.3, 26.1393, 5.25344, 0, 0, -0.492423, 0.870356, 300, 300, 100, 1, 0, 0, 0, 10), (90600+93, 1734, 1, -4420.88, -2947.3, 26.1393, 5.25344, 0, 0, -0.492423, 0.870356, 300, 300, 100, 1, 0, 0, 0, 10), (90600+94, 2040, 1, -4420.88, -2947.3, 26.1393, 5.25344, 0, 0, -0.492423, 0.870356, 300, 300, 100, 1, 0, 0, 0, 10), (90600+95, 2047, 1, -4420.88, -2947.3, 26.1393, 5.25344, 0, 0, -0.492423, 0.870356, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+50, 1, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (427135, 6000+50, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+92, 6000+50, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+93, 6000+50, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+94, 6000+50, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+95, 6000+50, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+50, 9906, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427137);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+96, 1733, 1, -2793.1, -2622.57, 33.5015, 5.25344, 0, 0, -0.492423, 0.870356, 300, 300, 100, 1, 0, 0, 0, 10), (90600+97, 1734, 1, -2793.1, -2622.57, 33.5015, 5.25344, 0, 0, -0.492423, 0.870356, 300, 300, 100, 1, 0, 0, 0, 10), (90600+98, 2040, 1, -2793.1, -2622.57, 33.5015, 5.25344, 0, 0, -0.492423, 0.870356, 300, 300, 100, 1, 0, 0, 0, 10), (90600+99, 2047, 1, -2793.1, -2622.57, 33.5015, 5.25344, 0, 0, -0.492423, 0.870356, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+51, 1, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (427137, 6000+51, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+96, 6000+51, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+97, 6000+51, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+98, 6000+51, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+99, 6000+51, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+51, 9906, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427138);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+100, 1733, 1, -2791.36, -2560.33, 29.9303, 5.75959, 0, 0, -0.258819, 0.965926, 300, 300, 100, 1, 0, 0, 0, 10), (90600+101, 1734, 1, -2791.36, -2560.33, 29.9303, 5.75959, 0, 0, -0.258819, 0.965926, 300, 300, 100, 1, 0, 0, 0, 10), (90600+102, 2040, 1, -2791.36, -2560.33, 29.9303, 5.75959, 0, 0, -0.258819, 0.965926, 300, 300, 100, 1, 0, 0, 0, 10), (90600+103, 2047, 1, -2791.36, -2560.33, 29.9303, 5.75959, 0, 0, -0.258819, 0.965926, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+52, 1, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (427138, 6000+52, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+100, 6000+52, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+101, 6000+52, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+102, 6000+52, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+103, 6000+52, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+52, 9906, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427139);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+104, 1733, 1, -2747.91, -2619.6, 29.4423, 1.58825, 0, 0, 0.71325, 0.70091, 300, 300, 100, 1, 0, 0, 0, 10), (90600+105, 1734, 1, -2747.91, -2619.6, 29.4423, 1.58825, 0, 0, 0.71325, 0.70091, 300, 300, 100, 1, 0, 0, 0, 10), (90600+106, 2040, 1, -2747.91, -2619.6, 29.4423, 1.58825, 0, 0, 0.71325, 0.70091, 300, 300, 100, 1, 0, 0, 0, 10), (90600+107, 2047, 1, -2747.91, -2619.6, 29.4423, 1.58825, 0, 0, 0.71325, 0.70091, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+53, 1, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (427139, 6000+53, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+104, 6000+53, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+105, 6000+53, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+106, 6000+53, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+107, 6000+53, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+53, 9906, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (56057);

DELETE FROM `pool_template` WHERE `entry` = 2871;

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+108, 1733, 1, -4381.84, -3001.28, 31.054, 3.194, 0, 0, -0.999657, 0.0262017, 300, 300, 100, 1, 0, 0, 0, 10), (90600+109, 1734, 1, -4381.84, -3001.28, 31.054, 3.194, 0, 0, -0.999657, 0.0262017, 300, 300, 100, 1, 0, 0, 0, 10), (90600+110, 2040, 1, -4381.84, -3001.28, 31.054, 3.194, 0, 0, -0.999657, 0.0262017, 300, 300, 100, 1, 0, 0, 0, 10), (90600+111, 2047, 1, -4381.84, -3001.28, 31.054, 3.194, 0, 0, -0.999657, 0.0262017, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+54, 1, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (56057, 6000+54, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+108, 6000+54, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+109, 6000+54, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+110, 6000+54, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+111, 6000+54, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+54, 9906, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220828);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+112, 1733, 1, -4381.84, -2918.38, 26.5274, 3.9619, 0, 0, -0.91706, 0.39875, 300, 300, 100, 1, 0, 0, 0, 10), (90600+113, 1734, 1, -4381.84, -2918.38, 26.5274, 3.9619, 0, 0, -0.91706, 0.39875, 300, 300, 100, 1, 0, 0, 0, 10), (90600+114, 1735, 1, -4381.84, -2918.38, 26.5274, 3.9619, 0, 0, -0.91706, 0.39875, 300, 300, 100, 1, 0, 0, 0, 10), (90600+115, 2047, 1, -4381.84, -2918.38, 26.5274, 3.9619, 0, 0, -0.91706, 0.39875, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+55, 1, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220828, 6000+55, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+112, 6000+55, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+113, 6000+55, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+114, 6000+55, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+115, 6000+55, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+55, 9906, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (328, 220415);

DELETE FROM `pool_template` WHERE `entry` = 2223;

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+116, 1733, 1, -4346.2, -3005.47, 35.1079, 1.11701, 0, 0, 0.529919, 0.848048, 300, 300, 100, 1, 0, 0, 0, 10), (90600+117, 1734, 1, -4346.2, -3005.47, 35.1079, 1.11701, 0, 0, 0.529919, 0.848048, 300, 300, 100, 1, 0, 0, 0, 10), (90600+118, 2047, 1, -4346.2, -3005.47, 35.1079, 1.11701, 0, 0, 0.529919, 0.848048, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+56, 1, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (328, 6000+56, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (220415, 6000+56, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+116, 6000+56, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+117, 6000+56, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+118, 6000+56, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+56, 9906, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (56033, 220414);

DELETE FROM `pool_template` WHERE `entry` = 23046;

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+119, 1733, 1, -4566.88, -2734.63, 45.9512, -1.55334, 0, 0, 0.700909, -0.71325, 300, 300, 100, 1, 0, 0, 0, 10), (90600+120, 1734, 1, -4566.88, -2734.63, 45.9512, -1.55334, 0, 0, 0.700909, -0.71325, 300, 300, 100, 1, 0, 0, 0, 10), (90600+121, 2047, 1, -4566.88, -2734.63, 45.9512, -1.55334, 0, 0, 0.700909, -0.71325, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+57, 1, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (56033, 6000+57, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (220414, 6000+57, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+119, 6000+57, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+120, 6000+57, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+121, 6000+57, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+57, 9906, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (2223, 220419);

DELETE FROM `pool_template` WHERE `entry` = 9006;

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (90600+122, 1733, 1, -2766.39, -2606.88, 34.2974, 5.41052, 0, 0, -0.422618, 0.906308, 300, 300, 100, 1, 0, 0, 0, 10), (90600+123, 1734, 1, -2766.39, -2606.88, 34.2974, 5.41052, 0, 0, -0.422618, 0.906308, 300, 300, 100, 1, 0, 0, 0, 10), (90600+124, 1735, 1, -2766.39, -2606.88, 34.2974, 5.41052, 0, 0, -0.422618, 0.906308, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (6000+58, 1, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (2223, 6000+58, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (220419, 6000+58, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+122, 6000+58, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+123, 6000+58, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10), (90600+124, 6000+58, 0, 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6000+58, 9906, 0, 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` = 157;

DELETE FROM `gameobject` WHERE `guid` = 157;

DELETE FROM `pool_template` WHERE `entry` = 323;

UPDATE `gameobject` SET `id` = 1734 WHERE `guid` = 22035;

-- Add Chance
 -- Gold Vein / Mithril Deposit / Truesilver Deposit
 UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1734);

UPDATE `pool_gameobject` SET `chance` = 90 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 2040);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 2047);

-- Silver Vein / Gold Vein / Iron Deposit
 UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1733);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1734);

UPDATE `pool_gameobject` SET `chance` = 90 WHERE `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1735);

-- Tin Vein / Silver Vein
 UPDATE `pool_gameobject` SET `chance` = 95 WHERE `description` = 'Tin Vein / Silver Vein - Dustwallow Marsh' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1732);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Tin Vein / Silver Vein - Dustwallow Marsh' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1733);

-- Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit
 UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1733);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1734);

UPDATE `pool_gameobject` SET `chance` = 40 WHERE `description` = 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1735);

UPDATE `pool_gameobject` SET `chance` = 45 WHERE `description` = 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 2040);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Silver Vein / Gold Vein / Iron Deposit / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 2047);

-- End of migration.
;
