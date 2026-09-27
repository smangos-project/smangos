INSERT INTO `migrations` VALUES ('20260529030008');

-- Add your query below.
 -- Proof: https://www.youtube.com/watch?v=xkbKOzwAihw&t=226
 -- At 3:47, Firelord in background is pulled, and the other nearby Firelord does not aggro.
 DELETE FROM `creature_groups` WHERE `leader_guid` = 56800;

-- End of migration.
;
