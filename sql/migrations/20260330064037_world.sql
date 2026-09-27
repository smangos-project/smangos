INSERT INTO `migrations` VALUES ('20260330064037');

DELETE FROM `pool_pool` WHERE `pool_id` IN (4651, 4640, 4637, 4633, 4634);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Arathi Highlands - Group 1' WHERE `entry` IN (4651, 4640, 4637, 4633, 4634);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Arathi Highlands - Group 1' WHERE `pool_entry` IN (4651, 4640, 4637, 4633, 4634);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4599+1, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Arathi Highlands - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (4651, 4599+1, 0, 'Arathi Highlands - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4640, 4599+1, 0, 'Arathi Highlands - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4637, 4599+1, 0, 'Arathi Highlands - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4633, 4599+1, 0, 'Arathi Highlands - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4634, 4599+1, 0, 'Arathi Highlands - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (4644, 4645, 4646, 4629, 4628);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Arathi Highlands - Group 2' WHERE `entry` IN (4644, 4645, 4646, 4629, 4628);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Arathi Highlands - Group 2' WHERE `pool_entry` IN (4644, 4645, 4646, 4629, 4628);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4599+2, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Arathi Highlands - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (4644, 4599+2, 0, 'Arathi Highlands - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4645, 4599+2, 0, 'Arathi Highlands - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4646, 4599+2, 0, 'Arathi Highlands - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4629, 4599+2, 0, 'Arathi Highlands - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4628, 4599+2, 0, 'Arathi Highlands - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (4632, 4649, 4635, 4638, 4639);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Arathi Highlands - Group 3' WHERE `entry` IN (4632, 4649, 4635, 4638, 4639);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Arathi Highlands - Group 3' WHERE `pool_entry` IN (4632, 4649, 4635, 4638, 4639);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4599+3, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Arathi Highlands - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (4632, 4599+3, 0, 'Arathi Highlands - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4649, 4599+3, 0, 'Arathi Highlands - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4635, 4599+3, 0, 'Arathi Highlands - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4638, 4599+3, 0, 'Arathi Highlands - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4639, 4599+3, 0, 'Arathi Highlands - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (4642, 4636, 4650, 4631, 4641);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Arathi Highlands - Group 4' WHERE `entry` IN (4642, 4636, 4650, 4631, 4641);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Arathi Highlands - Group 4' WHERE `pool_entry` IN (4642, 4636, 4650, 4631, 4641);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4599+4, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Arathi Highlands - Group 4 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (4642, 4599+4, 0, 'Arathi Highlands - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4636, 4599+4, 0, 'Arathi Highlands - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4650, 4599+4, 0, 'Arathi Highlands - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4631, 4599+4, 0, 'Arathi Highlands - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4641, 4599+4, 0, 'Arathi Highlands - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (4647, 4630, 4648, 4643);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Arathi Highlands - Group 5' WHERE `entry` IN (4647, 4630, 4648, 4643);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Arathi Highlands - Group 5' WHERE `pool_entry` IN (4647, 4630, 4648, 4643);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4599+5, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Arathi Highlands - Group 5 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (4647, 4599+5, 0, 'Arathi Highlands - Group 5 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4630, 4599+5, 0, 'Arathi Highlands - Group 5 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4648, 4599+5, 0, 'Arathi Highlands - Group 5 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4643, 4599+5, 0, 'Arathi Highlands - Group 5 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (5330, 4655, 4653, 4652, 4658);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Arathi Highlands - Group 6' WHERE `entry` IN (5330, 4655, 4653, 4652, 4658);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Arathi Highlands - Group 6' WHERE `pool_entry` IN (5330, 4655, 4653, 4652, 4658);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4599+6, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Arathi Highlands - Group 6 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5330, 4599+6, 0, 'Arathi Highlands - Group 6 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4655, 4599+6, 0, 'Arathi Highlands - Group 6 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4653, 4599+6, 0, 'Arathi Highlands - Group 6 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4652, 4599+6, 0, 'Arathi Highlands - Group 6 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4658, 4599+6, 0, 'Arathi Highlands - Group 6 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (4656, 4657, 4659, 4654);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Arathi Highlands - Group 7' WHERE `entry` IN (4656, 4657, 4659, 4654);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Arathi Highlands - Group 7' WHERE `pool_entry` IN (4656, 4657, 4659, 4654);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4599+7, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Arathi Highlands - Group 7 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (4656, 4599+7, 0, 'Arathi Highlands - Group 7 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4657, 4599+7, 0, 'Arathi Highlands - Group 7 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4659, 4599+7, 0, 'Arathi Highlands - Group 7 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (4654, 4599+7, 0, 'Arathi Highlands - Group 7 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_template` WHERE `entry` = 1358;

DELETE FROM `gameobject` WHERE `guid` = 25333;

DELETE FROM `pool_template` WHERE `entry` = 298;

DELETE FROM `pool_gameobject` WHERE `pool_entry` = 298;

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4286+1, 1, 'Lesser Bloodstone Deposit - Arathi Highlands - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (112801, 4286+1, 'Lesser Bloodstone Deposit - Arathi Highlands - Group 1'), (25342, 4286+1, 'Lesser Bloodstone Deposit - Arathi Highlands - Group 1'), (45341, 4286+1, 'Lesser Bloodstone Deposit - Arathi Highlands - Group 1');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4286+2, 1, 'Lesser Bloodstone Deposit - Arathi Highlands - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (25332, 4286+2, 'Lesser Bloodstone Deposit - Arathi Highlands - Group 2'), (25337, 4286+2, 'Lesser Bloodstone Deposit - Arathi Highlands - Group 2'), (112800, 4286+2, 'Lesser Bloodstone Deposit - Arathi Highlands - Group 2');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4286+3, 1, 'Lesser Bloodstone Deposit - Arathi Highlands - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (25335, 4286+3, 'Lesser Bloodstone Deposit - Arathi Highlands - Group 3'), (112802, 4286+3, 'Lesser Bloodstone Deposit - Arathi Highlands - Group 3'), (25338, 4286+3, 'Lesser Bloodstone Deposit - Arathi Highlands - Group 3');

UPDATE `pool_template` SET `max_limit` =1 WHERE `entry` =146 AND `patch_min` =0 AND `patch_max` =10;

UPDATE `pool_template` SET `max_limit` =1 WHERE `entry` =152 AND `patch_min` =0 AND `patch_max` =10;

-- End of migration.
;
