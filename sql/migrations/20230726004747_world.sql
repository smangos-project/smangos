INSERT INTO `migrations` VALUES ('20230726004747');

-- Add your query below.
 -- Remove run script from Baron Geddon and assign extra flag.
 UPDATE `creature_movement` SET `script_id` =0 WHERE `id` =56655;

UPDATE `creature_template` SET `flags_extra` = (`flags_extra` | 33554432) WHERE `entry` =12056;

DELETE FROM `creature_movement_scripts` WHERE `id` =5;

-- End of migration.
;
