INSERT INTO `migrations` VALUES ('20260322090533');

DELETE FROM `pool_pool` WHERE `pool_id` IN (1394, 1397, 1392, 1390);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Un''Goro Crater - Group 1' WHERE `entry` IN (1394, 1397, 1392, 1390);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Un''Goro Crater - Group 1' WHERE `pool_entry` IN (1394, 1397, 1392, 1390);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4272+1, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Un''Goro Crater - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (1394, 4272+1, 0, 'Un''Goro Crater - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (1397, 4272+1, 0, 'Un''Goro Crater - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (1392, 4272+1, 0, 'Un''Goro Crater - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (1390, 4272+1, 0, 'Un''Goro Crater - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (1401, 1398, 1402, 1395);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Un''Goro Crater - Group 2' WHERE `entry` IN (1401, 1398, 1402, 1395);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Un''Goro Crater - Group 2' WHERE `pool_entry` IN (1401, 1398, 1402, 1395);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4272+2, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Un''Goro Crater - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (1401, 4272+2, 0, 'Un''Goro Crater - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (1398, 4272+2, 0, 'Un''Goro Crater - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (1402, 4272+2, 0, 'Un''Goro Crater - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (1395, 4272+2, 0, 'Un''Goro Crater - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (1393, 1391, 1396, 1388);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Un''Goro Crater - Group 3' WHERE `entry` IN (1393, 1391, 1396, 1388);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Un''Goro Crater - Group 3' WHERE `pool_entry` IN (1393, 1391, 1396, 1388);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4272+3, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Un''Goro Crater - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (1393, 4272+3, 0, 'Un''Goro Crater - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (1391, 4272+3, 0, 'Un''Goro Crater - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (1396, 4272+3, 0, 'Un''Goro Crater - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (1388, 4272+3, 0, 'Un''Goro Crater - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (1387, 1386, 1389, 1399);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Un''Goro Crater - Group 4' WHERE `entry` IN (1387, 1386, 1389, 1399);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Un''Goro Crater - Group 4' WHERE `pool_entry` IN (1387, 1386, 1389, 1399);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4272+4, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Un''Goro Crater - Group 4 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (1387, 4272+4, 0, 'Un''Goro Crater - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (1386, 4272+4, 0, 'Un''Goro Crater - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (1389, 4272+4, 0, 'Un''Goro Crater - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (1399, 4272+4, 0, 'Un''Goro Crater - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_template` WHERE `entry` = 1210;

UPDATE `pool_template` SET `description` = 'Ooze Covered Truesilver Deposit / Ooze Covered Thorium Vein - Un''Goro Crater - Group 1 - Master' WHERE `entry` =1222 AND `patch_min` =0 AND `patch_max` =10;

UPDATE `pool_template` SET `description` = 'Ooze Covered Truesilver Deposit / Ooze Covered Thorium Vein - Un''Goro Crater - Group 2 - Master' WHERE `entry` =1223 AND `patch_min` =0 AND `patch_max` =10;

-- End of migration.
;
