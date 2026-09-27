INSERT INTO `migrations` VALUES ('20260410164900');

DELETE FROM `pool_template` WHERE `entry` = 1139;

DELETE FROM `pool_gameobject` WHERE `pool_entry` = 1139;

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3557+1, 2, 'Copper Vein - Desolace - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (4737, 3557+1, 'Copper Vein - Desolace - Group 1'), (20647, 3557+1, 'Copper Vein - Desolace - Group 1'), (34182, 3557+1, 'Copper Vein - Desolace - Group 1'), (34159, 3557+1, 'Copper Vein - Desolace - Group 1'), (34185, 3557+1, 'Copper Vein - Desolace - Group 1');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3557+2, 2, 'Copper Vein - Desolace - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (20646, 3557+2, 'Copper Vein - Desolace - Group 2'), (5047, 3557+2, 'Copper Vein - Desolace - Group 2'), (20645, 3557+2, 'Copper Vein - Desolace - Group 2'), (20659, 3557+2, 'Copper Vein - Desolace - Group 2'), (5416, 3557+2, 'Copper Vein - Desolace - Group 2'), (5443, 3557+2, 'Copper Vein - Desolace - Group 2');

DELETE FROM `pool_template` WHERE `entry` = 1066;

DELETE FROM `pool_gameobject` WHERE `pool_entry` = 1066;

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4814+1, 2, 'Copper Vein - Hillsbrad Foothills - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (21274, 4814+1, 'Copper Vein - Hillsbrad Foothills - Group 1'), (21257, 4814+1, 'Copper Vein - Hillsbrad Foothills - Group 1'), (21256, 4814+1, 'Copper Vein - Hillsbrad Foothills - Group 1'), (21259, 4814+1, 'Copper Vein - Hillsbrad Foothills - Group 1'), (21273, 4814+1, 'Copper Vein - Hillsbrad Foothills - Group 1'), (5222, 4814+1, 'Copper Vein - Hillsbrad Foothills - Group 1');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4814+2, 2, 'Copper Vein - Hillsbrad Foothills - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (5391, 4814+2, 'Copper Vein - Hillsbrad Foothills - Group 2'), (5380, 4814+2, 'Copper Vein - Hillsbrad Foothills - Group 2'), (21271, 4814+2, 'Copper Vein - Hillsbrad Foothills - Group 2'), (21250, 4814+2, 'Copper Vein - Hillsbrad Foothills - Group 2'), (18263, 4814+2, 'Copper Vein - Hillsbrad Foothills - Group 2'), (34265, 4814+2, 'Copper Vein - Hillsbrad Foothills - Group 2');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4814+3, 2, 'Copper Vein - Hillsbrad Foothills - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (18264, 4814+3, 'Copper Vein - Hillsbrad Foothills - Group 3'), (34278, 4814+3, 'Copper Vein - Hillsbrad Foothills - Group 3'), (25046, 4814+3, 'Copper Vein - Hillsbrad Foothills - Group 3'), (21276, 4814+3, 'Copper Vein - Hillsbrad Foothills - Group 3'), (27640, 4814+3, 'Copper Vein - Hillsbrad Foothills - Group 3'), (27641, 4814+3, 'Copper Vein - Hillsbrad Foothills - Group 3');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4814+4, 2, 'Copper Vein - Hillsbrad Foothills - Group 4 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (34238, 4814+4, 'Copper Vein - Hillsbrad Foothills - Group 4'), (35356, 4814+4, 'Copper Vein - Hillsbrad Foothills - Group 4'), (30030, 4814+4, 'Copper Vein - Hillsbrad Foothills - Group 4'), (34233, 4814+4, 'Copper Vein - Hillsbrad Foothills - Group 4'), (34279, 4814+4, 'Copper Vein - Hillsbrad Foothills - Group 4'), (29993, 4814+4, 'Copper Vein - Hillsbrad Foothills - Group 4');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4814+5, 2, 'Copper Vein - Hillsbrad Foothills - Group 5 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (25045, 4814+5, 'Copper Vein - Hillsbrad Foothills - Group 5'), (25047, 4814+5, 'Copper Vein - Hillsbrad Foothills - Group 5'), (21237, 4814+5, 'Copper Vein - Hillsbrad Foothills - Group 5'), (21277, 4814+5, 'Copper Vein - Hillsbrad Foothills - Group 5'), (21251, 4814+5, 'Copper Vein - Hillsbrad Foothills - Group 5');

DELETE FROM `pool_template` WHERE `entry` IN (270, 1156);

DELETE FROM `pool_gameobject` WHERE `pool_entry` = 1156;

DELETE FROM `pool_gameobject` WHERE `guid` = 14920;

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4950+1, 2, 'Copper Vein - Wetlands - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (15031, 4950+1, 'Copper Vein - Wetlands - Group 1'), (15030, 4950+1, 'Copper Vein - Wetlands - Group 1'), (5283, 4950+1, 'Copper Vein - Wetlands - Group 1'), (5288, 4950+1, 'Copper Vein - Wetlands - Group 1'), (14900, 4950+1, 'Copper Vein - Wetlands - Group 1'), (5286, 4950+1, 'Copper Vein - Wetlands - Group 1');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4950+2, 2, 'Copper Vein - Wetlands - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (14870, 4950+2, 'Copper Vein - Wetlands - Group 2'), (5474, 4950+2, 'Copper Vein - Wetlands - Group 2'), (112738, 4950+2, 'Copper Vein - Wetlands - Group 2'), (4700, 4950+2, 'Copper Vein - Wetlands - Group 2'), (100578, 4950+2, 'Copper Vein - Wetlands - Group 2'), (4716, 4950+2, 'Copper Vein - Wetlands - Group 2');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4950+3, 2, 'Copper Vein - Wetlands - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (5048, 4950+3, 'Copper Vein - Wetlands - Group 3'), (5207, 4950+3, 'Copper Vein - Wetlands - Group 3'), (15214, 4950+3, 'Copper Vein - Wetlands - Group 3'), (5250, 4950+3, 'Copper Vein - Wetlands - Group 3'), (14875, 4950+3, 'Copper Vein - Wetlands - Group 3'), (14920, 4950+3, 'Copper Vein - Wetlands - Group 3');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4950+4, 2, 'Copper Vein - Wetlands - Group 4 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (14862, 4950+4, 'Copper Vein - Wetlands - Group 4'), (14906, 4950+4, 'Copper Vein - Wetlands - Group 4'), (34906, 4950+4, 'Copper Vein - Wetlands - Group 4'), (34133, 4950+4, 'Copper Vein - Wetlands - Group 4'), (33849, 4950+4, 'Copper Vein - Wetlands - Group 4');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4950+5, 2, 'Copper Vein - Wetlands - Group 5 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (14894, 4950+5, 'Copper Vein - Wetlands - Group 5'), (14564, 4950+5, 'Copper Vein - Wetlands - Group 5'), (14602, 4950+5, 'Copper Vein - Wetlands - Group 5'), (4705, 4950+5, 'Copper Vein - Wetlands - Group 5');

UPDATE `pool_gameobject` SET `pool_entry` =1028, `description` = 'Copper Vein - Mulgore' WHERE `guid` =5127;

-- End of migration.
;
