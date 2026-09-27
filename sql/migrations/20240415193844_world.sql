INSERT INTO `migrations` VALUES ('20240415193844');

-- Add your query below.
 -- Fix BWL Razorgore eggs.
 UPDATE `gameobject_template` SET `flags` =4 WHERE `entry` =177807;

-- End of migration.
;
