INSERT INTO `migrations` VALUES ('20260930070000');

-- Vanilla spell durations, shapeshift forms and item enchantments where the client differs.
INSERT INTO `dbc_spellduration` (`ID`, `Duration`, `DurationPerLevel`, `MaxDuration`) VALUES (507, 1100, 0, 1100);
INSERT INTO `dbc_spellduration` (`ID`, `Duration`, `DurationPerLevel`, `MaxDuration`) VALUES (508, 1100, 0, 1100);
UPDATE `dbc_spellshapeshiftform` SET `Name_lang` = 'Tree Form', `Flags` = 16, `CreatureType` = -1 WHERE `ID` = 2;
UPDATE `dbc_spellshapeshiftform` SET `Name_lang` = 'Battle Stance', `Flags` = 7, `CreatureType` = -1 WHERE `ID` = 17;
UPDATE `dbc_spellshapeshiftform` SET `Name_lang` = '', `Flags` = 0, `CreatureType` = 0 WHERE `ID` = 22;
UPDATE `dbc_spellshapeshiftform` SET `Name_lang` = '', `Flags` = 0, `CreatureType` = 0 WHERE `ID` = 24;
UPDATE `dbc_spellshapeshiftform` SET `Name_lang` = 'Shadowform', `Flags` = 9, `CreatureType` = -1 WHERE `ID` = 28;
UPDATE `dbc_spellshapeshiftform` SET `Name_lang` = 'Moonkin Form', `Flags` = 65, `CreatureType` = -1 WHERE `ID` = 31;
UPDATE `dbc_spellshapeshiftform` SET `Name_lang` = 'Spirit of Redemption', `Flags` = 0, `CreatureType` = -1 WHERE `ID` = 32;
UPDATE `dbc_spellitemenchantment` SET `Effect1` = 1, `Effect2` = 0, `Effect3` = 0, `EffectPointsMin1` = 100, `EffectPointsMin2` = 0, `EffectPointsMin3` = 0, `EffectArg1` = 8029, `EffectArg2` = 0, `EffectArg3` = 0, `ItemVisual` = 32, `Flags` = 1 WHERE `ID` = 3;
UPDATE `dbc_spellitemenchantment` SET `Effect1` = 1, `Effect2` = 0, `Effect3` = 0, `EffectPointsMin1` = 100, `EffectPointsMin2` = 0, `EffectPointsMin3` = 0, `EffectArg1` = 8028, `EffectArg2` = 0, `EffectArg3` = 0, `ItemVisual` = 32, `Flags` = 1 WHERE `ID` = 4;
UPDATE `dbc_spellitemenchantment` SET `Effect1` = 1, `Effect2` = 0, `Effect3` = 0, `EffectPointsMin1` = 100, `EffectPointsMin2` = 0, `EffectPointsMin3` = 0, `EffectArg1` = 8026, `EffectArg2` = 0, `EffectArg3` = 0, `ItemVisual` = 32, `Flags` = 1 WHERE `ID` = 5;
UPDATE `dbc_spellitemenchantment` SET `Effect1` = 1, `Effect2` = 0, `Effect3` = 0, `EffectPointsMin1` = 15, `EffectPointsMin2` = 0, `EffectPointsMin3` = 0, `EffectArg1` = 705, `EffectArg2` = 0, `EffectArg3` = 0, `ItemVisual` = 1, `Flags` = 0 WHERE `ID` = 25;
UPDATE `dbc_spellitemenchantment` SET `Effect1` = 1, `Effect2` = 0, `Effect3` = 0, `EffectPointsMin1` = 10, `EffectPointsMin2` = 0, `EffectPointsMin3` = 0, `EffectArg1` = 205, `EffectArg2` = 0, `EffectArg3` = 0, `ItemVisual` = 27, `Flags` = 0 WHERE `ID` = 26;
UPDATE `dbc_spellitemenchantment` SET `Effect1` = 1, `Effect2` = 0, `Effect3` = 0, `EffectPointsMin1` = 20, `EffectPointsMin2` = 0, `EffectPointsMin3` = 0, `EffectArg1` = 8233, `EffectArg2` = 0, `EffectArg3` = 0, `ItemVisual` = 81, `Flags` = 1 WHERE `ID` = 283;
UPDATE `dbc_spellitemenchantment` SET `Effect1` = 1, `Effect2` = 0, `Effect3` = 0, `EffectPointsMin1` = 20, `EffectPointsMin2` = 0, `EffectPointsMin3` = 0, `EffectArg1` = 8236, `EffectArg2` = 0, `EffectArg3` = 0, `ItemVisual` = 81, `Flags` = 1 WHERE `ID` = 284;
UPDATE `dbc_spellitemenchantment` SET `Effect1` = 1, `Effect2` = 0, `Effect3` = 0, `EffectPointsMin1` = 100, `EffectPointsMin2` = 0, `EffectPointsMin3` = 0, `EffectArg1` = 10445, `EffectArg2` = 0, `EffectArg3` = 0, `ItemVisual` = 32, `Flags` = 1 WHERE `ID` = 523;
UPDATE `dbc_spellitemenchantment` SET `Effect1` = 1, `Effect2` = 0, `Effect3` = 0, `EffectPointsMin1` = 20, `EffectPointsMin2` = 0, `EffectPointsMin3` = 0, `EffectArg1` = 10484, `EffectArg2` = 0, `EffectArg3` = 0, `ItemVisual` = 81, `Flags` = 1 WHERE `ID` = 525;
UPDATE `dbc_spellitemenchantment` SET `Effect1` = 1, `Effect2` = 0, `Effect3` = 0, `EffectPointsMin1` = 100, `EffectPointsMin2` = 0, `EffectPointsMin3` = 0, `EffectArg1` = 16343, `EffectArg2` = 0, `EffectArg3` = 0, `ItemVisual` = 32, `Flags` = 1 WHERE `ID` = 1665;
UPDATE `dbc_spellitemenchantment` SET `Effect1` = 1, `Effect2` = 0, `Effect3` = 0, `EffectPointsMin1` = 100, `EffectPointsMin2` = 0, `EffectPointsMin3` = 0, `EffectArg1` = 16344, `EffectArg2` = 0, `EffectArg3` = 0, `ItemVisual` = 32, `Flags` = 1 WHERE `ID` = 1666;
UPDATE `dbc_spellitemenchantment` SET `Effect1` = 1, `Effect2` = 0, `Effect3` = 0, `EffectPointsMin1` = 20, `EffectPointsMin2` = 0, `EffectPointsMin3` = 0, `EffectArg1` = 16361, `EffectArg2` = 0, `EffectArg3` = 0, `ItemVisual` = 81, `Flags` = 1 WHERE `ID` = 1669;
UPDATE `dbc_spellitemenchantment` SET `Effect1` = 1, `Effect2` = 0, `Effect3` = 0, `EffectPointsMin1` = 0, `EffectPointsMin2` = 0, `EffectPointsMin3` = 0, `EffectArg1` = 20007, `EffectArg2` = 0, `EffectArg3` = 0, `ItemVisual` = 103, `Flags` = 0 WHERE `ID` = 1900;
UPDATE `dbc_spellitemenchantment` SET `Effect1` = 3, `Effect2` = 0, `Effect3` = 0, `EffectPointsMin1` = 0, `EffectPointsMin2` = 0, `EffectPointsMin3` = 0, `EffectArg1` = 9346, `EffectArg2` = 0, `EffectArg3` = 0, `ItemVisual` = -1, `Flags` = 1 WHERE `ID` = 2605;
