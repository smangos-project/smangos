INSERT INTO `migrations` VALUES ('20260303145540');

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Group 1
 DELETE FROM `pool_gameobject` WHERE `guid` IN (52498, 16094, 16153, 16170, 16030);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4084+1, 1, 'Blasted Lands - Group 1 - Sungrass', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (52498, 4084+1, 0, 'Sungrass - Blasted Lands - Group 1', 0, 0, 10), (16094, 4084+1, 0, 'Sungrass - Blasted Lands - Group 1', 0, 0, 10), (16153, 4084+1, 0, 'Sungrass - Blasted Lands - Group 1', 0, 0, 10), (16170, 4084+1, 0, 'Sungrass - Blasted Lands - Group 1', 0, 0, 10), (16030, 4084+1, 0, 'Sungrass - Blasted Lands - Group 1', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Group 2
 DELETE FROM `pool_gameobject` WHERE `guid` IN (52502, 68868, 42440, 52500, 52497);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4084+2, 1, 'Blasted Lands - Group 2 - Sungrass', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (52502, 4084+2, 0, 'Sungrass - Blasted Lands - Group 2', 0, 0, 10), (68868, 4084+2, 0, 'Sungrass - Blasted Lands - Group 2', 0, 0, 10), (42440, 4084+2, 0, 'Sungrass - Blasted Lands - Group 2', 0, 0, 10), (52500, 4084+2, 0, 'Sungrass - Blasted Lands - Group 2', 0, 0, 10), (52497, 4084+2, 0, 'Sungrass - Blasted Lands - Group 2', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Group 3
 DELETE FROM `pool_gameobject` WHERE `guid` IN (52496, 32103, 52503, 16140, 52501);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4084+3, 1, 'Blasted Lands - Group 3 - Sungrass', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (52496, 4084+3, 0, 'Sungrass - Blasted Lands - Group 3', 0, 0, 10), (32103, 4084+3, 0, 'Sungrass - Blasted Lands - Group 3', 0, 0, 10), (52503, 4084+3, 0, 'Sungrass - Blasted Lands - Group 3', 0, 0, 10), (16140, 4084+3, 0, 'Sungrass - Blasted Lands - Group 3', 0, 0, 10), (52501, 4084+3, 0, 'Sungrass - Blasted Lands - Group 3', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Group 4
 DELETE FROM `pool_gameobject` WHERE `guid` IN (66966, 39996, 29684, 52499, 16228);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4084+4, 1, 'Blasted Lands - Group 4 - Sungrass', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (66966, 4084+4, 0, 'Sungrass - Blasted Lands - Group 4', 0, 0, 10), (39996, 4084+4, 0, 'Sungrass - Blasted Lands - Group 4', 0, 0, 10), (29684, 4084+4, 0, 'Sungrass - Blasted Lands - Group 4', 0, 0, 10), (52499, 4084+4, 0, 'Sungrass - Blasted Lands - Group 4', 0, 0, 10), (16228, 4084+4, 0, 'Sungrass - Blasted Lands - Group 4', 0, 0, 10);

DELETE FROM `pool_template` WHERE `entry` =1277 AND `patch_min` =0 AND `patch_max` =10;

-- End of migration.
;
