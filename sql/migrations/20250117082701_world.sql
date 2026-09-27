INSERT INTO `migrations` VALUES ('20250117082701');

-- Add your query below.
 -- Add Lord Valthalak's Amulet to player upon accepting Mea Culpa, Lord Valthalak.
 DELETE FROM `quest_start_scripts` WHERE `id` =8995;

INSERT INTO `quest_start_scripts` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (8995, 0, 0, 17, 22048, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Mea Culpa, Lord Valthalak: Player - Create Item Lord Valthalak''s Amulet');

UPDATE `quest_template` SET `StartScript` =8995 WHERE `entry` =8995;

-- End of migration.
;
