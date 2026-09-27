INSERT INTO `migrations` VALUES ('20240413115356');

-- Add your query below.

-- Add build to primary key of table.
CREATE TABLE `spell_threat__new` (
  `entry` INTEGER NOT NULL DEFAULT 0,
  `Threat` REAL NOT NULL DEFAULT 0,
  `multiplier` REAL NOT NULL DEFAULT 1,
  `ap_bonus` REAL NOT NULL DEFAULT 0,
  `build_min` INTEGER NOT NULL DEFAULT 0,
  `build_max` INTEGER NOT NULL DEFAULT 5875,
  PRIMARY KEY (`entry`, `build_min`, `build_max`)
);

INSERT INTO `spell_threat__new` (`entry`, `Threat`, `multiplier`, `ap_bonus`, `build_min`, `build_max`)
SELECT `entry`, `Threat`, `multiplier`, `ap_bonus`, `build_min`, `build_max` FROM `spell_threat`;

DROP TABLE `spell_threat`;
ALTER TABLE `spell_threat__new` RENAME TO `spell_threat`;

/*
 World of Warcraft Client Patch 1.7.0 (2005-09-13)
 - Shield Slam - Threat caused increased.
*/
-- No data for previous threat values, so just diving them by 2.
-- Tooltip changed from mederate amount to high amount of threat.
DELETE FROM `spell_threat` WHERE `entry` IN (23922, 23923, 23924, 23925);
INSERT INTO `spell_threat` (`entry`, `Threat`, `multiplier`, `ap_bonus`, `build_min`, `build_max`) VALUES (23922, 89, 1, 0, 4544, 4544);
INSERT INTO `spell_threat` (`entry`, `Threat`, `multiplier`, `ap_bonus`, `build_min`, `build_max`) VALUES (23923, 101, 1, 0, 4544, 4544);
INSERT INTO `spell_threat` (`entry`, `Threat`, `multiplier`, `ap_bonus`, `build_min`, `build_max`) VALUES (23924, 114, 1, 0, 4544, 4544);
INSERT INTO `spell_threat` (`entry`, `Threat`, `multiplier`, `ap_bonus`, `build_min`, `build_max`) VALUES (23925, 127, 1, 0, 4544, 4544);
INSERT INTO `spell_threat` (`entry`, `Threat`, `multiplier`, `ap_bonus`, `build_min`, `build_max`) VALUES (23922, 178, 1, 0, 4695, 5875);
INSERT INTO `spell_threat` (`entry`, `Threat`, `multiplier`, `ap_bonus`, `build_min`, `build_max`) VALUES (23923, 203, 1, 0, 4695, 5875);
INSERT INTO `spell_threat` (`entry`, `Threat`, `multiplier`, `ap_bonus`, `build_min`, `build_max`) VALUES (23924, 229, 1, 0, 4695, 5875);
INSERT INTO `spell_threat` (`entry`, `Threat`, `multiplier`, `ap_bonus`, `build_min`, `build_max`) VALUES (23925, 254, 1, 0, 4695, 5875);


-- End of migration.
