INSERT INTO `migrations` VALUES ('20260405161744');

DELETE FROM `pool_pool` WHERE `pool_id` IN (22918, 22916, 14258, 22914, 22912, 22913);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Burning Steppes - Group 1' WHERE `entry` IN (22918, 22916, 14258, 22914, 22912, 22913);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Burning Steppes - Group 1' WHERE `pool_entry` IN (22918, 22916, 14258, 22914, 22912, 22913);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4893+1, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Burning Steppes - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (22918, 4893+1, 0, 'Burning Steppes - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22916, 4893+1, 0, 'Burning Steppes - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (14258, 4893+1, 0, 'Burning Steppes - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22914, 4893+1, 0, 'Burning Steppes - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22912, 4893+1, 0, 'Burning Steppes - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22913, 4893+1, 0, 'Burning Steppes - Group 1 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (22911, 22917, 22937, 22924, 22933);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Burning Steppes - Group 2' WHERE `entry` IN (22911, 22917, 22937, 22924, 22933);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Burning Steppes - Group 2' WHERE `pool_entry` IN (22911, 22917, 22937, 22924, 22933);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4893+2, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Burning Steppes - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (22911, 4893+2, 0, 'Burning Steppes - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22917, 4893+2, 0, 'Burning Steppes - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22937, 4893+2, 0, 'Burning Steppes - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22924, 4893+2, 0, 'Burning Steppes - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22933, 4893+2, 0, 'Burning Steppes - Group 2 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (22915, 22919, 22935, 22938, 22939);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Burning Steppes - Group 3' WHERE `entry` IN (22915, 22919, 22935, 22938, 22939);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Burning Steppes - Group 3' WHERE `pool_entry` IN (22915, 22919, 22935, 22938, 22939);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4893+3, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Burning Steppes - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (22915, 4893+3, 0, 'Burning Steppes - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22919, 4893+3, 0, 'Burning Steppes - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22935, 4893+3, 0, 'Burning Steppes - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22938, 4893+3, 0, 'Burning Steppes - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22939, 4893+3, 0, 'Burning Steppes - Group 3 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (22936, 22934, 22920, 22929, 22922);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Burning Steppes - Group 4' WHERE `entry` IN (22936, 22934, 22920, 22929, 22922);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Burning Steppes - Group 4' WHERE `pool_entry` IN (22936, 22934, 22920, 22929, 22922);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4893+4, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Burning Steppes - Group 4 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (22936, 4893+4, 0, 'Burning Steppes - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22934, 4893+4, 0, 'Burning Steppes - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22920, 4893+4, 0, 'Burning Steppes - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22929, 4893+4, 0, 'Burning Steppes - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22922, 4893+4, 0, 'Burning Steppes - Group 4 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (22928, 22925, 22923, 22931, 22932);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Burning Steppes - Group 5' WHERE `entry` IN (22928, 22925, 22923, 22931, 22932);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Burning Steppes - Group 5' WHERE `pool_entry` IN (22928, 22925, 22923, 22931, 22932);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4893+5, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Burning Steppes - Group 5 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (22928, 4893+5, 0, 'Burning Steppes - Group 5 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22925, 4893+5, 0, 'Burning Steppes - Group 5 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22923, 4893+5, 0, 'Burning Steppes - Group 5 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22931, 4893+5, 0, 'Burning Steppes - Group 5 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22932, 4893+5, 0, 'Burning Steppes - Group 5 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (22921, 22927, 22930, 22926);

UPDATE `pool_template` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Burning Steppes - Group 6' WHERE `entry` IN (22921, 22927, 22930, 22926);

UPDATE `pool_gameobject` SET `description` = 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Burning Steppes - Group 6' WHERE `pool_entry` IN (22921, 22927, 22930, 22926);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4893+6, 1, 'Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit - Burning Steppes - Group 6 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (22921, 4893+6, 0, 'Burning Steppes - Group 6 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22927, 4893+6, 0, 'Burning Steppes - Group 6 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22930, 4893+6, 0, 'Burning Steppes - Group 6 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0), (22926, 4893+6, 0, 'Burning Steppes - Group 6 - Gold Vein / Mithril Deposit / Truesilver Deposit / Dark Iron Deposit', 0);

DELETE FROM `pool_template` WHERE `entry` = 1299;

DELETE FROM `pool_pool` WHERE `pool_id` IN (14245, 14230, 14225, 14232, 14251, 14243);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes - Group 1' WHERE `entry` IN (14245, 14230, 14225, 14232, 14251, 14243);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes - Group 1' WHERE `pool_entry` IN (14245, 14230, 14225, 14232, 14251, 14243);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4910+1, 1, 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (14245, 4910+1, 0, 'Burning Steppes - Group 1 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14230, 4910+1, 0, 'Burning Steppes - Group 1 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14225, 4910+1, 0, 'Burning Steppes - Group 1 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14232, 4910+1, 0, 'Burning Steppes - Group 1 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14251, 4910+1, 0, 'Burning Steppes - Group 1 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14243, 4910+1, 0, 'Burning Steppes - Group 1 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (14226, 14223, 14235, 14229, 14234);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes - Group 2' WHERE `entry` IN (14226, 14223, 14235, 14229, 14234);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes - Group 2' WHERE `pool_entry` IN (14226, 14223, 14235, 14229, 14234);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4910+2, 1, 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (14226, 4910+2, 0, 'Burning Steppes - Group 2 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14223, 4910+2, 0, 'Burning Steppes - Group 2 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14235, 4910+2, 0, 'Burning Steppes - Group 2 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14229, 4910+2, 0, 'Burning Steppes - Group 2 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14234, 4910+2, 0, 'Burning Steppes - Group 2 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (14250, 14241, 14256, 14254, 14257);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes - Group 3' WHERE `entry` IN (14250, 14241, 14256, 14254, 14257);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes - Group 3' WHERE `pool_entry` IN (14250, 14241, 14256, 14254, 14257);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4910+3, 1, 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (14250, 4910+3, 0, 'Burning Steppes - Group 3 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14241, 4910+3, 0, 'Burning Steppes - Group 3 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14256, 4910+3, 0, 'Burning Steppes - Group 3 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14254, 4910+3, 0, 'Burning Steppes - Group 3 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14257, 4910+3, 0, 'Burning Steppes - Group 3 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (14231, 14227, 14233, 14224, 17006);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes - Group 4' WHERE `entry` IN (14231, 14227, 14233, 14224, 17006);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes - Group 4' WHERE `pool_entry` IN (14231, 14227, 14233, 14224, 17006);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4910+4, 1, 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes - Group 4 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (14231, 4910+4, 0, 'Burning Steppes - Group 4 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14227, 4910+4, 0, 'Burning Steppes - Group 4 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14233, 4910+4, 0, 'Burning Steppes - Group 4 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14224, 4910+4, 0, 'Burning Steppes - Group 4 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (17006, 4910+4, 0, 'Burning Steppes - Group 4 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (14242, 14252, 14253, 22940, 14247);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes - Group 5' WHERE `entry` IN (14242, 14252, 14253, 22940, 14247);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes - Group 5' WHERE `pool_entry` IN (14242, 14252, 14253, 22940, 14247);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4910+5, 1, 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes - Group 5 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (14242, 4910+5, 0, 'Burning Steppes - Group 5 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14252, 4910+5, 0, 'Burning Steppes - Group 5 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14253, 4910+5, 0, 'Burning Steppes - Group 5 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (22940, 4910+5, 0, 'Burning Steppes - Group 5 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14247, 4910+5, 0, 'Burning Steppes - Group 5 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (14248, 14244, 14240, 14238);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes - Group 6' WHERE `entry` IN (14248, 14244, 14240, 14238);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes - Group 6' WHERE `pool_entry` IN (14248, 14244, 14240, 14238);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4910+6, 1, 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes - Group 6 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (14248, 4910+6, 0, 'Burning Steppes - Group 6 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14244, 4910+6, 0, 'Burning Steppes - Group 6 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14240, 4910+6, 0, 'Burning Steppes - Group 6 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14238, 4910+6, 0, 'Burning Steppes - Group 6 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (14246, 14237, 14236, 14239);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes - Group 7' WHERE `entry` IN (14246, 14237, 14236, 14239);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes - Group 7' WHERE `pool_entry` IN (14246, 14237, 14236, 14239);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4910+7, 1, 'Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein - Burning Steppes - Group 7 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (14246, 4910+7, 0, 'Burning Steppes - Group 7 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14237, 4910+7, 0, 'Burning Steppes - Group 7 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14236, 4910+7, 0, 'Burning Steppes - Group 7 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0), (14239, 4910+7, 0, 'Burning Steppes - Group 7 - Truesilver Deposit / Dark Iron Deposit / Small Thorium Vein', 0);

DELETE FROM `pool_template` WHERE `entry` IN (1296, 8114, 8113);

DELETE FROM `pool_pool` WHERE `pool_id` IN (21427, 21428, 21426, 21418, 21423);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes - Group 1' WHERE `entry` IN (21427, 21428, 21426, 21418, 21423);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes - Group 1' WHERE `pool_entry` IN (21427, 21428, 21426, 21418, 21423);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4794+1, 1, 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (21427, 4794+1, 0, 'Burning Steppes - Group 1 - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21428, 4794+1, 0, 'Burning Steppes - Group 1 - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21426, 4794+1, 0, 'Burning Steppes - Group 1 - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21418, 4794+1, 0, 'Burning Steppes - Group 1 - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21423, 4794+1, 0, 'Burning Steppes - Group 1 - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (21417, 21425, 21420, 21419, 21421);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes - Group 2' WHERE `entry` IN (21417, 21425, 21420, 21419, 21421);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes - Group 2' WHERE `pool_entry` IN (21417, 21425, 21420, 21419, 21421);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4794+2, 1, 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (21417, 4794+2, 0, 'Burning Steppes - Group 2 - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21425, 4794+2, 0, 'Burning Steppes - Group 2 - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21420, 4794+2, 0, 'Burning Steppes - Group 2 - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21419, 4794+2, 0, 'Burning Steppes - Group 2 - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21421, 4794+2, 0, 'Burning Steppes - Group 2 - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (21439, 21430, 21429, 21431, 21434);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes - Group 3' WHERE `entry` IN (21439, 21430, 21429, 21431, 21434);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes - Group 3' WHERE `pool_entry` IN (21439, 21430, 21429, 21431, 21434);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4794+3, 1, 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (21439, 4794+3, 0, 'Burning Steppes - Group 3 - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21430, 4794+3, 0, 'Burning Steppes - Group 3 - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21429, 4794+3, 0, 'Burning Steppes - Group 3 - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21431, 4794+3, 0, 'Burning Steppes - Group 3 - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21434, 4794+3, 0, 'Burning Steppes - Group 3 - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (21440, 21432, 21435, 21433, 21438);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes - Group 4' WHERE `entry` IN (21440, 21432, 21435, 21433, 21438);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes - Group 4' WHERE `pool_entry` IN (21440, 21432, 21435, 21433, 21438);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4794+4, 1, 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes - Group 4 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (21440, 4794+4, 0, 'Burning Steppes - Group 4 - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21432, 4794+4, 0, 'Burning Steppes - Group 4 - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21435, 4794+4, 0, 'Burning Steppes - Group 4 - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21433, 4794+4, 0, 'Burning Steppes - Group 4 - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21438, 4794+4, 0, 'Burning Steppes - Group 4 - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (21437, 21416, 21441, 21436);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes - Group 5' WHERE `entry` IN (21437, 21416, 21441, 21436);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes - Group 5' WHERE `pool_entry` IN (21437, 21416, 21441, 21436);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4794+5, 1, 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes - Group 5 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (21437, 4794+5, 0, 'Burning Steppes - Group 5 - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21416, 4794+5, 0, 'Burning Steppes - Group 5 - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21441, 4794+5, 0, 'Burning Steppes - Group 5 - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21436, 4794+5, 0, 'Burning Steppes - Group 5 - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0);

DELETE FROM `pool_template` WHERE `entry` IN (3557, 3558, 3555, 3559, 3556);

-- End of migration.
;
