INSERT INTO `migrations` VALUES ('20240617091421');

-- Add your query below.
 -- Remove skinning loot from Mother Smolderweb.
 UPDATE `creature_template` SET `skinning_loot_id` =0 WHERE `entry` =10596;

-- End of migration.
;
