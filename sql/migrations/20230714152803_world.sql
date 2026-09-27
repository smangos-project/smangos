INSERT INTO `migrations` VALUES ('20230714152803');

DELETE FROM `pool_template` WHERE `entry` IN (SELECT `pool_entry` FROM `pool_gameobject` WHERE `guid` IN (40692, 40693));

DELETE FROM `gameobject` WHERE `guid` IN (40692, 40693);

DELETE FROM `pool_gameobject` WHERE `guid` IN (40692, 40693);

INSERT INTO `gameobject` (`guid`, `id`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecsmin`, `spawntimesecsmax`, `animprogress`, `state`, `spawn_flags`, `visibility_mod`, `patch_min`, `patch_max`) VALUES (884+1, 131978, 70, -73.5079, 265.315, -47.6291, 2.82743, 0, 0, 0.987688, 0.156434, 604800, 604800, 100, 1, 0, 0, 0, 10), (884+2, 131978, 70, -150.166, 201.764, -49.9121, 3.29869, 0, 0, -0.996917, 0.0784664, 604800, 604800, 100, 1, 0, 0, 0, 10), (884+3, 131979, 70, -73.5079, 265.315, -47.6291, 2.82743, 0, 0, 0.987688, 0.156434, 604800, 604800, 100, 1, 0, 0, 0, 10), (884+4, 131979, 70, -150.166, 201.764, -49.9121, 3.29869, 0, 0, -0.996917, 0.0784664, 604800, 604800, 100, 1, 0, 0, 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `patch_min`, `patch_max`) VALUES (3956+1, 1, 'Large Mithril Bound Chest / Large Darkwood Chest - Uldaman', 0, 10), (3956+2, 1, 'Large Mithril Bound Chest / Large Darkwood Chest - Uldaman', 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`, `patch_min`, `patch_max`) VALUES (884+1, 3956+1, 'Large Mithril Bound Chest / Large Darkwood Chest - Uldaman', 0, 10), (884+2, 3956+2, 'Large Mithril Bound Chest / Large Darkwood Chest - Uldaman', 0, 10), (884+3, 3956+1, 'Large Mithril Bound Chest / Large Darkwood Chest - Uldaman', 0, 10), (884+4, 3956+2, 'Large Mithril Bound Chest / Large Darkwood Chest - Uldaman', 0, 10);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (3956+3, 1, 'Uldaman - Large Mithril Bound Chest / Large Darkwood Chest (Master Pool)', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (3956+1, 3956+3, 0, 'Large Mithril Bound Chest / Large Darkwood Chest - Uldaman', 0), (3956+2, 3956+3, 0, 'Large Mithril Bound Chest / Large Darkwood Chest - Uldaman', 0);

-- End of migration.
;
