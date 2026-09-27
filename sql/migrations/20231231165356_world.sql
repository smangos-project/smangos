INSERT INTO `migrations` VALUES ('20231231165356');

-- Add your query below.
 -- Move increased guard call for help range to db.
 UPDATE `creature_template` SET `call_for_help_range` =30 WHERE (`flags_extra` & 1024);

-- End of migration.
;
