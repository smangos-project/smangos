INSERT INTO `migrations` VALUES ('20250202161500');

-- Kingsblood / Grave Moss - Scarlet Monastery Graveyard
 UPDATE `pool_template` SET `description` = 'Kingsblood / Grave Moss - Scarlet Monastery Graveyard' WHERE `entry` IN (2250, 2251, 2253, 2256, 2257, 2258, 2259, 2260, 2252, 2261, 2254, 2255);

-- Misc
 UPDATE `pool_template` SET `description` = 'Stranglethorn Vale - Stonescale Eel Swarm / Oily Blackmouth School / Floating Wreckage / Firefin Snapper School (Master Pool)' WHERE `entry` = 2974;

-- Tanaris
 -- Noxious Lair Hive 1
 UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (17172, 17173, 43814, 43841, 43842, 17174, 43838, 7164, 7161, 7153, 7160));

DELETE FROM `pool_pool` WHERE `pool_id` IN (17172, 17173, 43814, 43841, 43842, 17174, 43838, 7164, 7161, 7153, 7160);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Tanaris - Noxious Lair Hive 1' WHERE `entry` IN (17172, 17173, 43814, 43841, 43842, 17174, 43838);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Tanaris - Noxious Lair Hive 1' WHERE `pool_entry` IN (17172, 17173, 43814, 43841, 43842, 17174, 43838);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Tanaris - Noxious Lair Hive 1' WHERE `entry` IN (7164, 7161, 7153, 7160);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Tanaris - Noxious Lair Hive 1' WHERE `pool_entry` IN (7164, 7161, 7153, 7160);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (8138+1, 3, 'Tanaris - Noxious Lair Hive 1 - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (17172, 8138+1, 0, 'Tanaris - Noxious Lair Hive 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (17173, 8138+1, 0, 'Tanaris - Noxious Lair Hive 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (43814, 8138+1, 0, 'Tanaris - Noxious Lair Hive 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (43841, 8138+1, 0, 'Tanaris - Noxious Lair Hive 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (43842, 8138+1, 0, 'Tanaris - Noxious Lair Hive 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (17174, 8138+1, 0, 'Tanaris - Noxious Lair Hive 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (43838, 8138+1, 0, 'Tanaris - Noxious Lair Hive 1 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (7164, 8138+1, 0, 'Tanaris - Noxious Lair Hive 1 - Truesilver Deposit / Small Thorium Vein', 0), (7161, 8138+1, 0, 'Tanaris - Noxious Lair Hive 1 - Truesilver Deposit / Small Thorium Vein', 0), (7153, 8138+1, 0, 'Tanaris - Noxious Lair Hive 1 - Truesilver Deposit / Small Thorium Vein', 0), (7160, 8138+1, 0, 'Tanaris - Noxious Lair Hive 1 - Truesilver Deposit / Small Thorium Vein', 0);

-- Noxious Lair Hive 2
 UPDATE `gameobject` SET `spawntimesecsmin` = 900, `spawntimesecsmax` = 1800 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (17175, 43845, 43846, 43844, 43843));

DELETE FROM `pool_pool` WHERE `pool_id` IN (17175, 43845, 43846, 43844, 43843);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Tanaris - Noxious Lair Hive 2' WHERE `entry` IN (17175, 43845, 43846, 43844, 43843);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Tanaris - Noxious Lair Hive 2' WHERE `pool_entry` IN (17175, 43845, 43846, 43844, 43843);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (8138+2, 2, 'Tanaris - Noxious Lair Hive 2 - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (17175, 8138+2, 0, 'Tanaris - Noxious Lair Hive 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (43845, 8138+2, 0, 'Tanaris - Noxious Lair Hive 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (43846, 8138+2, 0, 'Tanaris - Noxious Lair Hive 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (43844, 8138+2, 0, 'Tanaris - Noxious Lair Hive 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (43843, 8138+2, 0, 'Tanaris - Noxious Lair Hive 2 - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

-- Gaping Chasm
 UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (43829, 43833, 43830, 43802, 43831, 43803, 17167, 43834, 43805, 7154, 7150, 7156));

DELETE FROM `pool_pool` WHERE `pool_id` IN (43829, 43833, 43830, 43802, 43831, 43803, 17167, 43834, 43805, 7154, 7150, 7156);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Tanaris - Gaping Chasm' WHERE `entry` IN (43829, 43833, 43830, 43802, 43831, 43803, 17167, 43834, 43805);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Tanaris - Gaping Chasm' WHERE `pool_entry` IN (43829, 43833, 43830, 43802, 43831, 43803, 17167, 43834, 43805);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Tanaris - Gaping Chasm' WHERE `entry` IN (7154, 7150, 7156);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Small Thorium Vein - Tanaris - Gaping Chasm' WHERE `pool_entry` IN (7154, 7150, 7156);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (8138+3, 4, 'Tanaris - Gaping Chasm - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (43829, 8138+3, 0, 'Tanaris - Gaping Chasm - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (43833, 8138+3, 0, 'Tanaris - Gaping Chasm - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (43830, 8138+3, 0, 'Tanaris - Gaping Chasm - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (43802, 8138+3, 0, 'Tanaris - Gaping Chasm - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (43831, 8138+3, 0, 'Tanaris - Gaping Chasm - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (43803, 8138+3, 0, 'Tanaris - Gaping Chasm - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (17167, 8138+3, 0, 'Tanaris - Gaping Chasm - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (43834, 8138+3, 0, 'Tanaris - Gaping Chasm - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (43805, 8138+3, 0, 'Tanaris - Gaping Chasm - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (7154, 8138+3, 0, 'Tanaris - Gaping Chasm - Truesilver Deposit / Small Thorium Vein', 0), (7150, 8138+3, 0, 'Tanaris - Gaping Chasm - Truesilver Deposit / Small Thorium Vein', 0), (7156, 8138+3, 0, 'Tanaris - Gaping Chasm - Truesilver Deposit / Small Thorium Vein', 0);

UPDATE `pool_template` SET `max_limit` = 9 WHERE `entry` = 1203;

-- Stranglethorn Vale
 UPDATE `pool_pool` SET `mother_pool` = 9911 WHERE `description` LIKE 'Stranglethorn Vale - Silver Vein / Gold Vein / Iron Deposit';

-- Crystalvein Mine
 UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (5713, 5734, 5708, 5747, 5748, 5746, 5646, 5662, 5624));

DELETE FROM `pool_pool` WHERE `pool_id` IN (5713, 5734, 5708, 5747, 5748, 5746, 5646, 5662, 5624);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale - Crystalvein Mine' WHERE `entry` IN (5713, 5734, 5708, 5747, 5748, 5746);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale - Crystalvein Mine' WHERE `pool_entry` IN (5713, 5734, 5708, 5747, 5748, 5746);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Crystalvein Mine' WHERE `entry` IN (5646, 5662, 5624);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Crystalvein Mine' WHERE `pool_entry` IN (5646, 5662, 5624);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (8138+4, 3, 'Stranglethorn Vale - Crystalvein Mine - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5713, 8138+4, 0, 'Stranglethorn Vale - Crystalvein Mine - Silver Vein / Gold Vein / Iron Deposit', 0), (5734, 8138+4, 0, 'Stranglethorn Vale - Crystalvein Mine - Silver Vein / Gold Vein / Iron Deposit', 0), (5708, 8138+4, 0, 'Stranglethorn Vale - Crystalvein Mine - Silver Vein / Gold Vein / Iron Deposit', 0), (5747, 8138+4, 0, 'Stranglethorn Vale - Crystalvein Mine - Silver Vein / Gold Vein / Iron Deposit', 0), (5748, 8138+4, 0, 'Stranglethorn Vale - Crystalvein Mine - Silver Vein / Gold Vein / Iron Deposit', 0), (5746, 8138+4, 0, 'Stranglethorn Vale - Crystalvein Mine - Silver Vein / Gold Vein / Iron Deposit', 0), (5646, 8138+4, 0, 'Stranglethorn Vale - Crystalvein Mine - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5662, 8138+4, 0, 'Stranglethorn Vale - Crystalvein Mine - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5624, 8138+4, 0, 'Stranglethorn Vale - Crystalvein Mine - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

-- Mosh'Ogg Ogre Mound
 INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (32001, 1734, 0, -12369.8, -1055.09, 4.36752, 1.8326, 0, 0, 0.793353, 0.608761, 300, 300, 100, 1, 0, 0, 0, 10), (32002, 1735, 0, -12369.8, -1055.09, 4.36752, 1.8326, 0, 0, 0.793353, 0.608761, 300, 300, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `patch_max`) VALUES (5283, 1, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale - Mosh''Ogg Ogre Mound', 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`, `patch_max`) VALUES (31588, 5283, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale - Mosh''Ogg Ogre Mound', 10), (32001, 5283, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale - Mosh''Ogg Ogre Mound', 10), (32002, 5283, 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale - Mosh''Ogg Ogre Mound', 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5283, 9911, 0, 'Stranglethorn Vale - Mosh''Ogg Ogre Mound - Silver Vein / Gold Vein / Iron Deposit', 0);

UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (5733, 5752, 5739, 5753, 5754, 5711, 5755, 5283, 5670, 5631, 5637));

DELETE FROM `pool_pool` WHERE `pool_id` IN (5733, 5752, 5739, 5753, 5754, 5711, 5755, 5283, 5670, 5631, 5637);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale - Mosh''Ogg Ogre Mound' WHERE `entry` IN (5733, 5752, 5739, 5753, 5754, 5711, 5755, 5283);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale - Mosh''Ogg Ogre Mound' WHERE `pool_entry` IN (5733, 5752, 5739, 5753, 5754, 5711, 5755, 5283);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Mosh''Ogg Ogre Mound' WHERE `entry` IN (5670, 5631, 5637);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Mosh''Ogg Ogre Mound' WHERE `pool_entry` IN (5670, 5631, 5637);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (8138+5, 3, 'Stranglethorn Vale - Mosh''Ogg Ogre Mound - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5733, 8138+5, 0, 'Stranglethorn Vale - Mosh''Ogg Ogre Mound - Silver Vein / Gold Vein / Iron Deposit', 0), (5752, 8138+5, 0, 'Stranglethorn Vale - Mosh''Ogg Ogre Mound - Silver Vein / Gold Vein / Iron Deposit', 0), (5739, 8138+5, 0, 'Stranglethorn Vale - Mosh''Ogg Ogre Mound - Silver Vein / Gold Vein / Iron Deposit', 0), (5753, 8138+5, 0, 'Stranglethorn Vale - Mosh''Ogg Ogre Mound - Silver Vein / Gold Vein / Iron Deposit', 0), (5754, 8138+5, 0, 'Stranglethorn Vale - Mosh''Ogg Ogre Mound - Silver Vein / Gold Vein / Iron Deposit', 0), (5711, 8138+5, 0, 'Stranglethorn Vale - Mosh''Ogg Ogre Mound - Silver Vein / Gold Vein / Iron Deposit', 0), (5755, 8138+5, 0, 'Stranglethorn Vale - Mosh''Ogg Ogre Mound - Silver Vein / Gold Vein / Iron Deposit', 0), (5283, 8138+5, 0, 'Stranglethorn Vale - Mosh''Ogg Ogre Mound - Silver Vein / Gold Vein / Iron Deposit', 0), (5670, 8138+5, 0, 'Stranglethorn Vale - Mosh''Ogg Ogre Mound - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5631, 8138+5, 0, 'Stranglethorn Vale - Mosh''Ogg Ogre Mound - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5637, 8138+5, 0, 'Stranglethorn Vale - Mosh''Ogg Ogre Mound - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

-- Stockpile
 UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (5715, 5718, 5767, 5738, 5763, 5764, 5765, 5768, 5770, 5707, 5769, 5766, 5671, 5636, 5641));

DELETE FROM `pool_pool` WHERE `pool_id` IN (5715, 5718, 5767, 5738, 5763, 5764, 5765, 5768, 5770, 5707, 5769, 5766, 5671, 5636, 5641);

UPDATE `pool_template` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale - Stockpile' WHERE `entry` IN (5715, 5718, 5767, 5738, 5763, 5764, 5765, 5768, 5770, 5707, 5769, 5766);

UPDATE `pool_gameobject` SET `description` = 'Silver Vein / Gold Vein / Iron Deposit - Stranglethorn Vale - Stockpile' WHERE `pool_entry` IN (5715, 5718, 5767, 5738, 5763, 5764, 5765, 5768, 5770, 5707, 5769, 5766);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Stockpile' WHERE `entry` IN (5671, 5636, 5641);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Stranglethorn Vale - Stockpile' WHERE `pool_entry` IN (5671, 5636, 5641);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (8138+6, 4, 'Stranglethorn Vale - Stockpile - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5715, 8138+6, 0, 'Stranglethorn Vale - Stockpile - Silver Vein / Gold Vein / Iron Deposit', 0), (5718, 8138+6, 0, 'Stranglethorn Vale - Stockpile - Silver Vein / Gold Vein / Iron Deposit', 0), (5767, 8138+6, 0, 'Stranglethorn Vale - Stockpile - Silver Vein / Gold Vein / Iron Deposit', 0), (5738, 8138+6, 0, 'Stranglethorn Vale - Stockpile - Silver Vein / Gold Vein / Iron Deposit', 0), (5763, 8138+6, 0, 'Stranglethorn Vale - Stockpile - Silver Vein / Gold Vein / Iron Deposit', 0), (5764, 8138+6, 0, 'Stranglethorn Vale - Stockpile - Silver Vein / Gold Vein / Iron Deposit', 0), (5765, 8138+6, 0, 'Stranglethorn Vale - Stockpile - Silver Vein / Gold Vein / Iron Deposit', 0), (5768, 8138+6, 0, 'Stranglethorn Vale - Stockpile - Silver Vein / Gold Vein / Iron Deposit', 0), (5770, 8138+6, 0, 'Stranglethorn Vale - Stockpile - Silver Vein / Gold Vein / Iron Deposit', 0), (5707, 8138+6, 0, 'Stranglethorn Vale - Stockpile - Silver Vein / Gold Vein / Iron Deposit', 0), (5769, 8138+6, 0, 'Stranglethorn Vale - Stockpile - Silver Vein / Gold Vein / Iron Deposit', 0), (5766, 8138+6, 0, 'Stranglethorn Vale - Stockpile - Silver Vein / Gold Vein / Iron Deposit', 0), (5671, 8138+6, 0, 'Stranglethorn Vale - Stockpile - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5636, 8138+6, 0, 'Stranglethorn Vale - Stockpile - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5641, 8138+6, 0, 'Stranglethorn Vale - Stockpile - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

UPDATE `pool_template` SET `max_limit` = 6 WHERE `entry` = 9910;

UPDATE `pool_template` SET `max_limit` = 13 WHERE `entry` = 9911;

UPDATE `pool_template` SET `max_limit` = 3 WHERE `entry` = 9912;

-- Silithus
 UPDATE `pool_gameobject` SET `description` = 'Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein - Silithus' WHERE `guid` IN (24053, 23983, 24052, 24054);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Truesilver Deposit / Small Thorium Vein - Silithus' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 2047);

UPDATE `pool_gameobject` SET `chance` = 95 WHERE `description` = 'Truesilver Deposit / Small Thorium Vein - Silithus' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 324);

UPDATE `pool_gameobject` SET `chance` = 5 WHERE `description` = 'Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein - Silithus' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 123309);

UPDATE `pool_gameobject` SET `chance` = 95 WHERE `description` = 'Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein - Silithus' AND `guid` IN (SELECT `guid` FROM `gameobject` WHERE `id` = 177388);

UPDATE `pool_pool` SET `mother_pool` = 2338, `description` = 'Azshara - Gold Vein / Mithril Deposit / Truesilver Deposit' WHERE `pool_id` IN (23033, 23034, 23035);

-- Hive'Regal
 UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (3121, 3122, 3123, 3990, 3120, 3124, 3116, 3114, 3113, 3119, 3118, 3117, 3115, 3112, 14454, 14453));

DELETE FROM `pool_pool` WHERE `pool_id` IN (3121, 3122, 3123, 3990, 3120, 3124, 3116, 3114, 3113, 3119, 3118, 3117, 3115, 3112, 14454, 14453);

UPDATE `pool_template` SET `description` = 'Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein - Silithus - Hive''Regal' WHERE `entry` IN (3121, 3122, 3123, 3990, 3120, 3124, 3116, 3114, 3113, 3119, 3118, 3117, 3115, 3112, 14454, 14453);

UPDATE `pool_gameobject` SET `description` = 'Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein - Silithus - Hive''Regal' WHERE `pool_entry` IN (3121, 3122, 3123, 3990, 3120, 3124, 3116, 3114, 3113, 3119, 3118, 3117, 3115, 3112, 14454, 14453);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (8138+7, 5, 'Silithus - Hive''Regal - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (3121, 8138+7, 0, 'Silithus - Hive''Regal - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3122, 8138+7, 0, 'Silithus - Hive''Regal - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3123, 8138+7, 0, 'Silithus - Hive''Regal - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3990, 8138+7, 0, 'Silithus - Hive''Regal - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3120, 8138+7, 0, 'Silithus - Hive''Regal - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3124, 8138+7, 0, 'Silithus - Hive''Regal - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3116, 8138+7, 0, 'Silithus - Hive''Regal - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3114, 8138+7, 0, 'Silithus - Hive''Regal - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3113, 8138+7, 0, 'Silithus - Hive''Regal - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3119, 8138+7, 0, 'Silithus - Hive''Regal - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3118, 8138+7, 0, 'Silithus - Hive''Regal - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3117, 8138+7, 0, 'Silithus - Hive''Regal - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3115, 8138+7, 0, 'Silithus - Hive''Regal - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3112, 8138+7, 0, 'Silithus - Hive''Regal - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (14454, 8138+7, 0, 'Silithus - Hive''Regal - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (14453, 8138+7, 0, 'Silithus - Hive''Regal - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0);

-- Hive'Zora
 UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (3126, 3979, 3129, 3135, 3132, 3131, 3136, 3133, 3134, 3128, 3125, 3127, 3130, 3137, 3138, 3139, 14456, 14455));

DELETE FROM `pool_pool` WHERE `pool_id` IN (3126, 3979, 3129, 3135, 3132, 3131, 3136, 3133, 3134, 3128, 3125, 3127, 3130, 3137, 3138, 3139, 14456, 14455);

UPDATE `pool_template` SET `description` = 'Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein - Silithus - Hive''Zora' WHERE `entry` IN (3126, 3979, 3129, 3135, 3132, 3131, 3136, 3133, 3134, 3128, 3125, 3127, 3130, 3137, 3138, 3139, 14456, 14455);

UPDATE `pool_gameobject` SET `description` = 'Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein - Silithus - Hive''Zora' WHERE `pool_entry` IN (3126, 3979, 3129, 3135, 3132, 3131, 3136, 3133, 3134, 3128, 3125, 3127, 3130, 3137, 3138, 3139, 14456, 14455);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (8138+8, 5, 'Silithus - Hive''Zora - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (3979, 8138+8, 0, 'Silithus - Hive''Zora - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3129, 8138+8, 0, 'Silithus - Hive''Zora - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3135, 8138+8, 0, 'Silithus - Hive''Zora - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3132, 8138+8, 0, 'Silithus - Hive''Zora - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3131, 8138+8, 0, 'Silithus - Hive''Zora - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3136, 8138+8, 0, 'Silithus - Hive''Zora - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3133, 8138+8, 0, 'Silithus - Hive''Zora - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3134, 8138+8, 0, 'Silithus - Hive''Zora - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3128, 8138+8, 0, 'Silithus - Hive''Zora - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3125, 8138+8, 0, 'Silithus - Hive''Zora - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3127, 8138+8, 0, 'Silithus - Hive''Zora - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3130, 8138+8, 0, 'Silithus - Hive''Zora - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3137, 8138+8, 0, 'Silithus - Hive''Zora - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3138, 8138+8, 0, 'Silithus - Hive''Zora - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3139, 8138+8, 0, 'Silithus - Hive''Zora - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3126, 8138+8, 0, 'Silithus - Hive''Zora - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (14456, 8138+8, 0, 'Silithus - Hive''Zora - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (14455, 8138+8, 0, 'Silithus - Hive''Zora - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0);

-- Hive'Ashi
 UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (3142, 3152, 3154, 3140, 3145, 3144, 3146, 3155, 3156, 3143, 3141, 3153, 3151, 3157, 3149, 3148, 3150, 3158, 3159, 3160, 3162, 3161, 3163, 3147, 3164));

DELETE FROM `pool_pool` WHERE `pool_id` IN (3142, 3152, 3154, 3140, 3145, 3144, 3146, 3155, 3156, 3143, 3141, 3153, 3151, 3157, 3149, 3148, 3150, 3158, 3159, 3160, 3162, 3161, 3163, 3147, 3164);

UPDATE `pool_template` SET `description` = 'Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein - Silithus - Hive''Ashi' WHERE `entry` IN (3142, 3152, 3154, 3140, 3145, 3144, 3146, 3155, 3156, 3143, 3141, 3153, 3151, 3157, 3149, 3148, 3150, 3158, 3159, 3160, 3162, 3161, 3163, 3147, 3164);

UPDATE `pool_gameobject` SET `description` = 'Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein - Silithus - Hive''Ashi' WHERE `pool_entry` IN (3142, 3152, 3154, 3140, 3145, 3144, 3146, 3155, 3156, 3143, 3141, 3153, 3151, 3157, 3149, 3148, 3150, 3158, 3159, 3160, 3162, 3161, 3163, 3147, 3164);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (8138+9, 5, 'Silithus - Hive''Ashi - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (3142, 8138+9, 0, 'Silithus - Hive''Ashi - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3152, 8138+9, 0, 'Silithus - Hive''Ashi - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3154, 8138+9, 0, 'Silithus - Hive''Ashi - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3140, 8138+9, 0, 'Silithus - Hive''Ashi - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3145, 8138+9, 0, 'Silithus - Hive''Ashi - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3144, 8138+9, 0, 'Silithus - Hive''Ashi - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3146, 8138+9, 0, 'Silithus - Hive''Ashi - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3155, 8138+9, 0, 'Silithus - Hive''Ashi - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3156, 8138+9, 0, 'Silithus - Hive''Ashi - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3143, 8138+9, 0, 'Silithus - Hive''Ashi - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3141, 8138+9, 0, 'Silithus - Hive''Ashi - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3153, 8138+9, 0, 'Silithus - Hive''Ashi - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3151, 8138+9, 0, 'Silithus - Hive''Ashi - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3157, 8138+9, 0, 'Silithus - Hive''Ashi - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3149, 8138+9, 0, 'Silithus - Hive''Ashi - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3148, 8138+9, 0, 'Silithus - Hive''Ashi - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3150, 8138+9, 0, 'Silithus - Hive''Ashi - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3158, 8138+9, 0, 'Silithus - Hive''Ashi - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3159, 8138+9, 0, 'Silithus - Hive''Ashi - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3160, 8138+9, 0, 'Silithus - Hive''Ashi - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3162, 8138+9, 0, 'Silithus - Hive''Ashi - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3161, 8138+9, 0, 'Silithus - Hive''Ashi - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3163, 8138+9, 0, 'Silithus - Hive''Ashi - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3147, 8138+9, 0, 'Silithus - Hive''Ashi - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0), (3164, 8138+9, 0, 'Silithus - Hive''Ashi - Ooze Covered Truesilver Deposit / Ooze Covered Rich Thorium Vein', 0);

DELETE FROM `pool_template` WHERE `entry` = 1220;

UPDATE `pool_template` SET `max_limit` = 9 WHERE `entry` = 1219;

-- Desolace
 UPDATE `pool_template` SET `description` = 'Copper Veins - Desolace (Master Pool)' WHERE `entry` = 1139;

-- Maraudon Entrance
 UPDATE `gameobject` SET `spawntimesecsmin` = 600, `spawntimesecsmax` = 2700 WHERE `guid` IN (SELECT `guid` FROM `pool_gameobject` WHERE `pool_entry` IN (5408, 5417, 5112, 5108, 5113, 5104, 5125, 5404, 5123, 5122, 5419, 5135, 5136, 5137, 5406, 5124));

DELETE FROM `pool_pool` WHERE `pool_id` IN (5408, 5417, 5112, 5108, 5113, 5104, 5125, 5404, 5123, 5122, 5419, 5135, 5136, 5137, 5406, 5124);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Desolace - Maraudon Entrance' WHERE `entry` IN (5408, 5417, 5112, 5108, 5113, 5104, 5125, 5404, 5123, 5122, 5419, 5135, 5136, 5137, 5406, 5124);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit - Desolace - Maraudon Entrance' WHERE `pool_entry` IN (5408, 5417, 5112, 5108, 5113, 5104, 5125, 5404, 5123, 5122, 5419, 5135, 5136, 5137, 5406, 5124);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (8138+10, 3, 'Desolace - Maraudon Entrance - Minerals (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (5408, 8138+10, 0, 'Desolace - Maraudon Entrance - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5417, 8138+10, 0, 'Desolace - Maraudon Entrance - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5112, 8138+10, 0, 'Desolace - Maraudon Entrance - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5108, 8138+10, 0, 'Desolace - Maraudon Entrance - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5113, 8138+10, 0, 'Desolace - Maraudon Entrance - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5104, 8138+10, 0, 'Desolace - Maraudon Entrance - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5125, 8138+10, 0, 'Desolace - Maraudon Entrance - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5404, 8138+10, 0, 'Desolace - Maraudon Entrance - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5123, 8138+10, 0, 'Desolace - Maraudon Entrance - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5122, 8138+10, 0, 'Desolace - Maraudon Entrance - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5419, 8138+10, 0, 'Desolace - Maraudon Entrance - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5135, 8138+10, 0, 'Desolace - Maraudon Entrance - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5136, 8138+10, 0, 'Desolace - Maraudon Entrance - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5137, 8138+10, 0, 'Desolace - Maraudon Entrance - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5406, 8138+10, 0, 'Desolace - Maraudon Entrance - Gold Vein / Mithril Deposit / Truesilver Deposit', 0), (5124, 8138+10, 0, 'Desolace - Maraudon Entrance - Gold Vein / Mithril Deposit / Truesilver Deposit', 0);

UPDATE `pool_template` SET `max_limit` = 10 WHERE `entry` = 9907;

UPDATE `pool_template` SET `max_limit` = 13 WHERE `entry` = 9908;

UPDATE `pool_template` SET `max_limit` = 8 WHERE `entry` = 9909;

UPDATE `pool_template` SET `max_limit` = 4 WHERE `entry` = 1139;

-- End of migration.
;
