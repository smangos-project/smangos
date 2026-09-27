INSERT INTO `migrations` VALUES ('20260323181336');

DELETE FROM `pool_pool` WHERE `pool_id` IN (25617, 25616, 25613, 25615, 25614);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Hinterlands - Group 1' WHERE `entry` IN (25617, 25616, 25613, 25615, 25614);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Hinterlands - Group 1' WHERE `pool_entry` IN (25617, 25616, 25613, 25615, 25614);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3656+1, 1, 'Truesilver Deposit / Small Thorium Vein - Hinterlands - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (25617, 3656+1, 0, 'Hinterlands - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (25616, 3656+1, 0, 'Hinterlands - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (25613, 3656+1, 0, 'Hinterlands - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (25615, 3656+1, 0, 'Hinterlands - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (25614, 3656+1, 0, 'Hinterlands - Group 1 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (25619, 3031, 25612, 25611);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Hinterlands - Group 2' WHERE `entry` IN (25619, 3031, 25612, 25611);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Hinterlands - Group 2' WHERE `pool_entry` IN (25619, 3031, 25612, 25611);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3656+2, 1, 'Truesilver Deposit / Small Thorium Vein - Hinterlands - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (25619, 3656+2, 0, 'Hinterlands - Group 2 - Truesilver Deposit / Small Thorium Vein', 0), (3031, 3656+2, 0, 'Hinterlands - Group 2 - Truesilver Deposit / Small Thorium Vein', 0), (25612, 3656+2, 0, 'Hinterlands - Group 2 - Truesilver Deposit / Small Thorium Vein', 0), (25611, 3656+2, 0, 'Hinterlands - Group 2 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (9440, 25618, 9441, 17206);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Hinterlands - Group 3' WHERE `entry` IN (9440, 25618, 9441, 17206);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Hinterlands - Group 3' WHERE `pool_entry` IN (9440, 25618, 9441, 17206);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3656+3, 1, 'Truesilver Deposit / Small Thorium Vein - Hinterlands - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (9440, 3656+3, 0, 'Hinterlands - Group 3 - Truesilver Deposit / Small Thorium Vein', 0), (25618, 3656+3, 0, 'Hinterlands - Group 3 - Truesilver Deposit / Small Thorium Vein', 0), (9441, 3656+3, 0, 'Hinterlands - Group 3 - Truesilver Deposit / Small Thorium Vein', 0), (17206, 3656+3, 0, 'Hinterlands - Group 3 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_template` WHERE `entry` IN (295, 294, 1070);

-- End of migration.
;
