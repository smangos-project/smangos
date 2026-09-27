INSERT INTO `migrations` VALUES ('20260319203329');

DELETE FROM `pool_pool` WHERE `pool_id` IN (25480, 25469, 25473, 25472, 9417);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Western Plaguelands - Group 1' WHERE `entry` IN (25480, 25469, 25473, 25472, 9417);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Western Plaguelands - Group 1' WHERE `pool_entry` IN (25480, 25469, 25473, 25472, 9417);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4267+1, 1, 'Western Plaguelands - Group 1 - Truesilver Deposit / Small Thorium Vein', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (25480, 4267+1, 0, 'Western Plaguelands - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (25469, 4267+1, 0, 'Western Plaguelands - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (25473, 4267+1, 0, 'Western Plaguelands - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (25472, 4267+1, 0, 'Western Plaguelands - Group 1 - Truesilver Deposit / Small Thorium Vein', 0), (9417, 4267+1, 0, 'Western Plaguelands - Group 1 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (25482, 25486, 9418, 25476, 25475);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Western Plaguelands - Group 2' WHERE `entry` IN (25482, 25486, 9418, 25476, 25475);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Western Plaguelands - Group 2' WHERE `pool_entry` IN (25482, 25486, 9418, 25476, 25475);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4267+2, 1, 'Western Plaguelands - Group 2 - Truesilver Deposit / Small Thorium Vein', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (25482, 4267+2, 0, 'Western Plaguelands - Group 2 - Truesilver Deposit / Small Thorium Vein', 0), (25486, 4267+2, 0, 'Western Plaguelands - Group 2 - Truesilver Deposit / Small Thorium Vein', 0), (9418, 4267+2, 0, 'Western Plaguelands - Group 2 - Truesilver Deposit / Small Thorium Vein', 0), (25476, 4267+2, 0, 'Western Plaguelands - Group 2 - Truesilver Deposit / Small Thorium Vein', 0), (25475, 4267+2, 0, 'Western Plaguelands - Group 2 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (25470, 25479, 25487, 25481, 14428, 25484);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Western Plaguelands - Group 3' WHERE `entry` IN (25470, 25479, 25487, 25481, 14428, 25484);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Western Plaguelands - Group 3' WHERE `pool_entry` IN (25470, 25479, 25487, 25481, 14428, 25484);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4267+3, 1, 'Western Plaguelands - Group 3 - Truesilver Deposit / Small Thorium Vein', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (25470, 4267+3, 0, 'Western Plaguelands - Group 3 - Truesilver Deposit / Small Thorium Vein', 0), (25479, 4267+3, 0, 'Western Plaguelands - Group 3 - Truesilver Deposit / Small Thorium Vein', 0), (25487, 4267+3, 0, 'Western Plaguelands - Group 3 - Truesilver Deposit / Small Thorium Vein', 0), (25481, 4267+3, 0, 'Western Plaguelands - Group 3 - Truesilver Deposit / Small Thorium Vein', 0), (14428, 4267+3, 0, 'Western Plaguelands - Group 3 - Truesilver Deposit / Small Thorium Vein', 0), (25484, 4267+3, 0, 'Western Plaguelands - Group 3 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (25474, 14429, 25471, 25478, 25483, 25477);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Western Plaguelands - Group 4' WHERE `entry` IN (25474, 14429, 25471, 25478, 25483, 25477);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Western Plaguelands - Group 4' WHERE `pool_entry` IN (25474, 14429, 25471, 25478, 25483, 25477);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4267+4, 1, 'Western Plaguelands - Group 4 - Truesilver Deposit / Small Thorium Vein', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (25474, 4267+4, 0, 'Western Plaguelands - Group 4 - Truesilver Deposit / Small Thorium Vein', 0), (14429, 4267+4, 0, 'Western Plaguelands - Group 4 - Truesilver Deposit / Small Thorium Vein', 0), (25471, 4267+4, 0, 'Western Plaguelands - Group 4 - Truesilver Deposit / Small Thorium Vein', 0), (25478, 4267+4, 0, 'Western Plaguelands - Group 4 - Truesilver Deposit / Small Thorium Vein', 0), (25483, 4267+4, 0, 'Western Plaguelands - Group 4 - Truesilver Deposit / Small Thorium Vein', 0), (25477, 4267+4, 0, 'Western Plaguelands - Group 4 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (9419, 23011, 9420, 9421, 21662);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Western Plaguelands - Group 5' WHERE `entry` IN (9419, 23011, 9420, 9421, 21662);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Western Plaguelands - Group 5' WHERE `pool_entry` IN (9419, 23011, 9420, 9421, 21662);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4267+5, 1, 'Western Plaguelands - Group 5 - Truesilver Deposit / Small Thorium Vein', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (9419, 4267+5, 0, 'Western Plaguelands - Group 5 - Truesilver Deposit / Small Thorium Vein', 0), (23011, 4267+5, 0, 'Western Plaguelands - Group 5 - Truesilver Deposit / Small Thorium Vein', 0), (9420, 4267+5, 0, 'Western Plaguelands - Group 5 - Truesilver Deposit / Small Thorium Vein', 0), (9421, 4267+5, 0, 'Western Plaguelands - Group 5 - Truesilver Deposit / Small Thorium Vein', 0), (21662, 4267+5, 0, 'Western Plaguelands - Group 5 - Truesilver Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` = 25485;

DELETE FROM `pool_gameobject` WHERE `guid` IN (261416, 261435);

DELETE FROM `gameobject` WHERE `guid` IN (261416, 261435);

DELETE FROM `pool_template` WHERE `entry` IN (1331, 8121, 25485);

-- End of migration.
;
