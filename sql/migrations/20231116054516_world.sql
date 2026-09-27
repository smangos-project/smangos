INSERT INTO `migrations` VALUES ('20231116054516');

-- Add your query below.

CREATE TABLE `spell_proc_event__new` (
  `entry` INTEGER NOT NULL DEFAULT 0,
  `SchoolMask` INTEGER NOT NULL DEFAULT 0,
  `SpellFamilyName` INTEGER NOT NULL DEFAULT 0,
  `SpellFamilyMask0` INTEGER NOT NULL DEFAULT 0,
  `SpellFamilyMask1` INTEGER NOT NULL DEFAULT 0,
  `SpellFamilyMask2` INTEGER NOT NULL DEFAULT 0,
  `procFlags` INTEGER NOT NULL DEFAULT 0,
  `procEx` INTEGER NOT NULL DEFAULT 0,
  `ppmRate` REAL NOT NULL DEFAULT 0,
  `CustomChance` REAL NOT NULL DEFAULT 0,
  `Cooldown` INTEGER NOT NULL DEFAULT 0,
  `build_min` INTEGER NOT NULL DEFAULT 0,
  `build_max` INTEGER NOT NULL DEFAULT 5875,
  PRIMARY KEY (`entry`, `build_min`, `build_max`)
);

INSERT INTO `spell_proc_event__new` (`entry`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `procFlags`, `procEx`, `ppmRate`, `CustomChance`, `Cooldown`, `build_min`, `build_max`)
SELECT `entry`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `procFlags`, `procEx`, `ppmRate`, `CustomChance`, `Cooldown`, `build_min`, `build_max` FROM `spell_proc_event`;

DROP TABLE `spell_proc_event`;
ALTER TABLE `spell_proc_event__new` RENAME TO `spell_proc_event`;

/*
World of Warcraft Client Patch 1.10.0 (2006-03-28)
- If a paladin avoids damage with Divine Shield, Eye for an Eye will
  not react.
*/
-- Eye for an Eye should proc from absorbed crits too.
DELETE FROM `spell_proc_event` WHERE `entry`=9799;
INSERT INTO `spell_proc_event` (`entry`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `procFlags`, `procEx`, `ppmRate`, `CustomChance`, `Cooldown`, `build_min`, `build_max`) VALUES (9799, 0, 0, 0, 0, 0, 131072, 1282, 0, 0, 0, 5086, 5086);
INSERT INTO `spell_proc_event` (`entry`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `procFlags`, `procEx`, `ppmRate`, `CustomChance`, `Cooldown`, `build_min`, `build_max`) VALUES (9799, 0, 0, 0, 0, 0, 0, 1026, 0, 0, 0, 5302, 5875);


-- End of migration.
