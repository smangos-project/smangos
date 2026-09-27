INSERT INTO `migrations` VALUES ('20260411162511');

DELETE FROM `pool_template` WHERE `entry` = 1216;

DELETE FROM `pool_gameobject` WHERE `pool_entry` = 1216;

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4348+1, 1, 'Blindweed - Un''Goro Crater - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (16323, 4348+1, 'Blindweed - Un''Goro Crater - Group 1'), (16329, 4348+1, 'Blindweed - Un''Goro Crater - Group 1'), (16392, 4348+1, 'Blindweed - Un''Goro Crater - Group 1'), (16396, 4348+1, 'Blindweed - Un''Goro Crater - Group 1'), (17591, 4348+1, 'Blindweed - Un''Goro Crater - Group 1');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4348+2, 1, 'Blindweed - Un''Goro Crater - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (16303, 4348+2, 'Blindweed - Un''Goro Crater - Group 2'), (16378, 4348+2, 'Blindweed - Un''Goro Crater - Group 2'), (17590, 4348+2, 'Blindweed - Un''Goro Crater - Group 2'), (22186, 4348+2, 'Blindweed - Un''Goro Crater - Group 2'), (22187, 4348+2, 'Blindweed - Un''Goro Crater - Group 2'), (68348, 4348+2, 'Blindweed - Un''Goro Crater - Group 2');

-- End of migration.
;
