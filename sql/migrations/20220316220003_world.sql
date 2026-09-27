INSERT INTO `migrations` VALUES ('20220316220003');

-- Add your query below.
 INSERT INTO `conditions` VALUES (7715, 11, 30056, 1, 0, 0, 0);

UPDATE `gossip_menu` SET `condition_id` =7715 WHERE `entry` =6421 AND `text_id` =7714;

UPDATE `gossip_menu` SET `condition_id` =0 WHERE `entry` =6421 AND `text_id` =7614;

-- End of migration.
;
