INSERT INTO `migrations` VALUES ('20260406075700');

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (246+0, 1732, 0, -2596.69970703125, -1893.5811767578125, 20.57378578186035156, 1.099556446075439453, 0, 0, 0.522498130798339843, 0.852640450000762939, 300, 900, 100, 1, 0, 0, 0, 10), (246+1, 1733, 0, -2596.69970703125, -1893.5811767578125, 20.57378578186035156, 1.099556446075439453, 0, 0, 0.522498130798339843, 0.852640450000762939, 300, 900, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (298+0, 1, 'Tin Vein / Silver Vein - Wetlands', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (246+0, 298+0, 95, 'Tin Vein / Silver Vein - Wetlands', 0, 0, 10), (246+1, 298+0, 5, 'Tin Vein / Silver Vein - Wetlands', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (298+0, 1348, 0, 'Wetlands - Tin Vein / Silver Vein', 0);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (4009+0, 1732, 0, -444.3253173828125, 1074.275390625, 92.75608062744140625, 5.218535900115966796, 0, 0, -0.507537841796875, 0.861629426479339599, 300, 900, 100, 1, 0, 0, 0, 10), (4009+1, 1733, 0, -444.3253173828125, 1074.275390625, 92.75608062744140625, 5.218535900115966796, 0, 0, -0.507537841796875, 0.861629426479339599, 300, 900, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (325+0, 1, 'Tin Vein / Silver Vein - Silverpine Forest', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (4009+0, 325+0, 95, 'Tin Vein / Silver Vein - Silverpine Forest', 0, 0, 10), (4009+1, 325+0, 5, 'Tin Vein / Silver Vein - Silverpine Forest', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (325+0, 1354, 0, 'Silverpine Forest - Tin Vein / Silver Vein', 0);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (4011+0, 1732, 1, 7420.14501953125, -789.52685546875, 16.9228973388671875, 5.637413978576660156, 0, 0, -0.31730461120605468, 0.948323667049407958, 300, 900, 100, 1, 0, 0, 0, 10), (4011+1, 1733, 1, 7420.14501953125, -789.52685546875, 16.9228973388671875, 5.637413978576660156, 0, 0, -0.31730461120605468, 0.948323667049407958, 300, 900, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (402+0, 1, 'Tin Vein / Silver Vein - Darkshore', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (4011+0, 402+0, 95, 'Tin Vein / Silver Vein - Darkshore', 0, 0, 10), (4011+1, 402+0, 5, 'Tin Vein / Silver Vein - Darkshore', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (402+0, 1099, 0, 'Darkshore - Tin Vein / Silver Vein', 0);

UPDATE `gameobject` SET `position_x` = 7968.3974609375, `position_y` = -1087.4107666015625, `position_z` = 44.57505416870117187, `orientation` = 0.122172988951206207, `rotation2` = 0.061048507690429687, `rotation3` = 0.998134791851043701 WHERE `guid` IN (365548, 365584);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (4013+0, 1732, 1, 954.4083251953125, -352.142669677734375, 5.487726211547851562, 3.94444584846496582, 0, 0, -0.92050457000732421, 0.3907318115234375, 300, 900, 100, 1, 0, 0, 0, 10), (4013+1, 1733, 1, 954.4083251953125, -352.142669677734375, 5.487726211547851562, 3.94444584846496582, 0, 0, -0.92050457000732421, 0.3907318115234375, 300, 900, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (406+0, 1, 'Tin Vein / Silver Vein - Stonetalon Mountains - Windshear Mine', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (4013+0, 406+0, 95, 'Tin Vein / Silver Vein - Stonetalon Mountains - Windshear Mine', 0, 0, 10), (4013+1, 406+0, 5, 'Tin Vein / Silver Vein - Stonetalon Mountains - Windshear Mine', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (406+0, 386, 0, 'Stonetalon Mountains - Windshear Mine - Tin Vein / Silver Vein', 0);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (4015+0, 1732, 1, 962.618408203125, -270.985687255859375, -1.75297904014587402, 1.134462952613830566, 0, 0, 0.537299156188964843, 0.843391716480255126, 300, 900, 100, 1, 0, 0, 0, 10), (4015+1, 1733, 1, 962.618408203125, -270.985687255859375, -1.75297904014587402, 1.134462952613830566, 0, 0, 0.537299156188964843, 0.843391716480255126, 300, 900, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (412+0, 1, 'Tin Vein / Silver Vein - Stonetalon Mountains - Windshear Mine', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (4015+0, 412+0, 95, 'Tin Vein / Silver Vein - Stonetalon Mountains - Windshear Mine', 0, 0, 10), (4015+1, 412+0, 5, 'Tin Vein / Silver Vein - Stonetalon Mountains - Windshear Mine', 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (412+0, 386, 0, 'Stonetalon Mountains - Windshear Mine - Tin Vein / Silver Vein', 0);

-- Correct Patch For Dark Iron Inside Instances
 UPDATE `gameobject` SET `patch_min` = 0 WHERE `id` =165658 AND (`map` = 230 OR `map` = 409);

UPDATE `pool_gameobject` SET `patch_min` = 0 WHERE `pool_entry` = 25622 OR `pool_entry` = 25609;

update `pool_template` SET `patch_min` = 0 WHERE `entry` = 25609 OR `entry` = 25622;

-- Remove Rogue Plaguebloom
 DELETE FROM `gameobject` WHERE `guid` = 20108;

DELETE FROM `pool_gameobject` WHERE `guid` = 20108;

-- End of migration.
;
