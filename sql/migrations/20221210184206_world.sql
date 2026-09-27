INSERT INTO `migrations` VALUES ('20221210184206');

-- Add your query below.
 -- Correct drop chance of Scarlet Belt.
 UPDATE `creature_loot_template` SET `ChanceOrQuestChance` =3 WHERE `item` =10329 AND `entry` =4298;

UPDATE `creature_loot_template` SET `ChanceOrQuestChance` =2 WHERE `item` =10329 AND `entry` =4295;

-- End of migration.
;
