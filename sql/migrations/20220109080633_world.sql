INSERT INTO `migrations` VALUES ('20220109080633');

-- Add your query below.
 -- Correct drop chance of Wavethrasher Scales.
 UPDATE `creature_loot_template` SET `ChanceOrQuestChance` =-20 WHERE `item` =20087 AND `entry` =6347;

UPDATE `creature_loot_template` SET `ChanceOrQuestChance` =-25 WHERE `item` =20087 AND `entry` =6348;

UPDATE `creature_loot_template` SET `ChanceOrQuestChance` =-10 WHERE `item` =20087 AND `entry` =6349;

-- End of migration.
;
