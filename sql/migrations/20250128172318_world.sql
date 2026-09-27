INSERT INTO `migrations` VALUES ('20250128172318');

-- Blasted Lands
 UPDATE `pool_gameobject` SET `chance` = 95 WHERE `description` = 'Truesilver Deposit / Small Thorium Vein - Blasted Lands - Garrison Armory' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 150082);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Truesilver Deposit / Small Thorium Vein - Blasted Lands - Garrison Armory' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 150081);

-- Dreadmaul Post
 UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (14472));

UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (14402));

DELETE FROM `pool_pool` WHERE `pool_id` IN (14472, 14402);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Blasted Lands - Dreadmaul Post' WHERE `entry` IN (14472);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Blasted Lands - Dreadmaul Post' WHERE `pool_entry` IN (14472);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Blasted Lands - Dreadmaul Post' WHERE `entry` IN (14402);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Blasted Lands - Dreadmaul Post' WHERE `pool_entry` IN (14402);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (8110+1, 1, 'Blasted Lands - Dreadmaul Post - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (14472, 8110+1, 0, 'Blasted Lands - Dreadmaul Post - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (14402, 8110+1, 0, 'Blasted Lands - Dreadmaul Post - Truesilver Deposit / Small Thorium Vein', 0);

-- Serpent's Coil
 UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (14459, 14460, 14403));

UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (21522, 21519, 21521));

DELETE FROM `pool_pool` WHERE `pool_id` IN (14459, 14460, 14403, 21522, 21519, 21521);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Blasted Lands - Serpent''s Coil' WHERE `entry` IN (14459, 14460, 14403);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Blasted Lands - Serpent''s Coil' WHERE `pool_entry` IN (14459, 14460, 14403);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Blasted Lands - Serpent''s Coil' WHERE `entry` IN (21522, 21519, 21521);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Blasted Lands - Serpent''s Coil' WHERE `pool_entry` IN (21522, 21519, 21521);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (8110+2, 3, 'Blasted Lands - Serpent''s Coil - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (14459, 8110+2, 0, 'Blasted Lands - Serpent''s Coil - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (14460, 8110+2, 0, 'Blasted Lands - Serpent''s Coil - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (14403, 8110+2, 0, 'Blasted Lands - Serpent''s Coil - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (21522, 8110+2, 0, 'Blasted Lands - Serpent''s Coil - Truesilver Deposit / Small Thorium Vein', 0), (21519, 8110+2, 0, 'Blasted Lands - Serpent''s Coil - Truesilver Deposit / Small Thorium Vein', 0), (21521, 8110+2, 0, 'Blasted Lands - Serpent''s Coil - Truesilver Deposit / Small Thorium Vein', 0);

UPDATE `pool_template` SET `max_limit` = 5 WHERE `entry` = 1359;

UPDATE `pool_template` SET `max_limit` = 5 WHERE `entry` = 1279;

UPDATE `pool_template` SET `max_limit` = 2 WHERE `entry` = 4345;

-- Burning Steppes
 UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes' WHERE `entry` = 22940;

UPDATE `pool_pool` SET `description` = 'Burning Steppes - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein' WHERE `pool_id` = 22940;

UPDATE `pool_pool` SET `mother_pool` = 1296 WHERE `pool_id` = 22940;

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Burning Steppes' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1734);

UPDATE `pool_gameobject` SET `chance` = 75 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Burning Steppes' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 2040);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Burning Steppes' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 2047);

UPDATE `pool_gameobject` SET `chance` = 15 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Burning Steppes' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 165658);

UPDATE `pool_gameobject` SET `chance` = 80 WHERE `description` = 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 324);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 2047);

UPDATE `pool_gameobject` SET `chance` = 15 WHERE `description` = 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 165658);

UPDATE `pool_gameobject` SET `chance` = 80 WHERE `description` = 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 175404);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 2047);

UPDATE `pool_gameobject` SET `chance` = 15 WHERE `description` = 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 165658);

-- Dreadmaul Rock Lower Cave
 UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (22921, 22927, 22930, 22926));

UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (14248, 14240, 14244, 14238));

