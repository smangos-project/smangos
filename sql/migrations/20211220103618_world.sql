INSERT INTO `migrations` VALUES ('20211220103618');

-- Add your query below.
 -- Remove EventAI from Mottled Boar.
 DELETE FROM `creature_ai_scripts` WHERE `id` IN (309801);

DELETE FROM `creature_ai_events` WHERE `creature_id` =3098;

UPDATE `creature_template` SET `ai_name` = '' WHERE `entry` =3098;

-- Kodos Calf does not call for help.
 UPDATE `creature_template` SET `call_for_help_range` =0 WHERE `entry` =2972;

-- End of migration.
;
