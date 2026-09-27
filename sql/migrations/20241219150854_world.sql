INSERT INTO `migrations` VALUES ('20241219150854');

-- Add your query below.
 -- Remove Double Mail Vest from Mithril Lockbox.
 DELETE FROM `reference_loot_template` WHERE `entry` =2061;

DELETE FROM `item_loot_template` WHERE `item` =2061;

-- End of migration.
;
