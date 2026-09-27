INSERT INTO `migrations` VALUES ('20260322162413');

DELETE FROM `pool_pool` WHERE `pool_id` IN (7151, 7155, 7152, 7157, 7162);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Tanaris - Group 1' WHERE `entry` IN (7151, 7155, 7152, 7157, 7162);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Tanaris - Group 1' WHERE `pool_entry` IN (7151, 7155, 7152, 7157, 7162);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4276+1, 1, 'Truesilver Deposit / Small Thorium Vein - Tanaris - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (7151, 4276+1, 0, 'Tanaris - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (7155, 4276+1, 0, 'Tanaris - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (7152, 4276+1, 0, 'Tanaris - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (7157, 4276+1, 0, 'Tanaris - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (7162, 4276+1, 0, 'Tanaris - Group 1 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (7153, 7161, 9443, 7160, 7164);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Tanaris - Group 2' WHERE `entry` IN (7153, 7161, 9443, 7160, 7164);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Tanaris - Group 2' WHERE `pool_entry` IN (7153, 7161, 9443, 7160, 7164);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4276+2, 1, 'Truesilver Deposit / Small Thorium Vein - Tanaris - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (7153, 4276+2, 0, 'Tanaris - Group 2 - Truesilver Deposit / Small Thorium Vein', 0), (7161, 4276+2, 0, 'Tanaris - Group 2 - Truesilver Deposit / Small Thorium Vein', 0), (9443, 4276+2, 0, 'Tanaris - Group 2 - Truesilver Deposit / Small Thorium Vein', 0), (7160, 4276+2, 0, 'Tanaris - Group 2 - Truesilver Deposit / Small Thorium Vein', 0), (7164, 4276+2, 0, 'Tanaris - Group 2 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (7156, 9444, 7150, 7154, 9445);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Tanaris - Group 3' WHERE `entry` IN (7156, 9444, 7150, 7154, 9445);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Tanaris - Group 3' WHERE `pool_entry` IN (7156, 9444, 7150, 7154, 9445);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4276+3, 1, 'Truesilver Deposit / Small Thorium Vein - Tanaris - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (7156, 4276+3, 0, 'Tanaris - Group 3 - Truesilver Deposit / Small Thorium Vein', 0), (9444, 4276+3, 0, 'Tanaris - Group 3 - Truesilver Deposit / Small Thorium Vein', 0), (7150, 4276+3, 0, 'Tanaris - Group 3 - Truesilver Deposit / Small Thorium Vein', 0), (7154, 4276+3, 0, 'Tanaris - Group 3 - Truesilver Deposit / Small Thorium Vein', 0), (9445, 4276+3, 0, 'Tanaris - Group 3 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (7159, 9442, 7163, 7158);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Tanaris - Group 4' WHERE `entry` IN (7159, 9442, 7163, 7158);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Tanaris - Group 4' WHERE `pool_entry` IN (7159, 9442, 7163, 7158);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4276+4, 1, 'Truesilver Deposit / Small Thorium Vein - Tanaris - Group 4 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (7159, 4276+4, 0, 'Tanaris - Group 4 - Truesilver Deposit / Small Thorium Vein', 0), (9442, 4276+4, 0, 'Tanaris - Group 4 - Truesilver Deposit / Small Thorium Vein', 0), (7163, 4276+4, 0, 'Tanaris - Group 4 - Truesilver Deposit / Small Thorium Vein', 0), (7158, 4276+4, 0, 'Tanaris - Group 4 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_template` WHERE `entry` = 9915;

UPDATE `pool_template` SET `max_limit` = 2 WHERE `entry` = 8141;

UPDATE `pool_template` SET `max_limit` = 1 WHERE `entry` = 8139;

UPDATE `pool_template` SET `max_limit` = 1 WHERE `entry` = 8140;

-- End of migration.
;
