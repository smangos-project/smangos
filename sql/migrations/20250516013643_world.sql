INSERT INTO `migrations` VALUES ('20250516013643');

-- Add your query below.
 -- Delete custom creature Vam.
 DELETE FROM `creature_template` WHERE `entry` =15547;

DELETE FROM `creature_onkill_reputation` WHERE `creature_id` =15547;

-- End of migration.
;
