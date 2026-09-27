INSERT INTO `migrations` VALUES ('20260329133202');

DELETE FROM `pool_pool` WHERE `pool_id` IN (8851, 8850, 8849, 8852, 8847, 8848);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Stonetalon Mountains - Group 1' WHERE `entry` IN (8851, 8850, 8849, 8852, 8847, 8848);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Stonetalon Mountains - Group 1' WHERE `pool_entry` IN (8851, 8850, 8849, 8852, 8847, 8848);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (293+1, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stonetalon Mountains - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (8851, 293+1, 0, 'Stonetalon Mountains - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0), (8850, 293+1, 0, 'Stonetalon Mountains - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0), (8849, 293+1, 0, 'Stonetalon Mountains - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0), (8852, 293+1, 0, 'Stonetalon Mountains - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0), (8847, 293+1, 0, 'Stonetalon Mountains - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0), (8848, 293+1, 0, 'Stonetalon Mountains - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (8846, 8845, 8843, 8842, 8853, 8844);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Stonetalon Mountains - Group 2' WHERE `entry` IN (8846, 8845, 8843, 8842, 8853, 8844);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Stonetalon Mountains - Group 2' WHERE `pool_entry` IN (8846, 8845, 8843, 8842, 8853, 8844);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (293+2, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stonetalon Mountains - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (8846, 293+2, 0, 'Stonetalon Mountains - Group 2 - Silver Vein / Gold Vein / Iron Deposit', 0), (8845, 293+2, 0, 'Stonetalon Mountains - Group 2 - Silver Vein / Gold Vein / Iron Deposit', 0), (8843, 293+2, 0, 'Stonetalon Mountains - Group 2 - Silver Vein / Gold Vein / Iron Deposit', 0), (8842, 293+2, 0, 'Stonetalon Mountains - Group 2 - Silver Vein / Gold Vein / Iron Deposit', 0), (8853, 293+2, 0, 'Stonetalon Mountains - Group 2 - Silver Vein / Gold Vein / Iron Deposit', 0), (8844, 293+2, 0, 'Stonetalon Mountains - Group 2 - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_template` WHERE `entry` = 1091;

-- End of migration.
;
