INSERT INTO `migrations` VALUES ('20250519095233');

-- Add your query below.
 -- Remove triggered flag from goblin bombs.
 UPDATE `item_template` SET `extra_flags` =0 WHERE `entry` IN (4403, 10577);

UPDATE `spell_template` SET `script_name` = 'spell_instant_cast' WHERE `entry` IN (4067, 13237);

-- End of migration.
;
