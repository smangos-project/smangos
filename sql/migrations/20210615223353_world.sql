INSERT INTO `migrations` VALUES ('20210615223353');

-- Add your query below.

-- Ratchet and Booty Bay
UPDATE `transports` SET `period`=350822 WHERE `entry`=20808 AND `guid`=1;
-- Teldrassil and Auberdine
UPDATE `transports` SET `period`=316253 WHERE `entry`=176244 AND `guid`=2;
-- Menethil Harbor and Theramore Isle
UPDATE `transports` SET `period`=329315 WHERE `entry`=176231 AND `guid`=3;
-- Menethil Harbor and Auberdine
UPDATE `transports` SET `period`=295580 WHERE `entry`=176310 AND `guid`=4;
-- Forgotton Coast and Feathermoon Stronghold
UPDATE `transports` SET `period`=317044 WHERE `entry`=177233 AND `guid`=5;
-- Orgrimmar and Undercity
UPDATE `transports` SET `period`=356287 WHERE `entry`=164871 AND `guid`=6;
-- Grom'Gol Base Camp and Orgrimmar
UPDATE `transports` SET `period`=303466 WHERE `entry`=175080 AND `guid`=7;
-- Grom'Gol Base Camp and Undercity
UPDATE `transports` SET `period`=335297 WHERE `entry`=176495 AND `guid`=8;

-- Remove guid from table. A trailing DROP COLUMN needs no table rebuild (world-sql.md).
ALTER TABLE `transports` DROP COLUMN `guid`;

-- Correct summon range of Eye of Kilrog.
UPDATE `spell_effect_mod` SET `effectRadiusIndex`=7 WHERE `Id` IN (126, 11403);

-- End of migration.
