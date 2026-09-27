INSERT INTO `migrations` VALUES ('20211221002727');

-- Add your query below.
 -- Superior Mana Draught and Superior Healing Draught do not have a reputation requirement on the Arathi Basin vendors.
 DELETE FROM `conditions` WHERE `condition_entry` IN (174, 179);

UPDATE `npc_vendor` SET `condition_id` =0 WHERE `item` IN (17349, 17352) AND `entry` IN (15127, 15126);

-- End of migration.
;
