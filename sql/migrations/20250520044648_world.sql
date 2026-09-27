INSERT INTO `migrations` VALUES ('20250520044648');

-- Add your query below.
 UPDATE `spell_template` SET `script_name` = 'spell_showlabel_off' WHERE `entry` =456 AND `build` =5464;

UPDATE `spell_template` SET `script_name` = 'spell_showlabel_on' WHERE `entry` =2765 AND `build` =5464;

UPDATE `spell_template` SET `script_name` = 'spell_gm_off' WHERE `entry` =1509 AND `build` =5464;

UPDATE `spell_template` SET `script_name` = 'spell_gm_on' WHERE `entry` =18139 AND `build` =5464;

UPDATE `spell_template` SET `script_name` = 'spell_invis_off' WHERE `entry` =6147 AND `build` =5464;

UPDATE `spell_template` SET `script_name` = 'spell_invis_on' WHERE `entry` =2763 AND `build` =5464;

UPDATE `spell_template` SET `script_name` = 'spell_bm_off' WHERE `entry` =20114 AND `build` =5464;

UPDATE `spell_template` SET `script_name` = 'spell_bm_off' WHERE `entry` =24675;

UPDATE `spell_template` SET `script_name` = 'spell_bm_on' WHERE `entry` =20115 AND `build` =5464;

UPDATE `spell_template` SET `script_name` = 'spell_bm_on' WHERE `entry` =24676;

UPDATE `spell_template` SET `script_name` = 'spell_clear_all_cooldowns' WHERE `entry` =29313;

-- End of migration.
;
