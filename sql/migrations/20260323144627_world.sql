INSERT INTO `migrations` VALUES ('20260323144627');

DELETE FROM `pool_pool` WHERE `pool_id` IN (42915, 42913, 42929, 42928, 17099);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Tanaris - Group 1' WHERE `entry` IN (42915, 42913, 42929, 42928, 17099);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Tanaris - Group 1' WHERE `pool_entry` IN (42915, 42913, 42929, 42928, 17099);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4280+1, 1, 'Silver Vein / Gold Vein / Iron Deposit - Tanaris - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (42915, 4280+1, 0, 'Tanaris - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0), (42913, 4280+1, 0, 'Tanaris - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0), (42929, 4280+1, 0, 'Tanaris - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0), (42928, 4280+1, 0, 'Tanaris - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0), (17099, 4280+1, 0, 'Tanaris - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2165, 42931, 42914, 42916, 42912);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Tanaris - Group 2' WHERE `entry` IN (2165, 42931, 42914, 42916, 42912);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Tanaris - Group 2' WHERE `pool_entry` IN (2165, 42931, 42914, 42916, 42912);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4280+2, 1, 'Silver Vein / Gold Vein / Iron Deposit - Tanaris - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (2165, 4280+2, 0, 'Tanaris - Group 2 - Silver Vein / Gold Vein / Iron Deposit', 0), (42931, 4280+2, 0, 'Tanaris - Group 2 - Silver Vein / Gold Vein / Iron Deposit', 0), (42914, 4280+2, 0, 'Tanaris - Group 2 - Silver Vein / Gold Vein / Iron Deposit', 0), (42916, 4280+2, 0, 'Tanaris - Group 2 - Silver Vein / Gold Vein / Iron Deposit', 0), (42912, 4280+2, 0, 'Tanaris - Group 2 - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2166, 42926, 42932, 42911, 17098);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Tanaris - Group 3' WHERE `entry` IN (2166, 42926, 42932, 42911, 17098);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Tanaris - Group 3' WHERE `pool_entry` IN (2166, 42926, 42932, 42911, 17098);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4280+3, 1, 'Silver Vein / Gold Vein / Iron Deposit - Tanaris - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (2166, 4280+3, 0, 'Tanaris - Group 3 - Silver Vein / Gold Vein / Iron Deposit', 0), (42926, 4280+3, 0, 'Tanaris - Group 3 - Silver Vein / Gold Vein / Iron Deposit', 0), (42932, 4280+3, 0, 'Tanaris - Group 3 - Silver Vein / Gold Vein / Iron Deposit', 0), (42911, 4280+3, 0, 'Tanaris - Group 3 - Silver Vein / Gold Vein / Iron Deposit', 0), (17098, 4280+3, 0, 'Tanaris - Group 3 - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (42917, 17101, 42923, 42919, 17100);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Tanaris - Group 4' WHERE `entry` IN (42917, 17101, 42923, 42919, 17100);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Tanaris - Group 4' WHERE `pool_entry` IN (42917, 17101, 42923, 42919, 17100);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4280+4, 1, 'Silver Vein / Gold Vein / Iron Deposit - Tanaris - Group 4 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (42917, 4280+4, 0, 'Tanaris - Group 4 - Silver Vein / Gold Vein / Iron Deposit', 0), (17101, 4280+4, 0, 'Tanaris - Group 4 - Silver Vein / Gold Vein / Iron Deposit', 0), (42923, 4280+4, 0, 'Tanaris - Group 4 - Silver Vein / Gold Vein / Iron Deposit', 0), (42919, 4280+4, 0, 'Tanaris - Group 4 - Silver Vein / Gold Vein / Iron Deposit', 0), (17100, 4280+4, 0, 'Tanaris - Group 4 - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (42924, 42920, 42922, 42925);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Tanaris - Group 5' WHERE `entry` IN (42924, 42920, 42922, 42925);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Tanaris - Group 5' WHERE `pool_entry` IN (42924, 42920, 42922, 42925);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4280+5, 1, 'Silver Vein / Gold Vein / Iron Deposit - Tanaris - Group 5 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (42924, 4280+5, 0, 'Tanaris - Group 5 - Silver Vein / Gold Vein / Iron Deposit', 0), (42920, 4280+5, 0, 'Tanaris - Group 5 - Silver Vein / Gold Vein / Iron Deposit', 0), (42922, 4280+5, 0, 'Tanaris - Group 5 - Silver Vein / Gold Vein / Iron Deposit', 0), (42925, 4280+5, 0, 'Tanaris - Group 5 - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (42930, 42927, 42918, 42921);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Tanaris - Group 6' WHERE `entry` IN (42930, 42927, 42918, 42921);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Tanaris - Group 6' WHERE `pool_entry` IN (42930, 42927, 42918, 42921);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4280+6, 1, 'Silver Vein / Gold Vein / Iron Deposit - Tanaris - Group 6 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (42930, 4280+6, 0, 'Tanaris - Group 6 - Silver Vein / Gold Vein / Iron Deposit', 0), (42927, 4280+6, 0, 'Tanaris - Group 6 - Silver Vein / Gold Vein / Iron Deposit', 0), (42918, 4280+6, 0, 'Tanaris - Group 6 - Silver Vein / Gold Vein / Iron Deposit', 0), (42921, 4280+6, 0, 'Tanaris - Group 6 - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_template` WHERE `entry` = 1200;

-- End of migration.
;
