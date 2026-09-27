INSERT INTO `migrations` VALUES ('20250420193928');

-- Add your query below.
 -- Delete TBC items
 DELETE FROM `item_template` WHERE `entry` IN (20470, 20472, 20474, 20482, 20483);

-- End of migration.
;
