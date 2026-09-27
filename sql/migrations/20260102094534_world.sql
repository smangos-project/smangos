INSERT INTO `migrations` VALUES ('20260102094534');

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 DELETE FROM `pool_gameobject` WHERE `guid` IN (57858, 68819, 57860, 68823, 57850, 57839);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3648+1, 1, 'Felwood - Morlos''Aran - Plaguebloom', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (57858, 3648+1, 0, 'Plaguebloom - Felwood - Morlos''Aran', 0, 0, 10), (68819, 3648+1, 0, 'Plaguebloom - Felwood - Morlos''Aran', 0, 0, 10), (57860, 3648+1, 0, 'Plaguebloom - Felwood - Morlos''Aran', 0, 0, 10), (68823, 3648+1, 0, 'Plaguebloom - Felwood - Morlos''Aran', 0, 0, 10), (57850, 3648+1, 0, 'Plaguebloom - Felwood - Morlos''Aran', 0, 0, 10), (57839, 3648+1, 0, 'Plaguebloom - Felwood - Morlos''Aran', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 DELETE FROM `pool_gameobject` WHERE `guid` IN (68821, 68824, 623, 57861, 57859, 57862);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3648+2, 1, 'Felwood - Deadwood Village - Plaguebloom', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (68821, 3648+2, 0, 'Plaguebloom - Felwood - Deadwood Village', 0, 0, 10), (68824, 3648+2, 0, 'Plaguebloom - Felwood - Deadwood Village', 0, 0, 10), (623, 3648+2, 0, 'Plaguebloom - Felwood - Deadwood Village', 0, 0, 10), (57861, 3648+2, 0, 'Plaguebloom - Felwood - Deadwood Village', 0, 0, 10), (57859, 3648+2, 0, 'Plaguebloom - Felwood - Deadwood Village', 0, 0, 10), (57862, 3648+2, 0, 'Plaguebloom - Felwood - Deadwood Village', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 DELETE FROM `pool_gameobject` WHERE `guid` IN (57852, 57863, 57848, 57864, 57844, 57847);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3648+3, 1, 'Felwood - Ruins of Constellas - Plaguebloom', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (57852, 3648+3, 0, 'Plaguebloom - Felwood - Ruins of Constellas', 0, 0, 10), (57863, 3648+3, 0, 'Plaguebloom - Felwood - Ruins of Constellas', 0, 0, 10), (57848, 3648+3, 0, 'Plaguebloom - Felwood - Ruins of Constellas', 0, 0, 10), (57864, 3648+3, 0, 'Plaguebloom - Felwood - Ruins of Constellas', 0, 0, 10), (57844, 3648+3, 0, 'Plaguebloom - Felwood - Ruins of Constellas', 0, 0, 10), (57847, 3648+3, 0, 'Plaguebloom - Felwood - Ruins of Constellas', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 DELETE FROM `pool_gameobject` WHERE `guid` IN (57851, 57842, 57857, 57853, 57849, 624);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3648+4, 1, 'Felwood - Jaedenar - Plaguebloom', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (57851, 3648+4, 0, 'Plaguebloom - Felwood - Jaedenar', 0, 0, 10), (57842, 3648+4, 0, 'Plaguebloom - Felwood - Jaedenar', 0, 0, 10), (57857, 3648+4, 0, 'Plaguebloom - Felwood - Jaedenar', 0, 0, 10), (57853, 3648+4, 0, 'Plaguebloom - Felwood - Jaedenar', 0, 0, 10), (57849, 3648+4, 0, 'Plaguebloom - Felwood - Jaedenar', 0, 0, 10), (624, 3648+4, 0, 'Plaguebloom - Felwood - Jaedenar', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 DELETE FROM `pool_gameobject` WHERE `guid` IN (57865, 57866, 57843, 57856, 57868, 57867);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3648+5, 1, 'Felwood - Shatter Scar Vale - Plaguebloom', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (57865, 3648+5, 0, 'Plaguebloom - Felwood - Shatter Scar Vale', 0, 0, 10), (57866, 3648+5, 0, 'Plaguebloom - Felwood - Shatter Scar Vale', 0, 0, 10), (57843, 3648+5, 0, 'Plaguebloom - Felwood - Shatter Scar Vale', 0, 0, 10), (57856, 3648+5, 0, 'Plaguebloom - Felwood - Shatter Scar Vale', 0, 0, 10), (57868, 3648+5, 0, 'Plaguebloom - Felwood - Shatter Scar Vale', 0, 0, 10), (57867, 3648+5, 0, 'Plaguebloom - Felwood - Shatter Scar Vale', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 DELETE FROM `pool_gameobject` WHERE `guid` IN (57869, 57870, 68822, 57872, 57845, 57855);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3648+6, 1, 'Felwood - Irontree Woods Lower - Plaguebloom', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (57869, 3648+6, 0, 'Plaguebloom - Felwood - Irontree Woods Lower', 0, 0, 10), (57870, 3648+6, 0, 'Plaguebloom - Felwood - Irontree Woods Lower', 0, 0, 10), (68822, 3648+6, 0, 'Plaguebloom - Felwood - Irontree Woods Lower', 0, 0, 10), (57872, 3648+6, 0, 'Plaguebloom - Felwood - Irontree Woods Lower', 0, 0, 10), (57845, 3648+6, 0, 'Plaguebloom - Felwood - Irontree Woods Lower', 0, 0, 10), (57855, 3648+6, 0, 'Plaguebloom - Felwood - Irontree Woods Lower', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 DELETE FROM `pool_gameobject` WHERE `guid` IN (68820, 57873, 57876, 57846, 57874, 57871);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3648+7, 1, 'Felwood - Irontree Woods Upper - Plaguebloom', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (68820, 3648+7, 0, 'Plaguebloom - Felwood - Irontree Woods Upper', 0, 0, 10), (57873, 3648+7, 0, 'Plaguebloom - Felwood - Irontree Woods Upper', 0, 0, 10), (57876, 3648+7, 0, 'Plaguebloom - Felwood - Irontree Woods Upper', 0, 0, 10), (57846, 3648+7, 0, 'Plaguebloom - Felwood - Irontree Woods Upper', 0, 0, 10), (57874, 3648+7, 0, 'Plaguebloom - Felwood - Irontree Woods Upper', 0, 0, 10), (57871, 3648+7, 0, 'Plaguebloom - Felwood - Irontree Woods Upper', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 DELETE FROM `pool_gameobject` WHERE `guid` IN (57841, 68825, 57840, 57875, 57854);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3648+8, 1, 'Felwood - Jadefire Run - Plaguebloom', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (57841, 3648+8, 0, 'Plaguebloom - Felwood - Jadefire Run', 0, 0, 10), (68825, 3648+8, 0, 'Plaguebloom - Felwood - Jadefire Run', 0, 0, 10), (57840, 3648+8, 0, 'Plaguebloom - Felwood - Jadefire Run', 0, 0, 10), (57875, 3648+8, 0, 'Plaguebloom - Felwood - Jadefire Run', 0, 0, 10), (57854, 3648+8, 0, 'Plaguebloom - Felwood - Jadefire Run', 0, 0, 10);

DELETE FROM `pool_template` WHERE `entry` = 1227;

-- End of migration.
;
