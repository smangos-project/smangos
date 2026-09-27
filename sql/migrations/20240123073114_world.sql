INSERT INTO `migrations` VALUES ('20240123073114');

-- Add your query below.
 -- Remove help for non existent commands.
 DELETE FROM `command` WHERE `name` = 'wareffortget';

DELETE FROM `command` WHERE `name` = 'wareffortset';

-- End of migration.
;
