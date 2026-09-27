INSERT INTO `migrations` VALUES ('20250216202539');

UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (17087, 17088, 17089, 21554, 1692, 1693, 1694, 1695, 1637, 1638, 5330, 4652, 4655, 4653, 4658, 4656, 4657, 4659, 4654));

DELETE FROM `pool_pool` WHERE `pool_id` IN (17087, 17088, 17089, 21554, 1692, 1693, 1694, 1695, 1637, 1638, 5330, 4652, 4655, 4653, 4658, 4656, 4657, 4659, 4654);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Arathi Highlands - Drywhisker Gorge' WHERE `entry` IN (17087, 17088, 17089, 21554, 1692, 1693, 1694, 1695, 1637, 1638);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Arathi Highlands - Drywhisker Gorge' WHERE `pool_entry` IN (17087, 17088, 17089, 21554, 1692, 1693, 1694, 1695, 1637, 1638);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Arathi Highlands - Drywhisker Gorge' WHERE `entry` IN (5330, 4652, 4655, 4653, 4658, 4656, 4657, 4659, 4654);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Arathi Highlands - Drywhisker Gorge' WHERE `pool_entry` IN (5330, 4652, 4655, 4653, 4658, 4656, 4657, 4659, 4654);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (145+1, 3, 'Arathi Highlands - Drywhisker Gorge - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (17087, 145+1, 0, 'Arathi Highlands - Drywhisker Gorge - Silver Vein / Gold Vein / Iron Deposit', 0), (17088, 145+1, 0, 'Arathi Highlands - Drywhisker Gorge - Silver Vein / Gold Vein / Iron Deposit', 0), (17089, 145+1, 0, 'Arathi Highlands - Drywhisker Gorge - Silver Vein / Gold Vein / Iron Deposit', 0), (21554, 145+1, 0, 'Arathi Highlands - Drywhisker Gorge - Silver Vein / Gold Vein / Iron Deposit', 0), (1692, 145+1, 0, 'Arathi Highlands - Drywhisker Gorge - Silver Vein / Gold Vein / Iron Deposit', 0), (1693, 145+1, 0, 'Arathi Highlands - Drywhisker Gorge - Silver Vein / Gold Vein / Iron Deposit', 0), (1694, 145+1, 0, 'Arathi Highlands - Drywhisker Gorge - Silver Vein / Gold Vein / Iron Deposit', 0), (1695, 145+1, 0, 'Arathi Highlands - Drywhisker Gorge - Silver Vein / Gold Vein / Iron Deposit', 0), (1637, 145+1, 0, 'Arathi Highlands - Drywhisker Gorge - Silver Vein / Gold Vein / Iron Deposit', 0), (1638, 145+1, 0, 'Arathi Highlands - Drywhisker Gorge - Silver Vein / Gold Vein / Iron Deposit', 0), (5330, 145+1, 0, 'Arathi Highlands - Drywhisker Gorge - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4652, 145+1, 0, 'Arathi Highlands - Drywhisker Gorge - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4655, 145+1, 0, 'Arathi Highlands - Drywhisker Gorge - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4653, 145+1, 0, 'Arathi Highlands - Drywhisker Gorge - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4658, 145+1, 0, 'Arathi Highlands - Drywhisker Gorge - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4656, 145+1, 0, 'Arathi Highlands - Drywhisker Gorge - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4657, 145+1, 0, 'Arathi Highlands - Drywhisker Gorge - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4659, 145+1, 0, 'Arathi Highlands - Drywhisker Gorge - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4654, 145+1, 0, 'Arathi Highlands - Drywhisker Gorge - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (17077, 17039, 17050, 17075, 17072, 1648, 1650, 4630, 4648, 4647, 4643));

