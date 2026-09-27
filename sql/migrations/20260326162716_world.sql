INSERT INTO `migrations` VALUES ('20260326162716');

DELETE FROM `pool_pool` WHERE `pool_id` IN (21490, 9439, 23009, 14418, 21495);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Feralas - Group 1' WHERE `entry` IN (21490, 9439, 23009, 14418, 21495);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Feralas - Group 1' WHERE `pool_entry` IN (21490, 9439, 23009, 14418, 21495);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (1274+1, 1, 'Truesilver Deposit / Small Thorium Vein - Feralas - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (21490, 1274+1, 0, 'Feralas - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (9439, 1274+1, 0, 'Feralas - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (23009, 1274+1, 0, 'Feralas - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (14418, 1274+1, 0, 'Feralas - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (21495, 1274+1, 0, 'Feralas - Group 1 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (21494, 23008, 21492, 21491, 21493);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Feralas - Group 2' WHERE `entry` IN (21494, 23008, 21492, 21491, 21493);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Feralas - Group 2' WHERE `pool_entry` IN (21494, 23008, 21492, 21491, 21493);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (1274+2, 1, 'Truesilver Deposit / Small Thorium Vein - Feralas - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (21494, 1274+2, 0, 'Feralas - Group 2 - Truesilver Deposit / Small Thorium Vein', 0), (23008, 1274+2, 0, 'Feralas - Group 2 - Truesilver Deposit / Small Thorium Vein', 0), (21492, 1274+2, 0, 'Feralas - Group 2 - Truesilver Deposit / Small Thorium Vein', 0), (21491, 1274+2, 0, 'Feralas - Group 2 - Truesilver Deposit / Small Thorium Vein', 0), (21493, 1274+2, 0, 'Feralas - Group 2 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_template` WHERE `entry` IN (985, 296);

-- End of migration.
;
