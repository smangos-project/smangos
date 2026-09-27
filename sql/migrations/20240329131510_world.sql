INSERT INTO `migrations` VALUES ('20240329131510');

-- Add your query below.
 -- Set QUEST_METHOD_DISABLED flag for 2358 Horns of Nez'ra
 UPDATE `quest_template` SET `Method` =1 WHERE `entry` =2358 AND `patch` =0;

-- End of migration.
;
