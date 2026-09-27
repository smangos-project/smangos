INSERT INTO `migrations` VALUES ('20251117221718');

-- Add your query below.
 -- 3984: Target Is Friendly Or Better With Faction 749
 INSERT INTO `conditions` (`condition_entry`, `type`, `value1`, `value2`, `value3`, `value4`, `flags`) VALUES (3984, 5, 749, 4, 0, 0, 0);

-- Add missing Hydraxian Honor Guard gossips at Friendly and Honored.
 INSERT INTO `gossip_menu` (`entry`, `text_id`, `script_id`, `condition_id`) VALUES (5109, 6162, 0, 3984);

INSERT INTO `gossip_menu` (`entry`, `text_id`, `script_id`, `condition_id`) VALUES (5109, 6163, 0, 4203);

-- End of migration.
;