DELETE FROM `pool_pool` WHERE `pool_id` IN (17077, 17039, 17050, 17075, 17072, 1648, 1650, 4630, 4648, 4647, 4643);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Arathi Highlands - Witherbark Village' WHERE `entry` IN (17077, 17039, 17050, 17075, 17072, 1648, 1650);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Arathi Highlands - Witherbark Village' WHERE `pool_entry` IN (17077, 17039, 17050, 17075, 17072, 1648, 1650);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Arathi Highlands - Witherbark Village' WHERE `entry` IN (4630, 4648, 4647, 4643);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Arathi Highlands - Witherbark Village' WHERE `pool_entry` IN (4630, 4648, 4647, 4643);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (151+1, 3, 'Arathi Highlands - Witherbark Village - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (17077, 151+1, 0, 'Arathi Highlands - Witherbark Village - Silver Vein / Gold Vein / Iron Deposit', 0), (17039, 151+1, 0, 'Arathi Highlands - Witherbark Village - Silver Vein / Gold Vein / Iron Deposit', 0), (17050, 151+1, 0, 'Arathi Highlands - Witherbark Village - Silver Vein / Gold Vein / Iron Deposit', 0), (17075, 151+1, 0, 'Arathi Highlands - Witherbark Village - Silver Vein / Gold Vein / Iron Deposit', 0), (17072, 151+1, 0, 'Arathi Highlands - Witherbark Village - Silver Vein / Gold Vein / Iron Deposit', 0), (1648, 151+1, 0, 'Arathi Highlands - Witherbark Village - Silver Vein / Gold Vein / Iron Deposit', 0), (1650, 151+1, 0, 'Arathi Highlands - Witherbark Village - Silver Vein / Gold Vein / Iron Deposit', 0), (4630, 151+1, 0, 'Arathi Highlands - Witherbark Village - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4648, 151+1, 0, 'Arathi Highlands - Witherbark Village - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4647, 151+1, 0, 'Arathi Highlands - Witherbark Village - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4643, 151+1, 0, 'Arathi Highlands - Witherbark Village - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (1652, 1649, 1647, 17071, 17073, 23052, 21673, 21674, 17074, 17040));

DELETE FROM `pool_pool` WHERE `pool_id` IN (1652, 1649, 1647, 17071, 17073, 23052, 21673, 21674, 17074, 17040);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Arathi Highlands - Boulderfist Hall' WHERE `entry` IN (1652, 1649, 1647, 17071, 17073, 23052, 21673, 21674, 17074, 17040);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Arathi Highlands - Boulderfist Hall' WHERE `pool_entry` IN (1652, 1649, 1647, 17071, 17073, 23052, 21673, 21674, 17074, 17040);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (161+1, 2, 'Arathi Highlands - Boulderfist Hall - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (1652, 161+1, 0, 'Arathi Highlands - Boulderfist Hall - Silver Vein / Gold Vein / Iron Deposit', 0), (1649, 161+1, 0, 'Arathi Highlands - Boulderfist Hall - Silver Vein / Gold Vein / Iron Deposit', 0), (1647, 161+1, 0, 'Arathi Highlands - Boulderfist Hall - Silver Vein / Gold Vein / Iron Deposit', 0), (17071, 161+1, 0, 'Arathi Highlands - Boulderfist Hall - Silver Vein / Gold Vein / Iron Deposit', 0), (17073, 161+1, 0, 'Arathi Highlands - Boulderfist Hall - Silver Vein / Gold Vein / Iron Deposit', 0), (23052, 161+1, 0, 'Arathi Highlands - Boulderfist Hall - Silver Vein / Gold Vein / Iron Deposit', 0), (21673, 161+1, 0, 'Arathi Highlands - Boulderfist Hall - Silver Vein / Gold Vein / Iron Deposit', 0), (21674, 161+1, 0, 'Arathi Highlands - Boulderfist Hall - Silver Vein / Gold Vein / Iron Deposit', 0), (17074, 161+1, 0, 'Arathi Highlands - Boulderfist Hall - Silver Vein / Gold Vein / Iron Deposit', 0), (17040, 161+1, 0, 'Arathi Highlands - Boulderfist Hall - Silver Vein / Gold Vein / Iron Deposit', 0);

UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (17084, 21675, 1667, 1668, 1669, 1671, 1670, 1674));

DELETE FROM `pool_pool` WHERE `pool_id` IN (17084, 21675, 1667, 1668, 1669, 1671, 1670, 1674);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Arathi Highlands - Boulderfist Outpost' WHERE `entry` IN (17084, 21675, 1667, 1668, 1669, 1671, 1670, 1674);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Arathi Highlands - Boulderfist Outpost' WHERE `pool_entry` IN (17084, 21675, 1667, 1668, 1669, 1671, 1670, 1674);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (251+1, 2, 'Arathi Highlands - Boulderfist Outpost - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (17084, 251+1, 0, 'Arathi Highlands - Boulderfist Outpost - Silver Vein / Gold Vein / Iron Deposit', 0), (21675, 251+1, 0, 'Arathi Highlands - Boulderfist Outpost - Silver Vein / Gold Vein / Iron Deposit', 0), (1667, 251+1, 0, 'Arathi Highlands - Boulderfist Outpost - Silver Vein / Gold Vein / Iron Deposit', 0), (1668, 251+1, 0, 'Arathi Highlands - Boulderfist Outpost - Silver Vein / Gold Vein / Iron Deposit', 0), (1669, 251+1, 0, 'Arathi Highlands - Boulderfist Outpost - Silver Vein / Gold Vein / Iron Deposit', 0), (1671, 251+1, 0, 'Arathi Highlands - Boulderfist Outpost - Silver Vein / Gold Vein / Iron Deposit', 0), (1670, 251+1, 0, 'Arathi Highlands - Boulderfist Outpost - Silver Vein / Gold Vein / Iron Deposit', 0), (1674, 251+1, 0, 'Arathi Highlands - Boulderfist Outpost - Silver Vein / Gold Vein / Iron Deposit', 0);

UPDATE `pool_template` SET `max_limit` = 6 WHERE `entry` = 406;

UPDATE `pool_template` SET `max_limit` = 4 WHERE `entry` = 1358;

UPDATE `pool_template` SET `max_limit` = 14 WHERE `entry` = 323;

UPDATE `gameobject` SET `spawntimesecsmin` = 300, `spawntimesecsmax` = 900 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (6066, 6086, 6085, 7064, 6067, 6090));

DELETE FROM `pool_pool` WHERE `pool_id` IN (6066, 6086, 6085, 7064, 6067, 6090);

UPDATE `pool_template` SET `description` = 'Tin Vein / Silver Vein - Ashenvale - Thistlefur Hold' WHERE `entry` IN (6066, 6086, 6085, 7064, 6067, 6090);

UPDATE `pool_gameobject` SET `description` = 'Tin Vein / Silver Vein - Ashenvale - Thistlefur Hold' WHERE `pool_entry` IN (6066, 6086, 6085, 7064, 6067, 6090);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (263+1, 2, 'Ashenvale - Thistlefur Hold - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6066, 263+1, 0, 'Ashenvale - Thistlefur Hold - Tin Vein / Silver Vein', 0), (6086, 263+1, 0, 'Ashenvale - Thistlefur Hold - Tin Vein / Silver Vein', 0), (6085, 263+1, 0, 'Ashenvale - Thistlefur Hold - Tin Vein / Silver Vein', 0), (7064, 263+1, 0, 'Ashenvale - Thistlefur Hold - Tin Vein / Silver Vein', 0), (6067, 263+1, 0, 'Ashenvale - Thistlefur Hold - Tin Vein / Silver Vein', 0), (6090, 263+1, 0, 'Ashenvale - Thistlefur Hold - Tin Vein / Silver Vein', 0);

UPDATE `gameobject` SET `spawntimesecsmin` = 300, `spawntimesecsmax` = 900 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (9282, 6079, 9283, 9284, 6094, 6080, 9286, 9285, 6069));

UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (6182, 2865, 6185, 6186, 6184, 6187, 17127));

DELETE FROM `pool_pool` WHERE `pool_id` IN (9282, 6079, 9283, 9284, 6094, 6080, 9286, 9285, 6069, 6182, 2865, 6185, 6186, 6184, 6187, 17127);

UPDATE `pool_template` SET `description` = 'Tin Vein / Silver Vein - Ashenvale - Demon Fall Canyon' WHERE `entry` IN (9282, 6079, 9283, 9284, 6094, 6080, 9286, 9285, 6069);

UPDATE `pool_gameobject` SET `description` = 'Tin Vein / Silver Vein - Ashenvale - Demon Fall Canyon' WHERE `pool_entry` IN (9282, 6079, 9283, 9284, 6094, 6080, 9286, 9285, 6069);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Ashenvale - Demon Fall Canyon' WHERE `entry` IN (6182, 2865, 6185, 6186, 6184, 6187, 17127);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Ashenvale - Demon Fall Canyon' WHERE `pool_entry` IN (6182, 2865, 6185, 6186, 6184, 6187, 17127);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (264+1, 3, 'Ashenvale - Demon Fall Canyon - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (9282, 264+1, 0, 'Ashenvale - Demon Fall Canyon - Tin Vein / Silver Vein', 0), (6079, 264+1, 0, 'Ashenvale - Demon Fall Canyon - Tin Vein / Silver Vein', 0), (9283, 264+1, 0, 'Ashenvale - Demon Fall Canyon - Tin Vein / Silver Vein', 0), (9284, 264+1, 0, 'Ashenvale - Demon Fall Canyon - Tin Vein / Silver Vein', 0), (6094, 264+1, 0, 'Ashenvale - Demon Fall Canyon - Tin Vein / Silver Vein', 0), (6080, 264+1, 0, 'Ashenvale - Demon Fall Canyon - Tin Vein / Silver Vein', 0), (9286, 264+1, 0, 'Ashenvale - Demon Fall Canyon - Tin Vein / Silver Vein', 0), (9285, 264+1, 0, 'Ashenvale - Demon Fall Canyon - Tin Vein / Silver Vein', 0), (6069, 264+1, 0, 'Ashenvale - Demon Fall Canyon - Tin Vein / Silver Vein', 0), (6182, 264+1, 0, 'Ashenvale - Demon Fall Canyon - Silver Vein / Gold Vein / Iron Deposit', 0), (2865, 264+1, 0, 'Ashenvale - Demon Fall Canyon - Silver Vein / Gold Vein / Iron Deposit', 0), (6185, 264+1, 0, 'Ashenvale - Demon Fall Canyon - Silver Vein / Gold Vein / Iron Deposit', 0), (6186, 264+1, 0, 'Ashenvale - Demon Fall Canyon - Silver Vein / Gold Vein / Iron Deposit', 0), (6184, 264+1, 0, 'Ashenvale - Demon Fall Canyon - Silver Vein / Gold Vein / Iron Deposit', 0), (6187, 264+1, 0, 'Ashenvale - Demon Fall Canyon - Silver Vein / Gold Vein / Iron Deposit', 0), (17127, 264+1, 0, 'Ashenvale - Demon Fall Canyon - Silver Vein / Gold Vein / Iron Deposit', 0);

UPDATE `pool_template` SET `max_limit` = 13 WHERE `entry` = 1075;

UPDATE `pool_template` SET `max_limit` = 15 WHERE `entry` = 1080;

UPDATE `pool_template` SET `max_limit` = 9 WHERE `entry` = 1351;

UPDATE `gameobject` SET `spawntimesecsmin` = 300, `spawntimesecsmax` = 900 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (2477, 2500, 2497, 2495, 2501));

UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (6352, 6348, 6342, 6344));

DELETE FROM `pool_pool` WHERE `pool_id` IN (2477, 2500, 2497, 2495, 2501, 6352, 6348, 6342, 6344);

UPDATE `pool_template` SET `description` = 'Tin Vein / Silver Vein - Duskwood - Vul''Gol Ogre Mound' WHERE `entry` IN (2477, 2500, 2497, 2495, 2501);

UPDATE `pool_gameobject` SET `description` = 'Tin Vein / Silver Vein - Duskwood - Vul''Gol Ogre Mound' WHERE `pool_entry` IN (2477, 2500, 2497, 2495, 2501);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Duskwood - Vul''Gol Ogre Mound' WHERE `entry` IN (6352, 6348, 6342, 6344);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Duskwood - Vul''Gol Ogre Mound' WHERE `pool_entry` IN (6352, 6348, 6342, 6344);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (265+1, 3, 'Duskwood - Vul''Gol Ogre Mound - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (2477, 265+1, 0, 'Duskwood - Vul''Gol Ogre Mound - Tin Vein / Silver Vein', 0), (2500, 265+1, 0, 'Duskwood - Vul''Gol Ogre Mound - Tin Vein / Silver Vein', 0), (2497, 265+1, 0, 'Duskwood - Vul''Gol Ogre Mound - Tin Vein / Silver Vein', 0), (2495, 265+1, 0, 'Duskwood - Vul''Gol Ogre Mound - Tin Vein / Silver Vein', 0), (2501, 265+1, 0, 'Duskwood - Vul''Gol Ogre Mound - Tin Vein / Silver Vein', 0), (6352, 265+1, 0, 'Duskwood - Vul''Gol Ogre Mound - Silver Vein / Gold Vein / Iron Deposit', 0), (6348, 265+1, 0, 'Duskwood - Vul''Gol Ogre Mound - Silver Vein / Gold Vein / Iron Deposit', 0), (6342, 265+1, 0, 'Duskwood - Vul''Gol Ogre Mound - Silver Vein / Gold Vein / Iron Deposit', 0), (6344, 265+1, 0, 'Duskwood - Vul''Gol Ogre Mound - Silver Vein / Gold Vein / Iron Deposit', 0);

UPDATE `gameobject` SET `spawntimesecsmin` = 300, `spawntimesecsmax` = 900 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (7003, 2503, 2502, 2475));

UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (6354, 6346, 6330, 6350));

DELETE FROM `pool_pool` WHERE `pool_id` IN (7003, 2503, 2502, 2475, 6354, 6346, 6330, 6350);

UPDATE `pool_template` SET `description` = 'Tin Vein / Silver Vein - Duskwood - Roland''s Doom' WHERE `entry` IN (7003, 2503, 2502, 2475);

UPDATE `pool_gameobject` SET `description` = 'Tin Vein / Silver Vein - Duskwood - Roland''s Doom' WHERE `pool_entry` IN (7003, 2503, 2502, 2475);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Duskwood - Roland''s Doom' WHERE `entry` IN (6354, 6346, 6330, 6350);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Duskwood - Roland''s Doom' WHERE `pool_entry` IN (6354, 6346, 6330, 6350);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (266+1, 2, 'Duskwood - Roland''s Doom - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (2475, 266+1, 0, 'Duskwood - Roland''s Doom - Tin Vein / Silver Vein', 0), (7003, 266+1, 0, 'Duskwood - Roland''s Doom - Tin Vein / Silver Vein', 0), (2503, 266+1, 0, 'Duskwood - Roland''s Doom - Tin Vein / Silver Vein', 0), (2502, 266+1, 0, 'Duskwood - Roland''s Doom - Tin Vein / Silver Vein', 0), (6354, 266+1, 0, 'Duskwood - Roland''s Doom - Silver Vein / Gold Vein / Iron Deposit', 0), (6346, 266+1, 0, 'Duskwood - Roland''s Doom - Silver Vein / Gold Vein / Iron Deposit', 0), (6330, 266+1, 0, 'Duskwood - Roland''s Doom - Silver Vein / Gold Vein / Iron Deposit', 0), (6350, 266+1, 0, 'Duskwood - Roland''s Doom - Silver Vein / Gold Vein / Iron Deposit', 0);

UPDATE `gameobject` SET `spawntimesecsmin` = 300, `spawntimesecsmax` = 900 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (2487, 2490));

UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (6334, 6335));

DELETE FROM `pool_pool` WHERE `pool_id` IN (2487, 2490, 6334, 6335);

UPDATE `pool_template` SET `description` = 'Tin Vein / Silver Vein - Duskwood - Misc Cave' WHERE `entry` IN (2487, 2490);

UPDATE `pool_gameobject` SET `description` = 'Tin Vein / Silver Vein - Duskwood - Misc Cave' WHERE `pool_entry` IN (2487, 2490);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Duskwood - Misc Cave' WHERE `entry` IN (6334, 6335);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Duskwood - Misc Cave' WHERE `pool_entry` IN (6334, 6335);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (267+1, 1, 'Duskwood - Misc Cave - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (2487, 267+1, 0, 'Duskwood - Misc Cave - Tin Vein / Silver Vein', 0), (2490, 267+1, 0, 'Duskwood - Misc Cave - Tin Vein / Silver Vein', 0), (6334, 267+1, 0, 'Duskwood - Misc Cave - Silver Vein / Gold Vein / Iron Deposit', 0), (6335, 267+1, 0, 'Duskwood - Misc Cave - Silver Vein / Gold Vein / Iron Deposit', 0);

-- Wetlands
 UPDATE `pool_gameobject` SET `description` = 'Tin Vein / Silver Vein - Wetlands - Thelgen Rock' WHERE `guid` IN (251658, 251655);

UPDATE `gameobject` SET `spawntimesecsmin` = 300, `spawntimesecsmax` = 900 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (23022, 2808, 2819, 2818));

UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (17025, 6117, 6130, 6133, 6121, 6123, 6122, 6118, 6120, 2820, 6119, 6132));

DELETE FROM `pool_pool` WHERE `pool_id` IN (23022, 2808, 2819, 2818, 17025, 6117, 6130, 6133, 6121, 6123, 6122, 6118, 6120, 2820, 6119, 6132);

UPDATE `pool_template` SET `description` = 'Tin Vein / Silver Vein - Wetlands - Raptor Ridge' WHERE `entry` IN (23022, 2808, 2819, 2818);

