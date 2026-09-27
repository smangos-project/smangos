INSERT INTO `migrations` VALUES ('20221217024824');

-- Add your query below.
 -- Correct drop chance of Turtle Meat from Snapjaw.
 UPDATE `creature_loot_template` SET `ChanceOrQuestChance` =30 WHERE `item` =3712 AND `entry` =2408;

-- Flesh Eating Worm should not drop anything.
 DELETE FROM `creature_loot_template` WHERE `entry` =2462;

UPDATE `creature_template` SET `loot_id` =0 WHERE `entry` =2462;

-- Correct drop chance of Aged Gorilla Sinew.
 UPDATE `creature_loot_template` SET `ChanceOrQuestChance` =-1.6 WHERE `item` =3862;

-- End of migration.
;
