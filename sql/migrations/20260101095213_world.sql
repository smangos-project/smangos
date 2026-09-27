INSERT INTO `migrations` VALUES ('20260101095213');

-- ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 DELETE FROM `pool_gameobject` WHERE `guid` IN (68799, 45556, 57724, 57706, 57701, 57705, 20108);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3589+1, 1, 'Western Plaguelands - Caer Darrow - Plaguebloom', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (68799, 3589+1, 0, 'Plaguebloom - Western Plaguelands - Caer Darrow', 0, 0, 10), (45556, 3589+1, 0, 'Plaguebloom - Western Plaguelands - Caer Darrow', 0, 0, 10), (57724, 3589+1, 0, 'Plaguebloom - Western Plaguelands - Caer Darrow', 0, 0, 10), (57706, 3589+1, 0, 'Plaguebloom - Western Plaguelands - Caer Darrow', 0, 0, 10), (57701, 3589+1, 0, 'Plaguebloom - Western Plaguelands - Caer Darrow', 0, 0, 10), (20108, 3589+1, 0, 'Plaguebloom - Western Plaguelands - Caer Darrow', 0, 0, 10), (57705, 3589+1, 0, 'Plaguebloom - Western Plaguelands - Caer Darrow', 0, 0, 10);

-- ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 DELETE FROM `pool_gameobject` WHERE `guid` IN (57708, 57699, 68802, 57750, 45470, 57700);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3589+2, 1, 'Western Plaguelands - Gahrron''s Withering - Plaguebloom', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (57708, 3589+2, 0, 'Plaguebloom - Western Plaguelands - Gahrron''s Withering', 0, 0, 10), (57699, 3589+2, 0, 'Plaguebloom - Western Plaguelands - Gahrron''s Withering', 0, 0, 10), (68802, 3589+2, 0, 'Plaguebloom - Western Plaguelands - Gahrron''s Withering', 0, 0, 10), (57750, 3589+2, 0, 'Plaguebloom - Western Plaguelands - Gahrron''s Withering', 0, 0, 10), (45470, 3589+2, 0, 'Plaguebloom - Western Plaguelands - Gahrron''s Withering', 0, 0, 10), (57700, 3589+2, 0, 'Plaguebloom - Western Plaguelands - Gahrron''s Withering', 0, 0, 10);

-- ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 DELETE FROM `pool_gameobject` WHERE `guid` IN (68798, 20069, 45471, 57755, 57754, 57757);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3589+3, 1, 'Western Plaguelands - Dalson''s Tears - Plaguebloom', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (68798, 3589+3, 0, 'Plaguebloom - Western Plaguelands - Dalson''s Tears', 0, 0, 10), (20069, 3589+3, 0, 'Plaguebloom - Western Plaguelands - Dalson''s Tears', 0, 0, 10), (45471, 3589+3, 0, 'Plaguebloom - Western Plaguelands - Dalson''s Tears', 0, 0, 10), (57755, 3589+3, 0, 'Plaguebloom - Western Plaguelands - Dalson''s Tears', 0, 0, 10), (57754, 3589+3, 0, 'Plaguebloom - Western Plaguelands - Dalson''s Tears', 0, 0, 10), (57757, 3589+3, 0, 'Plaguebloom - Western Plaguelands - Dalson''s Tears', 0, 0, 10);

-- ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 DELETE FROM `pool_gameobject` WHERE `guid` IN (32206, 57710, 57752, 68801, 19980, 57712);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3589+4, 1, 'Western Plaguelands - Felstone Field - Plaguebloom', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (32206, 3589+4, 0, 'Plaguebloom - Western Plaguelands - Felstone Field', 0, 0, 10), (57710, 3589+4, 0, 'Plaguebloom - Western Plaguelands - Felstone Field', 0, 0, 10), (57752, 3589+4, 0, 'Plaguebloom - Western Plaguelands - Felstone Field', 0, 0, 10), (68801, 3589+4, 0, 'Plaguebloom - Western Plaguelands - Felstone Field', 0, 0, 10), (19980, 3589+4, 0, 'Plaguebloom - Western Plaguelands - Felstone Field', 0, 0, 10), (57712, 3589+4, 0, 'Plaguebloom - Western Plaguelands - Felstone Field', 0, 0, 10);

-- ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 DELETE FROM `pool_gameobject` WHERE `guid` IN (57702, 45469, 57737, 57704, 57709, 19992);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3589+5, 1, 'Western Plaguelands - The Writhing Haunt - Plaguebloom', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (57702, 3589+5, 0, 'Plaguebloom - Western Plaguelands - The Writhing Haunt', 0, 0, 10), (45469, 3589+5, 0, 'Plaguebloom - Western Plaguelands - The Writhing Haunt', 0, 0, 10), (57737, 3589+5, 0, 'Plaguebloom - Western Plaguelands - The Writhing Haunt', 0, 0, 10), (57704, 3589+5, 0, 'Plaguebloom - Western Plaguelands - The Writhing Haunt', 0, 0, 10), (57709, 3589+5, 0, 'Plaguebloom - Western Plaguelands - The Writhing Haunt', 0, 0, 10), (19992, 3589+5, 0, 'Plaguebloom - Western Plaguelands - The Writhing Haunt', 0, 0, 10);

-- ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 DELETE FROM `pool_gameobject` WHERE `guid` IN (19948, 45540, 57717, 57711, 57753);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3589+6, 1, 'Western Plaguelands - The Weeping Cave - Plaguebloom', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (19948, 3589+6, 0, 'Plaguebloom - Western Plaguelands - The Weeping Cave', 0, 0, 10), (45540, 3589+6, 0, 'Plaguebloom - Western Plaguelands - The Weeping Cave', 0, 0, 10), (57717, 3589+6, 0, 'Plaguebloom - Western Plaguelands - The Weeping Cave', 0, 0, 10), (57711, 3589+6, 0, 'Plaguebloom - Western Plaguelands - The Weeping Cave', 0, 0, 10), (57753, 3589+6, 0, 'Plaguebloom - Western Plaguelands - The Weeping Cave', 0, 0, 10);

-- ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 DELETE FROM `pool_gameobject` WHERE `guid` IN (57714, 45558, 57703, 57716, 57707);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3589+7, 1, 'Western Plaguelands - Misc - Plaguebloom', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (57714, 3589+7, 0, 'Plaguebloom - Western Plaguelands - Misc', 0, 0, 10), (45558, 3589+7, 0, 'Plaguebloom - Western Plaguelands - Misc', 0, 0, 10), (57703, 3589+7, 0, 'Plaguebloom - Western Plaguelands - Misc', 0, 0, 10), (57716, 3589+7, 0, 'Plaguebloom - Western Plaguelands - Misc', 0, 0, 10), (57707, 3589+7, 0, 'Plaguebloom - Western Plaguelands - Misc', 0, 0, 10);

-- ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 DELETE FROM `pool_gameobject` WHERE `guid` IN (45472, 68800, 57713, 57715, 19940);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3589+8, 1, 'Western Plaguelands - Northridge Lumber Camp - Plaguebloom', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (45472, 3589+8, 0, 'Plaguebloom - Western Plaguelands - Northridge Lumber Camp', 0, 0, 10), (68800, 3589+8, 0, 'Plaguebloom - Western Plaguelands - Northridge Lumber Camp', 0, 0, 10), (57713, 3589+8, 0, 'Plaguebloom - Western Plaguelands - Northridge Lumber Camp', 0, 0, 10), (57715, 3589+8, 0, 'Plaguebloom - Western Plaguelands - Northridge Lumber Camp', 0, 0, 10), (19940, 3589+8, 0, 'Plaguebloom - Western Plaguelands - Northridge Lumber Camp', 0, 0, 10);

DELETE FROM `gameobject` WHERE `guid` = 1369;

DELETE FROM `pool_gameobject` WHERE `guid` = 1369;

DELETE FROM `pool_template` WHERE `entry` = 1258;

-- End of migration.
;
