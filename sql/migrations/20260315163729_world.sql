INSERT INTO `migrations` VALUES ('20260315163729');

DELETE FROM `pool_pool` WHERE `pool_id` IN (5608, 5602, 5610, 5616, 5601);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Silithus - Group 1' WHERE `entry` IN (5608, 5602, 5610, 5616, 5601);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Silithus - Group 1' WHERE `pool_entry` IN (5608, 5602, 5610, 5616, 5601);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4095+1, 1, 'Silithus - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5608, 4095+1, 0, 'Silithus - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5602, 4095+1, 0, 'Silithus - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5610, 4095+1, 0, 'Silithus - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5616, 4095+1, 0, 'Silithus - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5601, 4095+1, 0, 'Silithus - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (5612, 5615, 5316, 5317, 5605);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Silithus - Group 2' WHERE `entry` IN (5612, 5615, 5316, 5317, 5605);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Silithus - Group 2' WHERE `pool_entry` IN (5612, 5615, 5316, 5317, 5605);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4095+2, 1, 'Silithus - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5612, 4095+2, 0, 'Silithus - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5615, 4095+2, 0, 'Silithus - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5316, 4095+2, 0, 'Silithus - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5317, 4095+2, 0, 'Silithus - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5605, 4095+2, 0, 'Silithus - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (5604, 5609, 5611, 5603, 5318);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Silithus - Group 3' WHERE `entry` IN (5604, 5609, 5611, 5603, 5318);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Silithus - Group 3' WHERE `pool_entry` IN (5604, 5609, 5611, 5603, 5318);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4095+3, 1, 'Silithus - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5604, 4095+3, 0, 'Silithus - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5609, 4095+3, 0, 'Silithus - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5611, 4095+3, 0, 'Silithus - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5603, 4095+3, 0, 'Silithus - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5318, 4095+3, 0, 'Silithus - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (5613, 5607, 5319, 5320, 5614, 5606);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Silithus - Group 4' WHERE `entry` IN (5613, 5607, 5319, 5320, 5614, 5606);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Silithus - Group 4' WHERE `pool_entry` IN (5613, 5607, 5319, 5320, 5614, 5606);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4095+4, 1, 'Silithus - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5613, 4095+4, 0, 'Silithus - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5607, 4095+4, 0, 'Silithus - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5319, 4095+4, 0, 'Silithus - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5320, 4095+4, 0, 'Silithus - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5614, 4095+4, 0, 'Silithus - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5606, 4095+4, 0, 'Silithus - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_template` WHERE `entry` = 326;

-- End of migration.
;