UPDATE `pool_gameobject` SET `description` = 'Tin Vein / Silver Vein - Wetlands - Raptor Ridge' WHERE `pool_entry` IN (23022, 2808, 2819, 2818);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Wetlands - Raptor Ridge' WHERE `entry` IN (17025, 6117, 6130, 6133, 6121, 6123, 6122, 6118, 6120, 2820, 6119, 6132);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Wetlands - Raptor Ridge' WHERE `pool_entry` IN (17025, 6117, 6130, 6133, 6121, 6123, 6122, 6118, 6120, 2820, 6119, 6132);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (268+1, 3, 'Wetlands - Raptor Ridge - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (23022, 268+1, 0, 'Wetlands - Raptor Ridge - Tin Vein / Silver Vein', 0), (2808, 268+1, 0, 'Wetlands - Raptor Ridge - Tin Vein / Silver Vein', 0), (2819, 268+1, 0, 'Wetlands - Raptor Ridge - Tin Vein / Silver Vein', 0), (2818, 268+1, 0, 'Wetlands - Raptor Ridge - Tin Vein / Silver Vein', 0), (17025, 268+1, 0, 'Wetlands - Raptor Ridge - Silver Vein / Gold Vein / Iron Deposit', 0), (6117, 268+1, 0, 'Wetlands - Raptor Ridge - Silver Vein / Gold Vein / Iron Deposit', 0), (6130, 268+1, 0, 'Wetlands - Raptor Ridge - Silver Vein / Gold Vein / Iron Deposit', 0), (6133, 268+1, 0, 'Wetlands - Raptor Ridge - Silver Vein / Gold Vein / Iron Deposit', 0), (6121, 268+1, 0, 'Wetlands - Raptor Ridge - Silver Vein / Gold Vein / Iron Deposit', 0), (6123, 268+1, 0, 'Wetlands - Raptor Ridge - Silver Vein / Gold Vein / Iron Deposit', 0), (6122, 268+1, 0, 'Wetlands - Raptor Ridge - Silver Vein / Gold Vein / Iron Deposit', 0), (6118, 268+1, 0, 'Wetlands - Raptor Ridge - Silver Vein / Gold Vein / Iron Deposit', 0), (6120, 268+1, 0, 'Wetlands - Raptor Ridge - Silver Vein / Gold Vein / Iron Deposit', 0), (2820, 268+1, 0, 'Wetlands - Raptor Ridge - Silver Vein / Gold Vein / Iron Deposit', 0), (6119, 268+1, 0, 'Wetlands - Raptor Ridge - Silver Vein / Gold Vein / Iron Deposit', 0), (6132, 268+1, 0, 'Wetlands - Raptor Ridge - Silver Vein / Gold Vein / Iron Deposit', 0);

UPDATE `gameobject` SET `spawntimesecsmin` = 300, `spawntimesecsmax` = 900 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (2785, 23020));

UPDATE `gameobject` SET `spawntimesecsmin` = 300, `spawntimesecsmax` = 900 WHERE `guid` = 14920;

DELETE FROM `pool_pool` WHERE `pool_id` IN (2785, 23020);

DELETE FROM `pool_gameobject` WHERE `guid` IN (14920);

UPDATE `pool_template` SET `description` = 'Tin Vein / Silver Vein - Wetlands - Misc Cave' WHERE `entry` IN (2785, 23020);

