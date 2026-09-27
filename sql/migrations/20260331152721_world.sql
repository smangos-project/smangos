INSERT INTO `migrations` VALUES ('20260331152721');

DELETE FROM `pool_pool` WHERE `pool_id` IN (3206, 3203, 3208, 3207, 3205, 3209);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Thousand Needles - Group 1' WHERE `entry` IN (3206, 3203, 3208, 3207, 3205, 3209);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Thousand Needles - Group 1' WHERE `pool_entry` IN (3206, 3203, 3208, 3207, 3205, 3209);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3953+1, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Thousand Needles - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (3206, 3953+1, 0, 'Thousand Needles - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (3203, 3953+1, 0, 'Thousand Needles - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (3208, 3953+1, 0, 'Thousand Needles - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (3207, 3953+1, 0, 'Thousand Needles - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (3205, 3953+1, 0, 'Thousand Needles - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (3209, 3953+1, 0, 'Thousand Needles - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (3202, 5321, 5322, 5323, 5826, 3204);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Thousand Needles - Group 2' WHERE `entry` IN (3202, 5321, 5322, 5323, 5826, 3204);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Thousand Needles - Group 2' WHERE `pool_entry` IN (3202, 5321, 5322, 5323, 5826, 3204);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3953+2, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Thousand Needles - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (3202, 3953+2, 0, 'Thousand Needles - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5321, 3953+2, 0, 'Thousand Needles - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5322, 3953+2, 0, 'Thousand Needles - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5323, 3953+2, 0, 'Thousand Needles - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5826, 3953+2, 0, 'Thousand Needles - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (3204, 3953+2, 0, 'Thousand Needles - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_template` WHERE `entry` = 1132;

UPDATE `pool_template` SET `max_limit` =1 WHERE `entry` =397 AND `patch_min` =0 AND `patch_max` =10;

-- End of migration.
;
