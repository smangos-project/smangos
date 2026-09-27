INSERT INTO `migrations` VALUES ('20260711200940');

-- Add your query below.
 -- Fadeleaf
 -- Add Missing
 INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (21, 2042, 1, -4201.8, -3419, 33.998, 1.0821, 0, 0, 0.515038, 0.857167, 300, 300, 100, 1, 0, 0, 0, 10), (248, 2042, 1, -4250.24, -3193.22, 38.5821, 1.3439, 0, 0, 0.622515, 0.782608, 300, 300, 100, 1, 0, 0, 0, 10), (254, 2042, 1, -4402.17, -3523.14, 37.8286, -2.04204, 0, 0, 0.85264, -0.522498, 300, 300, 100, 1, 0, 0, 0, 10), (261, 2042, 1, -4555.18, -2725.36, 40.8813, -0.994838, 0, 0, 0.477159, -0.878817, 300, 300, 100, 1, 0, 0, 0, 10), (262, 2042, 1, -3975.37, -3070.39, 36.4185, -1.23918, 0, 0, -0.580703, 0.814116, 300, 300, 100, 1, 0, 0, 0, 10), (263, 2042, 1, -4035.16, -3116.92, 41.9984, 3.57793, 0, 0, -0.976295, 0.216442, 300, 300, 100, 1, 0, 0, 0, 10), (1138, 2042, 0, -1738.67, -2272.35, 36.9894, -0.977383, 0, 0, -0.469471, 0.882948, 300, 300, 100, 1, 0, 0, 0, 10), (1139, 2042, 0, -13388.3, -364.414, 17.4184, 0.977383, 0, 0, 0.469471, 0.882948, 300, 300, 100, 1, 0, 0, 0, 10), (1141, 2042, 0, -13602, -30.4481, 40.0262, 0.383971, 0, 0, 0.190808, 0.981627, 300, 300, 100, 1, 0, 0, 0, 10), (1146, 2042, 0, -11740.4, 725.001, 51.4672, 1.41372, 0, 0, 0.649447, 0.760406, 300, 300, 100, 1, 0, 0, 0, 10), (1147, 2042, 0, 1140.04, -581.97, 80.4874, 2.51327, 0, 0, 0.951056, 0.309017, 300, 300, 100, 1, 0, 0, 0, 10), (1149, 2042, 0, 723.953, -854.599, 162.429, 6.02139, 0, 0, -0.130526, 0.991445, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (21, 1328, 0, 'Fadeleaf', 0, 0, 10), (248, 1328, 0, 'Fadeleaf', 0, 0, 10), (254, 1328, 0, 'Fadeleaf', 0, 0, 10), (261, 1328, 0, 'Fadeleaf', 0, 0, 10), (262, 1328, 0, 'Fadeleaf', 0, 0, 10), (263, 1328, 0, 'Fadeleaf', 0, 0, 10), (1138, 1183, 0, 'Fadeleaf', 0, 0, 10), (1139, 1172, 0, 'Fadeleaf', 0, 0, 10), (1141, 1172, 0, 'Fadeleaf', 0, 0, 10), (1146, 1172, 0, 'Fadeleaf', 0, 0, 10), (1147, 1071, 0, 'Fadeleaf', 0, 0, 10), (1149, 1071, 0, 'Fadeleaf', 0, 0, 10);

DELETE FROM `pool_template` WHERE `entry` = 1311;

DELETE FROM `pool_gameobject` WHERE `pool_entry` = 1311;

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3605+1, 1, 'Fadeleaf - Swamp of Sorrows - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (7586, 3605+1, 'Fadeleaf - Swamp of Sorrows - Group 1'), (7706, 3605+1, 'Fadeleaf - Swamp of Sorrows - Group 1'), (7737, 3605+1, 'Fadeleaf - Swamp of Sorrows - Group 1'), (7754, 3605+1, 'Fadeleaf - Swamp of Sorrows - Group 1'), (7830, 3605+1, 'Fadeleaf - Swamp of Sorrows - Group 1'), (28938, 3605+1, 'Fadeleaf - Swamp of Sorrows - Group 1');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3605+2, 1, 'Fadeleaf - Swamp of Sorrows - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (7643, 3605+2, 'Fadeleaf - Swamp of Sorrows - Group 2'), (7735, 3605+2, 'Fadeleaf - Swamp of Sorrows - Group 2'), (7767, 3605+2, 'Fadeleaf - Swamp of Sorrows - Group 2'), (7816, 3605+2, 'Fadeleaf - Swamp of Sorrows - Group 2'), (28936, 3605+2, 'Fadeleaf - Swamp of Sorrows - Group 2'), (30349, 3605+2, 'Fadeleaf - Swamp of Sorrows - Group 2');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3605+3, 1, 'Fadeleaf - Swamp of Sorrows - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (7673, 3605+3, 'Fadeleaf - Swamp of Sorrows - Group 3'), (7782, 3605+3, 'Fadeleaf - Swamp of Sorrows - Group 3'), (7802, 3605+3, 'Fadeleaf - Swamp of Sorrows - Group 3'), (7803, 3605+3, 'Fadeleaf - Swamp of Sorrows - Group 3'), (28937, 3605+3, 'Fadeleaf - Swamp of Sorrows - Group 3'), (32755, 3605+3, 'Fadeleaf - Swamp of Sorrows - Group 3');

DELETE FROM `pool_template` WHERE `entry` = 1183;

DELETE FROM `pool_gameobject` WHERE `pool_entry` = 1183;

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4969+1, 1, 'Fadeleaf - Arathi Highlands - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (7648, 4969+1, 'Fadeleaf - Arathi Highlands - Group 1'), (16813, 4969+1, 'Fadeleaf - Arathi Highlands - Group 1'), (28971, 4969+1, 'Fadeleaf - Arathi Highlands - Group 1'), (33378, 4969+1, 'Fadeleaf - Arathi Highlands - Group 1'), (68615, 4969+1, 'Fadeleaf - Arathi Highlands - Group 1'), (68618, 4969+1, 'Fadeleaf - Arathi Highlands - Group 1');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4969+2, 1, 'Fadeleaf - Arathi Highlands - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (1138, 4969+2, 'Fadeleaf - Arathi Highlands - Group 2'), (7612, 4969+2, 'Fadeleaf - Arathi Highlands - Group 2'), (16936, 4969+2, 'Fadeleaf - Arathi Highlands - Group 2'), (28969, 4969+2, 'Fadeleaf - Arathi Highlands - Group 2'), (35556, 4969+2, 'Fadeleaf - Arathi Highlands - Group 2'), (68616, 4969+2, 'Fadeleaf - Arathi Highlands - Group 2');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4969+3, 1, 'Fadeleaf - Arathi Highlands - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (16941, 4969+3, 'Fadeleaf - Arathi Highlands - Group 3'), (28970, 4969+3, 'Fadeleaf - Arathi Highlands - Group 3'), (33388, 4969+3, 'Fadeleaf - Arathi Highlands - Group 3'), (68614, 4969+3, 'Fadeleaf - Arathi Highlands - Group 3'), (68617, 4969+3, 'Fadeleaf - Arathi Highlands - Group 3'), (68946, 4969+3, 'Fadeleaf - Arathi Highlands - Group 3');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4969+4, 1, 'Fadeleaf - Arathi Highlands - Group 4 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (7587, 4969+4, 'Fadeleaf - Arathi Highlands - Group 4'), (7669, 4969+4, 'Fadeleaf - Arathi Highlands - Group 4'), (7793, 4969+4, 'Fadeleaf - Arathi Highlands - Group 4'), (28968, 4969+4, 'Fadeleaf - Arathi Highlands - Group 4'), (33381, 4969+4, 'Fadeleaf - Arathi Highlands - Group 4'), (35575, 4969+4, 'Fadeleaf - Arathi Highlands - Group 4');

DELETE FROM `pool_template` WHERE `entry` = 1285;

DELETE FROM `pool_gameobject` WHERE `pool_entry` = 1285;

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4973+1, 1, 'Fadeleaf - Badlands - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (7553, 4973+1, 'Fadeleaf - Badlands - Group 1'), (7638, 4973+1, 'Fadeleaf - Badlands - Group 1'), (7745, 4973+1, 'Fadeleaf - Badlands - Group 1'), (11486, 4973+1, 'Fadeleaf - Badlands - Group 1');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4973+2, 1, 'Fadeleaf - Badlands - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (7711, 4973+2, 'Fadeleaf - Badlands - Group 2'), (7821, 4973+2, 'Fadeleaf - Badlands - Group 2'), (11464, 4973+2, 'Fadeleaf - Badlands - Group 2'), (28935, 4973+2, 'Fadeleaf - Badlands - Group 2');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4973+3, 1, 'Fadeleaf - Badlands - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (7565, 4973+3, 'Fadeleaf - Badlands - Group 3'), (7626, 4973+3, 'Fadeleaf - Badlands - Group 3'), (7666, 4973+3, 'Fadeleaf - Badlands - Group 3'), (11466, 4973+3, 'Fadeleaf - Badlands - Group 3');

-- Grave Moss
 -- Add Missing
 INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (1150, 1628, 0, -10971.9, -1321.31, 52.3461, 2.51327, 0, 0, 0.951056, 0.309017, 300, 300, 100, 1, 0, 0, 0, 10), (1153, 1628, 0, -11005.3, -1334.97, 52.1094, 1.62316, 0, 0, 0.725374, 0.688355, 300, 300, 100, 1, 0, 0, 0, 10), (1159, 1628, 1, -2020.59, 1333.22, 63.6953, -2.54818, 0, 0, -0.956305, 0.292372, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (1159, 107, 0, 'Grave Moss', 0, 0, 10);

DELETE FROM `pool_template` WHERE `entry` = 1083;

DELETE FROM `pool_gameobject` WHERE `pool_entry` = 1083;

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (1082+1, 1, 'Grave Moss - Barrens - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (13383, 1082+1, 'Grave Moss - Barrens - Group 1'), (29150, 1082+1, 'Grave Moss - Barrens - Group 1'), (29151, 1082+1, 'Grave Moss - Barrens - Group 1'), (68705, 1082+1, 'Grave Moss - Barrens - Group 1');

DELETE FROM `pool_template` WHERE `entry` = 1060;

DELETE FROM `pool_gameobject` WHERE `pool_entry` = 1060;

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (1059+1, 1, 'Grave Moss - Alterac Mountains - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (4445, 1059+1, 'Grave Moss - Alterac Mountains - Group 1'), (32644, 1059+1, 'Grave Moss - Alterac Mountains - Group 1'), (29129, 1059+1, 'Grave Moss - Alterac Mountains - Group 1');

DELETE FROM `pool_template` WHERE `entry` = 105;

DELETE FROM `pool_gameobject` WHERE `pool_entry` = 105;

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4976+1, 1, 'Grave Moss - Duskwood - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (4452, 4976+1, 'Grave Moss - Duskwood - Group 1'), (14026, 4976+1, 'Grave Moss - Duskwood - Group 1'), (29117, 4976+1, 'Grave Moss - Duskwood - Group 1'), (29118, 4976+1, 'Grave Moss - Duskwood - Group 1'), (29123, 4976+1, 'Grave Moss - Duskwood - Group 1'), (68702, 4976+1, 'Grave Moss - Duskwood - Group 1');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4976+2, 1, 'Grave Moss - Duskwood - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (4465, 4976+2, 'Grave Moss - Duskwood - Group 2'), (4502, 4976+2, 'Grave Moss - Duskwood - Group 2'), (17892, 4976+2, 'Grave Moss - Duskwood - Group 2'), (29120, 4976+2, 'Grave Moss - Duskwood - Group 2'), (33483, 4976+2, 'Grave Moss - Duskwood - Group 2'), (68703, 4976+2, 'Grave Moss - Duskwood - Group 2');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4976+3, 1, 'Grave Moss - Duskwood - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (4473, 4976+3, 'Grave Moss - Duskwood - Group 3'), (1150, 4976+3, 'Grave Moss - Duskwood - Group 3'), (1153, 4976+3, 'Grave Moss - Duskwood - Group 3'), (29119, 4976+3, 'Grave Moss - Duskwood - Group 3'), (29121, 4976+3, 'Grave Moss - Duskwood - Group 3'), (29122, 4976+3, 'Grave Moss - Duskwood - Group 3');

DELETE FROM `pool_template` WHERE `entry` = 104;

DELETE FROM `pool_gameobject` WHERE `pool_entry` = 104;

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (103+1, 1, 'Grave Moss - Arathi Highlands - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (4469, 103+1, 'Grave Moss - Arathi Highlands - Group 1'), (16751, 103+1, 'Grave Moss - Arathi Highlands - Group 1'), (29130, 103+1, 'Grave Moss - Arathi Highlands - Group 1'), (29131, 103+1, 'Grave Moss - Arathi Highlands - Group 1'), (68706, 103+1, 'Grave Moss - Arathi Highlands - Group 1'), (68707, 103+1, 'Grave Moss - Arathi Highlands - Group 1');

-- End of migration.
;
