INSERT INTO `migrations` VALUES ('20260409145906');

DELETE FROM `pool_pool` WHERE `pool_id` IN (5639, 5647, 5643, 5644);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Group 1' WHERE `entry` IN (5639, 5647, 5643, 5644);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Group 1' WHERE `pool_entry` IN (5639, 5647, 5643, 5644);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4941+1, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5639, 4941+1, 0, 'Stranglethorn Vale - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5647, 4941+1, 0, 'Stranglethorn Vale - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5643, 4941+1, 0, 'Stranglethorn Vale - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5644, 4941+1, 0, 'Stranglethorn Vale - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (5640, 5663, 5628, 5633, 5291, 5292);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Group 2' WHERE `entry` IN (5640, 5663, 5628, 5633, 5291, 5292);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Group 2' WHERE `pool_entry` IN (5640, 5663, 5628, 5633, 5291, 5292);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4941+2, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5640, 4941+2, 0, 'Stranglethorn Vale - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5663, 4941+2, 0, 'Stranglethorn Vale - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5628, 4941+2, 0, 'Stranglethorn Vale - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5633, 4941+2, 0, 'Stranglethorn Vale - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5291, 4941+2, 0, 'Stranglethorn Vale - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5292, 4941+2, 0, 'Stranglethorn Vale - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (5642, 5622, 5629, 5664, 5621, 5293);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Group 3' WHERE `entry` IN (5642, 5622, 5629, 5664, 5621, 5293);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Group 3' WHERE `pool_entry` IN (5642, 5622, 5629, 5664, 5621, 5293);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4941+3, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5642, 4941+3, 0, 'Stranglethorn Vale - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5622, 4941+3, 0, 'Stranglethorn Vale - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5629, 4941+3, 0, 'Stranglethorn Vale - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5664, 4941+3, 0, 'Stranglethorn Vale - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5621, 4941+3, 0, 'Stranglethorn Vale - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5293, 4941+3, 0, 'Stranglethorn Vale - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (5635, 5623, 5632, 5666, 5294, 5665);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Group 4' WHERE `entry` IN (5635, 5623, 5632, 5666, 5294, 5665);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Group 4' WHERE `pool_entry` IN (5635, 5623, 5632, 5666, 5294, 5665);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4941+4, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Group 4 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5635, 4941+4, 0, 'Stranglethorn Vale - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5623, 4941+4, 0, 'Stranglethorn Vale - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5632, 4941+4, 0, 'Stranglethorn Vale - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5666, 4941+4, 0, 'Stranglethorn Vale - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5294, 4941+4, 0, 'Stranglethorn Vale - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5665, 4941+4, 0, 'Stranglethorn Vale - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (5297, 5298, 5299, 5646, 5624, 5300);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Group 5' WHERE `entry` IN (5297, 5298, 5299, 5646, 5624, 5300);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Group 5' WHERE `pool_entry` IN (5297, 5298, 5299, 5646, 5624, 5300);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4941+5, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Group 5 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5297, 4941+5, 0, 'Stranglethorn Vale - Group 5 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5298, 4941+5, 0, 'Stranglethorn Vale - Group 5 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5299, 4941+5, 0, 'Stranglethorn Vale - Group 5 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5646, 4941+5, 0, 'Stranglethorn Vale - Group 5 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5624, 4941+5, 0, 'Stranglethorn Vale - Group 5 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5300, 4941+5, 0, 'Stranglethorn Vale - Group 5 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (5645, 5626, 5630, 5625, 5634, 5295);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Group 6' WHERE `entry` IN (5645, 5626, 5630, 5625, 5634, 5295);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Group 6' WHERE `pool_entry` IN (5645, 5626, 5630, 5625, 5634, 5295);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4941+6, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Group 6 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5645, 4941+6, 0, 'Stranglethorn Vale - Group 6 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5626, 4941+6, 0, 'Stranglethorn Vale - Group 6 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5630, 4941+6, 0, 'Stranglethorn Vale - Group 6 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5625, 4941+6, 0, 'Stranglethorn Vale - Group 6 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5634, 4941+6, 0, 'Stranglethorn Vale - Group 6 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5295, 4941+6, 0, 'Stranglethorn Vale - Group 6 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (5638, 5669, 5667, 5627, 5668, 5296);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Group 7' WHERE `entry` IN (5638, 5669, 5667, 5627, 5668, 5296);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Group 7' WHERE `pool_entry` IN (5638, 5669, 5667, 5627, 5668, 5296);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4941+7, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Group 7 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5638, 4941+7, 0, 'Stranglethorn Vale - Group 7 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5669, 4941+7, 0, 'Stranglethorn Vale - Group 7 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5667, 4941+7, 0, 'Stranglethorn Vale - Group 7 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5627, 4941+7, 0, 'Stranglethorn Vale - Group 7 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5668, 4941+7, 0, 'Stranglethorn Vale - Group 7 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5296, 4941+7, 0, 'Stranglethorn Vale - Group 7 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (5637, 5301, 5670, 5631, 5302, 5303);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Group 8' WHERE `entry` IN (5637, 5301, 5670, 5631, 5302, 5303);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Group 8' WHERE `pool_entry` IN (5637, 5301, 5670, 5631, 5302, 5303);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4941+8, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Group 8 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5637, 4941+8, 0, 'Stranglethorn Vale - Group 8 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5301, 4941+8, 0, 'Stranglethorn Vale - Group 8 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5670, 4941+8, 0, 'Stranglethorn Vale - Group 8 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5631, 4941+8, 0, 'Stranglethorn Vale - Group 8 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5302, 4941+8, 0, 'Stranglethorn Vale - Group 8 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5303, 4941+8, 0, 'Stranglethorn Vale - Group 8 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (5641, 5671, 5310, 5636, 5311, 5312);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Group 9' WHERE `entry` IN (5641, 5671, 5310, 5636, 5311, 5312);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Group 9' WHERE `pool_entry` IN (5641, 5671, 5310, 5636, 5311, 5312);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4941+9, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Group 9 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5641, 4941+9, 0, 'Stranglethorn Vale - Group 9 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5671, 4941+9, 0, 'Stranglethorn Vale - Group 9 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5310, 4941+9, 0, 'Stranglethorn Vale - Group 9 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5636, 4941+9, 0, 'Stranglethorn Vale - Group 9 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5311, 4941+9, 0, 'Stranglethorn Vale - Group 9 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5312, 4941+9, 0, 'Stranglethorn Vale - Group 9 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

DELETE FROM `pool_template` WHERE `entry` = 9910;

UPDATE `pool_template` SET `max_limit` =2 WHERE `entry` =8144;

UPDATE `pool_template` SET `max_limit` =2 WHERE `entry` =8142;

-- End of migration.
;
