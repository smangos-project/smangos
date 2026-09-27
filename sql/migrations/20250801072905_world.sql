INSERT INTO `migrations` VALUES ('20250801072905');

-- Add your query below.
 -- Tanaris Field Samples should not yield results every time.
 -- https://www.youtube.com/watch?v=ePYbVBvd4Iw
 UPDATE `spell_template` SET `script_name` = 'spell_tanaris_field_sampling' WHERE `entry` IN (11759, 11760, 11761);

-- End of migration.
;
