INSERT INTO `migrations` VALUES ('20220907080220');

-- Add your query below.
 -- Use correct gossip menu id for Frostwolf Stable Master.
 UPDATE `gossip_menu` SET `entry` =5261 WHERE `entry` =50002;

UPDATE `gossip_menu_option` SET `menu_id` =5261 WHERE `menu_id` =50002;

UPDATE `gossip_menu_option` SET `action_script_id` =5261 WHERE `action_script_id` =50002;

UPDATE `gossip_scripts` SET `id` =5261 WHERE `id` =50002;

UPDATE `creature_template` SET `gossip_menu_id` =5261 WHERE `gossip_menu_id` =50002;

-- End of migration.
;
