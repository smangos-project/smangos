INSERT INTO `migrations` VALUES ('20220905081210');

--  Missing spawns at Teldrassil
 INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (60042, 106318, 1, 9831.43, 1533.49, 1257.5, 1.36136, 0, 0, 0.62932, 0.777146, 300, 900, 100, 1, 0, 0, 0, 10), -- Banethil Barrow Den
 (60043, 106318, 1, 10511.2, 2002.39, 1327.38, 0.279252, 0, 0, 0.139173, 0.990268, 300, 900, 100, 1, 0, 0, 0, 10), -- The Oracle Glade
 (60044, 106318, 1, 10134.8, 1183.48, 1323.54, 5.37562, 0, 0, -0.438371, 0.898794, 300, 900, 100, 1, 0, 0, 0, 10), -- Fel Rock
 (60045, 106318, 1, 9631.6, 463.32, 1312.8, 2.67035, 0, 0, 0.972369, 0.233448, 300, 900, 100, 1, 0, 0, 0, 10), -- Camp East
 (60046, 106318, 1, 10751.3, 2214.46, 1331.51, 4.69494, 0, 0, -0.71325, 0.70091, 300, 900, 100, 1, 0, 0, 0, 10), -- The Oracle Glade
 (60047, 106318, 1, 9800.56, 1581.16, 1291.2, 5.65487, 0, 0, -0.309016, 0.951057, 300, 900, 100, 1, 0, 0, 0, 10), -- Banethil Barrow Den
 (60048, 106318, 1, 9730.49, 1576.73, 1269.44, 3.07177, 0, 0, 0.999391, 0.0349061, 300, 900, 100, 1, 0, 0, 0, 10), -- Banethil Barrow Den
 (60049, 106318, 1, 9776.05, 1547.38, 1299.49, 0.715585, 0, 0, 0.350207, 0.936672, 300, 900, 100, 1, 0, 0, 0, 10), -- Banethil Barrow Den
 (60050, 106318, 1, 10620.3, 2089.35, 1336.54, 4.7822, 0, 0, -0.681998, 0.731354, 300, 900, 100, 1, 0, 0, 0, 10), -- The Oracle Glade
 (60051, 106318, 1, 10890.4, 1968.21, 1321.79, 4.95674, 0, 0, -0.615661, 0.788011, 300, 900, 100, 1, 0, 0, 0, 10), -- The Oracle Glade
 (60052, 106318, 1, 10419.7, 1908.99, 1321.27, 2.61799, 0, 0, 0.965925, 0.258821, 300, 900, 100, 1, 0, 0, 0, 10), -- The Oracle Glade
 (60054, 106318, 1, 9673.75, 454.109, 1309.88, 1.64061, 0, 0, 0.731353, 0.681999, 300, 900, 100, 1, 0, 0, 0, 10), -- Camp East
 (60055, 106318, 1, 9183.7, 1372.79, 1315.07, 3.59538, 0, 0, -0.97437, 0.224951, 300, 900, 100, 1, 0, 0, 0, 10), -- Camp South
 (60056, 106318, 1, 10863.5, 2125.58, 1327.06, 2.63544, 0, 0, 0.968147, 0.250381, 300, 900, 100, 1, 0, 0, 0, 10);

-- The Oracle Glade
 -- Correct respawn times of existing chests in Teldrassil (source: CMangos)
 UPDATE `gameobject` SET `spawntimesecsmin` =300, `spawntimesecsmax` =900 WHERE `guid` IN (49621, 49622, 49623, 49624, 49625, 49626, 49627, 49628);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (20024, 1, 'Battered Chests (106318) - Teldrassil - Fel Rock', 0, 0, 0, 10), (20025, 1, 'Battered Chests (106318) - Teldrassil - Banethil Barrow Den', 0, 0, 0, 10), (20026, 1, 'Battered Chests (106318) - Teldrassil - Camp South', 0, 0, 0, 10), (20027, 1, 'Battered Chests (106318) - Teldrassil - Camp East', 0, 0, 0, 10), (20028, 1, 'Battered Chests (106318) - Teldrassil - The Oracle Glade', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (49628, 20024, 0, 'Battered Chests (106318) - Teldrassil - Fel Rock #1', 0, 0, 10), (60044, 20024, 0, 'Battered Chests (106318) - Teldrassil - Fel Rock #2', 0, 0, 10), (49627, 20025, 0, 'Battered Chests (106318) - Teldrassil - Banethil Barrow Den #1', 0, 0, 10), (60042, 20025, 0, 'Battered Chests (106318) - Teldrassil - Banethil Barrow Den #2', 0, 0, 10), (60047, 20025, 0, 'Battered Chests (106318) - Teldrassil - Banethil Barrow Den #3', 0, 0, 10), (60048, 20025, 0, 'Battered Chests (106318) - Teldrassil - Banethil Barrow Den #4', 0, 0, 10), (60049, 20025, 0, 'Battered Chests (106318) - Teldrassil - Banethil Barrow Den #5', 0, 0, 10), (49626, 20026, 0, 'Battered Chests (106318) - Teldrassil - Camp South #1', 0, 0, 10), (49624, 20026, 0, 'Battered Chests (106318) - Teldrassil - Camp South #2', 0, 0, 10), (60055, 20026, 0, 'Battered Chests (106318) - Teldrassil - Camp South #3', 0, 0, 10), (49622, 20027, 0, 'Battered Chests (106318) - Teldrassil - Camp East #1', 0, 0, 10), (60045, 20027, 0, 'Battered Chests (106318) - Teldrassil - Camp East #2', 0, 0, 10), (60054, 20027, 0, 'Battered Chests (106318) - Teldrassil - Camp East #3', 0, 0, 10), (49621, 20028, 0, 'Battered Chests (106318) - Teldrassil - The Oracle Glade #1', 0, 0, 10), (49623, 20028, 0, 'Battered Chests (106318) - Teldrassil - The Oracle Glade #2', 0, 0, 10), (49625, 20028, 0, 'Battered Chests (106318) - Teldrassil - The Oracle Glade #3', 0, 0, 10), (60043, 20028, 0, 'Battered Chests (106318) - Teldrassil - The Oracle Glade #4', 0, 0, 10), (60050, 20028, 0, 'Battered Chests (106318) - Teldrassil - The Oracle Glade #5', 0, 0, 10), (60051, 20028, 0, 'Battered Chests (106318) - Teldrassil - The Oracle Glade #6', 0, 0, 10), (60052, 20028, 0, 'Battered Chests (106318) - Teldrassil - The Oracle Glade #7', 0, 0, 10), (60056, 20028, 0, 'Battered Chests (106318) - Teldrassil - The Oracle Glade #8', 0, 0, 10);

-- End of migration.
;