DELETE FROM `pool_pool` WHERE `pool_id` IN (22921, 22927, 22930, 22926, 14248, 14240, 14244, 14238);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Burning Steppes - Dreadmaul Rock Lower Cave' WHERE `entry` IN (22921, 22927, 22930, 22926);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Burning Steppes - Dreadmaul Rock Lower Cave' WHERE `pool_entry` IN (22921, 22927, 22930, 22926);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes - Dreadmaul Rock Lower Cave' WHERE `entry` IN (14248, 14240, 14244, 14238);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes - Dreadmaul Rock Lower Cave' WHERE `pool_entry` IN (14248, 14240, 14244, 14238);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (8110+3, 2, 'Burning Steppes - Dreadmaul Rock Lower Cave - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (22921, 8110+3, 0, 'Burning Steppes - Dreadmaul Rock Lower Cave - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22927, 8110+3, 0, 'Burning Steppes - Dreadmaul Rock Lower Cave - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22930, 8110+3, 0, 'Burning Steppes - Dreadmaul Rock Lower Cave - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22926, 8110+3, 0, 'Burning Steppes - Dreadmaul Rock Lower Cave - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (14248, 8110+3, 0, 'Burning Steppes - Dreadmaul Rock Lower Cave - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14240, 8110+3, 0, 'Burning Steppes - Dreadmaul Rock Lower Cave - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14244, 8110+3, 0, 'Burning Steppes - Dreadmaul Rock Lower Cave - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14238, 8110+3, 0, 'Burning Steppes - Dreadmaul Rock Lower Cave - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0);

-- Dreadmaul Rock Upper Cave
 UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (22932, 22931, 22923, 22925, 22928));

UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (14246, 14237, 14239, 14236));

DELETE FROM `pool_pool` WHERE `pool_id` IN (22932, 22931, 22923, 22925, 22928, 14246, 14237, 14239, 14236);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Burning Steppes - Dreadmaul Rock Upper Cave' WHERE `entry` IN (22932, 22931, 22923, 22925, 22928);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Burning Steppes - Dreadmaul Rock Upper Cave' WHERE `pool_entry` IN (22932, 22931, 22923, 22925, 22928);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes - Dreadmaul Rock Upper Cave' WHERE `entry` IN (14246, 14237, 14239, 14236);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes - Dreadmaul Rock Upper Cave' WHERE `pool_entry` IN (14246, 14237, 14239, 14236);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (8110+4, 3, 'Burning Steppes - Dreadmaul Rock Upper Cave - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (22932, 8110+4, 0, 'Burning Steppes - Dreadmaul Rock Upper Cave - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22931, 8110+4, 0, 'Burning Steppes - Dreadmaul Rock Upper Cave - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22923, 8110+4, 0, 'Burning Steppes - Dreadmaul Rock Upper Cave - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22925, 8110+4, 0, 'Burning Steppes - Dreadmaul Rock Upper Cave - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22928, 8110+4, 0, 'Burning Steppes - Dreadmaul Rock Upper Cave - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (14246, 8110+4, 0, 'Burning Steppes - Dreadmaul Rock Upper Cave - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14237, 8110+4, 0, 'Burning Steppes - Dreadmaul Rock Upper Cave - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14239, 8110+4, 0, 'Burning Steppes - Dreadmaul Rock Upper Cave - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14236, 8110+4, 0, 'Burning Steppes - Dreadmaul Rock Upper Cave - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0);

UPDATE `pool_template` SET `max_limit` = 5 WHERE `entry` = 1299;

-- Eastern Plaguelands
 UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands' WHERE `guid` IN (25038, 25040, 25039, 25041);

-- Gold Vein / Mithril Deposit / Truesilver Deposit
 UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Eastern Plaguelands' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 1734);

UPDATE `pool_gameobject` SET `chance` = 90 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Eastern Plaguelands' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 2040);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Eastern Plaguelands' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 2047);

-- Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands
 UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 2047);

UPDATE `pool_gameobject` SET `chance` = 95 WHERE `description` = 'Truesilver Deposit / Rich Thorium Vein - Eastern Plaguelands' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 175404);

-- Truesilver Deposit / Small Thorium Vein - Eastern Plaguelands
 UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Truesilver Deposit / Small Thorium Vein - Eastern Plaguelands' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 2047);

UPDATE `pool_gameobject` SET `chance` = 95 WHERE `description` = 'Truesilver Deposit / Small Thorium Vein - Eastern Plaguelands' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 324);

-- Felwood
 UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Felwood' WHERE `guid` IN (100096, 100097, 100095);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Felwood' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 181109);

UPDATE `pool_gameobject` SET `chance` = 90 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Felwood' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 176645);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Felwood' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 181108);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Truesilver Deposit / Small Thorium Vein - Felwood' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 181108);

