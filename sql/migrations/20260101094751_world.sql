INSERT INTO `migrations` VALUES ('20260101094751');

-- Misc
 DELETE FROM `pool_pool` WHERE `pool_id` IN (21436, 21441, 21437, 21416);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes - Misc' WHERE `entry` IN (21436, 21441, 21437, 21416);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes - Misc' WHERE `pool_entry` IN (21436, 21441, 21437, 21416);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3554+1, 1, 'Burning Steppes - Misc - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (21436, 3554+1, 0, 'Burning Steppes - Misc - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21441, 3554+1, 0, 'Burning Steppes - Misc - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21437, 3554+1, 0, 'Burning Steppes - Misc - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21416, 3554+1, 0, 'Burning Steppes - Misc - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0);

-- Ruins of Thaurissian
 DELETE FROM `pool_pool` WHERE `pool_id` IN (21438, 21433, 21435, 21432, 21440);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes - Ruins of Thaurissian' WHERE `entry` IN (21438, 21433, 21435, 21432, 21440);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes - Ruins of Thaurissian' WHERE `pool_entry` IN (21438, 21433, 21435, 21432, 21440);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3554+2, 1, 'Burning Steppes - Ruins of Thaurissian - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (21438, 3554+2, 0, 'Burning Steppes - Ruins of Thaurissian - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21433, 3554+2, 0, 'Burning Steppes - Ruins of Thaurissian - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21435, 3554+2, 0, 'Burning Steppes - Ruins of Thaurissian - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21432, 3554+2, 0, 'Burning Steppes - Ruins of Thaurissian - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21440, 3554+2, 0, 'Burning Steppes - Ruins of Thaurissian - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0);

-- Blackrock Stronghold
 DELETE FROM `pool_pool` WHERE `pool_id` IN (21434, 21431, 21429, 21430, 21439);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes - Blackrock Stronghold' WHERE `entry` IN (21434, 21431, 21429, 21430, 21439);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes - Blackrock Stronghold' WHERE `pool_entry` IN (21434, 21431, 21429, 21430, 21439);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3554+3, 1, 'Burning Steppes - Blackrock Stronghold - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (21434, 3554+3, 0, 'Burning Steppes - Blackrock Stronghold - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21431, 3554+3, 0, 'Burning Steppes - Blackrock Stronghold - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21429, 3554+3, 0, 'Burning Steppes - Blackrock Stronghold - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21430, 3554+3, 0, 'Burning Steppes - Blackrock Stronghold - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21439, 3554+3, 0, 'Burning Steppes - Blackrock Stronghold - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0);

-- Draco\'dar
 DELETE FROM `pool_pool` WHERE `pool_id` IN (21427, 21418, 21423, 21426, 21428);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes - Draco''dar' WHERE `entry` IN (21427, 21418, 21423, 21426, 21428);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes - Draco''dar' WHERE `pool_entry` IN (21427, 21418, 21423, 21426, 21428);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3554+4, 1, 'Burning Steppes - Draco''dar - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (21427, 3554+4, 0, 'Burning Steppes - Draco''dar - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21418, 3554+4, 0, 'Burning Steppes - Draco''dar - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21423, 3554+4, 0, 'Burning Steppes - Draco''dar - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21426, 3554+4, 0, 'Burning Steppes - Draco''dar - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21428, 3554+4, 0, 'Burning Steppes - Draco''dar - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0);

-- Pillar of Ash
 DELETE FROM `pool_pool` WHERE `pool_id` IN (21417, 21425, 21420, 21419, 21421);

UPDATE `pool_template` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes - Pillar of Ash' WHERE `entry` IN (21417, 21425, 21420, 21419, 21421);

UPDATE `pool_gameobject` SET `description` = 'Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein - Burning Steppes - Pillar of Ash' WHERE `pool_entry` IN (21417, 21425, 21420, 21419, 21421);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3554+5, 1, 'Burning Steppes - Pillar of Ash - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (21417, 3554+5, 0, 'Burning Steppes - Pillar of Ash - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21425, 3554+5, 0, 'Burning Steppes - Pillar of Ash - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21420, 3554+5, 0, 'Burning Steppes - Pillar of Ash - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21419, 3554+5, 0, 'Burning Steppes - Pillar of Ash - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0), (21421, 3554+5, 0, 'Burning Steppes - Pillar of Ash - Truesilver Deposit / Dark Iron Deposit / Rich Thorium Vein', 0);

DELETE FROM `pool_template` WHERE `entry` = 1297;

-- End of migration.
;
