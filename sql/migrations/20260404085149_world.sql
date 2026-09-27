INSERT INTO `migrations` VALUES ('20260404085149');

DELETE FROM `pool_pool` WHERE `pool_id` IN (1374, 1375, 14425, 14426, 1376, 1380);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Eastern Plaguelands - Group 1' WHERE `entry` IN (1374, 1375, 14425, 14426, 1376, 1380);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Eastern Plaguelands - Group 1' WHERE `pool_entry` IN (1374, 1375, 14425, 14426, 1376, 1380);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4673+1, 1, 'Truesilver Deposit / Small Thorium Vein - Eastern Plaguelands - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (1374, 4673+1, 0, 'Eastern Plaguelands - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (1375, 4673+1, 0, 'Eastern Plaguelands - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (14425, 4673+1, 0, 'Eastern Plaguelands - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (14426, 4673+1, 0, 'Eastern Plaguelands - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (1376, 4673+1, 0, 'Eastern Plaguelands - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (1380, 4673+1, 0, 'Eastern Plaguelands - Group 1 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (14424, 1370, 1369, 1366, 1361);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Eastern Plaguelands - Group 2' WHERE `entry` IN (14424, 1370, 1369, 1366, 1361);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Eastern Plaguelands - Group 2' WHERE `pool_entry` IN (14424, 1370, 1369, 1366, 1361);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4673+2, 1, 'Truesilver Deposit / Small Thorium Vein - Eastern Plaguelands - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (14424, 4673+2, 0, 'Eastern Plaguelands - Group 2 - Truesilver Deposit / Small Thorium Vein', 0), (1370, 4673+2, 0, 'Eastern Plaguelands - Group 2 - Truesilver Deposit / Small Thorium Vein', 0), (1369, 4673+2, 0, 'Eastern Plaguelands - Group 2 - Truesilver Deposit / Small Thorium Vein', 0), (1366, 4673+2, 0, 'Eastern Plaguelands - Group 2 - Truesilver Deposit / Small Thorium Vein', 0), (1361, 4673+2, 0, 'Eastern Plaguelands - Group 2 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (1373, 1372, 1367, 17007, 1371);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Eastern Plaguelands - Group 3' WHERE `entry` IN (1373, 1372, 1367, 17007, 1371);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Eastern Plaguelands - Group 3' WHERE `pool_entry` IN (1373, 1372, 1367, 17007, 1371);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4673+3, 1, 'Truesilver Deposit / Small Thorium Vein - Eastern Plaguelands - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (1373, 4673+3, 0, 'Eastern Plaguelands - Group 3 - Truesilver Deposit / Small Thorium Vein', 0), (1372, 4673+3, 0, 'Eastern Plaguelands - Group 3 - Truesilver Deposit / Small Thorium Vein', 0), (1367, 4673+3, 0, 'Eastern Plaguelands - Group 3 - Truesilver Deposit / Small Thorium Vein', 0), (17007, 4673+3, 0, 'Eastern Plaguelands - Group 3 - Truesilver Deposit / Small Thorium Vein', 0), (1371, 4673+3, 0, 'Eastern Plaguelands - Group 3 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (1363, 1365, 1362, 1360, 1364);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Eastern Plaguelands - Group 4' WHERE `entry` IN (1363, 1365, 1362, 1360, 1364);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Eastern Plaguelands - Group 4' WHERE `pool_entry` IN (1363, 1365, 1362, 1360, 1364);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4673+4, 1, 'Truesilver Deposit / Small Thorium Vein - Eastern Plaguelands - Group 4 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (1363, 4673+4, 0, 'Eastern Plaguelands - Group 4 - Truesilver Deposit / Small Thorium Vein', 0), (1365, 4673+4, 0, 'Eastern Plaguelands - Group 4 - Truesilver Deposit / Small Thorium Vein', 0), (1362, 4673+4, 0, 'Eastern Plaguelands - Group 4 - Truesilver Deposit / Small Thorium Vein', 0), (1360, 4673+4, 0, 'Eastern Plaguelands - Group 4 - Truesilver Deposit / Small Thorium Vein', 0), (1364, 4673+4, 0, 'Eastern Plaguelands - Group 4 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (1381, 1377, 1378, 1379, 9438);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Eastern Plaguelands - Group 5' WHERE `entry` IN (1381, 1377, 1378, 1379, 9438);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Eastern Plaguelands - Group 5' WHERE `pool_entry` IN (1381, 1377, 1378, 1379, 9438);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4673+5, 1, 'Truesilver Deposit / Small Thorium Vein - Eastern Plaguelands - Group 5 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (1381, 4673+5, 0, 'Eastern Plaguelands - Group 5 - Truesilver Deposit / Small Thorium Vein', 0), (1377, 4673+5, 0, 'Eastern Plaguelands - Group 5 - Truesilver Deposit / Small Thorium Vein', 0), (1378, 4673+5, 0, 'Eastern Plaguelands - Group 5 - Truesilver Deposit / Small Thorium Vein', 0), (1379, 4673+5, 0, 'Eastern Plaguelands - Group 5 - Truesilver Deposit / Small Thorium Vein', 0), (9438, 4673+5, 0, 'Eastern Plaguelands - Group 5 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_template` WHERE `entry` = 1264;

DELETE FROM `pool_pool` WHERE `pool_id` IN (1435, 1417, 1415, 1434, 1413, 1368);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands - Group 1' WHERE `entry` IN (1435, 1417, 1415, 1434, 1413, 1368);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands - Group 1' WHERE `pool_entry` IN (1435, 1417, 1415, 1434, 1413, 1368);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4678+1, 1, 'Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (1435, 4678+1, 0, 'Eastern Plaguelands - Group 1 - Truesilver Deposit / Rich Thorium Vein', 0), (1417, 4678+1, 0, 'Eastern Plaguelands - Group 1 - Truesilver Deposit / Rich Thorium Vein', 0), (1415, 4678+1, 0, 'Eastern Plaguelands - Group 1 - Truesilver Deposit / Rich Thorium Vein', 0), (1434, 4678+1, 0, 'Eastern Plaguelands - Group 1 - Truesilver Deposit / Rich Thorium Vein', 0), (1413, 4678+1, 0, 'Eastern Plaguelands - Group 1 - Truesilver Deposit / Rich Thorium Vein', 0), (1368, 4678+1, 0, 'Eastern Plaguelands - Group 1 - Truesilver Deposit / Rich Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (1418, 1420, 1421, 1419, 1416, 1414);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands - Group 2' WHERE `entry` IN (1418, 1420, 1421, 1419, 1416, 1414);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands - Group 2' WHERE `pool_entry` IN (1418, 1420, 1421, 1419, 1416, 1414);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4678+2, 1, 'Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (1418, 4678+2, 0, 'Eastern Plaguelands - Group 2 - Truesilver Deposit / Rich Thorium Vein', 0), (1420, 4678+2, 0, 'Eastern Plaguelands - Group 2 - Truesilver Deposit / Rich Thorium Vein', 0), (1421, 4678+2, 0, 'Eastern Plaguelands - Group 2 - Truesilver Deposit / Rich Thorium Vein', 0), (1419, 4678+2, 0, 'Eastern Plaguelands - Group 2 - Truesilver Deposit / Rich Thorium Vein', 0), (1416, 4678+2, 0, 'Eastern Plaguelands - Group 2 - Truesilver Deposit / Rich Thorium Vein', 0), (1414, 4678+2, 0, 'Eastern Plaguelands - Group 2 - Truesilver Deposit / Rich Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (14436, 1428, 1436, 14435, 9433, 1431);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands - Group 3' WHERE `entry` IN (14436, 1428, 1436, 14435, 9433, 1431);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands - Group 3' WHERE `pool_entry` IN (14436, 1428, 1436, 14435, 9433, 1431);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4678+3, 1, 'Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (14436, 4678+3, 0, 'Eastern Plaguelands - Group 3 - Truesilver Deposit / Rich Thorium Vein', 0), (1428, 4678+3, 0, 'Eastern Plaguelands - Group 3 - Truesilver Deposit / Rich Thorium Vein', 0), (1436, 4678+3, 0, 'Eastern Plaguelands - Group 3 - Truesilver Deposit / Rich Thorium Vein', 0), (14435, 4678+3, 0, 'Eastern Plaguelands - Group 3 - Truesilver Deposit / Rich Thorium Vein', 0), (9433, 4678+3, 0, 'Eastern Plaguelands - Group 3 - Truesilver Deposit / Rich Thorium Vein', 0), (1431, 4678+3, 0, 'Eastern Plaguelands - Group 3 - Truesilver Deposit / Rich Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (9427, 3986, 1423, 1422, 9428);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands - Group 4' WHERE `entry` IN (9427, 3986, 1423, 1422, 9428);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands - Group 4' WHERE `pool_entry` IN (9427, 3986, 1423, 1422, 9428);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4678+4, 1, 'Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands - Group 4 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (9427, 4678+4, 0, 'Eastern Plaguelands - Group 4 - Truesilver Deposit / Rich Thorium Vein', 0), (3986, 4678+4, 0, 'Eastern Plaguelands - Group 4 - Truesilver Deposit / Rich Thorium Vein', 0), (1423, 4678+4, 0, 'Eastern Plaguelands - Group 4 - Truesilver Deposit / Rich Thorium Vein', 0), (1422, 4678+4, 0, 'Eastern Plaguelands - Group 4 - Truesilver Deposit / Rich Thorium Vein', 0), (9428, 4678+4, 0, 'Eastern Plaguelands - Group 4 - Truesilver Deposit / Rich Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (1411, 14438, 9429, 1409, 3985);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands - Group 5' WHERE `entry` IN (1411, 14438, 9429, 1409, 3985);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands - Group 5' WHERE `pool_entry` IN (1411, 14438, 9429, 1409, 3985);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4678+5, 1, 'Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands - Group 5 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (1411, 4678+5, 0, 'Eastern Plaguelands - Group 5 - Truesilver Deposit / Rich Thorium Vein', 0), (14438, 4678+5, 0, 'Eastern Plaguelands - Group 5 - Truesilver Deposit / Rich Thorium Vein', 0), (9429, 4678+5, 0, 'Eastern Plaguelands - Group 5 - Truesilver Deposit / Rich Thorium Vein', 0), (1409, 4678+5, 0, 'Eastern Plaguelands - Group 5 - Truesilver Deposit / Rich Thorium Vein', 0), (3985, 4678+5, 0, 'Eastern Plaguelands - Group 5 - Truesilver Deposit / Rich Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (1429, 1427, 1425, 9430, 1426);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands - Group 6' WHERE `entry` IN (1429, 1427, 1425, 9430, 1426);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands - Group 6' WHERE `pool_entry` IN (1429, 1427, 1425, 9430, 1426);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4678+6, 1, 'Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands - Group 6 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (1429, 4678+6, 0, 'Eastern Plaguelands - Group 6 - Truesilver Deposit / Rich Thorium Vein', 0), (1427, 4678+6, 0, 'Eastern Plaguelands - Group 6 - Truesilver Deposit / Rich Thorium Vein', 0), (1425, 4678+6, 0, 'Eastern Plaguelands - Group 6 - Truesilver Deposit / Rich Thorium Vein', 0), (9430, 4678+6, 0, 'Eastern Plaguelands - Group 6 - Truesilver Deposit / Rich Thorium Vein', 0), (1426, 4678+6, 0, 'Eastern Plaguelands - Group 6 - Truesilver Deposit / Rich Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (14437, 1424, 9434, 9435, 1430);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands - Group 7' WHERE `entry` IN (14437, 1424, 9434, 9435, 1430);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands - Group 7' WHERE `pool_entry` IN (14437, 1424, 9434, 9435, 1430);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4678+7, 1, 'Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands - Group 7 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (14437, 4678+7, 0, 'Eastern Plaguelands - Group 7 - Truesilver Deposit / Rich Thorium Vein', 0), (1424, 4678+7, 0, 'Eastern Plaguelands - Group 7 - Truesilver Deposit / Rich Thorium Vein', 0), (9434, 4678+7, 0, 'Eastern Plaguelands - Group 7 - Truesilver Deposit / Rich Thorium Vein', 0), (9435, 4678+7, 0, 'Eastern Plaguelands - Group 7 - Truesilver Deposit / Rich Thorium Vein', 0), (1430, 4678+7, 0, 'Eastern Plaguelands - Group 7 - Truesilver Deposit / Rich Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (1432, 9431, 1433, 9432);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands - Group 8' WHERE `entry` IN (1432, 9431, 1433, 9432);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands - Group 8' WHERE `pool_entry` IN (1432, 9431, 1433, 9432);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4678+8, 1, 'Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands - Group 8 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (1432, 4678+8, 0, 'Eastern Plaguelands - Group 8 - Truesilver Deposit / Rich Thorium Vein', 0), (9431, 4678+8, 0, 'Eastern Plaguelands - Group 8 - Truesilver Deposit / Rich Thorium Vein', 0), (1433, 4678+8, 0, 'Eastern Plaguelands - Group 8 - Truesilver Deposit / Rich Thorium Vein', 0), (9432, 4678+8, 0, 'Eastern Plaguelands - Group 8 - Truesilver Deposit / Rich Thorium Vein', 0);

DELETE FROM `pool_template` WHERE `entry` IN (1412, 1410, 1408, 8135, 8133, 8136, 8134, 325, 8132, 8137);

DELETE FROM `gameobject` WHERE `guid` IN (24980, 25009, 24982, 25011, 24984, 25013);

DELETE FROM `pool_gameobject` WHERE `pool_entry` IN (1412, 1410, 1408);

DELETE FROM `pool_pool` WHERE `pool_id` IN (1412, 1410, 1408);

-- End of migration.
;
