INSERT INTO `migrations` VALUES ('20260330125825');

DELETE FROM `pool_pool` WHERE `pool_id` IN (21483, 21479, 2168, 21482, 21484);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Hinterlands - Group 1' WHERE `entry` IN (21483, 21479, 2168, 21482, 21484);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Hinterlands - Group 1' WHERE `pool_entry` IN (21483, 21479, 2168, 21482, 21484);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3668+1, 1, 'Silver Vein / Gold Vein / Iron Deposit - Hinterlands - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (21483, 3668+1, 0, 'Hinterlands - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0), (21479, 3668+1, 0, 'Hinterlands - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0), (2168, 3668+1, 0, 'Hinterlands - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0), (21482, 3668+1, 0, 'Hinterlands - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0), (21484, 3668+1, 0, 'Hinterlands - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (21488, 17136, 21487, 21480, 21481);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Hinterlands - Group 2' WHERE `entry` IN (21488, 17136, 21487, 21480, 21481);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Hinterlands - Group 2' WHERE `pool_entry` IN (21488, 17136, 21487, 21480, 21481);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3668+2, 1, 'Silver Vein / Gold Vein / Iron Deposit - Hinterlands - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (21488, 3668+2, 0, 'Hinterlands - Group 2 - Silver Vein / Gold Vein / Iron Deposit', 0), (17136, 3668+2, 0, 'Hinterlands - Group 2 - Silver Vein / Gold Vein / Iron Deposit', 0), (21487, 3668+2, 0, 'Hinterlands - Group 2 - Silver Vein / Gold Vein / Iron Deposit', 0), (21480, 3668+2, 0, 'Hinterlands - Group 2 - Silver Vein / Gold Vein / Iron Deposit', 0), (21481, 3668+2, 0, 'Hinterlands - Group 2 - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_template` WHERE `entry` = 1349;

DELETE FROM `pool_pool` WHERE `pool_id` IN (4058, 4056, 4059, 4057);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Searing Gorge - Group 1' WHERE `entry` IN (4058, 4056, 4059, 4057);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Searing Gorge - Group 1' WHERE `pool_entry` IN (4058, 4056, 4059, 4057);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (295+1, 1, 'Silver Vein / Gold Vein / Iron Deposit - Searing Gorge - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (4058, 295+1, 0, 'Searing Gorge - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0), (4056, 295+1, 0, 'Searing Gorge - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0), (4059, 295+1, 0, 'Searing Gorge - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0), (4057, 295+1, 0, 'Searing Gorge - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_template` WHERE `entry` = 4351;

-- End of migration.
;
