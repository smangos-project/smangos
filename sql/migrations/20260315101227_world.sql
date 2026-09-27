INSERT INTO `migrations` VALUES ('20260315101227');

DELETE FROM `pool_pool` WHERE `pool_id` IN (14259, 14267, 14260, 14265, 14266, 14264);

UPDATE `pool_template` SET `description` = 'Ooze Covered Silver Vein / Ooze Covered Gold Vein / Ooze Covered Mithril Deposit - Thousand Needles - Group 1' WHERE `entry` IN (14259, 14267, 14260, 14265, 14266, 14264);

UPDATE `pool_gameobject` SET `description` = 'Ooze Covered Silver Vein / Ooze Covered Gold Vein / Ooze Covered Mithril Deposit - Thousand Needles - Group 1' WHERE `pool_entry` IN (14259, 14267, 14260, 14265, 14266, 14264);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4080+1, 1, 'Ooze Covered Silver Vein / Ooze Covered Gold Vein / Ooze Covered Mithril Deposit - Thousand Needles - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (14259, 4080+1, 0, 'Thousand Needles - Group 1 - Ooze Covered Silver Vein / Ooze Covered Gold Vein / Ooze Covered Mithril Deposit', 0), (14267, 4080+1, 0, 'Thousand Needles - Group 1 - Ooze Covered Silver Vein / Ooze Covered Gold Vein / Ooze Covered Mithril Deposit', 0), (14260, 4080+1, 0, 'Thousand Needles - Group 1 - Ooze Covered Silver Vein / Ooze Covered Gold Vein / Ooze Covered Mithril Deposit', 0), (14265, 4080+1, 0, 'Thousand Needles - Group 1 - Ooze Covered Silver Vein / Ooze Covered Gold Vein / Ooze Covered Mithril Deposit', 0), (14266, 4080+1, 0, 'Thousand Needles - Group 1 - Ooze Covered Silver Vein / Ooze Covered Gold Vein / Ooze Covered Mithril Deposit', 0), (14264, 4080+1, 0, 'Thousand Needles - Group 1 - Ooze Covered Silver Vein / Ooze Covered Gold Vein / Ooze Covered Mithril Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (14261, 14263, 14262);

UPDATE `pool_template` SET `description` = 'Ooze Covered Silver Vein / Ooze Covered Gold Vein / Ooze Covered Mithril Deposit - Thousand Needles - Group 2' WHERE `entry` IN (14261, 14263, 14262);

UPDATE `pool_gameobject` SET `description` = 'Ooze Covered Silver Vein / Ooze Covered Gold Vein / Ooze Covered Mithril Deposit - Thousand Needles - Group 2' WHERE `pool_entry` IN (14261, 14263, 14262);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4080+2, 1, 'Ooze Covered Silver Vein / Ooze Covered Gold Vein / Ooze Covered Mithril Deposit - Thousand Needles - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (14261, 4080+2, 0, 'Thousand Needles - Group2 - Ooze Covered Silver Vein / Ooze Covered Gold Vein / Ooze Covered Mithril Deposit', 0), (14263, 4080+2, 0, 'Thousand Needles - Group2 - Ooze Covered Silver Vein / Ooze Covered Gold Vein / Ooze Covered Mithril Deposit', 0), (14262, 4080+2, 0, 'Thousand Needles - Group2 - Ooze Covered Silver Vein / Ooze Covered Gold Vein / Ooze Covered Mithril Deposit', 0);

DELETE FROM `pool_template` WHERE `entry` = 304;

DELETE FROM `pool_pool` WHERE `pool_id` IN (3091, 3092, 3093);

UPDATE `pool_template` SET `description` = 'Ooze Covered Truesilver Deposit / Ooze Covered Thorium Vein - Feralas - Group 1' WHERE `entry` IN (3091, 3092, 3093);

UPDATE `pool_gameobject` SET `description` = 'Ooze Covered Truesilver Deposit / Ooze Covered Thorium Vein - Feralas - Group 1' WHERE `pool_entry` IN (3091, 3092, 3093);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3535+1, 1, 'Ooze Covered Truesilver Deposit / Ooze Covered Thorium Vein - Feralas - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (3091, 3535+1, 0, 'Feralas - Group 1 - Ooze Covered Truesilver Deposit / Ooze Covered Thorium Vein', 0), (3092, 3535+1, 0, 'Feralas - Group 1 - Ooze Covered Truesilver Deposit / Ooze Covered Thorium Vein', 0), (3093, 3535+1, 0, 'Feralas - Group 1 - Ooze Covered Truesilver Deposit / Ooze Covered Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (21631, 21632, 21633, 21634);

