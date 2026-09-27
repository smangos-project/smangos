INSERT INTO `migrations` VALUES ('20221104210724');

-- Add your query below.
 -- Correct damage and auras for Explosive Sheep.
 UPDATE `creature_template` SET `damage_multiplier` =1, `auras` = '4051 8327' WHERE `entry` =2675;

-- End of migration.
;
