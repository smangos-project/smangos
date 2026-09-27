INSERT INTO `migrations` VALUES ('20250408123201');

-- Add your query below.
 UPDATE `spell_template` SET `script_name` = 'spell_warlock_inferno' WHERE `entry` IN (1122, 24670);

UPDATE `spell_template` SET `script_name` = 'spell_thaddius_magnetic_pull' WHERE `entry` =28337;

UPDATE `spell_template` SET `script_name` = 'spell_maexxna_spider_web' WHERE `entry` =28434;

UPDATE `spell_template` SET `script_name` = 'spell_paladin_reckoning' WHERE `entry` =20178;

UPDATE `spell_template` SET `script_name` = 'spell_goblin_jumper_cables' WHERE `entry` =8342;

UPDATE `spell_template` SET `script_name` = 'spell_goblin_jumper_cables_xl' WHERE `entry` =22999;

-- End of migration.
;