UPDATE `pool_template` SET `description` = 'Ooze Covered Gold Vein / Ooze Covered Mithril Deposit / Ooze Covered Truesilver Deposit - Feralas - Group 1' WHERE `entry` IN (21631, 21632, 21633, 21634);

UPDATE `pool_gameobject` SET `description` = 'Ooze Covered Gold Vein / Ooze Covered Mithril Deposit / Ooze Covered Truesilver Deposit - Feralas - Group 1' WHERE `pool_entry` IN (21631, 21632, 21633, 21634);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3535+2, 1, 'Ooze Covered Gold Vein / Ooze Covered Mithril Deposit / Ooze Covered Truesilver Deposit - Feralas - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (21631, 3535+2, 0, 'Feralas - Group 1 - Ooze Covered Gold Vein / Ooze Covered Mithril Deposit / Ooze Covered Truesilver Deposit', 0), (21632, 3535+2, 0, 'Feralas - Group 1 - Ooze Covered Gold Vein / Ooze Covered Mithril Deposit / Ooze Covered Truesilver Deposit', 0), (21633, 3535+2, 0, 'Feralas - Group 1 - Ooze Covered Gold Vein / Ooze Covered Mithril Deposit / Ooze Covered Truesilver Deposit', 0), (21634, 3535+2, 0, 'Feralas - Group 1 - Ooze Covered Gold Vein / Ooze Covered Mithril Deposit / Ooze Covered Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (21635, 21636, 21637, 21638, 21639);

UPDATE `pool_template` SET `description` = 'Ooze Covered Gold Vein / Ooze Covered Mithril Deposit / Ooze Covered Truesilver Deposit - Feralas - Group 2' WHERE `entry` IN (21635, 21636, 21637, 21638, 21639);

UPDATE `pool_gameobject` SET `description` = 'Ooze Covered Gold Vein / Ooze Covered Mithril Deposit / Ooze Covered Truesilver Deposit - Feralas - Group 2' WHERE `pool_entry` IN (21635, 21636, 21637, 21638, 21639);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3535+3, 1, 'Ooze Covered Gold Vein / Ooze Covered Mithril Deposit / Ooze Covered Truesilver Deposit - Feralas - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (21635, 3535+3, 0, 'Feralas - Group 2 - Ooze Covered Gold Vein / Ooze Covered Mithril Deposit / Ooze Covered Truesilver Deposit', 0), (21636, 3535+3, 0, 'Feralas - Group 2 - Ooze Covered Gold Vein / Ooze Covered Mithril Deposit / Ooze Covered Truesilver Deposit', 0), (21637, 3535+3, 0, 'Feralas - Group 2 - Ooze Covered Gold Vein / Ooze Covered Mithril Deposit / Ooze Covered Truesilver Deposit', 0), (21638, 3535+3, 0, 'Feralas - Group 2 - Ooze Covered Gold Vein / Ooze Covered Mithril Deposit / Ooze Covered Truesilver Deposit', 0), (21639, 3535+3, 0, 'Feralas - Group 2 - Ooze Covered Gold Vein / Ooze Covered Mithril Deposit / Ooze Covered Truesilver Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (21640, 21641, 21642, 21643, 21644);

UPDATE `pool_template` SET `description` = 'Ooze Covered Gold Vein / Ooze Covered Mithril Deposit / Ooze Covered Truesilver Deposit - Feralas - Group 3' WHERE `entry` IN (21640, 21641, 21642, 21643, 21644);

