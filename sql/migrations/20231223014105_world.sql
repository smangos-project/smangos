INSERT INTO `migrations` VALUES ('20231223014105');

-- Add your query below.
 -- Remove unusued quest scripts.
 DELETE FROM `quest_start_scripts` WHERE `id` IN (945, 3447);

DELETE FROM `quest_end_scripts` WHERE `id` IN (2952, 4603, 4604, 5058);

-- End of migration.
;
