INSERT INTO `migrations` VALUES ('20251227092730');

-- Add your query below.
 UPDATE `spell_template` SET `script_name` = 'spell_nefarian_class_call_warlock' WHERE `entry` = 23427;

UPDATE `spell_template` SET `script_name` = 'spell_nefarian_class_call_rogue' WHERE `entry` = 23414;

UPDATE `spell_template` SET `script_name` = 'spell_nefarian_class_call_mage' WHERE `entry` = 23410;

-- End of migration.
;
