INSERT INTO `migrations` VALUES ('20250120195612');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (9910, 15, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit (Master Pool)', 0, 0, 0, 10);

DELETE FROM `pool_gameobject` WHERE `guid` IN (11679);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+1, 1734, 0, -13852, 457.751, 96.7813, 0.767945, 0, 0, 0.374607, 0.927184, 300, 300, 100, 1, 0, 0, 0, 10), (74200+2, 2040, 0, -13852, 457.751, 96.7813, 0.767945, 0, 0, 0.374607, 0.927184, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+1, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (11679, 5620+1, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+1, 5620+1, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+2, 5620+1, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+1, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (43802, 7306);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2900);

DELETE FROM `pool_template` WHERE `entry` IN (2900);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+3, 2047, 0, -13796.6, 610.099, 43.7884, 1.0472, 0, 0, 0.5, 0.866025, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+2, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (43802, 5620+2, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (7306, 5620+2, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+3, 5620+2, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+2, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220805, 43814);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2894);

DELETE FROM `pool_template` WHERE `entry` IN (2894);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+4, 1734, 0, -13258.2, -211.438, 35.7719, 2.1293, 0, 0, 0.874619, 0.48481, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+3, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220805, 5620+3, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (43814, 5620+3, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+4, 5620+3, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+3, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (34751, 11696);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2895);

DELETE FROM `pool_template` WHERE `entry` IN (2895);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+5, 1734, 0, -13231.2, -540.241, 7.93169, 3.89209, 0, 0, -0.930417, 0.366502, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+4, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (34751, 5620+4, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (11696, 5620+4, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+5, 5620+4, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+4, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220806, 43815);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2896);

DELETE FROM `pool_template` WHERE `entry` IN (2896);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+6, 1734, 0, -13118.3, 164.409, 42.8251, 2.1293, 0, 0, 0.874619, 0.48481, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+5, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220806, 5620+5, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (43815, 5620+5, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+6, 5620+5, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+5, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (7176, 43813);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2897);

DELETE FROM `pool_template` WHERE `entry` IN (2897);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+7, 1734, 0, -12851.3, 268.715, 19.0539, 1.69297, 0, 0, 0.748956, 0.66262, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+6, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (7176, 5620+6, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (43813, 5620+6, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+7, 5620+6, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+6, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (7113);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+8, 1734, 0, -13090.9, -751.573, 85.1796, -2.6529, 0, 0, 0.970296, -0.241922, 300, 300, 100, 1, 0, 0, 0, 10), (74200+9, 2047, 0, -13090.9, -751.573, 85.1796, -2.6529, 0, 0, 0.970296, -0.241922, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+7, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (7113, 5620+7, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+8, 5620+7, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+9, 5620+7, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+7, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (7143);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+10, 1734, 0, -14360.8, 152.343, 14.1746, 5.61996, 0, 0, -0.325567, 0.945519, 300, 300, 100, 1, 0, 0, 0, 10), (74200+11, 2047, 0, -14360.8, 152.343, 14.1746, 5.61996, 0, 0, -0.325567, 0.945519, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+8, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (7143, 5620+8, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+10, 5620+8, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+11, 5620+8, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+8, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (7194);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+12, 1734, 0, -13832, 530.611, 57.572, 2.67, 0, 0, 0.97237, 0.233445, 300, 300, 100, 1, 0, 0, 0, 10), (74200+13, 2047, 0, -13832, 530.611, 57.572, 2.67, 0, 0, 0.97237, 0.233445, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+9, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (7194, 5620+9, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+12, 5620+9, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+13, 5620+9, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+9, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (7221);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+14, 1734, 0, -12932, 316.983, 23.4552, 0.314158, 0, 0, 0.156434, 0.987688, 300, 300, 100, 1, 0, 0, 0, 10), (74200+15, 2047, 0, -12932, 316.983, 23.4552, 0.314158, 0, 0, 0.156434, 0.987688, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+10, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (7221, 5620+10, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+14, 5620+10, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+15, 5620+10, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+10, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (11155);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+16, 1734, 0, -12365.8, -1119.57, 3.82046, -0.122173, 0, 0, 0.061049, -0.998135, 300, 300, 100, 1, 0, 0, 0, 10), (74200+17, 2047, 0, -12365.8, -1119.57, 3.82046, -0.122173, 0, 0, 0.061049, -0.998135, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+11, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (11155, 5620+11, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+16, 5620+11, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+17, 5620+11, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+11, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (11698);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+18, 1734, 0, -13292.1, -224.057, 20.6276, -0.314159, 0, 0, 0.156434, -0.987688, 300, 300, 100, 1, 0, 0, 0, 10), (74200+19, 2047, 0, -13292.1, -224.057, 20.6276, -0.314159, 0, 0, 0.156434, -0.987688, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+12, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (11698, 5620+12, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+18, 5620+12, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+19, 5620+12, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+12, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (12150);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+20, 1734, 0, -14254.5, 5.3367, 7.66996, -1.06465, 0, 0, 0.507538, -0.861629, 300, 300, 100, 1, 0, 0, 0, 10), (74200+21, 2047, 0, -14254.5, 5.3367, 7.66996, -1.06465, 0, 0, 0.507538, -0.861629, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+13, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (12150, 5620+13, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+20, 5620+13, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+21, 5620+13, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+13, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (12163);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+22, 1734, 0, -12948.1, 76.1396, 43.8091, -2.82743, 0, 0, 0.987688, -0.156434, 300, 300, 100, 1, 0, 0, 0, 10), (74200+23, 2047, 0, -12948.1, 76.1396, 43.8091, -2.82743, 0, 0, 0.987688, -0.156434, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+14, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (12163, 5620+14, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+22, 5620+14, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+23, 5620+14, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+14, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (29538);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+24, 1734, 0, -13275.7, -155.402, 21.6621, -2.82743, 0, 0, 0.987688, -0.156434, 300, 300, 100, 1, 0, 0, 0, 10), (74200+25, 2047, 0, -13275.7, -155.402, 21.6621, -2.82743, 0, 0, 0.987688, -0.156434, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+15, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (29538, 5620+15, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+24, 5620+15, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+25, 5620+15, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+15, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (34752);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+26, 1734, 0, -11406.3, -916.361, 19.125, 6.16101, 0, 0, -0.0610485, 0.998135, 300, 300, 100, 1, 0, 0, 0, 10), (74200+27, 2047, 0, -11406.3, -916.361, 19.125, 6.16101, 0, 0, -0.0610485, 0.998135, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+16, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (34752, 5620+16, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+26, 5620+16, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+27, 5620+16, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+16, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (34753);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+28, 1734, 0, -12389.8, -1067.46, 3.13742, 0.418879, 0, 0, 0.207911, 0.978148, 300, 300, 100, 1, 0, 0, 0, 10), (74200+29, 2047, 0, -12389.8, -1067.46, 3.13742, 0.418879, 0, 0, 0.207911, 0.978148, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+17, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (34753, 5620+17, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+28, 5620+17, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+29, 5620+17, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+17, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (34754);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+30, 1734, 0, -12840.5, -610.494, 66.9924, 5.09636, 0, 0, -0.559193, 0.829038, 300, 300, 100, 1, 0, 0, 0, 10), (74200+31, 2047, 0, -12840.5, -610.494, 66.9924, 5.09636, 0, 0, -0.559193, 0.829038, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+18, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (34754, 5620+18, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+30, 5620+18, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+31, 5620+18, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+18, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (34755);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+32, 1734, 0, -14577.6, -241.361, 16.7222, 3.57793, 0, 0, -0.976295, 0.216442, 300, 300, 100, 1, 0, 0, 0, 10), (74200+33, 2047, 0, -14577.6, -241.361, 16.7222, 3.57793, 0, 0, -0.976295, 0.216442, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+19, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (34755, 5620+19, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+32, 5620+19, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+33, 5620+19, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+19, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (34756);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+34, 1734, 0, -14643.3, 481.62, 23.7943, 2.72271, 0, 0, 0.978148, 0.207912, 300, 300, 100, 1, 0, 0, 0, 10), (74200+35, 2047, 0, -14643.3, 481.62, 23.7943, 2.72271, 0, 0, 0.978148, 0.207912, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+20, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (34756, 5620+20, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+34, 5620+20, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+35, 5620+20, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+20, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (40021);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+36, 1734, 0, -11447.4, -845.448, 15.4786, 6.14356, 0, 0, -0.0697556, 0.997564, 300, 300, 100, 1, 0, 0, 0, 10), (74200+37, 2047, 0, -11447.4, -845.448, 15.4786, 6.14356, 0, 0, -0.0697556, 0.997564, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+21, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (40021, 5620+21, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+36, 5620+21, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+37, 5620+21, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+21, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220400);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+38, 1734, 0, -13734.4, 588.942, 28.3662, 0.366518, 0, 0, 0.182235, 0.983255, 300, 300, 100, 1, 0, 0, 0, 10), (74200+39, 2047, 0, -13734.4, 588.942, 28.3662, 0.366518, 0, 0, 0.182235, 0.983255, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+22, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220400, 5620+22, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+38, 5620+22, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+39, 5620+22, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+22, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220401);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+40, 1734, 0, -14641.5, -301.463, 27.0929, 3.83973, 0, 0, -0.939692, 0.34202, 300, 300, 100, 1, 0, 0, 0, 10), (74200+41, 2047, 0, -14641.5, -301.463, 27.0929, 3.83973, 0, 0, -0.939692, 0.34202, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+23, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220401, 5620+23, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+40, 5620+23, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+41, 5620+23, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+23, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220402);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+42, 1734, 0, -14800.5, -344.491, 13.0164, 4.57276, 0, 0, -0.754709, 0.656059, 300, 300, 100, 1, 0, 0, 0, 10), (74200+43, 2047, 0, -14800.5, -344.491, 13.0164, 4.57276, 0, 0, -0.754709, 0.656059, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+24, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220402, 5620+24, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+42, 5620+24, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+43, 5620+24, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+24, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (387417);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+44, 1734, 0, -12788.7, 222.431, 18.9157, 1.23918, 0, 0, 0.580703, 0.814116, 300, 300, 100, 1, 0, 0, 0, 10), (74200+45, 2047, 0, -12788.7, 222.431, 18.9157, 1.23918, 0, 0, 0.580703, 0.814116, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+25, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (387417, 5620+25, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+44, 5620+25, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+45, 5620+25, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+25, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (387418);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+46, 1734, 0, -13249.5, -519.636, 18.0377, 5.70723, 0, 0, -0.284015, 0.95882, 300, 300, 100, 1, 0, 0, 0, 10), (74200+47, 2047, 0, -13249.5, -519.636, 18.0377, 5.70723, 0, 0, -0.284015, 0.95882, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+26, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (387418, 5620+26, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+46, 5620+26, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+47, 5620+26, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+26, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427053);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+48, 2040, 0, -14690.7, -250.13, 18.0443, 0.855211, 0, 0, 0.414693, 0.909961, 300, 300, 100, 1, 0, 0, 0, 10), (74200+49, 2047, 0, -14690.7, -250.13, 18.0443, 0.855211, 0, 0, 0.414693, 0.909961, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+27, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (427053, 5620+27, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+48, 5620+27, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+49, 5620+27, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+27, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_template` WHERE `entry` IN (1178, 1177);

-- Pool Silver Vein / Gold Vein / Iron Deposit Spawns in Stranglethorn Vale.
 INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (9911, 10, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit (Master Pool)', 0, 0, 0, 10);

DELETE FROM `pool_gameobject` WHERE `guid` IN (43257);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+50, 1733, 0, -13801.1, -120.558, 24.5498, 4.92183, 0, 0, -0.62932, 0.777146, 300, 300, 100, 1, 0, 0, 0, 10), (74200+51, 1735, 0, -13801.1, -120.558, 24.5498, 4.92183, 0, 0, -0.62932, 0.777146, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+28, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (43257, 5620+28, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (74200+50, 5620+28, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (74200+51, 5620+28, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+28, 9911, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (6570, 6545);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2885);

DELETE FROM `pool_template` WHERE `entry` IN (2885);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+52, 1733, 0, -11754.6, 223.55, 21.0039, 1.83259, 0, 0, 0.793353, 0.608762, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+29, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (6570, 5620+29, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (6545, 5620+29, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (74200+52, 5620+29, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+29, 9911, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (6571, 389461);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2886);

DELETE FROM `pool_template` WHERE `entry` IN (2886);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+53, 1733, 0, -11681.5, 317.656, 43.8298, 5.86431, 0, 0, -0.207911, 0.978148, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+30, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (6571, 5620+30, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (389461, 5620+30, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (74200+53, 5620+30, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+30, 9911, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (5854, 10619);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2887);

DELETE FROM `pool_template` WHERE `entry` IN (2887);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+54, 1733, 0, -11490.4, 276.76, 22.9154, 4.50295, 0, 0, -0.777145, 0.629321, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+31, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (5854, 5620+31, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (10619, 5620+31, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (74200+54, 5620+31, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+31, 9911, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (6662, 12294);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2888);

DELETE FROM `pool_template` WHERE `entry` IN (2888);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+55, 1733, 0, -11468.3, 378.432, 75.4524, 4.20625, 0, 0, -0.861629, 0.507539, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+32, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (6662, 5620+32, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (12294, 5620+32, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (74200+55, 5620+32, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+32, 9911, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (43805, 43256);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2889);

DELETE FROM `pool_template` WHERE `entry` IN (2889);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+56, 1733, 0, -12741.6, -451.59, 32.6192, 5.3058, 0, 0, -0.469471, 0.882948, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+33, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (43805, 5620+33, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (43256, 5620+33, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (74200+56, 5620+33, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+33, 9911, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (32490, 5959);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2890);

DELETE FROM `pool_template` WHERE `entry` IN (2890);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+57, 1733, 0, -11934, 458.191, 44.5061, 6.19592, 0, 0, -0.0436192, 0.999048, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+34, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (32490, 5620+34, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (5959, 5620+34, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (74200+57, 5620+34, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+34, 9911, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (43804, 32096);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2891);

DELETE FROM `pool_template` WHERE `entry` IN (2891);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+58, 1733, 0, -11919.8, 178.668, 40.417, 3.89209, 0, 0, -0.930417, 0.366502, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+35, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (43804, 5620+35, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (32096, 5620+35, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (74200+58, 5620+35, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+35, 9911, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (12286, 389473);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2892);

DELETE FROM `pool_template` WHERE `entry` IN (2892);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+59, 1733, 0, -13946.2, 220.203, 18.3835, 5.09636, 0, 0, -0.559193, 0.829038, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+36, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (12286, 5620+36, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (389473, 5620+36, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (74200+59, 5620+36, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+36, 9911, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (43258, 6470);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2893);

DELETE FROM `pool_template` WHERE `entry` IN (2893);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+60, 1733, 0, -12906.5, 387.039, 53.7599, 3.6652, 0, 0, -0.965925, 0.258821, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+37, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (43258, 5620+37, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (6470, 5620+37, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (74200+60, 5620+37, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+37, 9911, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` = 2898;

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (2898, 9911, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` = 2899;

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (2899, 9911, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (6550, 427007);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2876);

DELETE FROM `pool_template` WHERE `entry` IN (2876);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+61, 1734, 0, -14099, 318.75, 24.437, 2.47837, 0, 0, 0.945518, 0.325568, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+38, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (6550, 5620+38, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (427007, 5620+38, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (74200+61, 5620+38, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+38, 9911, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427010, 389463);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2877);

DELETE FROM `pool_template` WHERE `entry` IN (2877);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+62, 1734, 0, -13894.2, -1.97103, 35.2379, 1.97222, 0, 0, 0.833885, 0.551938, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+39, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (427010, 5620+39, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (389463, 5620+39, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (74200+62, 5620+39, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+39, 9911, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (31518, 12168);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2878);

DELETE FROM `pool_template` WHERE `entry` IN (2878);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+63, 1734, 0, -11935, 543.475, 33.0384, 2.47837, 0, 0, 0.945518, 0.325568, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+40, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (31518, 5620+40, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (12168, 5620+40, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (74200+63, 5620+40, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+40, 9911, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (31532, 12173);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2879);

DELETE FROM `pool_template` WHERE `entry` IN (2879);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+64, 1734, 0, -11755.1, -547.294, 23.6612, 2.87979, 0, 0, 0.991445, 0.130528, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+41, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (31532, 5620+41, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (12173, 5620+41, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10), (74200+64, 5620+41, 0, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+41, 9911, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_template` WHERE `entry` IN (1176, 1175, 386);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale' WHERE `entry` IN (2899, 2898);

-- More Mithril
 DELETE FROM `pool_gameobject` WHERE `guid` IN (427001);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+65, 1734, 0, -13272, -574.499, 16.0171, 1.95477, 0, 0, 0.829038, 0.559193, 300, 300, 100, 1, 0, 0, 0, 10), (74200+66, 2040, 0, -13272, -574.499, 16.0171, 1.95477, 0, 0, 0.829038, 0.559193, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+42, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (427001, 5620+42, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+65, 5620+42, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+66, 5620+42, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+42, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220801);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+67, 1734, 0, -14588.1, 345.029, 8.2059, 5.41052, 0, 0, -0.422618, 0.906308, 300, 300, 100, 1, 0, 0, 0, 10), (74200+68, 2047, 0, -14588.1, 345.029, 8.2059, 5.41052, 0, 0, -0.422618, 0.906308, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+43, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220801, 5620+43, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+67, 5620+43, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+68, 5620+43, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+43, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220802);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+69, 1734, 0, -13775.4, 452.27, 92.77, 4.04917, 0, 0, -0.898793, 0.438373, 300, 300, 100, 1, 0, 0, 0, 10), (74200+70, 2047, 0, -13775.4, 452.27, 92.77, 4.04917, 0, 0, -0.898793, 0.438373, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+44, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220802, 5620+44, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+69, 5620+44, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+70, 5620+44, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+44, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220803);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+71, 1734, 0, -13375.2, -373.266, 23.5344, 5.96903, 0, 0, -0.156434, 0.987688, 300, 300, 100, 1, 0, 0, 0, 10), (74200+72, 2047, 0, -13375.2, -373.266, 23.5344, 5.96903, 0, 0, -0.156434, 0.987688, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+45, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220803, 5620+45, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+71, 5620+45, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+72, 5620+45, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+45, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220804);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+73, 1734, 0, -13271.5, -311.882, 25.4076, 3.26377, 0, 0, -0.998135, 0.0610518, 300, 300, 100, 1, 0, 0, 0, 10), (74200+74, 2047, 0, -13271.5, -311.882, 25.4076, 3.26377, 0, 0, -0.998135, 0.0610518, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+46, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220804, 5620+46, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+73, 5620+46, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+74, 5620+46, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+46, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220807);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+75, 1734, 0, -13102, -670.346, 79.2665, 1.97222, 0, 0, 0.833885, 0.551938, 300, 300, 100, 1, 0, 0, 0, 10), (74200+76, 2047, 0, -13102, -670.346, 79.2665, 1.97222, 0, 0, 0.833885, 0.551938, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+47, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220807, 5620+47, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+75, 5620+47, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+76, 5620+47, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+47, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220808);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+77, 1734, 0, -13003.7, -756.31, 59.3426, 1.93731, 0, 0, 0.824125, 0.566408, 300, 300, 100, 1, 0, 0, 0, 10), (74200+78, 2047, 0, -13003.7, -756.31, 59.3426, 1.93731, 0, 0, 0.824125, 0.566408, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+48, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220808, 5620+48, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+77, 5620+48, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+78, 5620+48, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+48, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220809);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+79, 1734, 0, -12977.2, -542.209, 73.2442, 1.27409, 0, 0, 0.594822, 0.803857, 300, 300, 100, 1, 0, 0, 0, 10), (74200+80, 2047, 0, -12977.2, -542.209, 73.2442, 1.27409, 0, 0, 0.594822, 0.803857, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+49, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220809, 5620+49, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+79, 5620+49, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+80, 5620+49, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+49, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220810);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+81, 1734, 0, -12306.5, -1111.94, 5.93664, 2.93214, 0, 0, 0.994521, 0.104536, 300, 300, 100, 1, 0, 0, 0, 10), (74200+82, 2047, 0, -12306.5, -1111.94, 5.93664, 2.93214, 0, 0, 0.994521, 0.104536, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+50, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220810, 5620+50, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+81, 5620+50, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+82, 5620+50, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+50, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220811);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+83, 1734, 0, -11444.6, -935.033, 22.5053, 1.88495, 0, 0, 0.809016, 0.587786, 300, 300, 100, 1, 0, 0, 0, 10), (74200+84, 2047, 0, -11444.6, -935.033, 22.5053, 1.88495, 0, 0, 0.809016, 0.587786, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+51, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220811, 5620+51, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+83, 5620+51, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10), (74200+84, 5620+51, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+51, 9910, 0, 'Stranglethorn Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

-- Pool Silver Vein / Gold Vein / Iron Deposit Spawns in Stranglethorn Vale.
 INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (9912, 5, 'Stranglethorn Vale - Tin Vein / Silver Vein (Master Pool)', 0, 0, 0, 10);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2880, 2881, 2882, 2883, 2884, 23027);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (2880, 9912, 0, 'Stranglethorn Vale - Tin Vein / Silver Vein', 0), (2881, 9912, 0, 'Stranglethorn Vale - Tin Vein / Silver Vein', 0), (2882, 9912, 0, 'Stranglethorn Vale - Tin Vein / Silver Vein', 0), (2883, 9912, 0, 'Stranglethorn Vale - Tin Vein / Silver Vein', 0), (2884, 9912, 0, 'Stranglethorn Vale - Tin Vein / Silver Vein', 0), (23027, 9912, 0, 'Stranglethorn Vale - Tin Vein / Silver Vein', 0);

UPDATE `pool_template` SET `description` = 'Tin Vein / Silver Vein - Stranglethorn Vale' WHERE `entry` IN (2880, 2881, 2882, 2883, 2884, 23027);

DELETE FROM `pool_gameobject` WHERE `guid` IN (43252);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+85, 1733, 0, -11717.2, -139.426, 19.3768, 0.122173, 0, 0, 0.0610485, 0.998135, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+52, 1, 'Tin Vein / Silver Vein - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (43252, 5620+52, 0, 'Tin Vein / Silver Vein - Stranglethorn Vale', 0, 0, 10), (74200+85, 5620+52, 0, 'Tin Vein / Silver Vein - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+52, 9912, 0, 'Stranglethorn Vale - Tin Vein / Silver Vein', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (57639);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+86, 1733, 0, -11447.7, -321.118, 59.8395, 2.53072, 0, 0, 0.953716, 0.300708, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+53, 1, 'Tin Vein / Silver Vein - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (57639, 5620+53, 0, 'Tin Vein / Silver Vein - Stranglethorn Vale', 0, 0, 10), (74200+86, 5620+53, 0, 'Tin Vein / Silver Vein - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+53, 9912, 0, 'Stranglethorn Vale - Tin Vein / Silver Vein', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (57641);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+87, 1733, 0, -11654.8, -549.555, 29.7414, 1.0472, 0, 0, 0.5, 0.866025, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+54, 1, 'Tin Vein / Silver Vein - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (57641, 5620+54, 0, 'Tin Vein / Silver Vein - Stranglethorn Vale', 0, 0, 10), (74200+87, 5620+54, 0, 'Tin Vein / Silver Vein - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+54, 9912, 0, 'Stranglethorn Vale - Tin Vein / Silver Vein', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (57643);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+88, 1733, 0, -11463, -93.0085, 35.1165, 0.925024, 0, 0, 0.446198, 0.894935, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+55, 1, 'Tin Vein / Silver Vein - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (57643, 5620+55, 0, 'Tin Vein / Silver Vein - Stranglethorn Vale', 0, 0, 10), (74200+88, 5620+55, 0, 'Tin Vein / Silver Vein - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+55, 9912, 0, 'Stranglethorn Vale - Tin Vein / Silver Vein', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (251501);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+89, 1733, 0, -11729.8, -681.524, 36.0557, 2.67035, 0, 0, 0.972369, 0.233448, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+56, 1, 'Tin Vein / Silver Vein - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (251501, 5620+56, 0, 'Tin Vein / Silver Vein - Stranglethorn Vale', 0, 0, 10), (74200+89, 5620+56, 0, 'Tin Vein / Silver Vein - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+56, 9912, 0, 'Stranglethorn Vale - Tin Vein / Silver Vein', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (251503);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74200+90, 1733, 0, -11437.8, 25.9817, 35.6237, 1.88495, 0, 0, 0.809016, 0.587786, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5620+57, 1, 'Tin Vein / Silver Vein - Stranglethorn Vale', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (251503, 5620+57, 0, 'Tin Vein / Silver Vein - Stranglethorn Vale', 0, 0, 10), (74200+90, 5620+57, 0, 'Tin Vein / Silver Vein - Stranglethorn Vale', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5620+57, 9912, 0, 'Stranglethorn Vale - Tin Vein / Silver Vein', 0);

DELETE FROM `pool_template` WHERE `entry` = 405;

DELETE FROM `pool_gameobject` WHERE `guid` IN (6021, 6053, 6137, 6258, 10586, 10617, 10618, 11134, 11655, 11656, 11657, 11680, 11697, 11812, 12047, 12132, 12133, 12156, 12159, 12162, 12166, 12207, 29267, 29277, 29533, 29580, 29581, 30810, 30812, 30813, 32074, 34757, 34758, 34759, 34760, 34761, 389458, 389459, 389460, 389462, 427091, 427092, 427093, 427094, 427095, 427096, 427097, 427098, 427099, 427100, 427101, 427102, 427103, 427104, 427105, 427106, 427107, 427108, 427109, 427110, 427111, 427112, 427113, 427114, 427115, 427116, 427117, 427118, 427119, 427120, 427281);

DELETE FROM `gameobject` WHERE `guid` IN (6021, 6053, 6137, 6258, 10586, 10617, 10618, 11134, 11655, 11656, 11657, 11680, 11697, 11812, 12047, 12132, 12133, 12156, 12159, 12162, 12166, 12207, 29267, 29277, 29533, 29580, 29581, 30810, 30812, 30813, 32074, 34757, 34758, 34759, 34760, 34761, 389458, 389459, 389460, 389462, 427091, 427092, 427093, 427094, 427095, 427096, 427097, 427098, 427099, 427100, 427101, 427102, 427103, 427104, 427105, 427106, 427107, 427108, 427109, 427110, 427111, 427112, 427113, 427114, 427115, 427116, 427117, 427118, 427119, 427120, 427281);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74300+1, 1733, 0, -12981.6, 109.973, 39.8969, -2.3911, 0, 0, 0.930418, -0.366501, 300, 300, 100, 1, 0, 0, 0, 10), (74300+2, 1733, 0, -13420, 290.553, 45.682, 0.801, 0, 0, 0.389978, 0.920824, 300, 300, 100, 1, 0, 0, 0, 10), (74300+3, 1733, 0, -14071.1, 126.637, 23.9541, 0.855211, 0, 0, 0.414693, 0.909961, 300, 300, 100, 1, 0, 0, 0, 10), (74300+4, 1733, 0, -11574.7, 221.633, 22.9144, 2.37365, 0, 0, 0.927184, 0.374607, 300, 300, 100, 1, 0, 0, 0, 10), (74300+5, 1733, 0, -13521.4, 646.87, 10.4685, 0.244346, 0, 0, 0.121869, 0.992546, 300, 300, 100, 1, 0, 0, 0, 10), (74300+6, 1733, 0, -11518.7, -861.167, 72.1001, 6.16101, 0, 0, -0.0610485, 0.998135, 300, 300, 100, 1, 0, 0, 0, 10), (74300+7, 1733, 0, -11398.9, -826.872, 15.511, 3.35105, 0, 0, -0.994521, 0.104536, 300, 300, 100, 1, 0, 0, 0, 10), (74300+8, 1733, 0, -13236, -482.551, 6.44997, 1.3439, 0, 0, 0.622515, 0.782608, 300, 300, 100, 1, 0, 0, 0, 10), (74300+9, 1733, 0, -11804.5, -1008.84, 79.7801, 1.3439, 0, 0, 0.622515, 0.782608, 300, 300, 100, 1, 0, 0, 0, 10), (74300+10, 1733, 0, -12376.7, -971.19, 16.4219, 1.97222, 0, 0, 0.833886, 0.551937, 300, 300, 100, 1, 0, 0, 0, 10), (74300+11, 1733, 0, -12292.9, -1047.82, 12.9357, -0.575959, 0, 0, 0.284015, -0.95882, 300, 300, 100, 1, 0, 0, 0, 10), (74300+12, 1733, 0, -12460.7, -964.676, 50.3968, -2.47837, 0, 0, 0.945519, -0.325568, 300, 300, 100, 1, 0, 0, 0, 10), (74300+13, 1733, 0, -13269.9, -533.627, 19.1031, -0.977384, 0, 0, 0.469472, -0.882948, 300, 300, 100, 1, 0, 0, 0, 10), (74300+14, 1733, 0, -12034, -793.15, 22.6295, -0.523599, 0, 0, 0.258819, -0.965926, 300, 300, 100, 1, 0, 0, 0, 10), (74300+15, 1733, 0, -11521.3, -721.821, 35.7958, 2.47837, 0, 0, 0.945519, 0.325568, 300, 300, 100, 1, 0, 0, 0, 10), (74300+16, 1733, 0, -13883.7, -134.777, 14.5216, -1.25664, 0, 0, 0.587785, -0.809017, 300, 300, 100, 1, 0, 0, 0, 10), (74300+17, 1733, 0, -11820, 348.702, 42.1226, -0.122173, 0, 0, 0.061049, -0.998135, 300, 300, 100, 1, 0, 0, 0, 10), (74300+18, 1733, 0, -11486.6, -744.597, 35.9581, 1.93731, 0, 0, 0.824125, 0.566408, 300, 300, 100, 1, 0, 0, 0, 10), (74300+19, 1733, 0, -13676.5, -60.0927, 41.7262, 2.53073, 0, 0, 0.953717, 0.300706, 300, 300, 100, 1, 0, 0, 0, 10), (74300+20, 1733, 0, -13905.1, -49.6861, 35.3468, -0.244346, 0, 0, 0.121869, -0.992546, 300, 300, 100, 1, 0, 0, 0, 10), (74300+21, 1733, 0, -11596, -510.279, 28.4238, 6.23083, 0, 0, -0.0261765, 0.999657, 300, 300, 100, 1, 0, 0, 0, 10), (74300+22, 1733, 0, -12724.3, -1034.35, 100.084, 0.593412, 0, 0, 0.292372, 0.956305, 300, 300, 100, 1, 0, 0, 0, 10), (74300+23, 1733, 0, -11697.4, 173.533, 18.175, 1.65806, 0, 0, 0.737277, 0.67559, 300, 300, 100, 1, 0, 0, 0, 10), (74300+24, 1733, 0, -11771.9, 35.4992, 17.5232, 1.3439, 0, 0, 0.622514, 0.782609, 300, 300, 100, 1, 0, 0, 0, 10), (74300+25, 1733, 0, -12699.9, -291.701, 27.9929, 0.226893, 0, 0, 0.113203, 0.993572, 300, 300, 100, 1, 0, 0, 0, 10), (74300+26, 1733, 0, -12639.2, -39.5763, 27.4667, 0.488691, 0, 0, 0.241921, 0.970296, 300, 300, 100, 1, 0, 0, 0, 10), (74300+27, 1733, 0, -12676.2, -179.953, 36.221, 0.0349062, 0, 0, 0.0174522, 0.999848, 300, 300, 100, 1, 0, 0, 0, 10), (74300+28, 1733, 0, -13031.8, -421.141, 53.6756, 0.837758, 0, 0, 0.406737, 0.913545, 300, 300, 100, 1, 0, 0, 0, 10), (74300+29, 1733, 0, -12275.3, -626.938, 33.5051, 0.855211, 0, 0, 0.414693, 0.909961, 300, 300, 100, 1, 0, 0, 0, 10), (74300+30, 1733, 0, -13033.3, -352.442, 39.8206, -1.16937, 0, 0, 0.551937, -0.833886, 300, 300, 100, 1, 0, 0, 0, 10), (74300+31, 1733, 0, -13805.3, 75.9482, 36.7794, -2.93215, 0, 0, 0.994522, -0.104529, 300, 300, 100, 1, 0, 0, 0, 10), (74300+32, 1733, 0, -12048.4, -958.832, 50.9151, 0.209439, 0, 0, 0.104528, 0.994522, 300, 300, 100, 1, 0, 0, 0, 10), (74300+33, 1733, 0, -12327.2, -990.796, 19.8739, 4.15388, 0, 0, -0.874619, 0.48481, 300, 300, 100, 1, 0, 0, 0, 10), (74300+34, 1733, 0, -13201.1, -490.142, 4.77076, 0.139624, 0, 0, 0.0697556, 0.997564, 300, 300, 100, 1, 0, 0, 0, 10), (74300+35, 1733, 0, -13110.2, -323.97, 21.008, 6.23083, 0, 0, -0.0261765, 0.999657, 300, 300, 100, 1, 0, 0, 0, 10), (74300+36, 1733, 0, -13598.4, -379.535, 11.0353, 1.41372, 0, 0, 0.649447, 0.760406, 300, 300, 100, 1, 0, 0, 0, 10), (74300+37, 1733, 0, -11506.1, 480.384, 65.5531, 1.25664, 0, 0, 0.587785, 0.809017, 300, 300, 100, 1, 0, 0, 0, 10), (74300+38, 1733, 0, -11463.3, -872.963, 39.4088, 5.09636, 0, 0, -0.559193, 0.829038, 300, 300, 100, 1, 0, 0, 0, 10), (74300+39, 1733, 0, -12354.7, -992.975, 12.0588, 1.53589, 0, 0, 0.694658, 0.71934, 300, 300, 100, 1, 0, 0, 0, 10), (74300+40, 1733, 0, -13957.4, 172.505, 27.5382, 1.27409, 0, 0, 0.594822, 0.803857, 300, 300, 100, 1, 0, 0, 0, 10), (74300+41, 1733, 0, -13725.4, 346.743, 49.9747, 0.523598, 0, 0, 0.258819, 0.965926, 300, 300, 100, 1, 0, 0, 0, 10), (74300+42, 1733, 0, -13582.2, -201.529, 34.3294, 4.83456, 0, 0, -0.66262, 0.748956, 300, 300, 100, 1, 0, 0, 0, 10), (74300+43, 1733, 0, -13571, -141.395, 49.1922, 0.925024, 0, 0, 0.446198, 0.894935, 300, 300, 100, 1, 0, 0, 0, 10), (74300+44, 1733, 0, -13569.7, -16.018, 49.3623, 1.88495, 0, 0, 0.809016, 0.587786, 300, 300, 100, 1, 0, 0, 0, 10), (74300+45, 1733, 0, -13404.2, 609.748, 18.1996, 4.4855, 0, 0, -0.782608, 0.622515, 300, 300, 100, 1, 0, 0, 0, 10), (74300+46, 1733, 0, -13133, -474.707, 10.5634, 3.3685, 0, 0, -0.993571, 0.113208, 300, 300, 100, 1, 0, 0, 0, 10), (74300+47, 1733, 0, -13129.7, -569.609, 12.6068, 3.59538, 0, 0, -0.97437, 0.224951, 300, 300, 100, 1, 0, 0, 0, 10), (74300+48, 1733, 0, -13120.5, -522.741, 9.26869, 6.23083, 0, 0, -0.0261765, 0.999657, 300, 300, 100, 1, 0, 0, 0, 10), (74300+49, 1733, 0, -12970.8, -415.276, 42.9962, 1.02974, 0, 0, 0.492423, 0.870356, 300, 300, 100, 1, 0, 0, 0, 10), (74300+50, 1733, 0, -12808.4, -650.557, 67.8432, 1.64061, 0, 0, 0.731353, 0.681999, 300, 300, 100, 1, 0, 0, 0, 10), (74300+51, 1733, 0, -12384, -832.929, 49.2845, 2.44346, 0, 0, 0.939692, 0.34202, 300, 300, 100, 1, 0, 0, 0, 10), (74300+52, 1733, 0, -12367.7, -979.129, 14.7117, 2.25147, 0, 0, 0.902585, 0.430512, 300, 300, 100, 1, 0, 0, 0, 10), (74300+53, 1733, 0, -12352.9, -1021.89, 7.57272, 5.02655, 0, 0, -0.587785, 0.809017, 300, 300, 100, 1, 0, 0, 0, 10), (74300+54, 1733, 0, -12313, -1031.56, 9.10183, 5.77704, 0, 0, -0.25038, 0.968148, 300, 300, 100, 1, 0, 0, 0, 10), (74300+55, 1733, 0, -12291.5, -1096.73, 10.2265, 3.94445, 0, 0, -0.920505, 0.390732, 300, 300, 100, 1, 0, 0, 0, 10), (74300+56, 1733, 0, -12233.6, -1033.41, 61.6938, 1.95477, 0, 0, 0.829038, 0.559193, 300, 300, 100, 1, 0, 0, 0, 10), (74300+57, 1733, 0, -12128.2, -777.814, 23.5145, 3.76991, 0, 0, -0.951056, 0.309017, 300, 300, 100, 1, 0, 0, 0, 10), (74300+58, 1733, 0, -11897.4, -726.011, 23.0206, 2.07694, 0, 0, 0.861629, 0.507539, 300, 300, 100, 1, 0, 0, 0, 10), (74300+59, 1733, 0, -11822.3, 577.765, 48.6223, 0.90757, 0, 0, 0.438371, 0.898794, 300, 300, 100, 1, 0, 0, 0, 10), (74300+60, 1733, 0, -11746.5, 469.79, 52.5784, 5.70723, 0, 0, -0.284015, 0.95882, 300, 300, 100, 1, 0, 0, 0, 10), (74300+61, 1733, 0, -11745.3, -947.944, 58.7745, 3.4034, 0, 0, -0.991445, 0.130528, 300, 300, 100, 1, 0, 0, 0, 10), (74300+62, 1733, 0, -11553.7, -919.246, 70.8843, 0.418879, 0, 0, 0.207911, 0.978148, 300, 300, 100, 1, 0, 0, 0, 10), (74300+63, 1733, 0, -11542.6, -919.15, 27.9049, 3.63029, 0, 0, -0.970295, 0.241925, 300, 300, 100, 1, 0, 0, 0, 10), (74300+64, 1733, 0, -11508.1, -904.844, 29.4344, 1.27409, 0, 0, 0.594822, 0.803857, 300, 300, 100, 1, 0, 0, 0, 10), (74300+65, 1733, 0, -11471.5, -951.882, 28.4177, 0.767944, 0, 0, 0.374606, 0.927184, 300, 300, 100, 1, 0, 0, 0, 10), (74300+66, 1733, 0, -11464.5, -792.715, 16.9409, 1.93731, 0, 0, 0.824125, 0.566408, 300, 300, 100, 1, 0, 0, 0, 10), (74300+67, 1733, 0, -11444.7, -718.875, 31.5775, 0.122173, 0, 0, 0.0610485, 0.998135, 300, 300, 100, 1, 0, 0, 0, 10), (74300+68, 1733, 0, -11439.9, -898.279, 21.0529, 1.0472, 0, 0, 0.5, 0.866025, 300, 300, 100, 1, 0, 0, 0, 10), (74300+69, 1733, 0, -11410.6, -761.634, 16.6565, 1.97222, 0, 0, 0.833885, 0.551938, 300, 300, 100, 1, 0, 0, 0, 10), (74300+70, 1733, 0, -11405.4, -946.741, 21.5275, 2.67035, 0, 0, 0.972369, 0.233448, 300, 300, 100, 1, 0, 0, 0, 10), (74300+71, 1733, 0, -12796.9, -818.581, 74.8231, 2.6529, 0, 0, 0.970296, 0.241922, 300, 300, 100, 1, 0, 0, 0, 10), (74300+72, 1734, 0, -12981.6, 109.973, 39.8969, -2.3911, 0, 0, 0.930418, -0.366501, 300, 300, 100, 1, 0, 0, 0, 10), (74300+73, 1734, 0, -13420, 290.553, 45.682, 0.801, 0, 0, 0.389978, 0.920824, 300, 300, 100, 1, 0, 0, 0, 10), (74300+74, 1734, 0, -14071.1, 126.637, 23.9541, 0.855211, 0, 0, 0.414693, 0.909961, 300, 300, 100, 1, 0, 0, 0, 10), (74300+75, 1734, 0, -11574.7, 221.633, 22.9144, 2.37365, 0, 0, 0.927184, 0.374607, 300, 300, 100, 1, 0, 0, 0, 10), (74300+76, 1734, 0, -13521.4, 646.87, 10.4685, 0.244346, 0, 0, 0.121869, 0.992546, 300, 300, 100, 1, 0, 0, 0, 10), (74300+77, 1734, 0, -11518.7, -861.167, 72.1001, 6.16101, 0, 0, -0.0610485, 0.998135, 300, 300, 100, 1, 0, 0, 0, 10), (74300+78, 1734, 0, -11398.9, -826.872, 15.511, 3.35105, 0, 0, -0.994521, 0.104536, 300, 300, 100, 1, 0, 0, 0, 10), (74300+79, 1734, 0, -13236, -482.551, 6.44997, 1.3439, 0, 0, 0.622515, 0.782608, 300, 300, 100, 1, 0, 0, 0, 10), (74300+80, 1734, 0, -11804.5, -1008.84, 79.7801, 1.3439, 0, 0, 0.622515, 0.782608, 300, 300, 100, 1, 0, 0, 0, 10), (74300+81, 1734, 0, -12376.7, -971.19, 16.4219, 1.97222, 0, 0, 0.833886, 0.551937, 300, 300, 100, 1, 0, 0, 0, 10), (74300+82, 1734, 0, -12292.9, -1047.82, 12.9357, -0.575959, 0, 0, 0.284015, -0.95882, 300, 300, 100, 1, 0, 0, 0, 10), (74300+83, 1734, 0, -12460.7, -964.676, 50.3968, -2.47837, 0, 0, 0.945519, -0.325568, 300, 300, 100, 1, 0, 0, 0, 10), (74300+84, 1734, 0, -13269.9, -533.627, 19.1031, -0.977384, 0, 0, 0.469472, -0.882948, 300, 300, 100, 1, 0, 0, 0, 10), (74300+85, 1734, 0, -12034, -793.15, 22.6295, -0.523599, 0, 0, 0.258819, -0.965926, 300, 300, 100, 1, 0, 0, 0, 10), (74300+86, 1734, 0, -11521.3, -721.821, 35.7958, 2.47837, 0, 0, 0.945519, 0.325568, 300, 300, 100, 1, 0, 0, 0, 10), (74300+87, 1734, 0, -13883.7, -134.777, 14.5216, -1.25664, 0, 0, 0.587785, -0.809017, 300, 300, 100, 1, 0, 0, 0, 10), (74300+88, 1734, 0, -11820, 348.702, 42.1226, -0.122173, 0, 0, 0.061049, -0.998135, 300, 300, 100, 1, 0, 0, 0, 10), (74300+89, 1734, 0, -11486.6, -744.597, 35.9581, 1.93731, 0, 0, 0.824125, 0.566408, 300, 300, 100, 1, 0, 0, 0, 10), (74300+90, 1734, 0, -13676.5, -60.0927, 41.7262, 2.53073, 0, 0, 0.953717, 0.300706, 300, 300, 100, 1, 0, 0, 0, 10), (74300+91, 1734, 0, -13905.1, -49.6861, 35.3468, -0.244346, 0, 0, 0.121869, -0.992546, 300, 300, 100, 1, 0, 0, 0, 10), (74300+92, 1734, 0, -11596, -510.279, 28.4238, 6.23083, 0, 0, -0.0261765, 0.999657, 300, 300, 100, 1, 0, 0, 0, 10), (74300+93, 1734, 0, -12724.3, -1034.35, 100.084, 0.593412, 0, 0, 0.292372, 0.956305, 300, 300, 100, 1, 0, 0, 0, 10), (74300+94, 1734, 0, -11697.4, 173.533, 18.175, 1.65806, 0, 0, 0.737277, 0.67559, 300, 300, 100, 1, 0, 0, 0, 10), (74300+95, 1734, 0, -11771.9, 35.4992, 17.5232, 1.3439, 0, 0, 0.622514, 0.782609, 300, 300, 100, 1, 0, 0, 0, 10), (74300+96, 1734, 0, -12699.9, -291.701, 27.9929, 0.226893, 0, 0, 0.113203, 0.993572, 300, 300, 100, 1, 0, 0, 0, 10), (74300+97, 1734, 0, -12639.2, -39.5763, 27.4667, 0.488691, 0, 0, 0.241921, 0.970296, 300, 300, 100, 1, 0, 0, 0, 10), (74300+98, 1734, 0, -12676.2, -179.953, 36.221, 0.0349062, 0, 0, 0.0174522, 0.999848, 300, 300, 100, 1, 0, 0, 0, 10), (74300+99, 1734, 0, -13031.8, -421.141, 53.6756, 0.837758, 0, 0, 0.406737, 0.913545, 300, 300, 100, 1, 0, 0, 0, 10), (74300+100, 1734, 0, -12275.3, -626.938, 33.5051, 0.855211, 0, 0, 0.414693, 0.909961, 300, 300, 100, 1, 0, 0, 0, 10), (74300+101, 1734, 0, -13033.3, -352.442, 39.8206, -1.16937, 0, 0, 0.551937, -0.833886, 300, 300, 100, 1, 0, 0, 0, 10), (74300+102, 1734, 0, -13805.3, 75.9482, 36.7794, -2.93215, 0, 0, 0.994522, -0.104529, 300, 300, 100, 1, 0, 0, 0, 10), (74300+103, 1734, 0, -12048.4, -958.832, 50.9151, 0.209439, 0, 0, 0.104528, 0.994522, 300, 300, 100, 1, 0, 0, 0, 10), (74300+104, 1734, 0, -12327.2, -990.796, 19.8739, 4.15388, 0, 0, -0.874619, 0.48481, 300, 300, 100, 1, 0, 0, 0, 10), (74300+105, 1734, 0, -13201.1, -490.142, 4.77076, 0.139624, 0, 0, 0.0697556, 0.997564, 300, 300, 100, 1, 0, 0, 0, 10), (74300+106, 1734, 0, -13110.2, -323.97, 21.008, 6.23083, 0, 0, -0.0261765, 0.999657, 300, 300, 100, 1, 0, 0, 0, 10), (74300+107, 1734, 0, -13598.4, -379.535, 11.0353, 1.41372, 0, 0, 0.649447, 0.760406, 300, 300, 100, 1, 0, 0, 0, 10), (74300+108, 1734, 0, -11506.1, 480.384, 65.5531, 1.25664, 0, 0, 0.587785, 0.809017, 300, 300, 100, 1, 0, 0, 0, 10), (74300+109, 1734, 0, -11463.3, -872.963, 39.4088, 5.09636, 0, 0, -0.559193, 0.829038, 300, 300, 100, 1, 0, 0, 0, 10), (74300+110, 1734, 0, -12354.7, -992.975, 12.0588, 1.53589, 0, 0, 0.694658, 0.71934, 300, 300, 100, 1, 0, 0, 0, 10), (74300+111, 1734, 0, -13957.4, 172.505, 27.5382, 1.27409, 0, 0, 0.594822, 0.803857, 300, 300, 100, 1, 0, 0, 0, 10), (74300+112, 1734, 0, -13725.4, 346.743, 49.9747, 0.523598, 0, 0, 0.258819, 0.965926, 300, 300, 100, 1, 0, 0, 0, 10), (74300+113, 1734, 0, -13582.2, -201.529, 34.3294, 4.83456, 0, 0, -0.66262, 0.748956, 300, 300, 100, 1, 0, 0, 0, 10), (74300+114, 1734, 0, -13571, -141.395, 49.1922, 0.925024, 0, 0, 0.446198, 0.894935, 300, 300, 100, 1, 0, 0, 0, 10), (74300+115, 1734, 0, -13569.7, -16.018, 49.3623, 1.88495, 0, 0, 0.809016, 0.587786, 300, 300, 100, 1, 0, 0, 0, 10), (74300+116, 1734, 0, -13404.2, 609.748, 18.1996, 4.4855, 0, 0, -0.782608, 0.622515, 300, 300, 100, 1, 0, 0, 0, 10), (74300+117, 1734, 0, -13133, -474.707, 10.5634, 3.3685, 0, 0, -0.993571, 0.113208, 300, 300, 100, 1, 0, 0, 0, 10), (74300+118, 1734, 0, -13129.7, -569.609, 12.6068, 3.59538, 0, 0, -0.97437, 0.224951, 300, 300, 100, 1, 0, 0, 0, 10), (74300+119, 1734, 0, -13120.5, -522.741, 9.26869, 6.23083, 0, 0, -0.0261765, 0.999657, 300, 300, 100, 1, 0, 0, 0, 10), (74300+120, 1734, 0, -12970.8, -415.276, 42.9962, 1.02974, 0, 0, 0.492423, 0.870356, 300, 300, 100, 1, 0, 0, 0, 10), (74300+121, 1734, 0, -12808.4, -650.557, 67.8432, 1.64061, 0, 0, 0.731353, 0.681999, 300, 300, 100, 1, 0, 0, 0, 10), (74300+122, 1734, 0, -12384, -832.929, 49.2845, 2.44346, 0, 0, 0.939692, 0.34202, 300, 300, 100, 1, 0, 0, 0, 10), (74300+123, 1734, 0, -12367.7, -979.129, 14.7117, 2.25147, 0, 0, 0.902585, 0.430512, 300, 300, 100, 1, 0, 0, 0, 10), (74300+124, 1734, 0, -12352.9, -1021.89, 7.57272, 5.02655, 0, 0, -0.587785, 0.809017, 300, 300, 100, 1, 0, 0, 0, 10), (74300+125, 1734, 0, -12313, -1031.56, 9.10183, 5.77704, 0, 0, -0.25038, 0.968148, 300, 300, 100, 1, 0, 0, 0, 10), (74300+126, 1734, 0, -12291.5, -1096.73, 10.2265, 3.94445, 0, 0, -0.920505, 0.390732, 300, 300, 100, 1, 0, 0, 0, 10), (74300+127, 1734, 0, -12233.6, -1033.41, 61.6938, 1.95477, 0, 0, 0.829038, 0.559193, 300, 300, 100, 1, 0, 0, 0, 10), (74300+128, 1734, 0, -12128.2, -777.814, 23.5145, 3.76991, 0, 0, -0.951056, 0.309017, 300, 300, 100, 1, 0, 0, 0, 10), (74300+129, 1734, 0, -11897.4, -726.011, 23.0206, 2.07694, 0, 0, 0.861629, 0.507539, 300, 300, 100, 1, 0, 0, 0, 10), (74300+130, 1734, 0, -11822.3, 577.765, 48.6223, 0.90757, 0, 0, 0.438371, 0.898794, 300, 300, 100, 1, 0, 0, 0, 10), (74300+131, 1734, 0, -11746.5, 469.79, 52.5784, 5.70723, 0, 0, -0.284015, 0.95882, 300, 300, 100, 1, 0, 0, 0, 10), (74300+132, 1734, 0, -11745.3, -947.944, 58.7745, 3.4034, 0, 0, -0.991445, 0.130528, 300, 300, 100, 1, 0, 0, 0, 10), (74300+133, 1734, 0, -11553.7, -919.246, 70.8843, 0.418879, 0, 0, 0.207911, 0.978148, 300, 300, 100, 1, 0, 0, 0, 10), (74300+134, 1734, 0, -11542.6, -919.15, 27.9049, 3.63029, 0, 0, -0.970295, 0.241925, 300, 300, 100, 1, 0, 0, 0, 10), (74300+135, 1734, 0, -11508.1, -904.844, 29.4344, 1.27409, 0, 0, 0.594822, 0.803857, 300, 300, 100, 1, 0, 0, 0, 10), (74300+136, 1734, 0, -11471.5, -951.882, 28.4177, 0.767944, 0, 0, 0.374606, 0.927184, 300, 300, 100, 1, 0, 0, 0, 10), (74300+137, 1734, 0, -11464.5, -792.715, 16.9409, 1.93731, 0, 0, 0.824125, 0.566408, 300, 300, 100, 1, 0, 0, 0, 10), (74300+138, 1734, 0, -11444.7, -718.875, 31.5775, 0.122173, 0, 0, 0.0610485, 0.998135, 300, 300, 100, 1, 0, 0, 0, 10), (74300+139, 1734, 0, -11439.9, -898.279, 21.0529, 1.0472, 0, 0, 0.5, 0.866025, 300, 300, 100, 1, 0, 0, 0, 10), (74300+140, 1734, 0, -11410.6, -761.634, 16.6565, 1.97222, 0, 0, 0.833885, 0.551938, 300, 300, 100, 1, 0, 0, 0, 10), (74300+141, 1734, 0, -11405.4, -946.741, 21.5275, 2.67035, 0, 0, 0.972369, 0.233448, 300, 300, 100, 1, 0, 0, 0, 10), (74300+142, 1734, 0, -12796.9, -818.581, 74.8231, 2.6529, 0, 0, 0.970296, 0.241922, 300, 300, 100, 1, 0, 0, 0, 10), (74300+143, 1735, 0, -12981.6, 109.973, 39.8969, -2.3911, 0, 0, 0.930418, -0.366501, 300, 300, 100, 1, 0, 0, 0, 10), (74300+144, 1735, 0, -13420, 290.553, 45.682, 0.801, 0, 0, 0.389978, 0.920824, 300, 300, 100, 1, 0, 0, 0, 10), (74300+145, 1735, 0, -14071.1, 126.637, 23.9541, 0.855211, 0, 0, 0.414693, 0.909961, 300, 300, 100, 1, 0, 0, 0, 10), (74300+146, 1735, 0, -11574.7, 221.633, 22.9144, 2.37365, 0, 0, 0.927184, 0.374607, 300, 300, 100, 1, 0, 0, 0, 10), (74300+147, 1735, 0, -13521.4, 646.87, 10.4685, 0.244346, 0, 0, 0.121869, 0.992546, 300, 300, 100, 1, 0, 0, 0, 10), (74300+148, 1735, 0, -11518.7, -861.167, 72.1001, 6.16101, 0, 0, -0.0610485, 0.998135, 300, 300, 100, 1, 0, 0, 0, 10), (74300+149, 1735, 0, -11398.9, -826.872, 15.511, 3.35105, 0, 0, -0.994521, 0.104536, 300, 300, 100, 1, 0, 0, 0, 10), (74300+150, 1735, 0, -13236, -482.551, 6.44997, 1.3439, 0, 0, 0.622515, 0.782608, 300, 300, 100, 1, 0, 0, 0, 10), (74300+151, 1735, 0, -11804.5, -1008.84, 79.7801, 1.3439, 0, 0, 0.622515, 0.782608, 300, 300, 100, 1, 0, 0, 0, 10), (74300+152, 1735, 0, -12376.7, -971.19, 16.4219, 1.97222, 0, 0, 0.833886, 0.551937, 300, 300, 100, 1, 0, 0, 0, 10), (74300+153, 1735, 0, -12292.9, -1047.82, 12.9357, -0.575959, 0, 0, 0.284015, -0.95882, 300, 300, 100, 1, 0, 0, 0, 10), (74300+154, 1735, 0, -12460.7, -964.676, 50.3968, -2.47837, 0, 0, 0.945519, -0.325568, 300, 300, 100, 1, 0, 0, 0, 10), (74300+155, 1735, 0, -13269.9, -533.627, 19.1031, -0.977384, 0, 0, 0.469472, -0.882948, 300, 300, 100, 1, 0, 0, 0, 10), (74300+156, 1735, 0, -12034, -793.15, 22.6295, -0.523599, 0, 0, 0.258819, -0.965926, 300, 300, 100, 1, 0, 0, 0, 10), (74300+157, 1735, 0, -11521.3, -721.821, 35.7958, 2.47837, 0, 0, 0.945519, 0.325568, 300, 300, 100, 1, 0, 0, 0, 10), (74300+158, 1735, 0, -13883.7, -134.777, 14.5216, -1.25664, 0, 0, 0.587785, -0.809017, 300, 300, 100, 1, 0, 0, 0, 10), (74300+159, 1735, 0, -11820, 348.702, 42.1226, -0.122173, 0, 0, 0.061049, -0.998135, 300, 300, 100, 1, 0, 0, 0, 10), (74300+160, 1735, 0, -11486.6, -744.597, 35.9581, 1.93731, 0, 0, 0.824125, 0.566408, 300, 300, 100, 1, 0, 0, 0, 10), (74300+161, 1735, 0, -13676.5, -60.0927, 41.7262, 2.53073, 0, 0, 0.953717, 0.300706, 300, 300, 100, 1, 0, 0, 0, 10), (74300+162, 1735, 0, -13905.1, -49.6861, 35.3468, -0.244346, 0, 0, 0.121869, -0.992546, 300, 300, 100, 1, 0, 0, 0, 10), (74300+163, 1735, 0, -11596, -510.279, 28.4238, 6.23083, 0, 0, -0.0261765, 0.999657, 300, 300, 100, 1, 0, 0, 0, 10), (74300+164, 1735, 0, -12724.3, -1034.35, 100.084, 0.593412, 0, 0, 0.292372, 0.956305, 300, 300, 100, 1, 0, 0, 0, 10), (74300+165, 1735, 0, -11697.4, 173.533, 18.175, 1.65806, 0, 0, 0.737277, 0.67559, 300, 300, 100, 1, 0, 0, 0, 10), (74300+166, 1735, 0, -11771.9, 35.4992, 17.5232, 1.3439, 0, 0, 0.622514, 0.782609, 300, 300, 100, 1, 0, 0, 0, 10), (74300+167, 1735, 0, -12699.9, -291.701, 27.9929, 0.226893, 0, 0, 0.113203, 0.993572, 300, 300, 100, 1, 0, 0, 0, 10), (74300+168, 1735, 0, -12639.2, -39.5763, 27.4667, 0.488691, 0, 0, 0.241921, 0.970296, 300, 300, 100, 1, 0, 0, 0, 10), (74300+169, 1735, 0, -12676.2, -179.953, 36.221, 0.0349062, 0, 0, 0.0174522, 0.999848, 300, 300, 100, 1, 0, 0, 0, 10), (74300+170, 1735, 0, -13031.8, -421.141, 53.6756, 0.837758, 0, 0, 0.406737, 0.913545, 300, 300, 100, 1, 0, 0, 0, 10), (74300+171, 1735, 0, -12275.3, -626.938, 33.5051, 0.855211, 0, 0, 0.414693, 0.909961, 300, 300, 100, 1, 0, 0, 0, 10), (74300+172, 1735, 0, -13033.3, -352.442, 39.8206, -1.16937, 0, 0, 0.551937, -0.833886, 300, 300, 100, 1, 0, 0, 0, 10), (74300+173, 1735, 0, -13805.3, 75.9482, 36.7794, -2.93215, 0, 0, 0.994522, -0.104529, 300, 300, 100, 1, 0, 0, 0, 10), (74300+174, 1735, 0, -12048.4, -958.832, 50.9151, 0.209439, 0, 0, 0.104528, 0.994522, 300, 300, 100, 1, 0, 0, 0, 10), (74300+175, 1735, 0, -12327.2, -990.796, 19.8739, 4.15388, 0, 0, -0.874619, 0.48481, 300, 300, 100, 1, 0, 0, 0, 10), (74300+176, 1735, 0, -13201.1, -490.142, 4.77076, 0.139624, 0, 0, 0.0697556, 0.997564, 300, 300, 100, 1, 0, 0, 0, 10), (74300+177, 1735, 0, -13110.2, -323.97, 21.008, 6.23083, 0, 0, -0.0261765, 0.999657, 300, 300, 100, 1, 0, 0, 0, 10), (74300+178, 1735, 0, -13598.4, -379.535, 11.0353, 1.41372, 0, 0, 0.649447, 0.760406, 300, 300, 100, 1, 0, 0, 0, 10), (74300+179, 1735, 0, -11506.1, 480.384, 65.5531, 1.25664, 0, 0, 0.587785, 0.809017, 300, 300, 100, 1, 0, 0, 0, 10), (74300+180, 1735, 0, -11463.3, -872.963, 39.4088, 5.09636, 0, 0, -0.559193, 0.829038, 300, 300, 100, 1, 0, 0, 0, 10), (74300+181, 1735, 0, -12354.7, -992.975, 12.0588, 1.53589, 0, 0, 0.694658, 0.71934, 300, 300, 100, 1, 0, 0, 0, 10), (74300+182, 1735, 0, -13957.4, 172.505, 27.5382, 1.27409, 0, 0, 0.594822, 0.803857, 300, 300, 100, 1, 0, 0, 0, 10), (74300+183, 1735, 0, -13725.4, 346.743, 49.9747, 0.523598, 0, 0, 0.258819, 0.965926, 300, 300, 100, 1, 0, 0, 0, 10), (74300+184, 1735, 0, -13582.2, -201.529, 34.3294, 4.83456, 0, 0, -0.66262, 0.748956, 300, 300, 100, 1, 0, 0, 0, 10), (74300+185, 1735, 0, -13571, -141.395, 49.1922, 0.925024, 0, 0, 0.446198, 0.894935, 300, 300, 100, 1, 0, 0, 0, 10), (74300+186, 1735, 0, -13569.7, -16.018, 49.3623, 1.88495, 0, 0, 0.809016, 0.587786, 300, 300, 100, 1, 0, 0, 0, 10), (74300+187, 1735, 0, -13404.2, 609.748, 18.1996, 4.4855, 0, 0, -0.782608, 0.622515, 300, 300, 100, 1, 0, 0, 0, 10), (74300+188, 1735, 0, -13133, -474.707, 10.5634, 3.3685, 0, 0, -0.993571, 0.113208, 300, 300, 100, 1, 0, 0, 0, 10), (74300+189, 1735, 0, -13129.7, -569.609, 12.6068, 3.59538, 0, 0, -0.97437, 0.224951, 300, 300, 100, 1, 0, 0, 0, 10), (74300+190, 1735, 0, -13120.5, -522.741, 9.26869, 6.23083, 0, 0, -0.0261765, 0.999657, 300, 300, 100, 1, 0, 0, 0, 10), (74300+191, 1735, 0, -12970.8, -415.276, 42.9962, 1.02974, 0, 0, 0.492423, 0.870356, 300, 300, 100, 1, 0, 0, 0, 10), (74300+192, 1735, 0, -12808.4, -650.557, 67.8432, 1.64061, 0, 0, 0.731353, 0.681999, 300, 300, 100, 1, 0, 0, 0, 10), (74300+193, 1735, 0, -12384, -832.929, 49.2845, 2.44346, 0, 0, 0.939692, 0.34202, 300, 300, 100, 1, 0, 0, 0, 10), (74300+194, 1735, 0, -12367.7, -979.129, 14.7117, 2.25147, 0, 0, 0.902585, 0.430512, 300, 300, 100, 1, 0, 0, 0, 10), (74300+195, 1735, 0, -12352.9, -1021.89, 7.57272, 5.02655, 0, 0, -0.587785, 0.809017, 300, 300, 100, 1, 0, 0, 0, 10), (74300+196, 1735, 0, -12313, -1031.56, 9.10183, 5.77704, 0, 0, -0.25038, 0.968148, 300, 300, 100, 1, 0, 0, 0, 10), (74300+197, 1735, 0, -12291.5, -1096.73, 10.2265, 3.94445, 0, 0, -0.920505, 0.390732, 300, 300, 100, 1, 0, 0, 0, 10), (74300+198, 1735, 0, -12233.6, -1033.41, 61.6938, 1.95477, 0, 0, 0.829038, 0.559193, 300, 300, 100, 1, 0, 0, 0, 10), (74300+199, 1735, 0, -12128.2, -777.814, 23.5145, 3.76991, 0, 0, -0.951056, 0.309017, 300, 300, 100, 1, 0, 0, 0, 10), (74300+200, 1735, 0, -11897.4, -726.011, 23.0206, 2.07694, 0, 0, 0.861629, 0.507539, 300, 300, 100, 1, 0, 0, 0, 10), (74300+201, 1735, 0, -11822.3, 577.765, 48.6223, 0.90757, 0, 0, 0.438371, 0.898794, 300, 300, 100, 1, 0, 0, 0, 10), (74300+202, 1735, 0, -11746.5, 469.79, 52.5784, 5.70723, 0, 0, -0.284015, 0.95882, 300, 300, 100, 1, 0, 0, 0, 10), (74300+203, 1735, 0, -11745.3, -947.944, 58.7745, 3.4034, 0, 0, -0.991445, 0.130528, 300, 300, 100, 1, 0, 0, 0, 10), (74300+204, 1735, 0, -11553.7, -919.246, 70.8843, 0.418879, 0, 0, 0.207911, 0.978148, 300, 300, 100, 1, 0, 0, 0, 10), (74300+205, 1735, 0, -11542.6, -919.15, 27.9049, 3.63029, 0, 0, -0.970295, 0.241925, 300, 300, 100, 1, 0, 0, 0, 10), (74300+206, 1735, 0, -11508.1, -904.844, 29.4344, 1.27409, 0, 0, 0.594822, 0.803857, 300, 300, 100, 1, 0, 0, 0, 10), (74300+207, 1735, 0, -11471.5, -951.882, 28.4177, 0.767944, 0, 0, 0.374606, 0.927184, 300, 300, 100, 1, 0, 0, 0, 10), (74300+208, 1735, 0, -11464.5, -792.715, 16.9409, 1.93731, 0, 0, 0.824125, 0.566408, 300, 300, 100, 1, 0, 0, 0, 10), (74300+209, 1735, 0, -11444.7, -718.875, 31.5775, 0.122173, 0, 0, 0.0610485, 0.998135, 300, 300, 100, 1, 0, 0, 0, 10), (74300+210, 1735, 0, -11439.9, -898.279, 21.0529, 1.0472, 0, 0, 0.5, 0.866025, 300, 300, 100, 1, 0, 0, 0, 10), (74300+211, 1735, 0, -11410.6, -761.634, 16.6565, 1.97222, 0, 0, 0.833885, 0.551938, 300, 300, 100, 1, 0, 0, 0, 10), (74300+212, 1735, 0, -11405.4, -946.741, 21.5275, 2.67035, 0, 0, 0.972369, 0.233448, 300, 300, 100, 1, 0, 0, 0, 10), (74300+213, 1735, 0, -12796.9, -818.581, 74.8231, 2.6529, 0, 0, 0.970296, 0.241922, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `patch_max`) VALUES (5700+1, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+2, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+3, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+4, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+5, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+6, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+7, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+8, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+9, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+10, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+11, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+12, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+13, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+14, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+15, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+16, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+17, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+18, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+19, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+20, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+21, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+22, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+23, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+24, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+25, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+26, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+27, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+28, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+29, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+30, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+31, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+32, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+33, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+34, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+35, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+36, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+37, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+38, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+39, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+40, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+41, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+42, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+43, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+44, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+45, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+46, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+47, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+48, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+49, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+50, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+51, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+52, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+53, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+54, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+55, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+56, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+57, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+58, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+59, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+60, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+61, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+62, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+63, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+64, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+65, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+66, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+67, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+68, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+69, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+70, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (5700+71, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`, `patch_max`) VALUES (74300+1, 5700+1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+2, 5700+2, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+3, 5700+3, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+4, 5700+4, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+5, 5700+5, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+6, 5700+6, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+7, 5700+7, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+8, 5700+8, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+9, 5700+9, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+10, 5700+10, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+11, 5700+11, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+12, 5700+12, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+13, 5700+13, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+14, 5700+14, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+15, 5700+15, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+16, 5700+16, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+17, 5700+17, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+18, 5700+18, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+19, 5700+19, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+20, 5700+20, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+21, 5700+21, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+22, 5700+22, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+23, 5700+23, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+24, 5700+24, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+25, 5700+25, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+26, 5700+26, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+27, 5700+27, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+28, 5700+28, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+29, 5700+29, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+30, 5700+30, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+31, 5700+31, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+32, 5700+32, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+33, 5700+33, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+34, 5700+34, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+35, 5700+35, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+36, 5700+36, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+37, 5700+37, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+38, 5700+38, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+39, 5700+39, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+40, 5700+40, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+41, 5700+41, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+42, 5700+42, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+43, 5700+43, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+44, 5700+44, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+45, 5700+45, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+46, 5700+46, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+47, 5700+47, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+48, 5700+48, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+49, 5700+49, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+50, 5700+50, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+51, 5700+51, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+52, 5700+52, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+53, 5700+53, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+54, 5700+54, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+55, 5700+55, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+56, 5700+56, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+57, 5700+57, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+58, 5700+58, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+59, 5700+59, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+60, 5700+60, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+61, 5700+61, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+62, 5700+62, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+63, 5700+63, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+64, 5700+64, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+65, 5700+65, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+66, 5700+66, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+67, 5700+67, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+68, 5700+68, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+69, 5700+69, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+70, 5700+70, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+71, 5700+71, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+72, 5700+1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+73, 5700+2, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+74, 5700+3, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+75, 5700+4, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+76, 5700+5, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+77, 5700+6, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+78, 5700+7, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+79, 5700+8, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+80, 5700+9, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+81, 5700+10, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+82, 5700+11, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+83, 5700+12, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+84, 5700+13, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+85, 5700+14, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+86, 5700+15, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+87, 5700+16, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+88, 5700+17, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+89, 5700+18, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+90, 5700+19, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+91, 5700+20, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+92, 5700+21, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+93, 5700+22, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+94, 5700+23, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+95, 5700+24, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+96, 5700+25, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+97, 5700+26, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+98, 5700+27, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+99, 5700+28, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+100, 5700+29, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+101, 5700+30, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+102, 5700+31, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+103, 5700+32, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+104, 5700+33, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+105, 5700+34, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+106, 5700+35, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+107, 5700+36, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+108, 5700+37, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+109, 5700+38, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+110, 5700+39, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+111, 5700+40, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+112, 5700+41, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+113, 5700+42, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+114, 5700+43, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+115, 5700+44, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+116, 5700+45, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+117, 5700+46, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+118, 5700+47, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+119, 5700+48, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+120, 5700+49, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+121, 5700+50, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+122, 5700+51, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+123, 5700+52, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+124, 5700+53, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+125, 5700+54, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+126, 5700+55, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+127, 5700+56, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+128, 5700+57, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+129, 5700+58, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+130, 5700+59, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+131, 5700+60, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+132, 5700+61, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+133, 5700+62, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+134, 5700+63, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+135, 5700+64, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+136, 5700+65, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+137, 5700+66, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+138, 5700+67, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+139, 5700+68, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+140, 5700+69, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+141, 5700+70, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+142, 5700+71, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+143, 5700+1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+144, 5700+2, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+145, 5700+3, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+146, 5700+4, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+147, 5700+5, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+148, 5700+6, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+149, 5700+7, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+150, 5700+8, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+151, 5700+9, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+152, 5700+10, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+153, 5700+11, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+154, 5700+12, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+155, 5700+13, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+156, 5700+14, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+157, 5700+15, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+158, 5700+16, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+159, 5700+17, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+160, 5700+18, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+161, 5700+19, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+162, 5700+20, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+163, 5700+21, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+164, 5700+22, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+165, 5700+23, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+166, 5700+24, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+167, 5700+25, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+168, 5700+26, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+169, 5700+27, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+170, 5700+28, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+171, 5700+29, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+172, 5700+30, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+173, 5700+31, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+174, 5700+32, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+175, 5700+33, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+176, 5700+34, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+177, 5700+35, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+178, 5700+36, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+179, 5700+37, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+180, 5700+38, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+181, 5700+39, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+182, 5700+40, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+183, 5700+41, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+184, 5700+42, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+185, 5700+43, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+186, 5700+44, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+187, 5700+45, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+188, 5700+46, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+189, 5700+47, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+190, 5700+48, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+191, 5700+49, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+192, 5700+50, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+193, 5700+51, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+194, 5700+52, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+195, 5700+53, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+196, 5700+54, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+197, 5700+55, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+198, 5700+56, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+199, 5700+57, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+200, 5700+58, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+201, 5700+59, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+202, 5700+60, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+203, 5700+61, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+204, 5700+62, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+205, 5700+63, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+206, 5700+64, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+207, 5700+65, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+208, 5700+66, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+209, 5700+67, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+210, 5700+68, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+211, 5700+69, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+212, 5700+70, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10), (74300+213, 5700+71, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale', 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5700+1, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+2, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+3, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+4, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+5, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+6, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+7, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+8, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+9, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+10, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+11, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+12, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+13, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+14, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+15, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+16, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+17, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+18, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+19, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+20, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+21, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+22, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+23, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+24, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+25, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+26, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+27, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+28, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+29, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+30, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+31, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+32, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+33, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+34, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+35, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+36, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+37, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+38, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+39, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+40, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+41, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+42, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+43, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+44, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+45, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+46, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+47, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+48, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+49, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+50, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+51, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+52, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+53, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+54, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+55, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+56, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+57, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+58, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+59, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+60, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+61, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+62, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+63, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+64, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+65, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+66, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+67, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+68, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+69, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+70, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (5700+71, 9910, 0, 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `pool_entry` = 1175;

DELETE FROM `pool_template` WHERE `entry` = 23057;

-- Correct Naming
 UPDATE `pool_gameobject` SET `description` = REPLACE (`description`, 'Stranglethorn', 'Stranglethorn Vale') WHERE `description` LIKE '%Stranglethorn';

UPDATE `pool_template` SET `description` = REPLACE (`description`, 'Stranglethorn', 'Stranglethorn Vale') WHERE `description` LIKE '%Stranglethorn';

UPDATE `pool_pool` SET `description` = REPLACE (`description`, 'Stranglethorn', 'Stranglethorn Vale') WHERE `description` LIKE '%Stranglethorn';

UPDATE `pool_template` SET `description` = 'Stranglethorn Vale - Oily Blackmouth School / Firefin Snapper School / Floating Wreckage (Master Pool)' WHERE `entry` IN (3193, 2974);

-- Add Chance
 -- Gold Vein / Mithril Deposit / Truesilver Deposit
 UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1734);

UPDATE `pool_gameobject` SET `chance` = 90 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 2040);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 2047);

-- Silver Vein / Gold Vein / Iron Deposit
 UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1733);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1734);

UPDATE `pool_gameobject` SET `chance` = 90 WHERE `description` = 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1735);

-- Tin Vein / Silver Vein
 UPDATE `pool_gameobject` SET `chance` = 95 WHERE `description` = 'Tin Vein / Silver Vein - Stranglethorn Vale' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1732);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Tin Vein / Silver Vein - Stranglethorn Vale' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1733);

-- End of migration.
;
