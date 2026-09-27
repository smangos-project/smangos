INSERT INTO `migrations` VALUES ('20260403142051');

DELETE FROM `pool_pool` WHERE `pool_id` IN (990, 989, 988, 995, 987, 986);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Eastern Plaguelands - Group 1' WHERE `entry` IN (990, 989, 988, 995, 987, 986);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Eastern Plaguelands - Group 1' WHERE `pool_entry` IN (990, 989, 988, 995, 987, 986);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5097+1, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Eastern Plaguelands - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (990, 5097+1, 0, 'Eastern Plaguelands - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (989, 5097+1, 0, 'Eastern Plaguelands - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (988, 5097+1, 0, 'Eastern Plaguelands - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (995, 5097+1, 0, 'Eastern Plaguelands - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (987, 5097+1, 0, 'Eastern Plaguelands - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (986, 5097+1, 0, 'Eastern Plaguelands - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (994, 993, 992, 991, 17193);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Eastern Plaguelands - Group 2' WHERE `entry` IN (994, 993, 992, 991, 17193);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Eastern Plaguelands - Group 2' WHERE `pool_entry` IN (994, 993, 992, 991, 17193);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (5097+2, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Eastern Plaguelands - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (994, 5097+2, 0, 'Eastern Plaguelands - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (993, 5097+2, 0, 'Eastern Plaguelands - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (992, 5097+2, 0, 'Eastern Plaguelands - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (991, 5097+2, 0, 'Eastern Plaguelands - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (17193, 5097+2, 0, 'Eastern Plaguelands - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_template` WHERE `entry` = 1263;

-- End of migration.
;
