INSERT INTO `migrations` VALUES ('20260301162905');

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Group 1
 DELETE FROM `pool_gameobject` WHERE `guid` IN (49399, 68695, 30460, 66965);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3729+1, 1, 'Silithus - Group 1 - Golden Sansam', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (49399, 3729+1, 0, 'Golden Sansam - Silithus - Group 1', 0, 0, 10), (68695, 3729+1, 0, 'Golden Sansam - Silithus - Group 1', 0, 0, 10), (30460, 3729+1, 0, 'Golden Sansam - Silithus - Group 1', 0, 0, 10), (66965, 3729+1, 0, 'Golden Sansam - Silithus - Group 1', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Group 2
 DELETE FROM `pool_gameobject` WHERE `guid` IN (30457, 30455, 68696, 49398);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3729+2, 1, 'Silithus - Group 2 - Golden Sansam', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (30457, 3729+2, 0, 'Golden Sansam - Silithus - Group 2', 0, 0, 10), (30455, 3729+2, 0, 'Golden Sansam - Silithus - Group 2', 0, 0, 10), (68696, 3729+2, 0, 'Golden Sansam - Silithus - Group 2', 0, 0, 10), (49398, 3729+2, 0, 'Golden Sansam - Silithus - Group 2', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Group 3
 DELETE FROM `pool_gameobject` WHERE `guid` IN (30458, 30456, 49397, 591);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3729+3, 1, 'Silithus - Group 3 - Golden Sansam', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (30458, 3729+3, 0, 'Golden Sansam - Silithus - Group 3', 0, 0, 10), (30456, 3729+3, 0, 'Golden Sansam - Silithus - Group 3', 0, 0, 10), (49397, 3729+3, 0, 'Golden Sansam - Silithus - Group 3', 0, 0, 10), (591, 3729+3, 0, 'Golden Sansam - Silithus - Group 3', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Group 4
 DELETE FROM `pool_gameobject` WHERE `guid` IN (68697, 30461, 30454, 49400);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3729+4, 1, 'Silithus - Group 4 - Golden Sansam', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (68697, 3729+4, 0, 'Golden Sansam - Silithus - Group 4', 0, 0, 10), (30461, 3729+4, 0, 'Golden Sansam - Silithus - Group 4', 0, 0, 10), (30454, 3729+4, 0, 'Golden Sansam - Silithus - Group 4', 0, 0, 10), (49400, 3729+4, 0, 'Golden Sansam - Silithus - Group 4', 0, 0, 10);

DELETE FROM `pool_template` WHERE `entry` =1224 AND `patch_min` =0 AND `patch_max` =10;

-- End of migration.
;
