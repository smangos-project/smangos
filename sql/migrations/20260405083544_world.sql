INSERT INTO `migrations` VALUES ('20260405083544');

DELETE FROM `pool_pool` WHERE `pool_id` IN (21593, 17069, 17068, 21588, 21585, 17065);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Swamp of Sorrows - Group 1' WHERE `entry` IN (21593, 17069, 17068, 21588, 21585, 17065);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Swamp of Sorrows - Group 1' WHERE `pool_entry` IN (21593, 17069, 17068, 21588, 21585, 17065);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4691+1, 1, 'Silver Vein / Gold Vein / Iron Deposit - Swamp of Sorrows - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (21593, 4691+1, 0, 'Swamp of Sorrows - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0), (17069, 4691+1, 0, 'Swamp of Sorrows - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0), (17068, 4691+1, 0, 'Swamp of Sorrows - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0), (21588, 4691+1, 0, 'Swamp of Sorrows - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0), (21585, 4691+1, 0, 'Swamp of Sorrows - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0), (17065, 4691+1, 0, 'Swamp of Sorrows - Group 1 - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (21592, 34919, 21587, 17067, 17066, 21586);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Swamp of Sorrows - Group 2' WHERE `entry` IN (21592, 34919, 21587, 17067, 17066, 21586);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Swamp of Sorrows - Group 2' WHERE `pool_entry` IN (21592, 34919, 21587, 17067, 17066, 21586);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4691+2, 1, 'Silver Vein / Gold Vein / Iron Deposit - Swamp of Sorrows - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (21592, 4691+2, 0, 'Swamp of Sorrows - Group 2 - Silver Vein / Gold Vein / Iron Deposit', 0), (34919, 4691+2, 0, 'Swamp of Sorrows - Group 2 - Silver Vein / Gold Vein / Iron Deposit', 0), (21587, 4691+2, 0, 'Swamp of Sorrows - Group 2 - Silver Vein / Gold Vein / Iron Deposit', 0), (17067, 4691+2, 0, 'Swamp of Sorrows - Group 2 - Silver Vein / Gold Vein / Iron Deposit', 0), (17066, 4691+2, 0, 'Swamp of Sorrows - Group 2 - Silver Vein / Gold Vein / Iron Deposit', 0), (21586, 4691+2, 0, 'Swamp of Sorrows - Group 2 - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (21582, 21584, 21581, 21576, 21575, 21578);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Swamp of Sorrows - Group 3' WHERE `entry` IN (21582, 21584, 21581, 21576, 21575, 21578);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Swamp of Sorrows - Group 3' WHERE `pool_entry` IN (21582, 21584, 21581, 21576, 21575, 21578);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4691+3, 1, 'Silver Vein / Gold Vein / Iron Deposit - Swamp of Sorrows - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (21582, 4691+3, 0, 'Swamp of Sorrows - Group 3 - Silver Vein / Gold Vein / Iron Deposit', 0), (21584, 4691+3, 0, 'Swamp of Sorrows - Group 3 - Silver Vein / Gold Vein / Iron Deposit', 0), (21581, 4691+3, 0, 'Swamp of Sorrows - Group 3 - Silver Vein / Gold Vein / Iron Deposit', 0), (21576, 4691+3, 0, 'Swamp of Sorrows - Group 3 - Silver Vein / Gold Vein / Iron Deposit', 0), (21575, 4691+3, 0, 'Swamp of Sorrows - Group 3 - Silver Vein / Gold Vein / Iron Deposit', 0), (21578, 4691+3, 0, 'Swamp of Sorrows - Group 3 - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (21577, 21590, 21571, 17049, 21573);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Swamp of Sorrows - Group 4' WHERE `entry` IN (21577, 21590, 21571, 17049, 21573);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Swamp of Sorrows - Group 4' WHERE `pool_entry` IN (21577, 21590, 21571, 17049, 21573);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4691+4, 1, 'Silver Vein / Gold Vein / Iron Deposit - Swamp of Sorrows - Group 4 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (21577, 4691+4, 0, 'Swamp of Sorrows - Group 4 - Silver Vein / Gold Vein / Iron Deposit', 0), (21590, 4691+4, 0, 'Swamp of Sorrows - Group 4 - Silver Vein / Gold Vein / Iron Deposit', 0), (21571, 4691+4, 0, 'Swamp of Sorrows - Group 4 - Silver Vein / Gold Vein / Iron Deposit', 0), (17049, 4691+4, 0, 'Swamp of Sorrows - Group 4 - Silver Vein / Gold Vein / Iron Deposit', 0), (21573, 4691+4, 0, 'Swamp of Sorrows - Group 4 - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (21583, 21591, 21579, 21580);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Swamp of Sorrows - Group 5' WHERE `entry` IN (21583, 21591, 21579, 21580);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Swamp of Sorrows - Group 5' WHERE `pool_entry` IN (21583, 21591, 21579, 21580);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4691+5, 1, 'Silver Vein / Gold Vein / Iron Deposit - Swamp of Sorrows - Group 5 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (21583, 4691+5, 0, 'Swamp of Sorrows - Group 5 - Silver Vein / Gold Vein / Iron Deposit', 0), (21591, 4691+5, 0, 'Swamp of Sorrows - Group 5 - Silver Vein / Gold Vein / Iron Deposit', 0), (21579, 4691+5, 0, 'Swamp of Sorrows - Group 5 - Silver Vein / Gold Vein / Iron Deposit', 0), (21580, 4691+5, 0, 'Swamp of Sorrows - Group 5 - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (17064, 21574, 21572, 17063);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Swamp of Sorrows - Group 6' WHERE `entry` IN (17064, 21574, 21572, 17063);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Swamp of Sorrows - Group 6' WHERE `pool_entry` IN (17064, 21574, 21572, 17063);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4691+6, 1, 'Silver Vein / Gold Vein / Iron Deposit - Swamp of Sorrows - Group 6 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (17064, 4691+6, 0, 'Swamp of Sorrows - Group 6 - Silver Vein / Gold Vein / Iron Deposit', 0), (21574, 4691+6, 0, 'Swamp of Sorrows - Group 6 - Silver Vein / Gold Vein / Iron Deposit', 0), (21572, 4691+6, 0, 'Swamp of Sorrows - Group 6 - Silver Vein / Gold Vein / Iron Deposit', 0), (17063, 4691+6, 0, 'Swamp of Sorrows - Group 6 - Silver Vein / Gold Vein / Iron Deposit', 0);

DELETE FROM `pool_template` WHERE `entry` = 1318;

UPDATE `pool_template` SET `max_limit` =1 WHERE `entry` =8119 AND `patch_min` =0 AND `patch_max` =10;

-- End of migration.
;