UPDATE `pool_gameobject` SET `description` = 'Tin Vein / Silver Vein - Wetlands - Misc Cave' WHERE `pool_entry` IN (2785, 23020);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `patch_max`) VALUES (269+1, 1, 'Copper Veins - Wetlands - Misc Cave', 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`, `patch_max`) VALUES (14920, 269+1, 'Copper Veins - Wetlands - Misc Cave', 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (269+2, 1, 'Wetlands - Misc Cave - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (269+1, 269+2, 0, 'Wetlands - Misc Cave - Copper Veins', 0), (2785, 269+2, 0, 'Wetlands - Misc Cave - Tin Vein / Silver Vein', 0), (23020, 269+2, 0, 'Wetlands - Misc Cave - Tin Vein / Silver Vein', 0);

UPDATE `pool_template` SET `max_limit` = 9 WHERE `entry` = 1156;

-- Hinterlands
 UPDATE `pool_template` SET `description` = 'Hinterlands - Silver Vein / Gold Vein / Iron Deposit (Master Pool)' WHERE `entry` = 1349;

UPDATE `pool_template` SET `max_limit` = 17 WHERE `entry` = 1067;

UPDATE `pool_template` SET `max_limit` = 2 WHERE `entry` = 1349;

UPDATE `pool_template` SET `max_limit` = 3 WHERE `entry` = 1070;

UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (3069, 3067, 3075, 3073, 3065, 3063, 3070, 3079, 3078, 3071, 3066));

DELETE FROM `pool_pool` WHERE `pool_id` IN (3069, 3067, 3075, 3073, 3065, 3063, 3070, 3079, 3078, 3071, 3066);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Hinterlands - Skull Rock' WHERE `entry` IN (3069, 3067, 3075, 3073, 3065, 3063, 3070, 3079, 3078, 3071, 3066);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Hinterlands - Skull Rock' WHERE `pool_entry` IN (3069, 3067, 3075, 3073, 3065, 3063, 3070, 3079, 3078, 3071, 3066);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (292+1, 2, 'Hinterlands - Skull Rock - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (3069, 292+1, 0, 'Hinterlands - Skull Rock - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (3067, 292+1, 0, 'Hinterlands - Skull Rock - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (3075, 292+1, 0, 'Hinterlands - Skull Rock - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (3073, 292+1, 0, 'Hinterlands - Skull Rock - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (3065, 292+1, 0, 'Hinterlands - Skull Rock - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (3063, 292+1, 0, 'Hinterlands - Skull Rock - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (3070, 292+1, 0, 'Hinterlands - Skull Rock - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (3079, 292+1, 0, 'Hinterlands - Skull Rock - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (3078, 292+1, 0, 'Hinterlands - Skull Rock - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (3071, 292+1, 0, 'Hinterlands - Skull Rock - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (3066, 292+1, 0, 'Hinterlands - Skull Rock - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (9441, 9440, 25618, 17206));

DELETE FROM `pool_pool` WHERE `pool_id` IN (9441, 9440, 25618, 17206);

UPDATE `pool_template` SET `description` = 'Small Thorium Vein / Truesilver Deposit - Hinterlands - Jintha''Alor Cave 1' WHERE `entry` IN (9441, 9440, 25618, 17206);

UPDATE `pool_gameobject` SET `description` = 'Small Thorium Vein / Truesilver Deposit - Hinterlands - Jintha''Alor Cave 1' WHERE `pool_entry` IN (9441, 9440, 25618, 17206);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (293+1, 3, 'Hinterlands - Jintha''Alor Cave 1 - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (9441, 293+1, 0, 'Hinterlands - Jintha''Alor Cave 1 - Small Thorium Vein / Truesilver Deposit', 0), (9440, 293+1, 0, 'Hinterlands - Jintha''Alor Cave 1 - Small Thorium Vein / Truesilver Deposit', 0), (25618, 293+1, 0, 'Hinterlands - Jintha''Alor Cave 1 - Small Thorium Vein / Truesilver Deposit', 0), (17206, 293+1, 0, 'Hinterlands - Jintha''Alor Cave 1 - Small Thorium Vein / Truesilver Deposit', 0);

UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (25612, 25619, 25611, 3031));

DELETE FROM `pool_pool` WHERE `pool_id` IN (25612, 25619, 25611, 3031);

UPDATE `pool_template` SET `description` = 'Small Thorium Vein / Truesilver Deposit - Hinterlands - Jintha''Alor Cave 2' WHERE `entry` IN (25612, 25619, 25611, 3031);

UPDATE `pool_gameobject` SET `description` = 'Small Thorium Vein / Truesilver Deposit - Hinterlands - Jintha''Alor Cave 2' WHERE `pool_entry` IN (25612, 25619, 25611, 3031);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (294+1, 1, 'Hinterlands - Jintha''Alor Cave 2 - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (25612, 294+1, 0, 'Hinterlands - Jintha''Alor Cave 2 - Small Thorium Vein / Truesilver Deposit', 0), (25619, 294+1, 0, 'Hinterlands - Jintha''Alor Cave 2 - Small Thorium Vein / Truesilver Deposit', 0), (25611, 294+1, 0, 'Hinterlands - Jintha''Alor Cave 2 - Small Thorium Vein / Truesilver Deposit', 0), (3031, 294+1, 0, 'Hinterlands - Jintha''Alor Cave 2 - Small Thorium Vein / Truesilver Deposit', 0);

UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (21492, 21493, 23008, 21491, 21494));

DELETE FROM `pool_pool` WHERE `pool_id` IN (21492, 21493, 23008, 21491, 21494);

UPDATE `pool_template` SET `description` = 'Small Thorium Vein / Truesilver Deposit - Feralas - Ruins of Isildien' WHERE `entry` IN (21492, 21493, 23008, 21491, 21494);

UPDATE `pool_gameobject` SET `description` = 'Small Thorium Vein / Truesilver Deposit - Feralas - Ruins of Isildien' WHERE `pool_entry` IN (21492, 21493, 23008, 21491, 21494);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (295+1, 2, 'Feralas - Ruins of Isildien - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (21492, 295+1, 0, 'Feralas - Ruins of Isildien - Small Thorium Vein / Truesilver Deposit', 0), (21493, 295+1, 0, 'Feralas - Ruins of Isildien - Small Thorium Vein / Truesilver Deposit', 0), (23008, 295+1, 0, 'Feralas - Ruins of Isildien - Small Thorium Vein / Truesilver Deposit', 0), (21491, 295+1, 0, 'Feralas - Ruins of Isildien - Small Thorium Vein / Truesilver Deposit', 0), (21494, 295+1, 0, 'Feralas - Ruins of Isildien - Small Thorium Vein / Truesilver Deposit', 0);

UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (17116, 17117, 17115, 17053, 32988, 32982, 32983, 2167, 6364, 6374, 6363, 6376, 6375));

DELETE FROM `pool_pool` WHERE `pool_id` IN (17116, 17117, 17115, 17053, 32988, 32982, 32983, 2167, 6364, 6374, 6363, 6376, 6375);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Feralas - Feral Scar Vale' WHERE `entry` IN (17116, 17117, 17115, 17053, 32988, 32982, 32983, 2167);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Feralas - Feral Scar Vale' WHERE `pool_entry` IN (17116, 17117, 17115, 17053, 32988, 32982, 32983, 2167);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Feralas - Feral Scar Vale' WHERE `entry` IN (6364, 6374, 6363, 6376, 6375);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Feralas - Feral Scar Vale' WHERE `pool_entry` IN (6364, 6374, 6363, 6376, 6375);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (296+1, 3, 'Feralas - Feral Scar Vale - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (17116, 296+1, 0, 'Feralas - Feral Scar Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (17117, 296+1, 0, 'Feralas - Feral Scar Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (17115, 296+1, 0, 'Feralas - Feral Scar Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (17053, 296+1, 0, 'Feralas - Feral Scar Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (32988, 296+1, 0, 'Feralas - Feral Scar Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (32982, 296+1, 0, 'Feralas - Feral Scar Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (32983, 296+1, 0, 'Feralas - Feral Scar Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (2167, 296+1, 0, 'Feralas - Feral Scar Vale - Silver Vein / Gold Vein / Iron Deposit', 0), (6364, 296+1, 0, 'Feralas - Feral Scar Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (6374, 296+1, 0, 'Feralas - Feral Scar Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (6363, 296+1, 0, 'Feralas - Feral Scar Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (6376, 296+1, 0, 'Feralas - Feral Scar Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (6375, 296+1, 0, 'Feralas - Feral Scar Vale - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (6360, 6392, 6380, 6382, 6381, 6359));

DELETE FROM `pool_pool` WHERE `pool_id` IN (6360, 6392, 6380, 6382, 6381, 6359);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Feralas - Rage Scar Hold' WHERE `entry` IN (6360, 6392, 6380, 6382, 6381, 6359);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Feralas - Rage Scar Hold' WHERE `pool_entry` IN (6360, 6392, 6380, 6382, 6381, 6359);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (320+1, 2, 'Feralas - Rage Scar Hold - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6360, 320+1, 0, 'Feralas - Rage Scar Hold - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (6392, 320+1, 0, 'Feralas - Rage Scar Hold - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (6380, 320+1, 0, 'Feralas - Rage Scar Hold - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (6382, 320+1, 0, 'Feralas - Rage Scar Hold - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (6381, 320+1, 0, 'Feralas - Rage Scar Hold - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (6359, 320+1, 0, 'Feralas - Rage Scar Hold - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (6387, 6370, 6389, 6371, 6390, 6372, 6388));

DELETE FROM `pool_pool` WHERE `pool_id` IN (6387, 6370, 6389, 6371, 6390, 6372, 6388);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Feralas - Shalzaru''s Lair' WHERE `entry` IN (6387, 6370, 6389, 6371, 6390, 6372, 6388);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Feralas - Shalzaru''s Lair' WHERE `pool_entry` IN (6387, 6370, 6389, 6371, 6390, 6372, 6388);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (329+1, 3, 'Feralas - Shalzaru''s Lair - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6387, 329+1, 0, 'Feralas - Shalzaru''s Lair - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (6370, 329+1, 0, 'Feralas - Shalzaru''s Lair - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (6389, 329+1, 0, 'Feralas - Shalzaru''s Lair - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (6371, 329+1, 0, 'Feralas - Shalzaru''s Lair - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (6390, 329+1, 0, 'Feralas - Shalzaru''s Lair - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (6388, 329+1, 0, 'Feralas - Shalzaru''s Lair - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (6372, 329+1, 0, 'Feralas - Shalzaru''s Lair - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

-- Searing Gorge
 UPDATE `pool_template` SET `max_limit` = 3 WHERE `entry` = 4350;

UPDATE `pool_template` SET `max_limit` = 7 WHERE `entry` = 1338;

UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (4013, 4035, 4012, 4044, 4055, 4049));

DELETE FROM `pool_pool` WHERE `pool_id` IN (4013, 4035, 4012, 4044, 4055, 4049);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Searing Gorge - Firewatch Ridge' WHERE `entry` IN (4013, 4035, 4012);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Searing Gorge - Firewatch Ridge' WHERE `pool_entry` IN (4013, 4035, 4012);

UPDATE `pool_template` SET `description` = 'Small Thorium Vein / Truesilver Deposit / Dark Iron Deposit - Searing Gorge - Firewatch Ridge' WHERE `entry` IN (4044, 4055, 4049);

UPDATE `pool_gameobject` SET `description` = 'Small Thorium Vein / Truesilver Deposit / Dark Iron Deposit - Searing Gorge - Firewatch Ridge' WHERE `pool_entry` IN (4044, 4055, 4049);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (330+1, 3, 'Searing Gorge - Firewatch Ridge - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (4013, 330+1, 0, 'Searing Gorge - Firewatch Ridge - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (4035, 330+1, 0, 'Searing Gorge - Firewatch Ridge - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (4012, 330+1, 0, 'Searing Gorge - Firewatch Ridge - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (4044, 330+1, 0, 'Searing Gorge - Firewatch Ridge - Small Thorium Vein / Truesilver Deposit / Dark Iron Deposit', 0), (4055, 330+1, 0, 'Searing Gorge - Firewatch Ridge - Small Thorium Vein / Truesilver Deposit / Dark Iron Deposit', 0), (4049, 330+1, 0, 'Searing Gorge - Firewatch Ridge - Small Thorium Vein / Truesilver Deposit / Dark Iron Deposit', 0);

UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (4027, 4033, 4017, 4037, 4038, 4031, 4014, 4040, 4051, 4050, 4046, 4045));

DELETE FROM `pool_pool` WHERE `pool_id` IN (4027, 4033, 4017, 4037, 4038, 4031, 4014, 4040, 4051, 4050, 4046, 4045);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Searing Gorge - Slag Pit' WHERE `entry` IN (4027, 4033, 4017, 4037, 4038, 4031, 4014, 4040);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Searing Gorge - Slag Pit' WHERE `pool_entry` IN (4027, 4033, 4017, 4037, 4038, 4031, 4014, 4040);

UPDATE `pool_template` SET `description` = 'Small Thorium Vein / Truesilver Deposit / Dark Iron Deposit - Searing Gorge - Slag Pit' WHERE `entry` IN (4051, 4050, 4046, 4045);

UPDATE `pool_gameobject` SET `description` = 'Small Thorium Vein / Truesilver Deposit / Dark Iron Deposit - Searing Gorge - Slag Pit' WHERE `pool_entry` IN (4051, 4050, 4046, 4045);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (332+1, 3, 'Searing Gorge - Slag Pit - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (4027, 332+1, 0, 'Searing Gorge - Slag Pit - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (4033, 332+1, 0, 'Searing Gorge - Slag Pit - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (4017, 332+1, 0, 'Searing Gorge - Slag Pit - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (4037, 332+1, 0, 'Searing Gorge - Slag Pit - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (4038, 332+1, 0, 'Searing Gorge - Slag Pit - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (4031, 332+1, 0, 'Searing Gorge - Slag Pit - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (4014, 332+1, 0, 'Searing Gorge - Slag Pit - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (4040, 332+1, 0, 'Searing Gorge - Slag Pit - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (4051, 332+1, 0, 'Searing Gorge - Slag Pit - Small Thorium Vein / Truesilver Deposit / Dark Iron Deposit', 0), (4050, 332+1, 0, 'Searing Gorge - Slag Pit - Small Thorium Vein / Truesilver Deposit / Dark Iron Deposit', 0), (4046, 332+1, 0, 'Searing Gorge - Slag Pit - Small Thorium Vein / Truesilver Deposit / Dark Iron Deposit', 0), (4045, 332+1, 0, 'Searing Gorge - Slag Pit - Small Thorium Vein / Truesilver Deposit / Dark Iron Deposit', 0);

-- Barrens
 UPDATE `pool_template` SET `max_limit` = 11 WHERE `entry` = 2588;

UPDATE `pool_template` SET `max_limit` = 37 WHERE `entry` = 4303;

UPDATE `gameobject` SET `spawntimesecsmin` = 300, `spawntimesecsmax` = 900 WHERE `guid` IN (40279, 40280, 40281);

DELETE FROM `pool_gameobject` WHERE `guid` IN (40279, 40280, 40281);

UPDATE `gameobject` SET `spawntimesecsmin` = 300, `spawntimesecsmax` = 900 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (8896, 8895, 8897));

DELETE FROM `pool_pool` WHERE `pool_id` IN (8896, 8895, 8897);

UPDATE `pool_template` SET `description` = 'Tin Vein / Silver Vein - Barrens - Boulder Lode Mine' WHERE `entry` IN (8896, 8895, 8897);

UPDATE `pool_gameobject` SET `description` = 'Tin Vein / Silver Vein - Barrens - Boulder Lode Mine' WHERE `pool_entry` IN (8896, 8895, 8897);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `patch_max`) VALUES (391+1, 1, 'Copper Veins - Barrens - Boulder Lode Mine', 10), (391+2, 1, 'Copper Veins - Barrens - Boulder Lode Mine', 10), (391+3, 1, 'Copper Veins - Barrens - Boulder Lode Mine', 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`, `patch_max`) VALUES (40279, 391+1, 'Copper Veins - Barrens - Boulder Lode Mine', 10), (40280, 391+2, 'Copper Veins - Barrens - Boulder Lode Mine', 10), (40281, 391+3, 'Copper Veins - Barrens - Boulder Lode Mine', 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (391+4, 2, 'Barrens - Boulder Lode Mine - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (391+1, 391+4, 0, 'Barrens - Boulder Lode Mine - Copper Veins', 0), (391+2, 391+4, 0, 'Barrens - Boulder Lode Mine - Copper Veins', 0), (391+3, 391+4, 0, 'Barrens - Boulder Lode Mine - Copper Veins', 0), (8896, 391+4, 0, 'Barrens - Boulder Lode Mine - Tin Vein / Silver Vein', 0), (8895, 391+4, 0, 'Barrens - Boulder Lode Mine - Tin Vein / Silver Vein', 0), (8897, 391+4, 0, 'Barrens - Boulder Lode Mine - Tin Vein / Silver Vein', 0);

UPDATE `gameobject` SET `spawntimesecsmin` = 300, `spawntimesecsmax` = 900 WHERE `guid` IN (46505, 46506);

DELETE FROM `pool_gameobject` WHERE `guid` IN (46505, 46506);

UPDATE `gameobject` SET `spawntimesecsmin` = 300, `spawntimesecsmax` = 900 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (8914, 8889, 8890, 8891, 8900));

DELETE FROM `pool_pool` WHERE `pool_id` IN (8914, 8889, 8890, 8891, 8900);

UPDATE `pool_template` SET `description` = 'Tin Vein / Silver Vein - Barrens - Forgotten Pools' WHERE `entry` IN (8914, 8889, 8890, 8891, 8900);

UPDATE `pool_gameobject` SET `description` = 'Tin Vein / Silver Vein - Barrens - Forgotten Pools' WHERE `pool_entry` IN (8914, 8889, 8890, 8891, 8900);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `patch_max`) VALUES (515+1, 1, 'Copper Veins - Barrens - Forgotten Pools', 10), (515+2, 1, 'Copper Veins - Barrens - Forgotten Pools', 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`, `patch_max`) VALUES (46505, 515+1, 'Copper Veins - Barrens - Forgotten Pools', 10), (46506, 515+2, 'Copper Veins - Barrens - Forgotten Pools', 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (515+3, 1, 'Barrens - Forgotten Pools - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (515+1, 515+3, 0, 'Barrens - Forgotten Pools - Copper Veins', 0), (515+2, 515+3, 0, 'Barrens - Forgotten Pools - Copper Veins', 0), (8914, 515+3, 0, 'Barrens - Forgotten Pools - Tin Vein / Silver Vein', 0), (8889, 515+3, 0, 'Barrens - Forgotten Pools - Tin Vein / Silver Vein', 0), (8890, 515+3, 0, 'Barrens - Forgotten Pools - Tin Vein / Silver Vein', 0), (8891, 515+3, 0, 'Barrens - Forgotten Pools - Tin Vein / Silver Vein', 0), (8900, 515+3, 0, 'Barrens - Forgotten Pools - Tin Vein / Silver Vein', 0);

UPDATE `gameobject` SET `spawntimesecsmin` = 300, `spawntimesecsmax` = 900 WHERE `guid` IN (46495, 40211, 46494, 40210);

DELETE FROM `pool_gameobject` WHERE `guid` IN (46495, 40211, 46494, 40210);

UPDATE `gameobject` SET `spawntimesecsmin` = 300, `spawntimesecsmax` = 900 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (8913, 8886, 8885, 8884, 8912));

DELETE FROM `pool_pool` WHERE `pool_id` IN (8913, 8886, 8885, 8884, 8912);

UPDATE `pool_template` SET `description` = 'Tin Vein / Silver Vein - Barrens - Lushwater Oasis' WHERE `entry` IN (8913, 8886, 8885, 8884, 8912);

UPDATE `pool_gameobject` SET `description` = 'Tin Vein / Silver Vein - Barrens - Lushwater Oasis' WHERE `pool_entry` IN (8913, 8886, 8885, 8884, 8912);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `patch_max`) VALUES (1493+1, 1, 'Copper Veins - Barrens - Lushwater Oasis', 10), (1493+2, 1, 'Copper Veins - Barrens - Lushwater Oasis', 10), (1493+3, 1, 'Copper Veins - Barrens - Lushwater Oasis', 10), (1493+4, 1, 'Copper Veins - Barrens - Lushwater Oasis', 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`, `patch_max`) VALUES (46495, 1493+1, 'Copper Veins - Barrens - Lushwater Oasis', 10), (40211, 1493+2, 'Copper Veins - Barrens - Lushwater Oasis', 10), (46494, 1493+3, 'Copper Veins - Barrens - Lushwater Oasis', 10), (40210, 1493+4, 'Copper Veins - Barrens - Lushwater Oasis', 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (1493+5, 2, 'Barrens - Lushwater Oasis - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (1493+1, 1493+5, 0, 'Barrens - Lushwater Oasis - Copper Veins', 0), (1493+2, 1493+5, 0, 'Barrens - Lushwater Oasis - Copper Veins', 0), (1493+3, 1493+5, 0, 'Barrens - Lushwater Oasis - Copper Veins', 0), (1493+4, 1493+5, 0, 'Barrens - Lushwater Oasis - Copper Veins', 0), (8913, 1493+5, 0, 'Barrens - Lushwater Oasis - Tin Vein / Silver Vein', 0), (8886, 1493+5, 0, 'Barrens - Lushwater Oasis - Tin Vein / Silver Vein', 0), (8885, 1493+5, 0, 'Barrens - Lushwater Oasis - Tin Vein / Silver Vein', 0), (8884, 1493+5, 0, 'Barrens - Lushwater Oasis - Tin Vein / Silver Vein', 0), (8912, 1493+5, 0, 'Barrens - Lushwater Oasis - Tin Vein / Silver Vein', 0);

UPDATE `gameobject` SET `spawntimesecsmin` = 300, `spawntimesecsmax` = 900 WHERE `guid` IN (84133, 46493);

DELETE FROM `pool_gameobject` WHERE `guid` IN (84133, 46493);

UPDATE `gameobject` SET `spawntimesecsmin` = 300, `spawntimesecsmax` = 900 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (8898, 8911, 8883));

DELETE FROM `pool_pool` WHERE `pool_id` IN (8898, 8911, 8883);

UPDATE `pool_template` SET `description` = 'Tin Vein / Silver Vein - Barrens - Stagnant Oasis' WHERE `entry` IN (8898, 8911, 8883);

UPDATE `pool_gameobject` SET `description` = 'Tin Vein / Silver Vein - Barrens - Stagnant Oasis' WHERE `pool_entry` IN (8898, 8911, 8883);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `patch_max`) VALUES (1174+1, 1, 'Copper Veins - Barrens - Stagnant Oasis', 10), (1174+2, 1, 'Copper Veins - Barrens - Stagnant Oasis', 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`, `patch_max`) VALUES (84133, 1174+1, 'Copper Veins - Barrens - Stagnant Oasis', 10), (46493, 1174+2, 'Copper Veins - Barrens - Stagnant Oasis', 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (1174+3, 1, 'Barrens - Stagnant Oasis - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (1174+1, 1174+3, 0, 'Barrens - Stagnant Oasis - Copper Veins', 0), (1174+2, 1174+3, 0, 'Barrens - Stagnant Oasis - Copper Veins', 0), (8898, 1174+3, 0, 'Barrens - Stagnant Oasis - Tin Vein / Silver Vein', 0), (8911, 1174+3, 0, 'Barrens - Stagnant Oasis - Tin Vein / Silver Vein', 0), (8883, 1174+3, 0, 'Barrens - Stagnant Oasis - Tin Vein / Silver Vein', 0);

-- Stonetalon Mountains
 UPDATE `gameobject` SET `position_x` = 18.176 WHERE `guid` = 2516734;

UPDATE `pool_template` SET `max_limit` = 10 WHERE `entry` = 1090;

UPDATE `gameobject` SET `spawntimesecsmin` = 300, `spawntimesecsmax` = 900 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (23015, 2293, 2292, 2294, 2291, 1090));

DELETE FROM `pool_pool` WHERE `pool_id` IN (23015, 2293, 2292, 2294, 2291, 1090);

UPDATE `pool_template` SET `description` = 'Tin Vein / Silver Vein - Stonetalon Mountains - Boulderslide Cavern' WHERE `entry` IN (23015, 2293, 2292, 2294, 2291, 1090);

UPDATE `pool_gameobject` SET `description` = 'Tin Vein / Silver Vein - Stonetalon Mountains - Boulderslide Cavern' WHERE `pool_entry` IN (23015, 2293, 2292, 2294, 2291, 1090);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (333+1, 2, 'Stonetalon Mountains - Boulderslide Cavern - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (23015, 333+1, 0, 'Stonetalon Mountains - Boulderslide Cavern - Tin Vein / Silver Vein', 0), (2293, 333+1, 0, 'Stonetalon Mountains - Boulderslide Cavern - Tin Vein / Silver Vein', 0), (2292, 333+1, 0, 'Stonetalon Mountains - Boulderslide Cavern - Tin Vein / Silver Vein', 0), (2294, 333+1, 0, 'Stonetalon Mountains - Boulderslide Cavern - Tin Vein / Silver Vein', 0), (2291, 333+1, 0, 'Stonetalon Mountains - Boulderslide Cavern - Tin Vein / Silver Vein', 0), (1090, 333+1, 0, 'Stonetalon Mountains - Boulderslide Cavern - Tin Vein / Silver Vein', 0);

UPDATE `gameobject` SET `spawntimesecsmin` = 300, `spawntimesecsmax` = 900 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (2308, 2305, 2302, 2303, 2301, 2306, 2304, 23024, 23013));

DELETE FROM `pool_pool` WHERE `pool_id` IN (2308, 2305, 2302, 2303, 2301, 2306, 2304, 23024, 23013);

UPDATE `pool_template` SET `description` = 'Tin Vein / Silver Vein - Stonetalon Mountains - Windshear Mine' WHERE `entry` IN (2308, 2305, 2302, 2303, 2301, 2306, 2304, 23024, 23013);

UPDATE `pool_gameobject` SET `description` = 'Tin Vein / Silver Vein - Stonetalon Mountains - Windshear Mine' WHERE `pool_entry` IN (2308, 2305, 2302, 2303, 2301, 2306, 2304, 23024, 23013);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (385+1, 3, 'Stonetalon Mountains - Windshear Mine - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (2308, 385+1, 0, 'Stonetalon Mountains - Windshear Mine - Tin Vein / Silver Vein', 0), (2305, 385+1, 0, 'Stonetalon Mountains - Windshear Mine - Tin Vein / Silver Vein', 0), (2302, 385+1, 0, 'Stonetalon Mountains - Windshear Mine - Tin Vein / Silver Vein', 0), (2303, 385+1, 0, 'Stonetalon Mountains - Windshear Mine - Tin Vein / Silver Vein', 0), (2301, 385+1, 0, 'Stonetalon Mountains - Windshear Mine - Tin Vein / Silver Vein', 0), (2306, 385+1, 0, 'Stonetalon Mountains - Windshear Mine - Tin Vein / Silver Vein', 0), (2304, 385+1, 0, 'Stonetalon Mountains - Windshear Mine - Tin Vein / Silver Vein', 0), (23024, 385+1, 0, 'Stonetalon Mountains - Windshear Mine - Tin Vein / Silver Vein', 0), (23013, 385+1, 0, 'Stonetalon Mountains - Windshear Mine - Tin Vein / Silver Vein', 0);

-- Thousand Needles
 UPDATE `pool_template` SET `max_limit` = 14 WHERE `entry` = 329;

UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (5819, 5787, 5785, 5784, 5782, 5818, 5783));

DELETE FROM `pool_pool` WHERE `pool_id` IN (5819, 5787, 5785, 5784, 5782, 5818, 5783);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Thousand Needles - Roguefeather Den' WHERE `entry` IN (5819, 5787, 5785, 5784, 5782, 5818, 5783);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Thousand Needles - Roguefeather Den' WHERE `pool_entry` IN (5819, 5787, 5785, 5784, 5782, 5818, 5783);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (395+1, 2, 'Thousand Needles - Roguefeather Den - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5819, 395+1, 0, 'Thousand Needles - Roguefeather Den - Silver Vein / Gold Vein / Iron Deposit', 0), (5787, 395+1, 0, 'Thousand Needles - Roguefeather Den - Silver Vein / Gold Vein / Iron Deposit', 0), (5785, 395+1, 0, 'Thousand Needles - Roguefeather Den - Silver Vein / Gold Vein / Iron Deposit', 0), (5784, 395+1, 0, 'Thousand Needles - Roguefeather Den - Silver Vein / Gold Vein / Iron Deposit', 0), (5782, 395+1, 0, 'Thousand Needles - Roguefeather Den - Silver Vein / Gold Vein / Iron Deposit', 0), (5818, 395+1, 0, 'Thousand Needles - Roguefeather Den - Silver Vein / Gold Vein / Iron Deposit', 0), (5783, 395+1, 0, 'Thousand Needles - Roguefeather Den - Silver Vein / Gold Vein / Iron Deposit', 0);

UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (17112, 5825, 5800, 5799, 5806, 5830, 5828, 5827, 5809, 5808, 5802, 5804, 3203, 3206, 3209, 3205, 3207, 3208));

DELETE FROM `pool_pool` WHERE `pool_id` IN (17112, 5825, 5800, 5799, 5806, 5830, 5828, 5827, 5809, 5808, 5802, 5804, 3203, 3206, 3209, 3205, 3207, 3208);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Thousand Needles - Splithoof Hold' WHERE `entry` IN (17112, 5825, 5800, 5799, 5806, 5830, 5828, 5827, 5809, 5808, 5802, 5804);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Thousand Needles - Splithoof Hold' WHERE `pool_entry` IN (17112, 5825, 5800, 5799, 5806, 5830, 5828, 5827, 5809, 5808, 5802, 5804);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Thousand Needles - Splithoof Hold' WHERE `entry` IN (3203, 3206, 3209, 3205, 3207, 3208);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Thousand Needles - Splithoof Hold' WHERE `pool_entry` IN (3203, 3206, 3209, 3205, 3207, 3208);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (396+1, 3, 'Thousand Needles - Splithoof Hold - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (17112, 396+1, 0, 'Thousand Needles - Splithoof Hold - Silver Vein / Gold Vein / Iron Deposit', 0), (5825, 396+1, 0, 'Thousand Needles - Splithoof Hold - Silver Vein / Gold Vein / Iron Deposit', 0), (5800, 396+1, 0, 'Thousand Needles - Splithoof Hold - Silver Vein / Gold Vein / Iron Deposit', 0), (5799, 396+1, 0, 'Thousand Needles - Splithoof Hold - Silver Vein / Gold Vein / Iron Deposit', 0), (5806, 396+1, 0, 'Thousand Needles - Splithoof Hold - Silver Vein / Gold Vein / Iron Deposit', 0), (5830, 396+1, 0, 'Thousand Needles - Splithoof Hold - Silver Vein / Gold Vein / Iron Deposit', 0), (5828, 396+1, 0, 'Thousand Needles - Splithoof Hold - Silver Vein / Gold Vein / Iron Deposit', 0), (5827, 396+1, 0, 'Thousand Needles - Splithoof Hold - Silver Vein / Gold Vein / Iron Deposit', 0), (5809, 396+1, 0, 'Thousand Needles - Splithoof Hold - Silver Vein / Gold Vein / Iron Deposit', 0), (5808, 396+1, 0, 'Thousand Needles - Splithoof Hold - Silver Vein / Gold Vein / Iron Deposit', 0), (5802, 396+1, 0, 'Thousand Needles - Splithoof Hold - Silver Vein / Gold Vein / Iron Deposit', 0), (5804, 396+1, 0, 'Thousand Needles - Splithoof Hold - Silver Vein / Gold Vein / Iron Deposit', 0), (3203, 396+1, 0, 'Thousand Needles - Splithoof Hold - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (3206, 396+1, 0, 'Thousand Needles - Splithoof Hold - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (3209, 396+1, 0, 'Thousand Needles - Splithoof Hold - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (3205, 396+1, 0, 'Thousand Needles - Splithoof Hold - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (3207, 396+1, 0, 'Thousand Needles - Splithoof Hold - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (3208, 396+1, 0, 'Thousand Needles - Splithoof Hold - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

-- End of migration.
;
