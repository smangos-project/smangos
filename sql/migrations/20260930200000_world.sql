INSERT INTO `migrations` VALUES ('20260930200000');

CREATE TABLE `skill_line_ability_extra` (
  `id` INTEGER NOT NULL PRIMARY KEY,
  `superseded_by_spell` INTEGER NOT NULL DEFAULT 0
);

INSERT INTO `skill_line_ability_extra` (`id`, `superseded_by_spell`) VALUES
(935, 1528),
(936, 2997),
(3426, 7377),
(3427, 6184),
(6114, 7390),
(6255, 8974),
(6500, 19998);

UPDATE `dbc_skilllineability` SET `MinSkillLineRank` = 65 WHERE `ID` = 1635;
UPDATE `dbc_skilllineability` SET `MinSkillLineRank` = 30 WHERE `ID` = 1646;
UPDATE `dbc_skilllineability` SET `MinSkillLineRank` = 35 WHERE `ID` = 1651;
UPDATE `dbc_skilllineability` SET `ClassMask` = 7 WHERE `ID` = 2928;
UPDATE `dbc_skilllineability` SET `TrivialSkillLineRankHigh` = 0, `TrivialSkillLineRankLow` = 0 WHERE `ID` = 8676;
UPDATE `dbc_skilllineability` SET `SupercedesSpell` = 16689 WHERE `ID` = 9876;
UPDATE `dbc_skilllineability` SET `AcquireMethod` = 0 WHERE `ID` = 12416;
UPDATE `dbc_skilllineability` SET `AcquireMethod` = 0 WHERE `ID` = 12417;
UPDATE `dbc_skilllineability` SET `AcquireMethod` = 0 WHERE `ID` = 12419;
UPDATE `dbc_skilllineability` SET `AcquireMethod` = 0 WHERE `ID` = 12452;
UPDATE `dbc_skilllineability` SET `AcquireMethod` = 0 WHERE `ID` = 12453;

INSERT INTO `dbc_skilllineability` (`ID`, `SkillLine`, `Spell`, `RaceMask`, `ClassMask`, `MinSkillLineRank`, `SupercedesSpell`, `AcquireMethod`, `TrivialSkillLineRankHigh`, `TrivialSkillLineRankLow`, `CharacterPoints2`) VALUES
(2503, 142, 818, 0, 0, 1, 0, 0, 75, 25, 0),
(12576, 183, 2479, 0, 0, 1, 0, 2, 0, 0, 0),
(15029, 762, 33388, 0, 0, 1, 0, 0, 0, 0, 0),
(15030, 762, 33391, 0, 0, 1, 0, 0, 0, 0, 0);

DROP TABLE `skill_line_ability`;
