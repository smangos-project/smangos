INSERT INTO `migrations` VALUES ('20240123071946');

-- Add your query below.
 -- Fix wrong Z coordinate of Vilebranch mob.
 UPDATE `creature` SET `position_z` =230.717 WHERE `guid` =92963;

-- End of migration.
;
