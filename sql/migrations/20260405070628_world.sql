INSERT INTO `migrations` VALUES ('20260405070628');

DELETE FROM `pool_pool` WHERE `pool_id` IN (21529, 21523, 17201, 14401, 21516, 21524);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Blasted Lands - Group 1' WHERE `entry` IN (21529, 21523, 17201, 14401, 21516, 21524);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Blasted Lands - Group 1' WHERE `pool_entry` IN (21529, 21523, 17201, 14401, 21516, 21524);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4686+1, 1, 'Truesilver Deposit / Small Thorium Vein - Blasted Lands - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (21529, 4686+1, 0, 'Blasted Lands - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (21523, 4686+1, 0, 'Blasted Lands - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (17201, 4686+1, 0, 'Blasted Lands - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (14401, 4686+1, 0, 'Blasted Lands - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (21516, 4686+1, 0, 'Blasted Lands - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (21524, 4686+1, 0, 'Blasted Lands - Group 1 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (23012, 9436, 21525, 9437);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Blasted Lands - Group 2' WHERE `entry` IN (23012, 9436, 21525, 9437);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Blasted Lands - Group 2' WHERE `pool_entry` IN (23012, 9436, 21525, 9437);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4686+2, 1, 'Truesilver Deposit / Small Thorium Vein - Blasted Lands - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (23012, 4686+2, 0, 'Blasted Lands - Group 2 - Truesilver Deposit / Small Thorium Vein', 0), (9436, 4686+2, 0, 'Blasted Lands - Group 2 - Truesilver Deposit / Small Thorium Vein', 0), (21525, 4686+2, 0, 'Blasted Lands - Group 2 - Truesilver Deposit / Small Thorium Vein', 0), (9437, 4686+2, 0, 'Blasted Lands - Group 2 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (21528, 21518, 14402, 21513);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Blasted Lands - Group 3' WHERE `entry` IN (21528, 21518, 14402, 21513);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Blasted Lands - Group 3' WHERE `pool_entry` IN (21528, 21518, 14402, 21513);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4686+3, 1, 'Truesilver Deposit / Small Thorium Vein - Blasted Lands - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (21528, 4686+3, 0, 'Blasted Lands - Group 3 - Truesilver Deposit / Small Thorium Vein', 0), (21518, 4686+3, 0, 'Blasted Lands - Group 3 - Truesilver Deposit / Small Thorium Vein', 0), (14402, 4686+3, 0, 'Blasted Lands - Group 3 - Truesilver Deposit / Small Thorium Vein', 0), (21513, 4686+3, 0, 'Blasted Lands - Group 3 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (21517, 21515, 21514, 21520);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Blasted Lands - Group 4' WHERE `entry` IN (21517, 21515, 21514, 21520);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Blasted Lands - Group 4' WHERE `pool_entry` IN (21517, 21515, 21514, 21520);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4686+4, 1, 'Truesilver Deposit / Small Thorium Vein - Blasted Lands - Group 4 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (21517, 4686+4, 0, 'Blasted Lands - Group 4 - Truesilver Deposit / Small Thorium Vein', 0), (21515, 4686+4, 0, 'Blasted Lands - Group 4 - Truesilver Deposit / Small Thorium Vein', 0), (21514, 4686+4, 0, 'Blasted Lands - Group 4 - Truesilver Deposit / Small Thorium Vein', 0), (21520, 4686+4, 0, 'Blasted Lands - Group 4 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (21522, 14403, 21519, 21521);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Blasted Lands - Group 5' WHERE `entry` IN (21522, 14403, 21519, 21521);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Blasted Lands - Group 5' WHERE `pool_entry` IN (21522, 14403, 21519, 21521);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4686+5, 1, 'Truesilver Deposit / Small Thorium Vein - Blasted Lands - Group 5 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (21522, 4686+5, 0, 'Blasted Lands - Group 5 - Truesilver Deposit / Small Thorium Vein', 0), (14403, 4686+5, 0, 'Blasted Lands - Group 5 - Truesilver Deposit / Small Thorium Vein', 0), (21519, 4686+5, 0, 'Blasted Lands - Group 5 - Truesilver Deposit / Small Thorium Vein', 0), (21521, 4686+5, 0, 'Blasted Lands - Group 5 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_template` WHERE `entry` IN (8111, 8112, 1359);

-- End of migration.
;
