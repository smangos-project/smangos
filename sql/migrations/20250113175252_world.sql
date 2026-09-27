INSERT INTO `migrations` VALUES ('20250113175252');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (9901, 5, 'Alterac Mountains - Gold Vein / Mithril Deposit / Truesilver Deposit (Master Pool)', 0, 0, 0, 10);

DELETE FROM `pool_gameobject` WHERE `guid` IN (87879, 220432);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+1, 1734, 0, 742.411, -759.57, 179.781, 0.122173, 0, 0, 0.0610485, 0.998135, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+1, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (85100+1, 5000+1, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (87879, 5000+1, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (220432, 5000+1, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+1, 9901, 0, 'Alterac Mountains - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (87880, 387421);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+2, 1734, 0, 759.1, -212.715, 145.379, 1.39626, 0, 0, 0.642787, 0.766045, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+2, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (85100+2, 5000+2, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (87880, 5000+2, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (387421, 5000+2, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+2, 9901, 0, 'Alterac Mountains - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (87881, 33180);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+3, 1734, 0, 285.762, -121.496, 145.998, 0.383972, 0, 0, 0.190809, 0.981627, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+3, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (85100+3, 5000+3, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (87881, 5000+3, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (33180, 5000+3, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+3, 9901, 0, 'Alterac Mountains - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (87882, 220821);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+4, 1734, 0, 669.802, -606.522, 163.762, 2.1293, 0, 0, 0.874619, 0.48481, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+4, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (85100+4, 5000+4, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (87882, 5000+4, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (220821, 5000+4, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+4, 9901, 0, 'Alterac Mountains - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (87883, 220822);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+5, 1734, 0, 444.605, -80.3419, 149.462, 4.95674, 0, 0, -0.615661, 0.788011, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+5, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (85100+5, 5000+5, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (87883, 5000+5, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (220822, 5000+5, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+5, 9901, 0, 'Alterac Mountains - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (87884, 220410);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+6, 1734, 0, 722.529, -660.758, 161.868, 3.29869, 0, 0, -0.996917, 0.0784664, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+6, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (85100+6, 5000+6, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (87884, 5000+6, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (220410, 5000+6, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+6, 9901, 0, 'Alterac Mountains - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (33184);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+7, 1734, 0, 639.96, -678.248, 156.106, -0.418879, 0, 0, 0.207912, -0.978148, 300, 300, 100, 1, 0, 0, 0, 10), (85100+8, 2047, 0, 639.96, -678.248, 156.106, -0.418879, 0, 0, 0.207912, -0.978148, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+7, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (33184, 5000+7, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+7, 5000+7, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+8, 5000+7, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+7, 9901, 0, 'Alterac Mountains - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (35398);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+9, 1734, 0, 124.023, -252.108, 169.119, -2.93215, 0, 0, 0.994522, -0.104529, 300, 300, 100, 1, 0, 0, 0, 10), (85100+10, 2047, 0, 124.023, -252.108, 169.119, -2.93215, 0, 0, 0.994522, -0.104529, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+8, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (35398, 5000+8, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+9, 5000+8, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+10, 5000+8, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+8, 9901, 0, 'Alterac Mountains - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220406);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+11, 1734, 0, 889.775, -560.039, 143.449, 1.23918, 0, 0, 0.580703, 0.814116, 300, 300, 100, 1, 0, 0, 0, 10), (85100+12, 2047, 0, 889.775, -560.039, 143.449, 1.23918, 0, 0, 0.580703, 0.814116, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+9, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220406, 5000+9, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+11, 5000+9, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+12, 5000+9, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+9, 9901, 0, 'Alterac Mountains - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220407);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+13, 1734, 0, 649.918, -631.854, 157.594, 5.96903, 0, 0, -0.156434, 0.987688, 300, 300, 100, 1, 0, 0, 0, 10), (85100+14, 2047, 0, 649.918, -631.854, 157.594, 5.96903, 0, 0, -0.156434, 0.987688, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+10, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220407, 5000+10, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+13, 5000+10, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+14, 5000+10, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+10, 9901, 0, 'Alterac Mountains - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220408);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+15, 1734, 0, 812.972, -693.54, 173.467, 3.00195, 0, 0, 0.997563, 0.0697661, 300, 300, 100, 1, 0, 0, 0, 10), (85100+16, 2047, 0, 812.972, -693.54, 173.467, 3.00195, 0, 0, 0.997563, 0.0697661, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+11, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220408, 5000+11, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+15, 5000+11, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+16, 5000+11, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+11, 9901, 0, 'Alterac Mountains - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220411);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+17, 1734, 0, 591.945, -724.726, 151.695, 6.16101, 0, 0, -0.0610485, 0.998135, 300, 300, 100, 1, 0, 0, 0, 10), (85100+18, 2047, 0, 591.945, -724.726, 151.695, 6.16101, 0, 0, -0.0610485, 0.998135, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+12, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220411, 5000+12, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+17, 5000+12, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+18, 5000+12, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+12, 9901, 0, 'Alterac Mountains - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220412);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+19, 1734, 0, 860.572, -569.382, 144.673, 0.314158, 0, 0, 0.156434, 0.987688, 300, 300, 100, 1, 0, 0, 0, 10), (85100+20, 2047, 0, 860.572, -569.382, 144.673, 0.314158, 0, 0, 0.156434, 0.987688, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+13, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220412, 5000+13, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+19, 5000+13, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+20, 5000+13, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+13, 9901, 0, 'Alterac Mountains - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (43255, 220454);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+21, 2047, 0, 582.789, -746.925, 152.998, 4.20625, 0, 0, -0.861629, 0.507539, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+14, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (43255, 5000+14, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (220454, 5000+14, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+21, 5000+14, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+14, 9901, 0, 'Alterac Mountains - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220823);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+22, 1734, 0, 556.295, -716.672, 154.8, 1.25664, 0, 0, 0.587785, 0.809017, 300, 300, 100, 1, 0, 0, 0, 10), (85100+23, 2047, 0, 556.295, -716.672, 154.8, 1.25664, 0, 0, 0.587785, 0.809017, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+15, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220823, 5000+15, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+22, 5000+15, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+23, 5000+15, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+15, 9901, 0, 'Alterac Mountains - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220834);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+24, 1734, 0, 719.463, -502.486, 171.224, 1.309, 0, 0, 0.608761, 0.793353, 300, 300, 100, 1, 0, 0, 0, 10), (85100+25, 2047, 0, 719.463, -502.486, 171.224, 1.309, 0, 0, 0.608761, 0.793353, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+16, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220834, 5000+16, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+24, 5000+16, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+25, 5000+16, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+16, 9901, 0, 'Alterac Mountains - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (387419);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+26, 1734, 0, 631.671, -730.556, 155.593, 5.70723, 0, 0, -0.284015, 0.95882, 300, 300, 100, 1, 0, 0, 0, 10), (85100+27, 2047, 0, 631.671, -730.556, 155.593, 5.70723, 0, 0, -0.284015, 0.95882, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+17, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (387419, 5000+17, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+26, 5000+17, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+27, 5000+17, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+17, 9901, 0, 'Alterac Mountains - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (387420);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+28, 1734, 0, 892.592, -278.629, 152.45, 1.06465, 0, 0, 0.507538, 0.861629, 300, 300, 100, 1, 0, 0, 0, 10), (85100+29, 2047, 0, 892.592, -278.629, 152.45, 1.06465, 0, 0, 0.507538, 0.861629, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+18, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (387420, 5000+18, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+28, 5000+18, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+29, 5000+18, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+18, 9901, 0, 'Alterac Mountains - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (387422);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+30, 1734, 0, 654.105, -729.997, 160.305, -2.3911, 0, 0, 0.930418, -0.366501, 300, 300, 100, 1, 0, 0, 0, 10), (85100+31, 2047, 0, 654.105, -729.997, 160.305, -2.3911, 0, 0, 0.930418, -0.366501, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+19, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (387422, 5000+19, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+30, 5000+19, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+31, 5000+19, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+19, 9901, 0, 'Alterac Mountains - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427054);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+32, 2040, 0, 1200.03, -539.305, 77.9553, 3.47321, 0, 0, -0.986285, 0.16505, 300, 300, 100, 1, 0, 0, 0, 10), (85100+33, 2047, 0, 1200.03, -539.305, 77.9553, 3.47321, 0, 0, -0.986285, 0.16505, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+20, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (427054, 5000+20, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+32, 5000+20, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+33, 5000+20, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+20, 9901, 0, 'Alterac Mountains - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (2220, 220409);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+34, 1734, 0, 690.51, -623.184, 166.421, 3.45576, 0, 0, -0.987688, 0.156436, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+21, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (2220, 5000+21, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (220409, 5000+21, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10), (85100+34, 5000+21, 0, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+21, 9901, 0, 'Alterac Mountains - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_template` WHERE `entry` IN (407, 1069, 9003);

DELETE FROM `pool_pool` WHERE `pool_id` = 9003;

-- Pool Gold / Silver / Iron Spawns in Alterac Mountains.
 INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (9900, 12, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit (Master Pool)', 0, 0, 0, 10);

DELETE FROM `pool_gameobject` WHERE `guid` IN (6509);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+35, 1733, 0, 267.907, -277.657, 151.659, 5.96903, 0, 0, -0.156434, 0.987688, 300, 300, 100, 1, 0, 0, 0, 10), (85100+36, 1735, 0, 267.907, -277.657, 151.659, 5.96903, 0, 0, -0.156434, 0.987688, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+22, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (6509, 5000+22, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+35, 5000+22, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+36, 5000+22, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+22, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (6711);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+38, 1733, 0, 531.835, -1097.24, 154.998, 2.23402, 0, 0, 0.898793, 0.438373, 300, 300, 100, 1, 0, 0, 0, 10), (85100+39, 1734, 0, 531.835, -1097.24, 154.998, 2.23402, 0, 0, 0.898793, 0.438373, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+24, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (6711, 5000+24, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+38, 5000+24, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+39, 5000+24, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+24, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (33306);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+40, 1733, 0, 468.357, -160.365, 168.897, -1.39626, 0, 0, 0.642788, -0.766044, 300, 300, 100, 1, 0, 0, 0, 10), (85100+41, 1734, 0, 468.357, -160.365, 168.897, -1.39626, 0, 0, 0.642788, -0.766044, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+25, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (33306, 5000+25, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+40, 5000+25, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+41, 5000+25, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+25, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (33303);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+42, 1733, 0, 1037.81, -389.476, 69.1144, 2.04204, 0, 0, 0.85264, 0.522499, 300, 300, 100, 1, 0, 0, 0, 10), (85100+43, 1734, 0, 1037.81, -389.476, 69.1144, 2.04204, 0, 0, 0.85264, 0.522499, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+26, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (33303, 5000+26, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+42, 5000+26, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+43, 5000+26, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+26, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (33192);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+44, 1733, 0, 525.345, -378.22, 181.558, 0.994838, 0, 0, 0.477159, 0.878817, 300, 300, 100, 1, 0, 0, 0, 10), (85100+45, 1734, 0, 525.345, -378.22, 181.558, 0.994838, 0, 0, 0.477159, 0.878817, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+27, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (33192, 5000+27, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+44, 5000+27, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+45, 5000+27, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+27, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (33190);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+46, 1733, 0, 643.3, -732.88, 155.584, 2.53073, 0, 0, 0.953717, 0.300706, 300, 300, 100, 1, 0, 0, 0, 10), (85100+47, 1734, 0, 643.3, -732.88, 155.584, 2.53073, 0, 0, 0.953717, 0.300706, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+28, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (33190, 5000+28, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+46, 5000+28, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+47, 5000+28, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+28, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (33182);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+48, 1733, 0, 619.479, -685.931, 155.661, 1.0472, 0, 0, 0.5, 0.866025, 300, 300, 100, 1, 0, 0, 0, 10), (85100+49, 1734, 0, 619.479, -685.931, 155.661, 1.0472, 0, 0, 0.5, 0.866025, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+29, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (33182, 5000+29, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+48, 5000+29, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+49, 5000+29, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+29, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (33179);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+50, 1733, 0, 355.875, -130.356, 149.666, 0.820305, 0, 0, 0.398749, 0.91706, 300, 300, 100, 1, 0, 0, 0, 10), (85100+51, 1734, 0, 355.875, -130.356, 149.666, 0.820305, 0, 0, 0.398749, 0.91706, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+30, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (33179, 5000+30, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+50, 5000+30, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+51, 5000+30, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+30, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (32193);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+52, 1733, 0, 470.614, -992.025, 133.917, 3.10669, 0, 0, 0.999848, 0.017452, 300, 300, 100, 1, 0, 0, 0, 10), (85100+53, 1734, 0, 470.614, -992.025, 133.917, 3.10669, 0, 0, 0.999848, 0.017452, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+31, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (32193, 5000+31, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+52, 5000+31, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+53, 5000+31, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+31, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (32189);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+54, 1733, 0, 1050.18, -860.949, 104.537, 1.65806, 0, 0, 0.737277, 0.67559, 300, 300, 100, 1, 0, 0, 0, 10), (85100+55, 1734, 0, 1050.18, -860.949, 104.537, 1.65806, 0, 0, 0.737277, 0.67559, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+32, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (32189, 5000+32, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+54, 5000+32, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+55, 5000+32, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+32, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (35378);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+56, 1733, 0, 580.516, -996.459, 146.481, -0.872665, 0, 0, 0.422618, -0.906308, 300, 300, 100, 1, 0, 0, 0, 10), (85100+57, 1734, 0, 580.516, -996.459, 146.481, -0.872665, 0, 0, 0.422618, -0.906308, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+33, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (35378, 5000+33, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+56, 5000+33, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+57, 5000+33, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+33, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (35383);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+58, 1733, 0, 632.555, -697.682, 155.893, 1.88496, 0, 0, 0.809017, 0.587785, 300, 300, 100, 1, 0, 0, 0, 10), (85100+59, 1734, 0, 632.555, -697.682, 155.893, 1.88496, 0, 0, 0.809017, 0.587785, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+34, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (35383, 5000+34, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+58, 5000+34, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+59, 5000+34, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+34, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (35384);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+60, 1733, 0, 645.157, -650.045, 156.633, 1.93731, 0, 0, 0.824126, 0.566406, 300, 300, 100, 1, 0, 0, 0, 10), (85100+61, 1734, 0, 645.157, -650.045, 156.633, 1.93731, 0, 0, 0.824126, 0.566406, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+35, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (35384, 5000+35, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+60, 5000+35, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+61, 5000+35, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+35, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (35396);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+62, 1733, 0, 141.451, -364.934, 168.432, -0.2618, 0, 0, 0.130526, -0.991445, 300, 300, 100, 1, 0, 0, 0, 10), (85100+63, 1734, 0, 141.451, -364.934, 168.432, -0.2618, 0, 0, 0.130526, -0.991445, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+36, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (35396, 5000+36, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+62, 5000+36, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+63, 5000+36, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+36, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (35399);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+64, 1733, 0, 230.008, -255.811, 147.595, 0.314159, 0, 0, 0.156434, 0.987688, 300, 300, 100, 1, 0, 0, 0, 10), (85100+65, 1734, 0, 230.008, -255.811, 147.595, 0.314159, 0, 0, 0.156434, 0.987688, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+37, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (35399, 5000+37, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+64, 5000+37, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+65, 5000+37, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+37, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (35405);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+66, 1733, 0, 755.703, -136.77, 123.389, 0.541052, 0, 0, 0.267238, 0.96363, 300, 300, 100, 1, 0, 0, 0, 10), (85100+67, 1734, 0, 755.703, -136.77, 123.389, 0.541052, 0, 0, 0.267238, 0.96363, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+38, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (35405, 5000+38, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+66, 5000+38, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+67, 5000+38, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+38, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (10230);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+68, 1733, 0, 1216.42, -459.816, 69.9228, 2.47837, 0, 0, 0.945518, 0.325568, 300, 300, 100, 1, 0, 0, 0, 10), (85100+69, 1734, 0, 1216.42, -459.816, 69.9228, 2.47837, 0, 0, 0.945518, 0.325568, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+39, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (10230, 5000+39, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+68, 5000+39, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+69, 5000+39, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+39, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (33186);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+70, 1733, 0, 448.832, 7.38818, 140.777, 2.02458, 0, 0, 0.848047, 0.529921, 300, 300, 100, 1, 0, 0, 0, 10), (85100+71, 1734, 0, 448.832, 7.38818, 140.777, 2.02458, 0, 0, 0.848047, 0.529921, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+40, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (33186, 5000+40, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+70, 5000+40, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+71, 5000+40, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+40, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (10342);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+72, 1733, 0, 802.018, -298.852, 140.517, 0.95993, 0, 0, 0.461748, 0.887011, 300, 300, 100, 1, 0, 0, 0, 10), (85100+73, 1734, 0, 802.018, -298.852, 140.517, 0.95993, 0, 0, 0.461748, 0.887011, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+41, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (10342, 5000+41, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+72, 5000+41, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+73, 5000+41, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+41, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (10343);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+74, 1733, 0, 500.147, -106.041, 146.138, 4.10153, 0, 0, -0.887011, 0.461749, 300, 300, 100, 1, 0, 0, 0, 10), (85100+75, 1734, 0, 500.147, -106.041, 146.138, 4.10153, 0, 0, -0.887011, 0.461749, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+42, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (10343, 5000+42, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+74, 5000+42, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+75, 5000+42, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+42, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (10615);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+76, 1733, 0, 663.022, -590.838, 161.983, 0.767944, 0, 0, 0.374606, 0.927184, 300, 300, 100, 1, 0, 0, 0, 10), (85100+77, 1734, 0, 663.022, -590.838, 161.983, 0.767944, 0, 0, 0.374606, 0.927184, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+43, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (10615, 5000+43, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+76, 5000+43, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+77, 5000+43, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+43, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (35387);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+78, 1733, 0, 679.88, -652.475, 159.182, 2.47837, 0, 0, 0.945519, 0.325568, 300, 300, 100, 1, 0, 0, 0, 10), (85100+79, 1734, 0, 679.88, -652.475, 159.182, 2.47837, 0, 0, 0.945519, 0.325568, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+44, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (35387, 5000+44, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+78, 5000+44, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+79, 5000+44, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+44, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (389468);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+80, 1733, 0, 927.011, -342.925, 143.022, 1.53589, 0, 0, 0.694658, 0.71934, 300, 300, 100, 1, 0, 0, 0, 10), (85100+81, 1734, 0, 927.011, -342.925, 143.022, 1.53589, 0, 0, 0.694658, 0.71934, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+45, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (389468, 5000+45, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+80, 5000+45, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+81, 5000+45, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+45, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (389467);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+82, 1733, 0, 668.344, -416.631, 161.627, 0.872664, 0, 0, 0.422618, 0.906308, 300, 300, 100, 1, 0, 0, 0, 10), (85100+83, 1734, 0, 668.344, -416.631, 161.627, 0.872664, 0, 0, 0.422618, 0.906308, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+46, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (389467, 5000+46, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+82, 5000+46, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+83, 5000+46, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+46, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (389466);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+84, 1733, 0, 859.253, -547.027, 144.743, 1.93731, 0, 0, 0.824125, 0.566408, 300, 300, 100, 1, 0, 0, 0, 10), (85100+85, 1734, 0, 859.253, -547.027, 144.743, 1.93731, 0, 0, 0.824125, 0.566408, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+47, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (389466, 5000+47, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+84, 5000+47, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+85, 5000+47, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+47, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (389465);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+86, 1733, 0, 1127.2, -742.367, 79.6405, 1.83259, 0, 0, 0.793353, 0.608762, 300, 300, 100, 1, 0, 0, 0, 10), (85100+87, 1734, 0, 1127.2, -742.367, 79.6405, 1.83259, 0, 0, 0.793353, 0.608762, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+48, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (389465, 5000+48, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+86, 5000+48, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+87, 5000+48, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+48, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (389464);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+88, 1733, 0, 888.666, -574.497, 153.781, 1.27409, 0, 0, 0.594822, 0.803857, 300, 300, 100, 1, 0, 0, 0, 10), (85100+89, 1734, 0, 888.666, -574.497, 153.781, 1.27409, 0, 0, 0.594822, 0.803857, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+49, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (389464, 5000+49, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+88, 5000+49, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+89, 5000+49, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+49, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427121);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+90, 1733, 0, 235.168, -283.933, 148.17, 1.69297, 0, 0, 0.748956, 0.66262, 300, 300, 100, 1, 0, 0, 0, 10), (85100+91, 1734, 0, 235.168, -283.933, 148.17, 1.69297, 0, 0, 0.748956, 0.66262, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+50, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (427121, 5000+50, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+90, 5000+50, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+91, 5000+50, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+50, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427122);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+92, 1733, 0, 266.302, -508.268, 197.204, 4.03171, 0, 0, -0.902585, 0.430512, 300, 300, 100, 1, 0, 0, 0, 10), (85100+93, 1734, 0, 266.302, -508.268, 197.204, 4.03171, 0, 0, -0.902585, 0.430512, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+51, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (427122, 5000+51, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+92, 5000+51, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+93, 5000+51, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+51, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427123);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+94, 1733, 0, 361.617, -272.293, 174.191, 4.31097, 0, 0, -0.833885, 0.551938, 300, 300, 100, 1, 0, 0, 0, 10), (85100+95, 1734, 0, 361.617, -272.293, 174.191, 4.31097, 0, 0, -0.833885, 0.551938, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+52, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (427123, 5000+52, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+94, 5000+52, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+95, 5000+52, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+52, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427124);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+96, 1733, 0, 546.099, 25.8329, 132.257, 4.85202, 0, 0, -0.656058, 0.75471, 300, 300, 100, 1, 0, 0, 0, 10), (85100+97, 1734, 0, 546.099, 25.8329, 132.257, 4.85202, 0, 0, -0.656058, 0.75471, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+53, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (427124, 5000+53, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+96, 5000+53, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+97, 5000+53, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+53, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427125);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+98, 1733, 0, 660.074, -693.955, 163.728, 2.67035, 0, 0, 0.972369, 0.233448, 300, 300, 100, 1, 0, 0, 0, 10), (85100+99, 1734, 0, 660.074, -693.955, 163.728, 2.67035, 0, 0, 0.972369, 0.233448, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+54, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (427125, 5000+54, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+98, 5000+54, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+99, 5000+54, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+54, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427126);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+100, 1733, 0, 688.843, -1369.28, 111.343, 3.59538, 0, 0, -0.97437, 0.224951, 300, 300, 100, 1, 0, 0, 0, 10), (85100+101, 1734, 0, 688.843, -1369.28, 111.343, 3.59538, 0, 0, -0.97437, 0.224951, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+55, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (427126, 5000+55, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+100, 5000+55, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+101, 5000+55, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+55, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427127);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+102, 1733, 0, 716.587, -648.227, 170.116, 4.67748, 0, 0, -0.719339, 0.694659, 300, 300, 100, 1, 0, 0, 0, 10), (85100+103, 1734, 0, 716.587, -648.227, 170.116, 4.67748, 0, 0, -0.719339, 0.694659, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+56, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (427127, 5000+56, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+102, 5000+56, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+103, 5000+56, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+56, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427128);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+104, 1733, 0, 725.663, -1076.18, 157.488, 3.05433, 0, 0, 0.999048, 0.0436193, 300, 300, 100, 1, 0, 0, 0, 10), (85100+105, 1734, 0, 725.663, -1076.18, 157.488, 3.05433, 0, 0, 0.999048, 0.0436193, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+57, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (427128, 5000+57, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+104, 5000+57, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+105, 5000+57, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+57, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427129);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+106, 1733, 0, 781.768, -457.309, 146.165, 4.01426, 0, 0, -0.906307, 0.422619, 300, 300, 100, 1, 0, 0, 0, 10), (85100+107, 1734, 0, 781.768, -457.309, 146.165, 4.01426, 0, 0, -0.906307, 0.422619, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+58, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (427129, 5000+58, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+106, 5000+58, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+107, 5000+58, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+58, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427130);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+108, 1733, 0, 846.026, -1021.78, 144.245, 2.93214, 0, 0, 0.994521, 0.104536, 300, 300, 100, 1, 0, 0, 0, 10), (85100+109, 1734, 0, 846.026, -1021.78, 144.245, 2.93214, 0, 0, 0.994521, 0.104536, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+59, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (427130, 5000+59, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+108, 5000+59, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+109, 5000+59, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+59, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427131);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+110, 1733, 0, 868.787, -536.433, 146.763, 3.63029, 0, 0, -0.970295, 0.241925, 300, 300, 100, 1, 0, 0, 0, 10), (85100+111, 1734, 0, 868.787, -536.433, 146.763, 3.63029, 0, 0, -0.970295, 0.241925, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+60, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (427131, 5000+60, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+110, 5000+60, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+111, 5000+60, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+60, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427132);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+112, 1733, 0, 900.264, -560.414, 144.415, 5.09636, 0, 0, -0.559193, 0.829038, 300, 300, 100, 1, 0, 0, 0, 10), (85100+113, 1734, 0, 900.264, -560.414, 144.415, 5.09636, 0, 0, -0.559193, 0.829038, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+61, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (427132, 5000+61, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+112, 5000+61, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+113, 5000+61, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+61, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427133);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+114, 1733, 0, 943.097, -808.099, 126.39, 1.98967, 0, 0, 0.83867, 0.54464, 300, 300, 100, 1, 0, 0, 0, 10), (85100+115, 1734, 0, 943.097, -808.099, 126.39, 1.98967, 0, 0, 0.83867, 0.54464, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+62, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (427133, 5000+62, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+114, 5000+62, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+115, 5000+62, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+62, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220430);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+116, 1733, 0, -31.1745, -221.648, 138.074, -0.628319, 0, 0, 0.309017, -0.951056, 300, 300, 100, 1, 0, 0, 0, 10), (85100+117, 1734, 0, -31.1745, -221.648, 138.074, -0.628319, 0, 0, 0.309017, -0.951056, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+63, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220430, 5000+63, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+116, 5000+63, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+117, 5000+63, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+63, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (6410);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+118, 1733, 0, 669.41, -594, 164.72, 0.768, 0, 0, 0.374607, 0.927184, 300, 300, 100, 1, 0, 0, 0, 10), (85100+119, 1734, 0, 669.41, -594, 164.72, 0.768, 0, 0, 0.374607, 0.927184, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+23, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (6410, 5000+23, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+118, 5000+23, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+119, 5000+23, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+23, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (10583, 427013);

DELETE FROM `pool_template` WHERE `entry` IN (2907);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2907);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+120, 1734, 0, 779.732, -346.297, 151.973, 1.06465, 0, 0, 0.507538, 0.861629, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+64, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (10583, 5000+64, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (427013, 5000+64, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+120, 5000+64, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+64, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427153, 427002);

DELETE FROM `pool_template` WHERE `entry` IN (2908);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2908);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+121, 1734, 0, 995.454, -632.881, 124.128, 2.56563, 0, 0, 0.958819, 0.284016, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+65, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (427153, 5000+65, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (427002, 5000+65, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+121, 5000+65, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+65, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (32196, 427004);

DELETE FROM `pool_template` WHERE `entry` IN (2909);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2909);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+122, 1734, 0, 1302.34, -290.889, 67.2381, 0.994838, 0, 0, 0.477159, 0.878817, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+66, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (32196, 5000+66, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (427004, 5000+66, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+122, 5000+66, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+66, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (33290, 427019);

DELETE FROM `pool_template` WHERE `entry` IN (2910);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2910);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+123, 1734, 0, 627.304, -724.827, 188.74, 4.11898, 0, 0, -0.882947, 0.469473, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+67, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (33290, 5000+67, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (427019, 5000+67, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+123, 5000+67, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+67, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (31524, 35374);

DELETE FROM `pool_template` WHERE `entry` IN (2911);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2911);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+124, 1734, 0, 481.541, -1324.04, 79.9635, 6.16101, 0, 0, -0.0610485, 0.998135, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+68, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (31524, 5000+68, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (35374, 5000+68, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+124, 5000+68, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+68, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (10238, 31530);

DELETE FROM `pool_template` WHERE `entry` IN (2912);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2912);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+125, 1734, 0, 271.415, -259.615, 148.774, 2.1293, 0, 0, 0.874619, 0.48481, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+69, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (10238, 5000+69, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (31530, 5000+69, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+125, 5000+69, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+69, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (31527, 220431);

DELETE FROM `pool_template` WHERE `entry` IN (2913);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2913);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+126, 1734, 0, 545.352, -1686.1, 57.1031, 2.91469, 0, 0, 0.993571, 0.113208, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+70, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (31527, 5000+70, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (220431, 5000+70, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+126, 5000+70, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+70, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (43806, 43254);

DELETE FROM `pool_template` WHERE `entry` IN (2914);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2914);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+127, 1734, 0, -71.7572, -460.453, 102.154, 0.698131, 0, 0, 0.34202, 0.939693, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+71, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (43806, 5000+71, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (43254, 5000+71, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+127, 5000+71, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+71, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427134, 427023);

DELETE FROM `pool_template` WHERE `entry` IN (23026);

DELETE FROM `pool_pool` WHERE `pool_id` IN (23026);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+128, 1734, 0, 970.277, -916.3, 136.895, 2.37364, 0, 0, 0.927183, 0.374608, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+72, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (427134, 5000+72, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (427023, 5000+72, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+128, 5000+72, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+72, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (35403, 10622);

DELETE FROM `pool_template` WHERE `entry` IN (2916);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2916);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+129, 1733, 0, 243.813, -247.871, 147.019, 3.45576, 0, 0, -0.987688, 0.156436, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+73, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (35403, 5000+73, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (10622, 5000+73, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+129, 5000+73, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+73, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (14483, 427184);

DELETE FROM `pool_template` WHERE `entry` IN (2917);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2917);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+130, 1733, 0, 247.102, -286.261, 153.984, 1.23918, 0, 0, 0.580703, 0.814116, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+74, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (14483, 5000+74, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (427184, 5000+74, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+130, 5000+74, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+74, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (43803, 33185);

DELETE FROM `pool_template` WHERE `entry` IN (2918);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2918);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+131, 1733, 0, 264.654, -44.6943, 124.458, 5.11382, 0, 0, -0.551936, 0.833886, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+75, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (43803, 5000+75, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (33185, 5000+75, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+131, 5000+75, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+75, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (10572, 6575);

DELETE FROM `pool_template` WHERE `entry` IN (2919);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2919);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+132, 1733, 0, 412.711, -672.618, 179.944, 1.09956, 0, 0, 0.522498, 0.85264, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+76, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (10572, 5000+76, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (6575, 5000+76, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+132, 5000+76, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+76, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (33134, 6489);

DELETE FROM `pool_template` WHERE `entry` IN (2920);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2920);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+133, 1733, 0, 444.871, -1604.78, 68.803, 1.8326, 0, 0, 0.793353, 0.608761, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+77, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (33134, 5000+77, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (6489, 5000+77, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+133, 5000+77, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+77, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (32314, 6487);

DELETE FROM `pool_template` WHERE `entry` IN (2921);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2921);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+134, 1733, 0, 444.986, -906.441, 132.939, 5.34071, 0, 0, -0.45399, 0.891007, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+78, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (32314, 5000+78, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (6487, 5000+78, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+134, 5000+78, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+78, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (6488, 220429);

DELETE FROM `pool_template` WHERE `entry` IN (2922);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2922);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+135, 1733, 0, 502.952, -1391.69, 72.4759, 6.03884, 0, 0, -0.121869, 0.992546, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+79, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220429, 5000+79, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (6488, 5000+79, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+135, 5000+79, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+79, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (6460, 10341);

DELETE FROM `pool_template` WHERE `entry` IN (2923);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2923);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+136, 1733, 0, 550.242, -438.635, 170.444, 4.15388, 0, 0, -0.874619, 0.48481, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+80, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (10341, 5000+80, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (6460, 5000+80, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+136, 5000+80, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+80, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (6494, 43807);

DELETE FROM `pool_template` WHERE `entry` IN (2924);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2924);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+137, 1733, 0, 581.203, -703.03, 153.828, 6.16101, 0, 0, -0.0610485, 0.998135, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+81, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (43807, 5000+81, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (6494, 5000+81, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+137, 5000+81, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+81, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (12261, 10573);

DELETE FROM `pool_template` WHERE `entry` IN (2925);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2925);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+138, 1733, 0, 595.001, -651.02, 153.667, 4.20625, 0, 0, -0.861629, 0.507539, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+82, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (12261, 5000+82, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (10573, 5000+82, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+138, 5000+82, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+82, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (10571, 6420);

DELETE FROM `pool_template` WHERE `entry` IN (2926);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2926);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+139, 1733, 0, 617.05, -648.97, 154.328, 6.14356, 0, 0, -0.0697556, 0.997564, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+83, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (10571, 5000+83, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (6420, 5000+83, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+139, 5000+83, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+83, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (9907, 6517);

DELETE FROM `pool_template` WHERE `entry` IN (2927);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2927);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+140, 1733, 0, 648.067, -590.311, 164.735, 0.122173, 0, 0, 0.0610485, 0.998135, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+84, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (9907, 5000+84, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (6517, 5000+84, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+140, 5000+84, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+84, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (10254, 427055);

DELETE FROM `pool_template` WHERE `entry` IN (23047);

DELETE FROM `pool_pool` WHERE `pool_id` IN (23047);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+141, 1733, 0, 1220.72, -362.385, 56.5595, -1.25664, 0, 0, 0.587785, -0.809017, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+85, 1, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (10254, 5000+85, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (427055, 5000+85, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+141, 5000+85, 0, 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+85, 9900, 0, 'Alterac Mountains - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_template` WHERE `entry` IN (1353, 1068);

-- Pool Tin / Silver / Iron Spawns in Alterac Mountains.
 INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (9902, 4, 'Alterac Mountains - Tin Vein / Silver Vein / Iron Deposit (Master Pool)', 0, 0, 0, 10);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220423, 42202);

DELETE FROM `pool_template` WHERE `entry` IN (2901);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2901);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+142, 1733, 0, -161.117, 398.667, 105.474, 4.08407, 0, 0, -0.891006, 0.453991, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+86, 1, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220423, 5000+86, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (42202, 5000+86, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+142, 5000+86, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+86, 9902, 0, 'Alterac Mountains - Tin Vein / Silver Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (42204, 220424);

DELETE FROM `pool_template` WHERE `entry` IN (2902);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2902);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+143, 1733, 0, -130.354, -149.858, 120.737, 1.97222, 0, 0, 0.833885, 0.551938, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+87, 1, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (42204, 5000+87, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (220424, 5000+87, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+143, 5000+87, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+87, 9902, 0, 'Alterac Mountains - Tin Vein / Silver Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220425, 33198);

DELETE FROM `pool_template` WHERE `entry` IN (2903);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2903);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+144, 1733, 0, 12.0793, -148.168, 130.97, 3.3685, 0, 0, -0.993571, 0.113208, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+88, 1, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220425, 5000+88, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (33198, 5000+88, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+144, 5000+88, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+88, 9902, 0, 'Alterac Mountains - Tin Vein / Silver Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (42203, 220426);

DELETE FROM `pool_template` WHERE `entry` IN (2904);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2904);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+145, 1733, 0, 75.6466, -78.375, 114.581, 2.02458, 0, 0, 0.848047, 0.529921, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+89, 1, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (42203, 5000+89, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (220426, 5000+89, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+145, 5000+89, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+89, 9902, 0, 'Alterac Mountains - Tin Vein / Silver Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (33196, 220427);

DELETE FROM `pool_template` WHERE `entry` IN (2905);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2905);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+146, 1733, 0, 225.675, -72.2974, 110.061, 1.95477, 0, 0, 0.829038, 0.559193, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+90, 1, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (33196, 5000+90, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (220427, 5000+90, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+146, 5000+90, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+90, 9902, 0, 'Alterac Mountains - Tin Vein / Silver Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220428, 42200);

DELETE FROM `pool_template` WHERE `entry` IN (2906);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2906);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+147, 1733, 0, 244.997, -10.5061, 83.3397, 0.209439, 0, 0, 0.104528, 0.994522, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+91, 1, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (220428, 5000+91, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (42200, 5000+91, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+147, 5000+91, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+91, 9902, 0, 'Alterac Mountains - Tin Vein / Silver Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (31521, 42201);

DELETE FROM `pool_template` WHERE `entry` IN (2928);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2928);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+148, 1735, 0, -185.258, 375.062, 116.572, 1.02974, 0, 0, 0.492423, 0.870356, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+92, 1, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (31521, 5000+92, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (42201, 5000+92, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+148, 5000+92, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+92, 9902, 0, 'Alterac Mountains - Tin Vein / Silver Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (2405, 31522);

DELETE FROM `pool_template` WHERE `entry` IN (18870);

DELETE FROM `pool_pool` WHERE `pool_id` IN (18870);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+149, 1735, 0, -19.4989, -33.674, 106.887, 5.3058, 0, 0, -0.469471, 0.882948, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+93, 1, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (2405, 5000+93, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (31522, 5000+93, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+149, 5000+93, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+93, 9902, 0, 'Alterac Mountains - Tin Vein / Silver Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (396411, 31523, 2406);

DELETE FROM `pool_template` WHERE `entry` IN (2915);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2915);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+94, 1, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (396411, 5000+94, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (31523, 5000+94, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (2406, 5000+94, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+94, 9902, 0, 'Alterac Mountains - Tin Vein / Silver Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (2407, 2408, 220422);

DELETE FROM `pool_template` WHERE `entry` IN (18871);

DELETE FROM `pool_pool` WHERE `pool_id` IN (18871);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+95, 1, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (2407, 5000+95, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (2408, 5000+95, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (220422, 5000+95, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+95, 9902, 0, 'Alterac Mountains - Tin Vein / Silver Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (396412);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+150, 1733, 0, -41.0918, 415.415, 62.6462, 4.27606, 0, 0, -0.843391, 0.5373, 300, 300, 100, 1, 0, 0, 0, 10), (85100+151, 1735, 0, -41.0918, 415.415, 62.6462, 4.27606, 0, 0, -0.843391, 0.5373, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+96, 1, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (396412, 5000+96, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+150, 5000+96, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+151, 5000+96, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+96, 9902, 0, 'Alterac Mountains - Tin Vein / Silver Vein / Iron Deposit', 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427073, 42066);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (85100+152, 1735, 0, 1003.41, 96.0792, 18.3702, 1.309, 0, 0, 0.608761, 0.793354, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5000+97, 1, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (427073, 5000+97, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (42066, 5000+97, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10), (85100+152, 5000+97, 0, 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5000+97, 9902, 0, 'Alterac Mountains - Tin Vein / Silver Vein / Iron Deposit', 0);

DELETE FROM `pool_template` WHERE `entry` IN (397, 23056);

-- Add Chance
 -- Gold Vein / Mithril Deposit / Truesilver Deposit
 UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1734);

UPDATE `pool_gameobject` SET `chance` = 90 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 2040);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Alterac Mountains' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 2047);

-- Silver Vein / Gold Vein / Iron Deposit
 UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1733);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1734);

UPDATE `pool_gameobject` SET `chance` = 90 WHERE `description` = 'Silver Vein / Gold Vein / Iron Deposit - Alterac Mountains' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1735);

-- Tin Vein / Silver Vein / Iron Deposit
 UPDATE `pool_gameobject` SET `chance` = 55 WHERE `description` = 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1732);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1733);

UPDATE `pool_gameobject` SET `chance` = 40 WHERE `description` = 'Tin Vein / Silver Vein / Iron Deposit - Alterac Mountains' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1735);

-- End of migration.
;
