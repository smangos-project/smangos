INSERT INTO `migrations` VALUES ('20211204182213');

-- Add your query below.
 -- Remove Sleep Immunity From Sunken Temple Dragonkin
 UPDATE `creature_template` SET `mechanic_immune_mask` = 0 WHERE `entry` IN (5283, 5277, 8319, 5280);

-- End of migration.
;
