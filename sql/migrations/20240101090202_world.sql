INSERT INTO `migrations` VALUES ('20240101090202');

-- Add your query below.
 -- Fix unit flags of Marshal Reginald Windsor.
 UPDATE `creature_template` SET `unit_flags` =320 WHERE `entry` IN (9023, 9682);

-- End of migration.
;
