INSERT INTO `migrations` VALUES ('20240123072538');

-- Add your query below.
 -- Fix unit flags of battleground spirit guides.
 UPDATE `creature_template` SET `unit_flags` =832 WHERE `entry` IN (13116, 13117);

-- End of migration.
;
