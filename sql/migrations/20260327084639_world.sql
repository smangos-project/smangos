INSERT INTO `migrations` VALUES ('20260327084639');

DELETE FROM `pool_pool` WHERE `pool_id` IN (14294, 14299, 14292, 14300, 17183, 14288);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Winterspring - Group 1' WHERE `entry` IN (14294, 14299, 14292, 14300, 17183, 14288);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Winterspring - Group 1' WHERE `pool_entry` IN (14294, 14299, 14292, 14300, 17183, 14288);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4595+1, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Winterspring - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (14294, 4595+1, 0, 'Winterspring - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (14299, 4595+1, 0, 'Winterspring - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (14292, 4595+1, 0, 'Winterspring - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (14300, 4595+1, 0, 'Winterspring - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (17183, 4595+1, 0, 'Winterspring - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (14288, 4595+1, 0, 'Winterspring - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (17184, 17185, 14291, 14287, 14290);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Winterspring - Group 2' WHERE `entry` IN (17184, 17185, 14291, 14287, 14290);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Winterspring - Group 2' WHERE `pool_entry` IN (17184, 17185, 14291, 14287, 14290);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4595+2, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Winterspring - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (17184, 4595+2, 0, 'Winterspring - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (17185, 4595+2, 0, 'Winterspring - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (14291, 4595+2, 0, 'Winterspring - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (14287, 4595+2, 0, 'Winterspring - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (14290, 4595+2, 0, 'Winterspring - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (14301, 14293, 17186, 14289, 14286, 5329);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Winterspring - Group 3' WHERE `entry` IN (14301, 14293, 17186, 14289, 14286, 5329);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Winterspring - Group 3' WHERE `pool_entry` IN (14301, 14293, 17186, 14289, 14286, 5329);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4595+3, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Winterspring - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (14301, 4595+3, 0, 'Winterspring - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (14293, 4595+3, 0, 'Winterspring - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (17186, 4595+3, 0, 'Winterspring - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (14289, 4595+3, 0, 'Winterspring - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (14286, 4595+3, 0, 'Winterspring - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5329, 4595+3, 0, 'Winterspring - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (14298, 14297, 14295, 14296);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Winterspring - Group 4' WHERE `entry` IN (14298, 14297, 14295, 14296);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Winterspring - Group 4' WHERE `pool_entry` IN (14298, 14297, 14295, 14296);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4595+4, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Winterspring - Group 4 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (14298, 4595+4, 0, 'Winterspring - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (14297, 4595+4, 0, 'Winterspring - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (14295, 4595+4, 0, 'Winterspring - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (14296, 4595+4, 0, 'Winterspring - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_template` WHERE `entry` = 1234;

-- End of migration.
;
