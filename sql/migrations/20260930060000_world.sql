INSERT INTO `migrations` VALUES ('20260930060000');

-- Display ids that gameobject_template uses and the client lacks.
INSERT INTO `dbc_gameobjectdisplayinfo` (`ID`, `ModelName`) VALUES
(179, 'WORLD\GENERIC\HUMAN\PASSIVE DOODADS\SIGNS\TAILOR01.MDL'),
(233, 'World\SkillActivated\TradeskillEnablers\Tradeskill_Forge_01.mdx'),
(350, 'WORLD\GENERIC\ORC\PASSIVE DOODADS\BRAZIERS\MEDIUMBRAZIER01.MDL'),
(351, 'WORLD\GENERIC\ORC\PASSIVE DOODADS\BRAZIERS\SMALLBRAZIER01.MDL'),
(380, 'WORLD\GENERIC\HUMAN\PASSIVE DOODADS\CHAIRS\GENERALCHAIRLOEND01.MDL'),
(622, 'WORLD\GENERIC\HUMAN\PASSIVE DOODADS\BENCHES\STORMWINDBENCH01.MDX'),
(6615, 'World\Scale\50x50.mdx');

-- Riding skill.
INSERT INTO `dbc_skillline` (`ID`, `CategoryID`, `DisplayName_lang`) VALUES (762, 9, 'Riding');
INSERT INTO `dbc_skillraceclassinfo` (`ID`, `SkillID`, `RaceMask`, `ClassMask`, `Flags`, `MinLevel`, `SkillTierID`) VALUES (890, 762, 511, 1503, 16, 40, 223);

-- The client adds a second key (SoD item) to this lock.
UPDATE `dbc_lock` SET `Index2` = 0, `Type2` = 0 WHERE `ID` = 1647;

-- Player::UpdateSkillsForLevel keeps these skills at their maximum through SKILL_FLAG_MONO_VALUE.
UPDATE `dbc_skillraceclassinfo` SET `Flags` = `Flags` | 0x400
WHERE `ID` IN (5, 24, 25, 26, 31, 32, 33, 39, 54, 55, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 68, 69, 70, 71, 72, 73, 74, 75, 79, 81, 82, 87, 89, 92, 94, 95, 96, 97, 98, 101, 102, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 131, 132, 149, 301, 441, 442, 443, 445, 446, 461, 463, 464, 481, 501, 521, 522, 541, 544, 581, 781, 841, 861, 862);
