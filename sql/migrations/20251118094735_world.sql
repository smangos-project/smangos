INSERT INTO `migrations` VALUES ('20251118094735');

-- Add your query below.
 -- Remove leftover locale for deleted gameobject.
 DELETE FROM `locales_gameobject` WHERE `entry` =182509;

-- End of migration.
;
