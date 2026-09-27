INSERT INTO `migrations` VALUES ('20260324170726');

DELETE FROM `pool_pool` WHERE `pool_id` IN (8865, 8862, 8860, 8858, 8857);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stonetalon Mountains - Group 1' WHERE `entry` IN (8865, 8862, 8860, 8858, 8857);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stonetalon Mountains - Group 1' WHERE `pool_entry` IN (8865, 8862, 8860, 8858, 8857);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3687+1, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stonetalon Mountains - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (8865, 3687+1, 0, 'Stonetalon Mountains - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (8862, 3687+1, 0, 'Stonetalon Mountains - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (8860, 3687+1, 0, 'Stonetalon Mountains - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (8858, 3687+1, 0, 'Stonetalon Mountains - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (8857, 3687+1, 0, 'Stonetalon Mountains - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (8856, 8854, 8855, 8859, 5324);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stonetalon Mountains - Group 2' WHERE `entry` IN (8856, 8854, 8855, 8859, 5324);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stonetalon Mountains - Group 2' WHERE `pool_entry` IN (8856, 8854, 8855, 8859, 5324);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3687+2, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stonetalon Mountains - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (8856, 3687+2, 0, 'Stonetalon Mountains - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (8854, 3687+2, 0, 'Stonetalon Mountains - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (8855, 3687+2, 0, 'Stonetalon Mountains - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (8859, 3687+2, 0, 'Stonetalon Mountains - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5324, 3687+2, 0, 'Stonetalon Mountains - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (5325, 8863, 8864, 8861, 5326);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stonetalon Mountains - Group 3' WHERE `entry` IN (5325, 8863, 8864, 8861, 5326);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stonetalon Mountains - Group 3' WHERE `pool_entry` IN (5325, 8863, 8864, 8861, 5326);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3687+3, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stonetalon Mountains - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5325, 3687+3, 0, 'Stonetalon Mountains - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (8863, 3687+3, 0, 'Stonetalon Mountains - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (8864, 3687+3, 0, 'Stonetalon Mountains - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (8861, 3687+3, 0, 'Stonetalon Mountains - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5326, 3687+3, 0, 'Stonetalon Mountains - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_template` WHERE `entry` = 1927;

DELETE FROM `pool_pool` WHERE `pool_id` IN (7281, 7284, 7282, 7280, 5332, 7283);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Hillsbrad Foothills - Group 1' WHERE `entry` IN (7281, 7284, 7282, 7280, 5332, 7283);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Hillsbrad Foothills - Group 1' WHERE `pool_entry` IN (7281, 7284, 7282, 7280, 5332, 7283);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (303+1, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Hillsbrad Foothills - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (7281, 303+1, 0, 'Hillsbrad Foothills - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (7284, 303+1, 0, 'Hillsbrad Foothills - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (7282, 303+1, 0, 'Hillsbrad Foothills - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (7280, 303+1, 0, 'Hillsbrad Foothills - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5332, 303+1, 0, 'Hillsbrad Foothills - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (7283, 303+1, 0, 'Hillsbrad Foothills - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

-- End of migration.
;
