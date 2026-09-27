INSERT INTO `migrations` VALUES ('20250521050317');

-- Add your query below.
 -- Fix wrong type and data for Sapphiron's Ice Block.
 UPDATE `gameobject_template` SET `type` =0, `flags` =32, `faction` =114, `data2` =196608 WHERE `entry` =181247;

-- End of migration.
;
