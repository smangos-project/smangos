INSERT INTO `migrations` VALUES ('20251031173104');

-- Add your query below.
 -- Remove wrong text said at the end of Skirmish at Echo Ridge.
 -- This text should be said upon accepting Report to Goldshire.
 -- Proof: https://www.youtube.com/watch?v=SH3HhIsDZ4k&t=373s
 DELETE FROM `quest_end_scripts` WHERE `id` =21;

UPDATE `quest_template` SET `CompleteScript` =0 WHERE `entry` =21;

-- End of migration.
;
