INSERT INTO `migrations` VALUES ('20220804091054');

-- Add your query below.
 -- Add missing Suicide aura to Compact Harvest Reaper.
 UPDATE `creature_template` SET `auras` = '8328' WHERE `entry` =2676;

-- End of migration.
;