UPDATE `pool_gameobject` SET `description` = 'Ooze Covered Gold Vein / Ooze Covered Mithril Deposit / Ooze Covered Truesilver Deposit - Feralas - Group 3' WHERE `pool_entry` IN (21640, 21641, 21642, 21643, 21644);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3535+4, 1, 'Ooze Covered Gold Vein / Ooze Covered Mithril Deposit / Ooze Covered Truesilver Deposit - Feralas - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (21640, 3535+4, 0, 'Feralas - Group 3 - Ooze Covered Gold Vein / Ooze Covered Mithril Deposit / Ooze Covered Truesilver Deposit', 0), (21641, 3535+4, 0, 'Feralas - Group 3 - Ooze Covered Gold Vein / Ooze Covered Mithril Deposit / Ooze Covered Truesilver Deposit', 0), (21642, 3535+4, 0, 'Feralas - Group 3 - Ooze Covered Gold Vein / Ooze Covered Mithril Deposit / Ooze Covered Truesilver Deposit', 0), (21643, 3535+4, 0, 'Feralas - Group 3 - Ooze Covered Gold Vein / Ooze Covered Mithril Deposit / Ooze Covered Truesilver Deposit', 0), (21644, 3535+4, 0, 'Feralas - Group 3 - Ooze Covered Gold Vein / Ooze Covered Mithril Deposit / Ooze Covered Truesilver Deposit', 0);

DELETE FROM `pool_template` WHERE `entry` IN (1198, 413);

DELETE FROM `pool_pool` WHERE `pool_id` IN (2267, 2270, 2265, 2266, 2271);

UPDATE `pool_template` SET `description` = 'Ooze Covered Truesilver Deposit / Ooze Covered Thorium Vein - Un''Goro Crater - Group 1' WHERE `entry` IN (2267, 2270, 2265, 2266, 2271);

UPDATE `pool_gameobject` SET `description` = 'Ooze Covered Truesilver Deposit / Ooze Covered Thorium Vein - Un''Goro Crater - Group 1' WHERE `pool_entry` IN (2267, 2270, 2265, 2266, 2271);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (1221+1, 1, 'Ooze Covered Truesilver Deposit / Ooze Covered Thorium Vein - Un''Goro Crater - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (2267, 1221+1, 0, 'Un''Goro Crater - Group 1 - Ooze Covered Truesilver Deposit / Ooze Covered Thorium Vein', 0), (2270, 1221+1, 0, 'Un''Goro Crater - Group 1 - Ooze Covered Truesilver Deposit / Ooze Covered Thorium Vein', 0), (2265, 1221+1, 0, 'Un''Goro Crater - Group 1 - Ooze Covered Truesilver Deposit / Ooze Covered Thorium Vein', 0), (2266, 1221+1, 0, 'Un''Goro Crater - Group 1 - Ooze Covered Truesilver Deposit / Ooze Covered Thorium Vein', 0), (2271, 1221+1, 0, 'Un''Goro Crater - Group 1 - Ooze Covered Truesilver Deposit / Ooze Covered Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (14451, 2264, 2268, 2263, 2269);

UPDATE `pool_template` SET `description` = 'Ooze Covered Truesilver Deposit / Ooze Covered Thorium Vein - Un''Goro Crater - Group 2' WHERE `entry` IN (14451, 2264, 2268, 2263, 2269);

UPDATE `pool_gameobject` SET `description` = 'Ooze Covered Truesilver Deposit / Ooze Covered Thorium Vein - Un''Goro Crater - Group 2' WHERE `pool_entry` IN (14451, 2264, 2268, 2263, 2269);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (1221+2, 1, 'Ooze Covered Truesilver Deposit / Ooze Covered Thorium Vein - Un''Goro Crater - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (14451, 1221+2, 0, 'Un''Goro Crater - Group 2 - Ooze Covered Truesilver Deposit / Ooze Covered Thorium Vein', 0), (2264, 1221+2, 0, 'Un''Goro Crater - Group 2 - Ooze Covered Truesilver Deposit / Ooze Covered Thorium Vein', 0), (2268, 1221+2, 0, 'Un''Goro Crater - Group 2 - Ooze Covered Truesilver Deposit / Ooze Covered Thorium Vein', 0), (2263, 1221+2, 0, 'Un''Goro Crater - Group 2 - Ooze Covered Truesilver Deposit / Ooze Covered Thorium Vein', 0), (2269, 1221+2, 0, 'Un''Goro Crater - Group 2 - Ooze Covered Truesilver Deposit / Ooze Covered Thorium Vein', 0);

DELETE FROM `pool_template` WHERE `entry` = 412;

-- End of migration.
;
