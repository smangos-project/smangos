INSERT INTO `migrations` VALUES ('20221110200814');

-- Add your query below.
 -- Mailbox at bloodvenom post doesn't exist in vanilla
 DELETE FROM `gameobject` WHERE `guid` =48827;

-- Duplicate campfire  gameobject
 DELETE FROM `gameobject` WHERE `guid` =48822;

-- End of migration.
;
