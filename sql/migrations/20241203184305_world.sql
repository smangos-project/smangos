INSERT INTO `migrations` VALUES ('20241203184305');

-- Add your query below.
 -- Fixed an exploit that allowed you to summon infinite Crimson Hammersmiths from the Blacksmithing Plans in Stratholme.
 DELETE FROM `gameobject_scripts` WHERE `id` =42888;

-- End of migration.
;