UPDATE `pool_gameobject` SET `chance` = 95 WHERE `description` = 'Truesilver Deposit / Small Thorium Vein - Felwood' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 176643);

-- Shadow Hold
 UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (2358, 2373, 2357, 2354));

UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (3270, 3271, 3272, 3274, 3275));

DELETE FROM `pool_pool` WHERE `pool_id` IN (2358, 2373, 2357, 2354, 3270, 3271, 3272, 3274, 3275);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Felwood - Shadow Hold' WHERE `entry` IN (2358, 2373, 2357, 2354);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Felwood - Shadow Hold' WHERE `pool_entry` IN (2358, 2373, 2357, 2354);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Felwood - Shadow Hold' WHERE `entry` IN (3270, 3271, 3272, 3274, 3275);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Felwood - Shadow Hold' WHERE `pool_entry` IN (3270, 3271, 3272, 3274, 3275);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (8110+5, 2, 'Felwood - Shadow Hold - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (2358, 8110+5, 0, 'Felwood - Shadow Hold - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (2373, 8110+5, 0, 'Felwood - Shadow Hold - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (2357, 8110+5, 0, 'Felwood - Shadow Hold - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (2354, 8110+5, 0, 'Felwood - Shadow Hold - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (3270, 8110+5, 0, 'Felwood - Shadow Hold - Truesilver Deposit / Small Thorium Vein', 0), (3271, 8110+5, 0, 'Felwood - Shadow Hold - Truesilver Deposit / Small Thorium Vein', 0), (3272, 8110+5, 0, 'Felwood - Shadow Hold - Truesilver Deposit / Small Thorium Vein', 0), (3274, 8110+5, 0, 'Felwood - Shadow Hold - Truesilver Deposit / Small Thorium Vein', 0), (3275, 8110+5, 0, 'Felwood - Shadow Hold - Truesilver Deposit / Small Thorium Vein', 0);

-- Shrine of the Deceiver
 UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (2361, 2360, 2362, 2363, 2359));

DELETE FROM `pool_pool` WHERE `pool_id` IN (2361, 2360, 2362, 2363, 2359);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Felwood - Shrine of the Deceiver' WHERE `entry` IN (2361, 2360, 2362, 2363, 2359);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Felwood - Shrine of the Deceiver' WHERE `pool_entry` IN (2361, 2360, 2362, 2363, 2359);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (8110+6, 1, 'Felwood - Shrine of the Deceiver - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (2361, 8110+6, 0, 'Felwood - Shrine of the Deceiver - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (2360, 8110+6, 0, 'Felwood - Shrine of the Deceiver - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (2362, 8110+6, 0, 'Felwood - Shrine of the Deceiver - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (2363, 8110+6, 0, 'Felwood - Shrine of the Deceiver - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (2359, 8110+6, 0, 'Felwood - Shrine of the Deceiver - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

-- Irontree Cavern
 UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (2349, 2348, 2350));

UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (3282, 3281, 3280));

DELETE FROM `pool_pool` WHERE `pool_id` IN (2349, 2348, 2350, 3282, 3281, 3280);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Felwood - Irontree Cavern' WHERE `entry` IN (2349, 2348, 2350);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Felwood - Irontree Cavern' WHERE `pool_entry` IN (2349, 2348, 2350);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Felwood - Irontree Cavern' WHERE `entry` IN (3282, 3281, 3280);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Felwood - Irontree Cavern' WHERE `pool_entry` IN (3282, 3281, 3280);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (8110+7, 3, 'Felwood - Irontree Cavern - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (2349, 8110+7, 0, 'Felwood - Irontree Cavern - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (2348, 8110+7, 0, 'Felwood - Irontree Cavern - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (2350, 8110+7, 0, 'Felwood - Irontree Cavern - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (3282, 8110+7, 0, 'Felwood - Irontree Cavern - Truesilver Deposit / Small Thorium Vein', 0), (3281, 8110+7, 0, 'Felwood - Irontree Cavern - Truesilver Deposit / Small Thorium Vein', 0), (3280, 8110+7, 0, 'Felwood - Irontree Cavern - Truesilver Deposit / Small Thorium Vein', 0);

UPDATE `pool_template` SET `max_limit` = 5 WHERE `entry` = 328;

UPDATE `pool_template` SET `max_limit` = 7 WHERE `entry` = 1232;

-- End of migration.
;
