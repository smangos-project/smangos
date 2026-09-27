INSERT INTO `migrations` VALUES ('20250209160606');

-- Add your query below.
 -- Wetlands
 UPDATE `pool_template` SET `description` = 'Wetlands - Firefin Snapper School / Floating Wreckage / Oily Blackmouth School (Master Pool)' WHERE `entry` = 1971 AND `patch_min` = 7 AND `patch_max` = 10;

UPDATE `pool_template` SET `description` = 'Duskwood - Barrel of Melon Juice / Food Crate (Master Pool)' WHERE `entry` = 2082 AND `patch_min` = 0 AND `patch_max` = 10;

UPDATE `pool_template` SET `description` = 'Stranglethorn Vale - Greater Sagefish School / Floating Wreckage (Master Pool)' WHERE `entry` = 3647 AND `patch_min` = 7 AND `patch_max` = 10;

UPDATE `pool_template` SET `description` = 'Tanaris - Firefin Snapper School / Floating Wreckage / Oily Blackmouth School (Master Pool)' WHERE `entry` =4385 AND `patch_min` = 7 AND `patch_max` = 10;

UPDATE `pool_template` SET `description` = 'Desolace - Firefin Snapper School / Floating Wreckage / Oily Blackmouth School (Master Pool)' WHERE `entry` =4122 AND `patch_min` = 7 AND `patch_max` = 10;

UPDATE `pool_template` SET `description` = 'Copper Veins - Wetlands (Master Pool)' WHERE `entry` = 1156;

DELETE FROM `gameobject` WHERE `guid` IN (22913, 112820, 112821, 112822, 112823, 112824, 112825, 112826, 112827, 112828, 112829, 112830, 112831, 112832, 112833, 112834, 112835, 112836, 112837, 112838, 112839, 112840, 112841, 255259, 255260, 255261, 255262, 255263, 255264, 255265, 255266, 255267, 255268, 255269, 255270, 255271, 255272, 255273, 255274, 255275, 255276);

DELETE FROM `pool_gameobject` WHERE `guid` IN (22913, 112820, 112821, 112822, 112823, 112824, 112825, 112826, 112827, 112828, 112829, 112830, 112831, 112832, 112833, 112834, 112835, 112836, 112837, 112838, 112839, 112840, 112841, 255259, 255260, 255261, 255262, 255263, 255264, 255265, 255266, 255267, 255268, 255269, 255270, 255271, 255272, 255273, 255274, 255275, 255276);

DELETE FROM `pool_template` WHERE `entry` IN (1355, 17014, 17015, 17016, 17017, 17018, 17019, 17020, 17021, 17022, 17023, 17024, 22975, 22976, 22977, 22978, 22979, 22980, 22981, 22982, 22983);

