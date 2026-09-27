INSERT INTO `migrations` VALUES ('20221117042239');

-- Add your query below.
 -- Fix unit flags of Darrowshire Poltergeist.
 UPDATE `creature_template` SET `faction` =35, `unit_flags` =33555208 WHERE `entry` =11296;

-- End of migration.
;
