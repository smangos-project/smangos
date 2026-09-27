INSERT INTO `migrations` VALUES ('20260514000111');

-- Add your query below.

CREATE TABLE `spell_threat__new` (
  `entry` INTEGER NOT NULL DEFAULT 0,
  `threat` REAL NOT NULL DEFAULT 0,
  `multiplier` REAL NOT NULL DEFAULT 1,
  `inverse_effect_mask` INTEGER NOT NULL DEFAULT 0,
  `build_min` INTEGER NOT NULL DEFAULT 0,
  `build_max` INTEGER NOT NULL DEFAULT 5875,
  PRIMARY KEY (`entry`, `build_min`, `build_max`)
);

INSERT INTO `spell_threat__new` (`entry`, `threat`, `multiplier`, `build_min`, `build_max`)
SELECT `entry`, `Threat`, `multiplier`, `build_min`, `build_max` FROM `spell_threat`;

DROP TABLE `spell_threat`;
ALTER TABLE `spell_threat__new` RENAME TO `spell_threat`;

UPDATE `spell_threat` SET `inverse_effect_mask`=1 WHERE `entry`=126;
UPDATE `spell_threat` SET `inverse_effect_mask`=1 WHERE `entry`=6196;
UPDATE `spell_threat` SET `inverse_effect_mask`=2 WHERE `entry`=22812;


-- End of migration.
