INSERT INTO `migrations` VALUES ('20240105021338');

-- Add your query below.
 -- The Lobotomizer should not drop from Grunnda Wolfheart.
 DELETE FROM `creature_loot_template` WHERE `item` =19324;

-- End of migration.
;
