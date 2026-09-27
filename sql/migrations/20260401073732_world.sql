INSERT INTO `migrations` VALUES ('20260401073732');

DELETE FROM `pool_pool` WHERE `pool_id` IN (6045, 6036, 6038, 6041, 6039);

UPDATE `pool_template` SET `description` = 'Tin Vein / Silver Vein - Dustwallow Marsh - Group 1' WHERE `entry` IN (6045, 6036, 6038, 6041, 6039);

UPDATE `pool_gameobject` SET `description` = 'Tin Vein / Silver Vein - Dustwallow Marsh - Group 1' WHERE `pool_entry` IN (6045, 6036, 6038, 6041, 6039);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3996+1, 1, 'Tin Vein / Silver Vein - Dustwallow Marsh - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6045, 3996+1, 0, 'Dustwallow Marsh - Group 1 - Tin Vein / Silver Vein', 0), (6036, 3996+1, 0, 'Dustwallow Marsh - Group 1 - Tin Vein / Silver Vein', 0), (6038, 3996+1, 0, 'Dustwallow Marsh - Group 1 - Tin Vein / Silver Vein', 0), (6041, 3996+1, 0, 'Dustwallow Marsh - Group 1 - Tin Vein / Silver Vein', 0), (6039, 3996+1, 0, 'Dustwallow Marsh - Group 1 - Tin Vein / Silver Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (6044, 6040, 10019, 6042, 6043, 6037);

UPDATE `pool_template` SET `description` = 'Tin Vein / Silver Vein - Dustwallow Marsh - Group 2' WHERE `entry` IN (6044, 6040, 10019, 6042, 6043, 6037);

UPDATE `pool_gameobject` SET `description` = 'Tin Vein / Silver Vein - Dustwallow Marsh - Group 2' WHERE `pool_entry` IN (6044, 6040, 10019, 6042, 6043, 6037);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3996+2, 1, 'Tin Vein / Silver Vein - Dustwallow Marsh - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6044, 3996+2, 0, 'Dustwallow Marsh - Group 2 - Tin Vein / Silver Vein', 0), (6040, 3996+2, 0, 'Dustwallow Marsh - Group 2 - Tin Vein / Silver Vein', 0), (10019, 3996+2, 0, 'Dustwallow Marsh - Group 2 - Tin Vein / Silver Vein', 0), (6042, 3996+2, 0, 'Dustwallow Marsh - Group 2 - Tin Vein / Silver Vein', 0), (6043, 3996+2, 0, 'Dustwallow Marsh - Group 2 - Tin Vein / Silver Vein', 0), (6037, 3996+2, 0, 'Dustwallow Marsh - Group 2 - Tin Vein / Silver Vein', 0);

DELETE FROM `pool_template` WHERE `entry` = 9905;

-- End of migration.
;
