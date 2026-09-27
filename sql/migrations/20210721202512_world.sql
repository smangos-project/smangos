INSERT INTO `migrations` VALUES ('20210721202512');

-- Add your query below.
 -- Delete custom AV creatures.
 DELETE FROM `creature` WHERE `id` IN (20110, 20111, 20112);

DELETE FROM `creature_template` WHERE `entry` IN (20110, 20111, 20112);

-- End of migration.
;