DELETE FROM `pool_pool` WHERE `mother_pool` IN (1355);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74513+1, 1733, 0, -3420.22, -2121.09, 94.6608, 3.50812, 0, 0, -0.983254, 0.182238, 300, 300, 100, 1, 0, 0, 0, 10), (74513+2, 1733, 0, -3000.09, -3075, 75.7497, 1.0821, 0, 0, 0.515038, 0.857168, 300, 300, 100, 1, 0, 0, 0, 10), (74513+3, 1733, 0, -2808.98, -2934.11, 52.1863, 4.17134, 0, 0, -0.870356, 0.492424, 300, 300, 100, 1, 0, 0, 0, 10), (74513+4, 1733, 0, -2883.33, -3046.21, 63.1187, 5.93412, 0, 0, -0.173648, 0.984808, 300, 300, 100, 1, 0, 0, 0, 10), (74513+5, 1733, 0, -3085.14, -3260.26, 66.2112, 3.82227, 0, 0, -0.942641, 0.333808, 300, 300, 100, 1, 0, 0, 0, 10), (74513+6, 1733, 0, -2967.27, -3240.07, 54.206, 5.32326, 0, 0, -0.461748, 0.887011, 300, 300, 100, 1, 0, 0, 0, 10), (74513+7, 1733, 0, -2925.59, -3305.69, 54.2783, 3.4383, 0, 0, -0.989016, 0.147811, 300, 300, 100, 1, 0, 0, 0, 10), (74513+8, 1733, 0, -3003.79, -3222.2, 54.212, 1.32645, 0, 0, 0.615661, 0.788011, 300, 300, 100, 1, 0, 0, 0, 10), (74513+9, 1733, 0, -2906.9, -3236.44, 57.5807, 2.53072, 0, 0, 0.953716, 0.300708, 300, 300, 100, 1, 0, 0, 0, 10), (74513+10, 1733, 0, -2940.86, -3273.19, 61.9162, 1.48353, 0, 0, 0.67559, 0.737278, 300, 300, 100, 1, 0, 0, 0, 10), (74513+11, 1733, 0, -2935.62, -3243.31, 54.4793, 2.1293, 0, 0, 0.874619, 0.48481, 300, 300, 100, 1, 0, 0, 0, 10), (74513+12, 1733, 0, -2967.17, -3240.31, 54.206, -0.959931, 0, 0, 0.461749, -0.887011, 300, 300, 100, 1, 0, 0, 0, 10), (74513+13, 1733, 0, -2906.85, -3236.45, 57.6001, 2.53073, 0, 0, 0.953717, 0.300706, 300, 300, 100, 1, 0, 0, 0, 10), (74513+14, 1733, 0, -2843.21, -2995.76, 58.8304, 6.02139, 0, 0, -0.130526, 0.991445, 300, 300, 100, 1, 0, 0, 0, 10), (74513+15, 1733, 0, -3399.97, -1966.67, 121.708, 0.279252, 0, 0, 0.139173, 0.990268, 300, 300, 100, 1, 0, 0, 0, 10), (74513+16, 1733, 0, -3587.47, -2433.2, 89.9511, 4.60767, 0, 0, -0.743144, 0.669131, 300, 300, 100, 1, 0, 0, 0, 10), (74513+17, 1733, 0, -2729.08, -2895.95, 48.1052, -1.39626, 0, 0, 0.642788, -0.766044, 300, 300, 100, 1, 0, 0, 0, 10), (74513+18, 1733, 0, -2985.63, -3170.38, 58.5365, 4.95674, 0, 0, -0.615661, 0.788011, 300, 300, 100, 1, 0, 0, 0, 10), (74513+19, 1733, 0, -3584.03, -2588.48, 79.383, -1.0821, 0, 0, 0.515038, -0.857167, 300, 300, 100, 1, 0, 0, 0, 10), (74513+20, 1733, 0, -2973.01, -3330.81, 56.0359, -0.087267, 0, 0, 0.04362, -0.999048, 300, 300, 100, 1, 0, 0, 0, 10), (74513+21, 1733, 0, -2925.64, -3172.39, 57.2125, -0.366519, 0, 0, 0.182236, -0.983255, 300, 300, 100, 1, 0, 0, 0, 10), (74513+22, 1733, 0, -3533.33, -2399.93, 66.7631, 3.68265, 0, 0, -0.96363, 0.267241, 300, 300, 100, 1, 0, 0, 0, 10), (74513+23, 1734, 0, -3420.22, -2121.09, 94.6608, 3.50812, 0, 0, -0.983254, 0.182238, 300, 300, 100, 1, 0, 0, 0, 10), (74513+24, 1734, 0, -3000.09, -3075, 75.7497, 1.0821, 0, 0, 0.515038, 0.857168, 300, 300, 100, 1, 0, 0, 0, 10), (74513+25, 1734, 0, -2808.98, -2934.11, 52.1863, 4.17134, 0, 0, -0.870356, 0.492424, 300, 300, 100, 1, 0, 0, 0, 10), (74513+26, 1734, 0, -2883.33, -3046.21, 63.1187, 5.93412, 0, 0, -0.173648, 0.984808, 300, 300, 100, 1, 0, 0, 0, 10), (74513+27, 1734, 0, -3085.14, -3260.26, 66.2112, 3.82227, 0, 0, -0.942641, 0.333808, 300, 300, 100, 1, 0, 0, 0, 10), (74513+28, 1734, 0, -2967.27, -3240.07, 54.206, 5.32326, 0, 0, -0.461748, 0.887011, 300, 300, 100, 1, 0, 0, 0, 10), (74513+29, 1734, 0, -2925.59, -3305.69, 54.2783, 3.4383, 0, 0, -0.989016, 0.147811, 300, 300, 100, 1, 0, 0, 0, 10), (74513+30, 1734, 0, -3003.79, -3222.2, 54.212, 1.32645, 0, 0, 0.615661, 0.788011, 300, 300, 100, 1, 0, 0, 0, 10), (74513+31, 1734, 0, -2906.9, -3236.44, 57.5807, 2.53072, 0, 0, 0.953716, 0.300708, 300, 300, 100, 1, 0, 0, 0, 10), (74513+32, 1734, 0, -2940.86, -3273.19, 61.9162, 1.48353, 0, 0, 0.67559, 0.737278, 300, 300, 100, 1, 0, 0, 0, 10), (74513+33, 1734, 0, -2935.62, -3243.31, 54.4793, 2.1293, 0, 0, 0.874619, 0.48481, 300, 300, 100, 1, 0, 0, 0, 10), (74513+34, 1734, 0, -2967.17, -3240.31, 54.206, -0.959931, 0, 0, 0.461749, -0.887011, 300, 300, 100, 1, 0, 0, 0, 10), (74513+35, 1734, 0, -2906.85, -3236.45, 57.6001, 2.53073, 0, 0, 0.953717, 0.300706, 300, 300, 100, 1, 0, 0, 0, 10), (74513+36, 1734, 0, -2843.21, -2995.76, 58.8304, 6.02139, 0, 0, -0.130526, 0.991445, 300, 300, 100, 1, 0, 0, 0, 10), (74513+37, 1734, 0, -3399.97, -1966.67, 121.708, 0.279252, 0, 0, 0.139173, 0.990268, 300, 300, 100, 1, 0, 0, 0, 10), (74513+38, 1734, 0, -3587.47, -2433.2, 89.9511, 4.60767, 0, 0, -0.743144, 0.669131, 300, 300, 100, 1, 0, 0, 0, 10), (74513+39, 1734, 0, -2729.08, -2895.95, 48.1052, -1.39626, 0, 0, 0.642788, -0.766044, 300, 300, 100, 1, 0, 0, 0, 10), (74513+40, 1734, 0, -2985.63, -3170.38, 58.5365, 4.95674, 0, 0, -0.615661, 0.788011, 300, 300, 100, 1, 0, 0, 0, 10), (74513+41, 1734, 0, -3584.03, -2588.48, 79.383, -1.0821, 0, 0, 0.515038, -0.857167, 300, 300, 100, 1, 0, 0, 0, 10), (74513+42, 1734, 0, -2973.01, -3330.81, 56.0359, -0.087267, 0, 0, 0.04362, -0.999048, 300, 300, 100, 1, 0, 0, 0, 10), (74513+43, 1734, 0, -2925.64, -3172.39, 57.2125, -0.366519, 0, 0, 0.182236, -0.983255, 300, 300, 100, 1, 0, 0, 0, 10), (74513+44, 1734, 0, -3533.33, -2399.93, 66.7631, 3.68265, 0, 0, -0.96363, 0.267241, 300, 300, 100, 1, 0, 0, 0, 10), (74513+45, 1735, 0, -3420.22, -2121.09, 94.6608, 3.50812, 0, 0, -0.983254, 0.182238, 300, 300, 100, 1, 0, 0, 0, 10), (74513+46, 1735, 0, -3000.09, -3075, 75.7497, 1.0821, 0, 0, 0.515038, 0.857168, 300, 300, 100, 1, 0, 0, 0, 10), (74513+47, 1735, 0, -2808.98, -2934.11, 52.1863, 4.17134, 0, 0, -0.870356, 0.492424, 300, 300, 100, 1, 0, 0, 0, 10), (74513+48, 1735, 0, -2883.33, -3046.21, 63.1187, 5.93412, 0, 0, -0.173648, 0.984808, 300, 300, 100, 1, 0, 0, 0, 10), (74513+49, 1735, 0, -3085.14, -3260.26, 66.2112, 3.82227, 0, 0, -0.942641, 0.333808, 300, 300, 100, 1, 0, 0, 0, 10), (74513+50, 1735, 0, -2967.27, -3240.07, 54.206, 5.32326, 0, 0, -0.461748, 0.887011, 300, 300, 100, 1, 0, 0, 0, 10), (74513+51, 1735, 0, -2925.59, -3305.69, 54.2783, 3.4383, 0, 0, -0.989016, 0.147811, 300, 300, 100, 1, 0, 0, 0, 10), (74513+52, 1735, 0, -3003.79, -3222.2, 54.212, 1.32645, 0, 0, 0.615661, 0.788011, 300, 300, 100, 1, 0, 0, 0, 10), (74513+53, 1735, 0, -2906.9, -3236.44, 57.5807, 2.53072, 0, 0, 0.953716, 0.300708, 300, 300, 100, 1, 0, 0, 0, 10), (74513+54, 1735, 0, -2940.86, -3273.19, 61.9162, 1.48353, 0, 0, 0.67559, 0.737278, 300, 300, 100, 1, 0, 0, 0, 10), (74513+55, 1735, 0, -2935.62, -3243.31, 54.4793, 2.1293, 0, 0, 0.874619, 0.48481, 300, 300, 100, 1, 0, 0, 0, 10), (74513+56, 1735, 0, -2967.17, -3240.31, 54.206, -0.959931, 0, 0, 0.461749, -0.887011, 300, 300, 100, 1, 0, 0, 0, 10), (74513+57, 1735, 0, -2906.85, -3236.45, 57.6001, 2.53073, 0, 0, 0.953717, 0.300706, 300, 300, 100, 1, 0, 0, 0, 10), (74513+58, 1735, 0, -2843.21, -2995.76, 58.8304, 6.02139, 0, 0, -0.130526, 0.991445, 300, 300, 100, 1, 0, 0, 0, 10), (74513+59, 1735, 0, -3399.97, -1966.67, 121.708, 0.279252, 0, 0, 0.139173, 0.990268, 300, 300, 100, 1, 0, 0, 0, 10), (74513+60, 1735, 0, -3587.47, -2433.2, 89.9511, 4.60767, 0, 0, -0.743144, 0.669131, 300, 300, 100, 1, 0, 0, 0, 10), (74513+61, 1735, 0, -2729.08, -2895.95, 48.1052, -1.39626, 0, 0, 0.642788, -0.766044, 300, 300, 100, 1, 0, 0, 0, 10), (74513+62, 1735, 0, -2985.63, -3170.38, 58.5365, 4.95674, 0, 0, -0.615661, 0.788011, 300, 300, 100, 1, 0, 0, 0, 10), (74513+63, 1735, 0, -3584.03, -2588.48, 79.383, -1.0821, 0, 0, 0.515038, -0.857167, 300, 300, 100, 1, 0, 0, 0, 10), (74513+64, 1735, 0, -2973.01, -3330.81, 56.0359, -0.087267, 0, 0, 0.04362, -0.999048, 300, 300, 100, 1, 0, 0, 0, 10), (74513+65, 1735, 0, -2925.64, -3172.39, 57.2125, -0.366519, 0, 0, 0.182236, -0.983255, 300, 300, 100, 1, 0, 0, 0, 10), (74513+66, 1735, 0, -3533.33, -2399.93, 66.7631, 3.68265, 0, 0, -0.96363, 0.267241, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `patch_max`) VALUES (6112+1, 1, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (6112+2, 1, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (6112+3, 1, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (6112+4, 1, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (6112+5, 1, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (6112+6, 1, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (6112+7, 1, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (6112+8, 1, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (6112+9, 1, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (6112+10, 1, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (6112+11, 1, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (6112+12, 1, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (6112+13, 1, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (6112+14, 1, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (6112+15, 1, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (6112+16, 1, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (6112+17, 1, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (6112+18, 1, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (6112+19, 1, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (6112+20, 1, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (6112+21, 1, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (6112+22, 1, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`, `patch_max`) VALUES (74513+1, 6112+1, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+2, 6112+2, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+3, 6112+3, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+4, 6112+4, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+5, 6112+5, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+6, 6112+6, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+7, 6112+7, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+8, 6112+8, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+9, 6112+9, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+10, 6112+10, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+11, 6112+11, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+12, 6112+12, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+13, 6112+13, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+14, 6112+14, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+15, 6112+15, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+16, 6112+16, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+17, 6112+17, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+18, 6112+18, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+19, 6112+19, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+20, 6112+20, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+21, 6112+21, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+22, 6112+22, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+23, 6112+1, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+24, 6112+2, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+25, 6112+3, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+26, 6112+4, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+27, 6112+5, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+28, 6112+6, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+29, 6112+7, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+30, 6112+8, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+31, 6112+9, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+32, 6112+10, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+33, 6112+11, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+34, 6112+12, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+35, 6112+13, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+36, 6112+14, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+37, 6112+15, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+38, 6112+16, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+39, 6112+17, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+40, 6112+18, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+41, 6112+19, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+42, 6112+20, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+43, 6112+21, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+44, 6112+22, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+45, 6112+1, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+46, 6112+2, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+47, 6112+3, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+48, 6112+4, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+49, 6112+5, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+50, 6112+6, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+51, 6112+7, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+52, 6112+8, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+53, 6112+9, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+54, 6112+10, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+55, 6112+11, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+56, 6112+12, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+57, 6112+13, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+58, 6112+14, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+59, 6112+15, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+60, 6112+16, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+61, 6112+17, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+62, 6112+18, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+63, 6112+19, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+64, 6112+20, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+65, 6112+21, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10), (74513+66, 6112+22, 'Silver Vein / Gold Vein / Iron Deposit - Wetlands', 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (1355, 2, 'Wetlands - Silver Vein / Gold Vein / Iron Deposit (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6112+1, 1355, 0, 'Wetlands - Silver Vein / Gold Vein / Iron Deposit', 0), (6112+2, 1355, 0, 'Wetlands - Silver Vein / Gold Vein / Iron Deposit', 0), (6112+3, 1355, 0, 'Wetlands - Silver Vein / Gold Vein / Iron Deposit', 0), (6112+4, 1355, 0, 'Wetlands - Silver Vein / Gold Vein / Iron Deposit', 0), (6112+5, 1355, 0, 'Wetlands - Silver Vein / Gold Vein / Iron Deposit', 0), (6112+6, 1355, 0, 'Wetlands - Silver Vein / Gold Vein / Iron Deposit', 0), (6112+7, 1355, 0, 'Wetlands - Silver Vein / Gold Vein / Iron Deposit', 0), (6112+8, 1355, 0, 'Wetlands - Silver Vein / Gold Vein / Iron Deposit', 0), (6112+9, 1355, 0, 'Wetlands - Silver Vein / Gold Vein / Iron Deposit', 0), (6112+10, 1355, 0, 'Wetlands - Silver Vein / Gold Vein / Iron Deposit', 0), (6112+11, 1355, 0, 'Wetlands - Silver Vein / Gold Vein / Iron Deposit', 0), (6112+12, 1355, 0, 'Wetlands - Silver Vein / Gold Vein / Iron Deposit', 0), (6112+13, 1355, 0, 'Wetlands - Silver Vein / Gold Vein / Iron Deposit', 0), (6112+14, 1355, 0, 'Wetlands - Silver Vein / Gold Vein / Iron Deposit', 0), (6112+15, 1355, 0, 'Wetlands - Silver Vein / Gold Vein / Iron Deposit', 0), (6112+16, 1355, 0, 'Wetlands - Silver Vein / Gold Vein / Iron Deposit', 0), (6112+17, 1355, 0, 'Wetlands - Silver Vein / Gold Vein / Iron Deposit', 0), (6112+18, 1355, 0, 'Wetlands - Silver Vein / Gold Vein / Iron Deposit', 0), (6112+19, 1355, 0, 'Wetlands - Silver Vein / Gold Vein / Iron Deposit', 0), (6112+20, 1355, 0, 'Wetlands - Silver Vein / Gold Vein / Iron Deposit', 0), (6112+21, 1355, 0, 'Wetlands - Silver Vein / Gold Vein / Iron Deposit', 0), (6112+22, 1355, 0, 'Wetlands - Silver Vein / Gold Vein / Iron Deposit', 0);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Wetlands' WHERE `guid` IN (22871, 22910, 34146);

UPDATE `pool_pool` SET `mother_pool` = 1355, `description` = 'Wetlands - Silver Vein / Gold Vein / Iron Deposit' WHERE `pool_id` = 2820;

UPDATE `gameobject` SET `id` = 1733 WHERE `guid` = 22871;

UPDATE `gameobject` SET `id` = 1734 WHERE `guid` = 22910;

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Wetlands' WHERE `entry` = 2820;

-- Add Chance
 UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Silver Vein / Gold Vein / Iron Deposit - Wetlands' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1733);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Silver Vein / Gold Vein / Iron Deposit - Wetlands' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1734);

UPDATE `pool_gameobject` SET `chance` = 90 WHERE `description` = 'Silver Vein / Gold Vein / Iron Deposit - Wetlands' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1735);

UPDATE `pool_gameobject` SET `chance` = 95 WHERE `description` = 'Tin Vein / Silver Vein - Wetlands' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1732);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Tin Vein / Silver Vein - Wetlands' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1733);

-- Thelgen Rock
 DELETE FROM `gameobject` WHERE `guid` IN (58901, 58902, 58903, 58904, 58905, 58906, 58907, 58908, 58909, 58910, 58911, 58912, 58913, 58914, 58915, 58916, 58917, 58918, 58919, 58920, 58921, 58922, 58923, 58924, 58925, 58926, 58927, 58928, 58929, 58930, 58931, 58932, 58933, 58934, 58935, 58936, 58937, 58938, 58939, 58940, 58941, 58942, 58943, 58944, 58945, 58946, 58947, 58948, 58949, 58950, 58951, 58952, 58953, 58954, 58955, 58956, 58957, 58958, 58959, 58960, 58961, 58962, 58963, 58964, 58965, 58966, 58967, 58968, 58969, 58970, 58971, 58972, 58973, 58974, 58975, 58976, 58977, 58978, 58979, 58980, 58981, 58982, 58983, 58984, 58985, 58986, 58987, 58988, 58989, 58990, 58991, 58992, 58993, 58994, 58995, 58996, 58997, 58998, 58999, 59000, 59001, 59002, 59003, 59004, 59005, 59006, 59007, 59008);

DELETE FROM `pool_gameobject` WHERE `guid` IN (58901, 58902, 58903, 58904, 58905, 58906, 58907, 58908, 58909, 58910, 58911, 58912, 58913, 58914, 58915, 58916, 58917, 58918, 58919, 58920, 58921, 58922, 58923, 58924, 58925, 58926, 58927, 58928, 58929, 58930, 58931, 58932, 58933, 58934, 58935, 58936, 58937, 58938, 58939, 58940, 58941, 58942, 58943, 58944, 58945, 58946, 58947, 58948, 58949, 58950, 58951, 58952, 58953, 58954, 58955, 58956, 58957, 58958, 58959, 58960, 58961, 58962, 58963, 58964, 58965, 58966, 58967, 58968, 58969, 58970, 58971, 58972, 58973, 58974, 58975, 58976, 58977, 58978, 58979, 58980, 58981, 58982, 58983, 58984, 58985, 58986, 58987, 58988, 58989, 58990, 58991, 58992, 58993, 58994, 58995, 58996, 58997, 58998, 58999, 59000, 59001, 59002, 59003, 59004, 59005, 59006, 59007, 59008);

DELETE FROM `pool_template` WHERE `entry` IN (3660, 4601, 4602, 4603, 4604, 4605, 4606, 4607, 4608, 4609, 4610, 4611, 4612, 4613, 4614, 4615, 4616, 4617, 4618, 4619, 4620, 4621, 4622, 4623, 4624, 4625, 4626, 4627);

DELETE FROM `pool_pool` WHERE `mother_pool` IN (3660);

DELETE FROM `gameobject` WHERE `guid` = 118820;

DELETE FROM `pool_pool` WHERE `pool_id` = 7030;

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (7030, 9916, 0, 'Wetlands - Thelgen Rock - Tin Vein / Silver Vein', 0);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (74579+1, 1732, 0, -3886.07, -2401.82, 26.3338, 0.331613, 0, 0, 0.165048, 0.986286, 300, 300, 100, 1, 0, 0, 0, 10), (74579+2, 1732, 0, -3875.72, -2586.04, 44.1806, 2.74017, 0, 0, 0.979925, 0.199368, 300, 300, 100, 1, 0, 0, 0, 10), (74579+3, 1732, 0, -3874.11, -2340.36, 22.3002, -0.349066, 0, 0, 0.173648, -0.984808, 300, 300, 100, 1, 0, 0, 0, 10), (74579+4, 1732, 0, -3836.94, -2481.73, 35.6039, -0.069813, 0, 0, 0.034899, -0.999391, 300, 300, 100, 1, 0, 0, 0, 10), (74579+5, 1732, 0, -3829.84, -2420.19, 27.287, 2.67035, 0, 0, 0.97237, 0.233445, 300, 300, 100, 1, 0, 0, 0, 10), (74579+6, 1732, 0, -3809.23, -2396.89, 28.0163, -1.81514, 0, 0, 0.788011, -0.615661, 300, 300, 100, 1, 0, 0, 0, 10), (74579+7, 1732, 0, -3801.1, -2353.66, 33.2039, 0.034907, 0, 0, 0.017452, 0.999848, 300, 300, 100, 1, 0, 0, 0, 10), (74579+8, 1732, 0, -3756.49, -2480.01, 32.6535, 1.09956, 0, 0, 0.522499, 0.85264, 300, 300, 100, 1, 0, 0, 0, 10), (74579+9, 1732, 0, -3942.23, -2285.07, 17.1541, -0.2618, 0, 0, 0.130526, -0.991445, 300, 300, 100, 1, 0, 0, 0, 10), (74579+10, 1732, 0, -3880.72, -2459.3, 34.2359, 1.294, 0, 0, 0.602795, 0.797896, 300, 300, 100, 1, 0, 0, 0, 10), (74579+11, 1732, 0, -3830.66, -2501.42, 38.5958, 2.51327, 0, 0, 0.951057, 0.309017, 300, 300, 100, 1, 0, 0, 0, 10), (74579+12, 1732, 0, -3779.64, -2485.89, 26.3598, 3.624, 0, 0, 0.971051, -0.238872, 300, 300, 100, 1, 0, 0, 0, 10), (74579+13, 1733, 0, -3886.07, -2401.82, 26.3338, 0.331613, 0, 0, 0.165048, 0.986286, 300, 300, 100, 1, 0, 0, 0, 10), (74579+14, 1733, 0, -3875.72, -2586.04, 44.1806, 2.74017, 0, 0, 0.979925, 0.199368, 300, 300, 100, 1, 0, 0, 0, 10), (74579+15, 1733, 0, -3874.11, -2340.36, 22.3002, -0.349066, 0, 0, 0.173648, -0.984808, 300, 300, 100, 1, 0, 0, 0, 10), (74579+16, 1733, 0, -3836.94, -2481.73, 35.6039, -0.069813, 0, 0, 0.034899, -0.999391, 300, 300, 100, 1, 0, 0, 0, 10), (74579+17, 1733, 0, -3829.84, -2420.19, 27.287, 2.67035, 0, 0, 0.97237, 0.233445, 300, 300, 100, 1, 0, 0, 0, 10), (74579+18, 1733, 0, -3809.23, -2396.89, 28.0163, -1.81514, 0, 0, 0.788011, -0.615661, 300, 300, 100, 1, 0, 0, 0, 10), (74579+19, 1733, 0, -3801.1, -2353.66, 33.2039, 0.034907, 0, 0, 0.017452, 0.999848, 300, 300, 100, 1, 0, 0, 0, 10), (74579+20, 1733, 0, -3756.49, -2480.01, 32.6535, 1.09956, 0, 0, 0.522499, 0.85264, 300, 300, 100, 1, 0, 0, 0, 10), (74579+21, 1733, 0, -3942.23, -2285.07, 17.1541, -0.2618, 0, 0, 0.130526, -0.991445, 300, 300, 100, 1, 0, 0, 0, 10), (74579+22, 1733, 0, -3880.72, -2459.3, 34.2359, 1.294, 0, 0, 0.602795, 0.797896, 300, 300, 100, 1, 0, 0, 0, 10), (74579+23, 1733, 0, -3830.66, -2501.42, 38.5958, 2.51327, 0, 0, 0.951057, 0.309017, 300, 300, 100, 1, 0, 0, 0, 10), (74579+24, 1733, 0, -3779.64, -2485.89, 26.3598, 3.624, 0, 0, 0.971051, -0.238872, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `patch_max`) VALUES (4567+1, 1, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (4567+2, 1, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (4567+3, 1, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (4567+4, 1, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (4567+5, 1, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (4567+6, 1, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (4567+7, 1, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (4567+8, 1, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (4567+9, 1, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (4567+10, 1, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (4567+11, 1, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (4567+12, 1, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`, `patch_max`) VALUES (74579+1, 4567+1, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (74579+2, 4567+2, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (74579+3, 4567+3, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (74579+4, 4567+4, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (74579+5, 4567+5, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (74579+6, 4567+6, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (74579+7, 4567+7, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (74579+8, 4567+8, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (74579+9, 4567+9, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (74579+10, 4567+10, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (74579+11, 4567+11, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (74579+12, 4567+12, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (74579+13, 4567+1, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (74579+14, 4567+2, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (74579+15, 4567+3, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (74579+16, 4567+4, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (74579+17, 4567+5, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (74579+18, 4567+6, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (74579+19, 4567+7, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (74579+20, 4567+8, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (74579+21, 4567+9, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (74579+22, 4567+10, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (74579+23, 4567+11, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10), (74579+24, 4567+12, 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock', 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (9916, 3, 'Wetlands - Thelgen Rock - Tin Vein / Silver Vein (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (4567+1, 9916, 0, 'Wetlands - Thelgen Rock - Tin Vein / Silver Vein', 0), (4567+2, 9916, 0, 'Wetlands - Thelgen Rock - Tin Vein / Silver Vein', 0), (4567+3, 9916, 0, 'Wetlands - Thelgen Rock - Tin Vein / Silver Vein', 0), (4567+4, 9916, 0, 'Wetlands - Thelgen Rock - Tin Vein / Silver Vein', 0), (4567+5, 9916, 0, 'Wetlands - Thelgen Rock - Tin Vein / Silver Vein', 0), (4567+6, 9916, 0, 'Wetlands - Thelgen Rock - Tin Vein / Silver Vein', 0), (4567+7, 9916, 0, 'Wetlands - Thelgen Rock - Tin Vein / Silver Vein', 0), (4567+8, 9916, 0, 'Wetlands - Thelgen Rock - Tin Vein / Silver Vein', 0), (4567+9, 9916, 0, 'Wetlands - Thelgen Rock - Tin Vein / Silver Vein', 0), (4567+10, 9916, 0, 'Wetlands - Thelgen Rock - Tin Vein / Silver Vein', 0), (4567+11, 9916, 0, 'Wetlands - Thelgen Rock - Tin Vein / Silver Vein', 0), (4567+12, 9916, 0, 'Wetlands - Thelgen Rock - Tin Vein / Silver Vein', 0);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (68987+1, 1610, 0, -3929.86, -2318.63, 23.0019, 5.93412, 0, 0, -0.173648, 0.984808, 300, 300, 100, 1, 0, 0, 0, 10), (68987+2, 1610, 0, -3884.85, -2310.76, 25.4112, 2.89725, 0, 0, 0.992546, 0.121868, 300, 300, 100, 1, 0, 0, 0, 10), (68987+3, 1610, 0, -3870.25, -2383.63, 26.5792, 0.174532, 0, 0, 0.0871553, 0.996195, 300, 300, 100, 1, 0, 0, 0, 10), (68987+4, 1610, 0, -3864.54, -2323.02, 44.058, 1.0821, 0, 0, 0.515036, 0.857168, 300, 300, 100, 1, 0, 0, 0, 10), (68987+5, 1610, 0, -3863.12, -2476.62, 41.4935, -0.069813, 0, 0, -0.0348994, 0.999391, 300, 300, 100, 1, 0, 0, 0, 10), (68987+6, 1610, 0, -3837.11, -2527.33, 38.9886, -1.09956, 0, 0, -0.5225, 0.852639, 300, 300, 100, 1, 0, 0, 0, 10), (68987+7, 1610, 0, -3833.58, -2299.88, 30.4736, 4.46804, 0, 0, -0.788011, 0.615662, 300, 300, 100, 1, 0, 0, 0, 10), (68987+8, 1610, 0, -3830.57, -2431.7, 26.5262, 2.67035, 0, 0, 0.972369, 0.233447, 300, 300, 100, 1, 0, 0, 0, 10), (68987+9, 1610, 0, -3813.83, -2279.43, 33.753, -2.1293, 0, 0, -0.874619, 0.48481, 300, 300, 100, 1, 0, 0, 0, 10), (68987+10, 1610, 0, -3805.75, -2471.3, 27.4686, -0.785398, 0, 0, -0.382683, 0.92388, 300, 300, 100, 1, 0, 0, 0, 10), (68987+11, 1610, 0, -3799.52, -2390.45, 28.5253, 2.67035, 0, 0, 0.972369, 0.233448, 300, 300, 100, 1, 0, 0, 0, 10), (68987+12, 1610, 0, -3785.22, -2375.88, 30.6012, 3.31614, 0, 0, -0.996194, 0.087165, 300, 300, 100, 1, 0, 0, 0, 10), (68987+13, 1610, 0, -3758.25, -2456.14, 28.3123, 4.17134, 0, 0, -0.870356, 0.492424, 300, 300, 100, 1, 0, 0, 0, 10), (68987+14, 1610, 0, -3895.7, -2362.62, 22.8919, 0.994838, 0, 0, 0.477159, 0.878817, 300, 300, 100, 1, 0, 0, 0, 10), (68987+15, 1610, 0, -3877.42, -2484.44, 41.5919, 4.45059, 0, 0, -0.793353, 0.608762, 300, 300, 100, 1, 0, 0, 0, 10), (68987+16, 1667, 0, -3929.86, -2318.63, 23.0019, 5.93412, 0, 0, -0.173648, 0.984808, 300, 300, 100, 1, 0, 0, 0, 10), (68987+17, 1667, 0, -3884.85, -2310.76, 25.4112, 2.89725, 0, 0, 0.992546, 0.121868, 300, 300, 100, 1, 0, 0, 0, 10), (68987+18, 1667, 0, -3870.25, -2383.63, 26.5792, 0.174532, 0, 0, 0.0871553, 0.996195, 300, 300, 100, 1, 0, 0, 0, 10), (68987+19, 1667, 0, -3864.54, -2323.02, 44.058, 1.0821, 0, 0, 0.515036, 0.857168, 300, 300, 100, 1, 0, 0, 0, 10), (68987+20, 1667, 0, -3863.12, -2476.62, 41.4935, -0.069813, 0, 0, -0.0348994, 0.999391, 300, 300, 100, 1, 0, 0, 0, 10), (68987+21, 1667, 0, -3837.11, -2527.33, 38.9886, -1.09956, 0, 0, -0.5225, 0.852639, 300, 300, 100, 1, 0, 0, 0, 10), (68987+22, 1667, 0, -3833.58, -2299.88, 30.4736, 4.46804, 0, 0, -0.788011, 0.615662, 300, 300, 100, 1, 0, 0, 0, 10), (68987+23, 1667, 0, -3830.57, -2431.7, 26.5262, 2.67035, 0, 0, 0.972369, 0.233447, 300, 300, 100, 1, 0, 0, 0, 10), (68987+24, 1667, 0, -3813.83, -2279.43, 33.753, -2.1293, 0, 0, -0.874619, 0.48481, 300, 300, 100, 1, 0, 0, 0, 10), (68987+25, 1667, 0, -3805.75, -2471.3, 27.4686, -0.785398, 0, 0, -0.382683, 0.92388, 300, 300, 100, 1, 0, 0, 0, 10), (68987+26, 1667, 0, -3799.52, -2390.45, 28.5253, 2.67035, 0, 0, 0.972369, 0.233448, 300, 300, 100, 1, 0, 0, 0, 10), (68987+27, 1667, 0, -3785.22, -2375.88, 30.6012, 3.31614, 0, 0, -0.996194, 0.087165, 300, 300, 100, 1, 0, 0, 0, 10), (68987+28, 1667, 0, -3758.25, -2456.14, 28.3123, 4.17134, 0, 0, -0.870356, 0.492424, 300, 300, 100, 1, 0, 0, 0, 10), (68987+29, 1667, 0, -3895.7, -2362.62, 22.8919, 0.994838, 0, 0, 0.477159, 0.878817, 300, 300, 100, 1, 0, 0, 0, 10), (68987+30, 1667, 0, -3877.42, -2484.44, 41.5919, 4.45059, 0, 0, -0.793353, 0.608762, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `patch_max`) VALUES (6191+1, 1, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (6191+2, 1, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (6191+3, 1, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (6191+4, 1, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (6191+5, 1, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (6191+6, 1, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (6191+7, 1, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (6191+8, 1, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (6191+9, 1, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (6191+10, 1, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (6191+11, 1, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (6191+12, 1, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (6191+13, 1, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (6191+14, 1, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (6191+15, 1, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`, `patch_max`) VALUES (68987+1, 6191+1, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+2, 6191+2, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+3, 6191+3, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+4, 6191+4, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+5, 6191+5, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+6, 6191+6, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+7, 6191+7, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+8, 6191+8, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+9, 6191+9, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+10, 6191+10, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+11, 6191+11, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+12, 6191+12, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+13, 6191+13, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+14, 6191+14, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+15, 6191+15, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+16, 6191+1, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+17, 6191+2, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+18, 6191+3, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+19, 6191+4, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+20, 6191+5, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+21, 6191+6, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+22, 6191+7, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+23, 6191+8, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+24, 6191+9, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+25, 6191+10, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+26, 6191+11, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+27, 6191+12, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+28, 6191+13, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+29, 6191+14, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10), (68987+30, 6191+15, 'Incendicite Mineral Vein - Wetlands - Thelgen Rock', 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3257, 3, 'Wetlands - Thelgen Rock - Incendicite Mineral Vein (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6191+1, 3257, 0, 'Wetlands - Thelgen Rock - Incendicite Mineral Vein', 0), (6191+2, 3257, 0, 'Wetlands - Thelgen Rock - Incendicite Mineral Vein', 0), (6191+3, 3257, 0, 'Wetlands - Thelgen Rock - Incendicite Mineral Vein', 0), (6191+4, 3257, 0, 'Wetlands - Thelgen Rock - Incendicite Mineral Vein', 0), (6191+5, 3257, 0, 'Wetlands - Thelgen Rock - Incendicite Mineral Vein', 0), (6191+6, 3257, 0, 'Wetlands - Thelgen Rock - Incendicite Mineral Vein', 0), (6191+7, 3257, 0, 'Wetlands - Thelgen Rock - Incendicite Mineral Vein', 0), (6191+8, 3257, 0, 'Wetlands - Thelgen Rock - Incendicite Mineral Vein', 0), (6191+9, 3257, 0, 'Wetlands - Thelgen Rock - Incendicite Mineral Vein', 0), (6191+10, 3257, 0, 'Wetlands - Thelgen Rock - Incendicite Mineral Vein', 0), (6191+11, 3257, 0, 'Wetlands - Thelgen Rock - Incendicite Mineral Vein', 0), (6191+12, 3257, 0, 'Wetlands - Thelgen Rock - Incendicite Mineral Vein', 0), (6191+13, 3257, 0, 'Wetlands - Thelgen Rock - Incendicite Mineral Vein', 0), (6191+14, 3257, 0, 'Wetlands - Thelgen Rock - Incendicite Mineral Vein', 0), (6191+15, 3257, 0, 'Wetlands - Thelgen Rock - Incendicite Mineral Vein', 0);

UPDATE `pool_gameobject` SET `chance` = 95 WHERE `description` = 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1732);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1733);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (58826+1, 1734, 1, -4969.15185546875, -3877.734375000000, 57.78450012207031250, 1.815141916275024414, 0, 0, 0.788010597229003906, 0.615661680698394775, 300, 300, 100, 1, 0, 0, 0, 10), (58826+2, 1734, 1, -4979.04785156250, -3583.969970703125, 49.82926940917968750, 1.431168079376220703, 0, 0, 0.656058311462402343, 0.754710197448730468, 300, 300, 100, 1, 0, 0, 0, 10), (58826+3, 1734, 1, -4914.44580078125, -3295.939697265625, 56.76278686523437500, 1.117009282112121582, 0, 0, 0.529918670654296875, 0.848048448562622070, 300, 300, 100, 1, 0, 0, 0, 10), (58826+4, 1734, 1, -4903.81445312500, -3182.384521484375, 69.23558044433593750, 0.418878614902496337, 0, 0, 0.207911491394042968, 0.978147625923156738, 300, 300, 100, 1, 0, 0, 0, 10), (58826+5, 1734, 1, -4751.76855468750, -3116.785156250000, 67.46291351318359375, 1.343901276588439941, 0, 0, 0.622513771057128906, 0.782608807086944580, 300, 300, 100, 1, 0, 0, 0, 10), (58826+6, 1734, 1, -4441.83984375000, -3842.842041015625, 36.90475463867187500, 1.256635904312133789, 0, 0, 0.587784767150878906, 0.809017360210418701, 300, 300, 100, 1, 0, 0, 0, 10), (58826+7, 1734, 1, -4544.00976562500, -3974.585937500000, 48.42370223999023437, 2.164205789566040039, 0, 0, 0.882946968078613281, 0.469472706317901611, 300, 300, 100, 1, 0, 0, 0, 10), (58826+8, 1734, 1, -4834.18994140625, -4001.447509765625, 63.02287673950195312, 4.310965538024902343, 0, 0, -0.83388519287109375, 0.551937937736511230, 300, 300, 100, 1, 0, 0, 0, 10), (58826+9, 1734, 1, -4652.35742187500, -3061.899658203125, 51.58527755737304687, 1.658061861991882324, 0, 0, 0.737277030944824218, 0.675590515136718750, 300, 300, 100, 1, 0, 0, 0, 10), (58826+10, 1734, 1, -4639.07080078125, -2942.411376953125, 51.51849365234375000, 2.548179388046264648, 0, 0, 0.956304550170898437, 0.292372345924377441, 300, 300, 100, 1, 0, 0, 0, 10), (58826+11, 2040, 1, -4969.15185546875, -3877.734375000000, 57.78450012207031250, 1.815141916275024414, 0, 0, 0.788010597229003906, 0.615661680698394775, 300, 300, 100, 1, 0, 0, 0, 10), (58826+12, 2040, 1, -4979.04785156250, -3583.969970703125, 49.82926940917968750, 1.431168079376220703, 0, 0, 0.656058311462402343, 0.754710197448730468, 300, 300, 100, 1, 0, 0, 0, 10), (58826+13, 2040, 1, -4914.44580078125, -3295.939697265625, 56.76278686523437500, 1.117009282112121582, 0, 0, 0.529918670654296875, 0.848048448562622070, 300, 300, 100, 1, 0, 0, 0, 10), (58826+14, 2040, 1, -4903.81445312500, -3182.384521484375, 69.23558044433593750, 0.418878614902496337, 0, 0, 0.207911491394042968, 0.978147625923156738, 300, 300, 100, 1, 0, 0, 0, 10), (58826+15, 2040, 1, -4751.76855468750, -3116.785156250000, 67.46291351318359375, 1.343901276588439941, 0, 0, 0.622513771057128906, 0.782608807086944580, 300, 300, 100, 1, 0, 0, 0, 10), (58826+16, 2040, 1, -4441.83984375000, -3842.842041015625, 36.90475463867187500, 1.256635904312133789, 0, 0, 0.587784767150878906, 0.809017360210418701, 300, 300, 100, 1, 0, 0, 0, 10), (58826+17, 2040, 1, -4544.00976562500, -3974.585937500000, 48.42370223999023437, 2.164205789566040039, 0, 0, 0.882946968078613281, 0.469472706317901611, 300, 300, 100, 1, 0, 0, 0, 10), (58826+18, 2040, 1, -4834.18994140625, -4001.447509765625, 63.02287673950195312, 4.310965538024902343, 0, 0, -0.83388519287109375, 0.551937937736511230, 300, 300, 100, 1, 0, 0, 0, 10), (58826+19, 2040, 1, -4652.35742187500, -3061.899658203125, 51.58527755737304687, 1.658061861991882324, 0, 0, 0.737277030944824218, 0.675590515136718750, 300, 300, 100, 1, 0, 0, 0, 10), (58826+20, 2040, 1, -4639.07080078125, -2942.411376953125, 51.51849365234375000, 2.548179388046264648, 0, 0, 0.956304550170898437, 0.292372345924377441, 300, 300, 100, 1, 0, 0, 0, 10), (58826+21, 2047, 1, -4969.15185546875, -3877.734375000000, 57.78450012207031250, 1.815141916275024414, 0, 0, 0.788010597229003906, 0.615661680698394775, 300, 300, 100, 1, 0, 0, 0, 10), (58826+22, 2047, 1, -4979.04785156250, -3583.969970703125, 49.82926940917968750, 1.431168079376220703, 0, 0, 0.656058311462402343, 0.754710197448730468, 300, 300, 100, 1, 0, 0, 0, 10), (58826+23, 2047, 1, -4914.44580078125, -3295.939697265625, 56.76278686523437500, 1.117009282112121582, 0, 0, 0.529918670654296875, 0.848048448562622070, 300, 300, 100, 1, 0, 0, 0, 10), (58826+24, 2047, 1, -4903.81445312500, -3182.384521484375, 69.23558044433593750, 0.418878614902496337, 0, 0, 0.207911491394042968, 0.978147625923156738, 300, 300, 100, 1, 0, 0, 0, 10), (58826+25, 2047, 1, -4751.76855468750, -3116.785156250000, 67.46291351318359375, 1.343901276588439941, 0, 0, 0.622513771057128906, 0.782608807086944580, 300, 300, 100, 1, 0, 0, 0, 10), (58826+26, 2047, 1, -4441.83984375000, -3842.842041015625, 36.90475463867187500, 1.256635904312133789, 0, 0, 0.587784767150878906, 0.809017360210418701, 300, 300, 100, 1, 0, 0, 0, 10), (58826+27, 2047, 1, -4544.00976562500, -3974.585937500000, 48.42370223999023437, 2.164205789566040039, 0, 0, 0.882946968078613281, 0.469472706317901611, 300, 300, 100, 1, 0, 0, 0, 10), (58826+28, 2047, 1, -4834.18994140625, -4001.447509765625, 63.02287673950195312, 4.310965538024902343, 0, 0, -0.83388519287109375, 0.551937937736511230, 300, 300, 100, 1, 0, 0, 0, 10), (58826+29, 2047, 1, -4652.35742187500, -3061.899658203125, 51.58527755737304687, 1.658061861991882324, 0, 0, 0.737277030944824218, 0.675590515136718750, 300, 300, 100, 1, 0, 0, 0, 10), (58826+30, 2047, 1, -4639.07080078125, -2942.411376953125, 51.51849365234375000, 2.548179388046264648, 0, 0, 0.956304550170898437, 0.292372345924377441, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `patch_max`) VALUES (2453+1, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (2453+2, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (2453+3, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (2453+4, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (2453+5, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (2453+6, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (2453+7, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (2453+8, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (2453+9, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (2453+10, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`, `patch_max`) VALUES (58826+1, 2453+1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+2, 2453+2, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+3, 2453+3, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+4, 2453+4, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+5, 2453+5, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+6, 2453+6, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+7, 2453+7, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+8, 2453+8, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+9, 2453+9, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+10, 2453+10, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+11, 2453+1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+12, 2453+2, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+13, 2453+3, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+14, 2453+4, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+15, 2453+5, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+16, 2453+6, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+17, 2453+7, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+18, 2453+8, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+19, 2453+9, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+20, 2453+10, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+21, 2453+1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+22, 2453+2, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+23, 2453+3, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+24, 2453+4, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+25, 2453+5, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+26, 2453+6, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+27, 2453+7, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+28, 2453+8, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+29, 2453+9, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10), (58826+30, 2453+10, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh', 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (2453+1, 9903, 0, 'Dustwallow Marsh - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (2453+2, 9903, 0, 'Dustwallow Marsh - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (2453+3, 9903, 0, 'Dustwallow Marsh - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (2453+4, 9903, 0, 'Dustwallow Marsh - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (2453+5, 9903, 0, 'Dustwallow Marsh - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (2453+6, 9903, 0, 'Dustwallow Marsh - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (2453+7, 9903, 0, 'Dustwallow Marsh - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (2453+8, 9903, 0, 'Dustwallow Marsh - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (2453+9, 9903, 0, 'Dustwallow Marsh - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (2453+10, 9903, 0, 'Dustwallow Marsh - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (58856+1, 1734, 1, -4434.32031250000, -2957.626220703125, 26.79018974304199218, 4.223697185516357421, 0, 0, -0.85716724395751953, 0.515038192272186279, 600, 2700, 100, 1, 0, 0, 0, 10), (58856+2, 1734, 1, -4405.98535156250, -2952.440917968750, 23.67043495178222656, 5.009094715118408203, 0, 0, -0.59482288360595703, 0.803856790065765380, 600, 2700, 100, 1, 0, 0, 0, 10), (58856+3, 2040, 1, -4434.32031250000, -2957.626220703125, 26.79018974304199218, 4.223697185516357421, 0, 0, -0.85716724395751953, 0.515038192272186279, 600, 2700, 100, 1, 0, 0, 0, 10), (58856+4, 2040, 1, -4405.98535156250, -2952.440917968750, 23.67043495178222656, 5.009094715118408203, 0, 0, -0.59482288360595703, 0.803856790065765380, 600, 2700, 100, 1, 0, 0, 0, 10), (58856+5, 2047, 1, -4434.32031250000, -2957.626220703125, 26.79018974304199218, 4.223697185516357421, 0, 0, -0.85716724395751953, 0.515038192272186279, 600, 2700, 100, 1, 0, 0, 0, 10), (58856+6, 2047, 1, -4405.98535156250, -2952.440917968750, 23.67043495178222656, 5.009094715118408203, 0, 0, -0.59482288360595703, 0.803856790065765380, 600, 2700, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `patch_max`) VALUES (2773+1, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh - Den of Flames', 10), (2773+2, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh - Den of Flames', 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`, `patch_max`) VALUES (58856+1, 2773+1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh - Den of Flames', 10), (58856+2, 2773+2, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh - Den of Flames', 10), (58856+3, 2773+1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh - Den of Flames', 10), (58856+4, 2773+2, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh - Den of Flames', 10), (58856+5, 2773+1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh - Den of Flames', 10), (58856+6, 2773+2, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh - Den of Flames', 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (2773+1, 8104, 0, 'Dustwallow Marsh - Den of Flames - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (2773+2, 8104, 0, 'Dustwallow Marsh - Den of Flames - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `gameobject` WHERE `guid` IN (220414, 56033);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220414, 56033);

UPDATE `pool_pool` SET `mother_pool` = 9904, `description` = 'Dustwallow Marsh - Silver Vein / Gold Vein / Iron Deposit' WHERE `pool_id` = 6057;

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh' WHERE `guid` IN (90719, 90720, 90721);

UPDATE `gameobject` SET `id` = 1735 WHERE `guid` = 90721;

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh' WHERE `entry` = 6057;

UPDATE `pool_template` SET `max_limit` = 7 WHERE `entry` = 9904;

DELETE FROM `pool_template` WHERE `entry` = 9906;

-- Darkmist Cavern
 DELETE FROM `gameobject` WHERE `guid` IN (427137, 90696);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427137, 90696);

UPDATE `gameobject` SET `id` = 1733 WHERE `guid` = 90697;

UPDATE `gameobject` SET `id` = 1734 WHERE `guid` = 90698;

UPDATE `gameobject` SET `id` = 1735 WHERE `guid` = 90699;

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh - Darkmist Cavern' WHERE `entry` = 6051;

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh - Darkmist Cavern' WHERE `pool_entry` = 6051;

UPDATE `pool_pool` SET `description` = 'Dustwallow Marsh - Darkmist Cavern - Silver Vein / Gold Vein / Iron Deposit' WHERE `pool_id` = 6051;

DELETE FROM `gameobject` WHERE `guid` IN (55948, 90680);

DELETE FROM `pool_gameobject` WHERE `guid` IN (55948, 90680);

UPDATE `gameobject` SET `id` = 1733 WHERE `guid` = 90681;

UPDATE `gameobject` SET `id` = 1734 WHERE `guid` = 90682;

UPDATE `gameobject` SET `id` = 1735 WHERE `guid` = 90683;

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh - Darkmist Cavern' WHERE `entry` = 6047;

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh - Darkmist Cavern' WHERE `pool_entry` = 6047;

UPDATE `pool_pool` SET `description` = 'Dustwallow Marsh - Darkmist Cavern - Silver Vein / Gold Vein / Iron Deposit' WHERE `pool_id` = 6047;

DELETE FROM `gameobject` WHERE `guid` IN (14612, 90676);

DELETE FROM `pool_gameobject` WHERE `guid` IN (14612, 90676);

UPDATE `gameobject` SET `id` = 1733 WHERE `guid` = 90677;

UPDATE `gameobject` SET `id` = 1734 WHERE `guid` = 90678;

UPDATE `gameobject` SET `id` = 1735 WHERE `guid` = 90679;

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh - Darkmist Cavern' WHERE `entry` = 6046;

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh - Darkmist Cavern' WHERE `pool_entry` = 6046;

UPDATE `pool_pool` SET `description` = 'Dustwallow Marsh - Darkmist Cavern - Silver Vein / Gold Vein / Iron Deposit' WHERE `pool_id` = 6046;

DELETE FROM `gameobject` WHERE `guid` IN (55950, 90684);

DELETE FROM `pool_gameobject` WHERE `guid` IN (55950, 90684);

UPDATE `gameobject` SET `id` = 1733 WHERE `guid` = 90685;

UPDATE `gameobject` SET `id` = 1734 WHERE `guid` = 90686;

UPDATE `gameobject` SET `id` = 1735 WHERE `guid` = 90687;

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh - Darkmist Cavern' WHERE `entry` = 6048;

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh - Darkmist Cavern' WHERE `pool_entry` = 6048;

UPDATE `pool_pool` SET `description` = 'Dustwallow Marsh - Darkmist Cavern - Silver Vein / Gold Vein / Iron Deposit' WHERE `pool_id` = 6048;

DELETE FROM `gameobject` WHERE `guid` IN (427139, 90704);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427139, 90704);

UPDATE `gameobject` SET `id` = 1733 WHERE `guid` = 90705;

UPDATE `gameobject` SET `id` = 1734 WHERE `guid` = 90706;

UPDATE `gameobject` SET `id` = 1735 WHERE `guid` = 90707;

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh - Darkmist Cavern' WHERE `entry` = 6053;

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh - Darkmist Cavern' WHERE `pool_entry` = 6053;

UPDATE `pool_pool` SET `description` = 'Dustwallow Marsh - Darkmist Cavern - Silver Vein / Gold Vein / Iron Deposit' WHERE `pool_id` = 6053;

DELETE FROM `gameobject` WHERE `guid` IN (427138, 90700);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427138, 90700);

UPDATE `gameobject` SET `id` = 1733 WHERE `guid` = 90701;

UPDATE `gameobject` SET `id` = 1734 WHERE `guid` = 90702;

UPDATE `gameobject` SET `id` = 1735 WHERE `guid` = 90703;

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh - Darkmist Cavern' WHERE `entry` = 6052;

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh - Darkmist Cavern' WHERE `pool_entry` = 6052;

UPDATE `pool_pool` SET `description` = 'Dustwallow Marsh - Darkmist Cavern - Silver Vein / Gold Vein / Iron Deposit' WHERE `pool_id` = 6052;

DELETE FROM `gameobject` WHERE `guid` IN (2223, 220419);

DELETE FROM `pool_gameobject` WHERE `guid` IN (2223, 220419);

UPDATE `gameobject` SET `id` = 1734 WHERE `guid` = 90722;

UPDATE `gameobject` SET `id` = 2040 WHERE `guid` = 90723;

UPDATE `gameobject` SET `id` = 2047 WHERE `guid` = 90724;

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh - Darkmist Cavern' WHERE `entry` = 6058;

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh - Darkmist Cavern' WHERE `pool_entry` = 6058;

UPDATE `pool_pool` SET `description` = 'Dustwallow Marsh - Darkmist Cavern - Gold Vein / Mithril Deposit / Truesilver Deposit' WHERE `pool_id` = 6058;

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh - Darkmist Cavern' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1733);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh - Darkmist Cavern' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1734);

UPDATE `pool_gameobject` SET `chance` = 90 WHERE `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh - Darkmist Cavern' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1735);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh - Darkmist Cavern' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1734);

UPDATE `pool_gameobject` SET `chance` = 90 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh - Darkmist Cavern' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 2040);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh - Darkmist Cavern' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 2047);

-- Den of Flames
 DELETE FROM `gameobject` WHERE `guid` IN (56058, 90688);

DELETE FROM `pool_gameobject` WHERE `guid` IN (56058, 90688);

UPDATE `gameobject` SET `id` = 1733 WHERE `guid` = 90689;

UPDATE `gameobject` SET `id` = 1734 WHERE `guid` = 90690;

UPDATE `gameobject` SET `id` = 1735 WHERE `guid` = 90691;

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh - Den of Flames' WHERE `entry` = 6049;

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh - Den of Flames' WHERE `pool_entry` = 6049;

UPDATE `pool_pool` SET `description` = 'Dustwallow Marsh - Den of Flames - Silver Vein / Gold Vein / Iron Deposit' WHERE `pool_id` = 6049;

DELETE FROM `gameobject` WHERE `guid` IN (427135, 90692);

DELETE FROM `pool_gameobject` WHERE `guid` IN (427135, 90692);

UPDATE `gameobject` SET `id` = 1733 WHERE `guid` = 90693;

UPDATE `gameobject` SET `id` = 1734 WHERE `guid` = 90694;

UPDATE `gameobject` SET `id` = 1735 WHERE `guid` = 90695;

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh - Den of Flames' WHERE `entry` = 6050;

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh - Den of Flames' WHERE `pool_entry` = 6050;

UPDATE `pool_pool` SET `description` = 'Dustwallow Marsh - Den of Flames - Silver Vein / Gold Vein / Iron Deposit' WHERE `pool_id` = 6050;

DELETE FROM `gameobject` WHERE `guid` IN (56057, 90708);

DELETE FROM `pool_gameobject` WHERE `guid` IN (56057, 90708);

UPDATE `gameobject` SET `id` = 1733 WHERE `guid` = 90709;

UPDATE `gameobject` SET `id` = 1734 WHERE `guid` = 90710;

UPDATE `gameobject` SET `id` = 1735 WHERE `guid` = 90711;

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh - Den of Flames' WHERE `entry` = 6054;

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh - Den of Flames' WHERE `pool_entry` = 6054;

UPDATE `pool_pool` SET `description` = 'Dustwallow Marsh - Den of Flames - Silver Vein / Gold Vein / Iron Deposit' WHERE `pool_id` = 6054;

DELETE FROM `gameobject` WHERE `guid` IN (328, 220415);

DELETE FROM `pool_gameobject` WHERE `guid` IN (328, 220415);

UPDATE `gameobject` SET `id` = 1733 WHERE `guid` = 90716;

UPDATE `gameobject` SET `id` = 1734 WHERE `guid` = 90717;

UPDATE `gameobject` SET `id` = 1735 WHERE `guid` = 90718;

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh - Den of Flames' WHERE `entry` = 6056;

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh - Den of Flames' WHERE `pool_entry` = 6056;

UPDATE `pool_pool` SET `description` = 'Dustwallow Marsh - Den of Flames - Silver Vein / Gold Vein / Iron Deposit' WHERE `pool_id` = 6056;

DELETE FROM `gameobject` WHERE `guid` IN (220828, 90712);

DELETE FROM `pool_gameobject` WHERE `guid` IN (220828, 90712);

UPDATE `gameobject` SET `id` = 1734 WHERE `guid` = 90713;

UPDATE `gameobject` SET `id` = 2040 WHERE `guid` = 90714;

UPDATE `gameobject` SET `id` = 2047 WHERE `guid` = 90715;

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh - Den of Flames' WHERE `entry` = 6055;

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh - Den of Flames' WHERE `pool_entry` = 6055;

UPDATE `pool_pool` SET `description` = 'Dustwallow Marsh - Den of Flames - Gold Vein / Mithril Deposit / Truesilver Deposit' WHERE `pool_id` = 6055;

-- Add Chance
 UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh - Den of Flames' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1733);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh - Den of Flames' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1734);

UPDATE `pool_gameobject` SET `chance` = 90 WHERE `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh - Den of Flames' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1735);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh - Den of Flames' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1734);

UPDATE `pool_gameobject` SET `chance` = 90 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh - Den of Flames' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 2040);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh - Den of Flames' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 2047);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1734);

UPDATE `pool_gameobject` SET `chance` = 90 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 2040);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Dustwallow Marsh' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 2047);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1733);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1734);

UPDATE `pool_gameobject` SET `chance` = 90 WHERE `description` = 'Silver Vein / Gold Vein / Iron Deposit - Dustwallow Marsh' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1735);

-- End of migration.
;
