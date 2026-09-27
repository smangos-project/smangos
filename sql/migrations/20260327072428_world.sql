INSERT INTO `migrations` VALUES ('20260327072428');

DELETE FROM `pool_pool` WHERE `pool_id` IN (32986, 17118, 32987, 32993, 32992, 32991);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Feralas - Group 1' WHERE `entry` IN (32986, 17118, 32987, 32993, 32992, 32991);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Feralas - Group 1' WHERE `pool_entry` IN (32986, 17118, 32987, 32993, 32992, 32991);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4591+1, 1, 'Silver Vein / Gold Vein / Iron Deposit - Feralas - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (32986, 4591+1, 0, 'Feralas - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0), (17118, 4591+1, 0, 'Feralas - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0), (32987, 4591+1, 0, 'Feralas - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0), (32993, 4591+1, 0, 'Feralas - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0), (32992, 4591+1, 0, 'Feralas - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0), (32991, 4591+1, 0, 'Feralas - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (23025, 32985, 32989, 32981, 32984);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Feralas - Group 2' WHERE `entry` IN (23025, 32985, 32989, 32981, 32984);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Feralas - Group 2' WHERE `pool_entry` IN (23025, 32985, 32989, 32981, 32984);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4591+2, 1, 'Silver Vein / Gold Vein / Iron Deposit - Feralas - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (23025, 4591+2, 0, 'Feralas - Group 2 - Silver Vein / Gold Vein / Iron Deposit', 0), (32985, 4591+2, 0, 'Feralas - Group 2 - Silver Vein / Gold Vein / Iron Deposit', 0), (32989, 4591+2, 0, 'Feralas - Group 2 - Silver Vein / Gold Vein / Iron Deposit', 0), (32981, 4591+2, 0, 'Feralas - Group 2 - Silver Vein / Gold Vein / Iron Deposit', 0), (32984, 4591+2, 0, 'Feralas - Group 2 - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (17117, 17115, 17053, 32982);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Feralas - Group 3' WHERE `entry` IN (17117, 17115, 17053, 32982);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Feralas - Group 3' WHERE `pool_entry` IN (17117, 17115, 17053, 32982);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4591+3, 1, 'Silver Vein / Gold Vein / Iron Deposit - Feralas - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (17117, 4591+3, 0, 'Feralas - Group 3 - Silver Vein / Gold Vein / Iron Deposit', 0), (17115, 4591+3, 0, 'Feralas - Group 3 - Silver Vein / Gold Vein / Iron Deposit', 0), (17053, 4591+3, 0, 'Feralas - Group 3 - Silver Vein / Gold Vein / Iron Deposit', 0), (32982, 4591+3, 0, 'Feralas - Group 3 - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (17116, 32988, 2167, 32983);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Feralas - Group 4' WHERE `entry` IN (17116, 32988, 2167, 32983);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Feralas - Group 4' WHERE `pool_entry` IN (17116, 32988, 2167, 32983);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4591+4, 1, 'Silver Vein / Gold Vein / Iron Deposit - Feralas - Group 4 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (17116, 4591+4, 0, 'Feralas - Group 4 - Silver Vein / Gold Vein / Iron Deposit', 0), (32988, 4591+4, 0, 'Feralas - Group 4 - Silver Vein / Gold Vein / Iron Deposit', 0), (2167, 4591+4, 0, 'Feralas - Group 4 - Silver Vein / Gold Vein / Iron Deposit', 0), (32983, 4591+4, 0, 'Feralas - Group 4 - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_template` WHERE `entry` = 1196;

UPDATE `pool_template` SET `max_limit` = 1 WHERE `entry` = 321;

UPDATE `pool_template` SET `max_limit` = 1 WHERE `entry` = 297;

-- End of migration.
;
