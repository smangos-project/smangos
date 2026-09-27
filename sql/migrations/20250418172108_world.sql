INSERT INTO `migrations` VALUES ('20250418172108');

-- Add your query below.
 -- Correct levelrange for Morbent Fel Pre 1.6
 UPDATE `creature_template` SET `level_max` =35 WHERE `entry` =1200 AND `patch` =0;

-- End of migration.
;
