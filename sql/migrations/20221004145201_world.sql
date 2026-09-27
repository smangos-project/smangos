INSERT INTO `migrations` VALUES ('20221004145201');

-- Add your query below.
 -- Correct PPM of Jeklik's Crusher.
 -- https://old.reddit.com/r/classicwow/comments/gxsawo/jekliks_crusher_stats_2024_chanceonhit_4ppm_its/
 UPDATE `item_template` SET `spellppmrate_1` =4 WHERE `entry` =19918;

-- End of migration.
;
