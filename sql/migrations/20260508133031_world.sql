INSERT INTO `migrations` VALUES ('20260508133031');

DELETE FROM `pool_template` WHERE `entry` = 1148;

DELETE FROM `pool_gameobject` WHERE `pool_entry` = 1148;

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3603+1, 1, 'Grave Moss - Wetlands - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (29128, 3603+1, 'Grave Moss - Wetlands - Group 1'), (68704, 3603+1, 'Grave Moss - Wetlands - Group 1'), (29124, 3603+1, 'Grave Moss - Wetlands - Group 1'), (14662, 3603+1, 'Grave Moss - Wetlands - Group 1'), (34110, 3603+1, 'Grave Moss - Wetlands - Group 1');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3603+2, 1, 'Grave Moss - Wetlands - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (29127, 3603+2, 'Grave Moss - Wetlands - Group 2'), (29125, 3603+2, 'Grave Moss - Wetlands - Group 2'), (15015, 3603+2, 'Grave Moss - Wetlands - Group 2'), (29126, 3603+2, 'Grave Moss - Wetlands - Group 2');

-- End of migration.
;
