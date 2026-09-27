INSERT INTO `migrations` VALUES ('20260302083859');

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Group 1
 DELETE FROM `pool_gameobject` WHERE `guid` IN (12316, 55465, 55471, 55470, 12278);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5152+1, 1, 'Blasted Lands - Group 1 - Firebloom', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (12316, 5152+1, 0, 'Firebloom - Blasted Lands - Group 1', 0, 0, 10), (55465, 5152+1, 0, 'Firebloom - Blasted Lands - Group 1', 0, 0, 10), (55471, 5152+1, 0, 'Firebloom - Blasted Lands - Group 1', 0, 0, 10), (55470, 5152+1, 0, 'Firebloom - Blasted Lands - Group 1', 0, 0, 10), (12278, 5152+1, 0, 'Firebloom - Blasted Lands - Group 1', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Group 2
 DELETE FROM `pool_gameobject` WHERE `guid` IN (55468, 68622, 55464, 32308, 32105);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5152+2, 1, 'Blasted Lands - Group 2 - Firebloom', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (55468, 5152+2, 0, 'Firebloom - Blasted Lands - Group 2', 0, 0, 10), (68622, 5152+2, 0, 'Firebloom - Blasted Lands - Group 2', 0, 0, 10), (55464, 5152+2, 0, 'Firebloom - Blasted Lands - Group 2', 0, 0, 10), (32308, 5152+2, 0, 'Firebloom - Blasted Lands - Group 2', 0, 0, 10), (32105, 5152+2, 0, 'Firebloom - Blasted Lands - Group 2', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Group 3
 DELETE FROM `pool_gameobject` WHERE `guid` IN (32104, 29685, 32303, 32760, 55467);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5152+3, 1, 'Blasted Lands - Group 3 - Firebloom', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (32104, 5152+3, 0, 'Firebloom - Blasted Lands - Group 3', 0, 0, 10), (29685, 5152+3, 0, 'Firebloom - Blasted Lands - Group 3', 0, 0, 10), (32303, 5152+3, 0, 'Firebloom - Blasted Lands - Group 3', 0, 0, 10), (32760, 5152+3, 0, 'Firebloom - Blasted Lands - Group 3', 0, 0, 10), (55467, 5152+3, 0, 'Firebloom - Blasted Lands - Group 3', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Group 4
 DELETE FROM `pool_gameobject` WHERE `guid` IN (29683, 55469, 32102, 32360, 55466);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5152+4, 1, 'Blasted Lands - Group 4 - Firebloom', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (29683, 5152+4, 0, 'Firebloom - Blasted Lands - Group 4', 0, 0, 10), (55469, 5152+4, 0, 'Firebloom - Blasted Lands - Group 4', 0, 0, 10), (32102, 5152+4, 0, 'Firebloom - Blasted Lands - Group 4', 0, 0, 10), (32360, 5152+4, 0, 'Firebloom - Blasted Lands - Group 4', 0, 0, 10), (55466, 5152+4, 0, 'Firebloom - Blasted Lands - Group 4', 0, 0, 10);

DELETE FROM `pool_template` WHERE `entry` =1276 AND `patch_min` =0 AND `patch_max` =10;

-- End of migration.
;
