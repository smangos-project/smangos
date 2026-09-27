INSERT INTO `migrations` VALUES ('20260301150201');

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- The Dark Portal Pool 1
 INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `patch_min`) VALUES (251+1, 142145, 0, -11755.8, -3155.15, -19.205, 4.11898, 0, 0, -0.882947, 0.469473, 300, 300, 100, 1, 0), (251+2, 142145, 0, -11785.9, -3237.88, -24.6383, 3.22886, 0, 0, -0.999048, 0.0436193, 300, 300, 100, 1, 0);

DELETE FROM `pool_gameobject` WHERE `guid` IN (42121, 42117, 42123, 42184);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3662+1, 1, 'Blasted Lands - The Dark Portal Pool 1 - Gromsblood', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (42121, 3662+1, 0, 'Gromsblood - Blasted Lands - The Dark Portal Pool 1', 0, 0, 10), (42117, 3662+1, 0, 'Gromsblood - Blasted Lands - The Dark Portal Pool 1', 0, 0, 10), (42123, 3662+1, 0, 'Gromsblood - Blasted Lands - The Dark Portal Pool 1', 0, 0, 10), (42184, 3662+1, 0, 'Gromsblood - Blasted Lands - The Dark Portal Pool 1', 0, 0, 10), (251+1, 3662+1, 0, 'Gromsblood - Blasted Lands - The Dark Portal Pool 1', 0, 0, 10), (251+2, 3662+1, 0, 'Gromsblood - Blasted Lands - The Dark Portal Pool 1', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- The Dark Portal Pool 2
 DELETE FROM `gameobject` WHERE `guid` = 29687;

DELETE FROM `pool_gameobject` WHERE `guid` IN (42125, 32304, 42127, 42118, 32106, 42124, 29687);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3662+2, 1, 'Blasted Lands - The Dark Portal Pool 2 - Gromsblood', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (42125, 3662+2, 0, 'Gromsblood - Blasted Lands - The Dark Portal Pool 2', 0, 0, 10), (32304, 3662+2, 0, 'Gromsblood - Blasted Lands - The Dark Portal Pool 2', 0, 0, 10), (42127, 3662+2, 0, 'Gromsblood - Blasted Lands - The Dark Portal Pool 2', 0, 0, 10), (42118, 3662+2, 0, 'Gromsblood - Blasted Lands - The Dark Portal Pool 2', 0, 0, 10), (32106, 3662+2, 0, 'Gromsblood - Blasted Lands - The Dark Portal Pool 2', 0, 0, 10), (42124, 3662+2, 0, 'Gromsblood - Blasted Lands - The Dark Portal Pool 2', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Dreadmaul Post
 DELETE FROM `pool_gameobject` WHERE `guid` IN (68718, 16453, 16497, 16503, 42122, 42128);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3662+3, 1, 'Blasted Lands - Dreadmaul Post - Gromsblood', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (68718, 3662+3, 0, 'Gromsblood - Blasted Lands - Dreadmaul Post', 0, 0, 10), (16453, 3662+3, 0, 'Gromsblood - Blasted Lands - Dreadmaul Post', 0, 0, 10), (16497, 3662+3, 0, 'Gromsblood - Blasted Lands - Dreadmaul Post', 0, 0, 10), (16503, 3662+3, 0, 'Gromsblood - Blasted Lands - Dreadmaul Post', 0, 0, 10), (42122, 3662+3, 0, 'Gromsblood - Blasted Lands - Dreadmaul Post', 0, 0, 10), (42128, 3662+3, 0, 'Gromsblood - Blasted Lands - Dreadmaul Post', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Altar of Storms
 DELETE FROM `pool_gameobject` WHERE `guid` IN (16463, 42130, 32309, 42131, 589);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3662+4, 1, 'Blasted Lands - Altar of Storms - Gromsblood', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (16463, 3662+4, 0, 'Gromsblood - Blasted Lands - Altar of Storms', 0, 0, 10), (42130, 3662+4, 0, 'Gromsblood - Blasted Lands - Altar of Storms', 0, 0, 10), (32309, 3662+4, 0, 'Gromsblood - Blasted Lands - Altar of Storms', 0, 0, 10), (42131, 3662+4, 0, 'Gromsblood - Blasted Lands - Altar of Storms', 0, 0, 10), (589, 3662+4, 0, 'Gromsblood - Blasted Lands - Altar of Storms', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Rise of the Defiler
 DELETE FROM `pool_gameobject` WHERE `guid` IN (16479, 16499, 32758, 42120, 35307);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3662+5, 1, 'Blasted Lands - Rise of the Defiler - Gromsblood', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (16479, 3662+5, 0, 'Gromsblood - Blasted Lands - Rise of the Defiler', 0, 0, 10), (16499, 3662+5, 0, 'Gromsblood - Blasted Lands - Rise of the Defiler', 0, 0, 10), (32758, 3662+5, 0, 'Gromsblood - Blasted Lands - Rise of the Defiler', 0, 0, 10), (42120, 3662+5, 0, 'Gromsblood - Blasted Lands - Rise of the Defiler', 0, 0, 10), (35307, 3662+5, 0, 'Gromsblood - Blasted Lands - Rise of the Defiler', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Dreadmaul Hold
 DELETE FROM `pool_gameobject` WHERE `guid` IN (16472, 16450, 16462, 42119, 16454);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3662+6, 1, 'Blasted Lands - Dreadmaul Hold - Gromsblood', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (16472, 3662+6, 0, 'Gromsblood - Blasted Lands - Dreadmaul Hold', 0, 0, 10), (16450, 3662+6, 0, 'Gromsblood - Blasted Lands - Dreadmaul Hold', 0, 0, 10), (16462, 3662+6, 0, 'Gromsblood - Blasted Lands - Dreadmaul Hold', 0, 0, 10), (42119, 3662+6, 0, 'Gromsblood - Blasted Lands - Dreadmaul Hold', 0, 0, 10), (16454, 3662+6, 0, 'Gromsblood - Blasted Lands - Dreadmaul Hold', 0, 0, 10);

DELETE FROM `pool_template` WHERE `entry` =1275 AND `patch_min` =0 AND `patch_max` =10;

-- End of migration.
;
