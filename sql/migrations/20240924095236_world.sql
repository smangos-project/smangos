INSERT INTO `migrations` VALUES ('20240924095236');

-- Add your query below.
 -- Correct condition for Chapter V gossip menu
 UPDATE `gossip_menu_option` SET `condition_id` = 150 WHERE `menu_id` = 6669;

-- Correct condition for Chapter VII gossip menu
 UPDATE `gossip_menu_option` SET `condition_id` = 151 WHERE `menu_id` = 6668;

-- End of migration.
;
