INSERT INTO `migrations` VALUES ('20260303150148');

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Group 1
 DELETE FROM `pool_gameobject` WHERE `guid` IN (68647, 68642, 68646, 66927, 553);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4088+1, 1, 'Hinterlands - Group 1 - Ghost Mushroom', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (68647, 4088+1, 0, 'Ghost Mushroom - Hinterlands - Group 1', 0, 0, 10), (68642, 4088+1, 0, 'Ghost Mushroom - Hinterlands - Group 1', 0, 0, 10), (68646, 4088+1, 0, 'Ghost Mushroom - Hinterlands - Group 1', 0, 0, 10), (66927, 4088+1, 0, 'Ghost Mushroom - Hinterlands - Group 1', 0, 0, 10), (553, 4088+1, 0, 'Ghost Mushroom - Hinterlands - Group 1', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Group 2
 DELETE FROM `pool_gameobject` WHERE `guid` IN (66968, 66925, 68643, 552, 551);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4088+2, 1, 'Hinterlands - Group 2 - Ghost Mushroom', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (66968, 4088+2, 0, 'Ghost Mushroom - Hinterlands - Group 2', 0, 0, 10), (66925, 4088+2, 0, 'Ghost Mushroom - Hinterlands - Group 2', 0, 0, 10), (68643, 4088+2, 0, 'Ghost Mushroom - Hinterlands - Group 2', 0, 0, 10), (552, 4088+2, 0, 'Ghost Mushroom - Hinterlands - Group 2', 0, 0, 10), (551, 4088+2, 0, 'Ghost Mushroom - Hinterlands - Group 2', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Group 3
 DELETE FROM `pool_gameobject` WHERE `guid` IN (68644, 68645, 68641, 66926, 554);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4088+3, 1, 'Hinterlands - Group 3 - Ghost Mushroom', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (68644, 4088+3, 0, 'Ghost Mushroom - Hinterlands - Group 3', 0, 0, 10), (68645, 4088+3, 0, 'Ghost Mushroom - Hinterlands - Group 3', 0, 0, 10), (68641, 4088+3, 0, 'Ghost Mushroom - Hinterlands - Group 3', 0, 0, 10), (66926, 4088+3, 0, 'Ghost Mushroom - Hinterlands - Group 3', 0, 0, 10), (554, 4088+3, 0, 'Ghost Mushroom - Hinterlands - Group 3', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Group 4
 DELETE FROM `pool_gameobject` WHERE `guid` IN (66936, 66933, 66934, 46364, 66937);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4088+4, 1, 'Hinterlands - Group 4 - Ghost Mushroom', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (66936, 4088+4, 0, 'Ghost Mushroom - Hinterlands - Group 4', 0, 0, 10), (66933, 4088+4, 0, 'Ghost Mushroom - Hinterlands - Group 4', 0, 0, 10), (66934, 4088+4, 0, 'Ghost Mushroom - Hinterlands - Group 4', 0, 0, 10), (46364, 4088+4, 0, 'Ghost Mushroom - Hinterlands - Group 4', 0, 0, 10), (66937, 4088+4, 0, 'Ghost Mushroom - Hinterlands - Group 4', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Group 5
 DELETE FROM `pool_gameobject` WHERE `guid` IN (66929, 46264, 66928, 559);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4088+5, 1, 'Hinterlands - Group 5 - Ghost Mushroom', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (66929, 4088+5, 0, 'Ghost Mushroom - Hinterlands - Group 5', 0, 0, 10), (46264, 4088+5, 0, 'Ghost Mushroom - Hinterlands - Group 5', 0, 0, 10), (66928, 4088+5, 0, 'Ghost Mushroom - Hinterlands - Group 5', 0, 0, 10), (559, 4088+5, 0, 'Ghost Mushroom - Hinterlands - Group 5', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Group 6
 DELETE FROM `pool_gameobject` WHERE `guid` IN (46365, 68640, 33858, 66931);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4088+6, 1, 'Hinterlands - Group 6 - Ghost Mushroom', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (46365, 4088+6, 0, 'Ghost Mushroom - Hinterlands - Group 6', 0, 0, 10), (68640, 4088+6, 0, 'Ghost Mushroom - Hinterlands - Group 6', 0, 0, 10), (33858, 4088+6, 0, 'Ghost Mushroom - Hinterlands - Group 6', 0, 0, 10), (66931, 4088+6, 0, 'Ghost Mushroom - Hinterlands - Group 6', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Group 7
 DELETE FROM `pool_gameobject` WHERE `guid` IN (66935, 66930, 46366, 66932, 16434);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4088+7, 1, 'Hinterlands - Group 7 - Ghost Mushroom', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (66935, 4088+7, 0, 'Ghost Mushroom - Hinterlands - Group 7', 0, 0, 10), (66930, 4088+7, 0, 'Ghost Mushroom - Hinterlands - Group 7', 0, 0, 10), (46366, 4088+7, 0, 'Ghost Mushroom - Hinterlands - Group 7', 0, 0, 10), (66932, 4088+7, 0, 'Ghost Mushroom - Hinterlands - Group 7', 0, 0, 10);

DELETE FROM `gameobject` WHERE `guid` = 16434;

DELETE FROM `pool_template` WHERE `entry` =1247 AND `patch_min` =0 AND `patch_max` =10;

-- End of migration.
;
