INSERT INTO `migrations` VALUES ('20260324084200');

DELETE FROM `pool_pool` WHERE `pool_id` IN (6195, 6194, 6192, 6200);

UPDATE `pool_template` SET `description` = 'Incendicite Mineral Vein - Wetlands - Group 1' WHERE `entry` IN (6195, 6194, 6192, 6200);

UPDATE `pool_gameobject` SET `description` = 'Incendicite Mineral Vein - Wetlands - Group 1' WHERE `pool_entry` IN (6195, 6194, 6192, 6200);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4344+1, 1, 'Incendicite Mineral Vein - Wetlands - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6195, 4344+1, 0, 'Wetlands - Group 1 - Incendicite Mineral Vein', 0), (6194, 4344+1, 0, 'Wetlands - Group 1 - Incendicite Mineral Vein', 0), (6192, 4344+1, 0, 'Wetlands - Group 1 - Incendicite Mineral Vein', 0), (6200, 4344+1, 0, 'Wetlands - Group 1 - Incendicite Mineral Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (6202, 6196, 6197, 6201);

UPDATE `pool_template` SET `description` = 'Incendicite Mineral Vein - Wetlands - Group 2' WHERE `entry` IN (6202, 6196, 6197, 6201);

UPDATE `pool_gameobject` SET `description` = 'Incendicite Mineral Vein - Wetlands - Group 2' WHERE `pool_entry` IN (6202, 6196, 6197, 6201);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4344+2, 1, 'Incendicite Mineral Vein - Wetlands - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6202, 4344+2, 0, 'Wetlands - Group 2 - Incendicite Mineral Vein', 0), (6196, 4344+2, 0, 'Wetlands - Group 2 - Incendicite Mineral Vein', 0), (6197, 4344+2, 0, 'Wetlands - Group 2 - Incendicite Mineral Vein', 0), (6201, 4344+2, 0, 'Wetlands - Group 2 - Incendicite Mineral Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (6203, 6205, 1615, 6198);

UPDATE `pool_template` SET `description` = 'Incendicite Mineral Vein - Wetlands - Group 3' WHERE `entry` IN (6203, 6205, 1615, 6198);

UPDATE `pool_gameobject` SET `description` = 'Incendicite Mineral Vein - Wetlands - Group 3' WHERE `pool_entry` IN (6203, 6205, 1615, 6198);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4344+3, 1, 'Incendicite Mineral Vein - Wetlands - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6203, 4344+3, 0, 'Wetlands - Group 3 - Incendicite Mineral Vein', 0), (6205, 4344+3, 0, 'Wetlands - Group 3 - Incendicite Mineral Vein', 0), (1615, 4344+3, 0, 'Wetlands - Group 3 - Incendicite Mineral Vein', 0), (6198, 4344+3, 0, 'Wetlands - Group 3 - Incendicite Mineral Vein', 0);

DELETE FROM `pool_pool` WHERE `pool_id` IN (6199, 6206, 6204, 6193);

UPDATE `pool_template` SET `description` = 'Incendicite Mineral Vein - Wetlands - Group 4' WHERE `entry` IN (6199, 6206, 6204, 6193);

UPDATE `pool_gameobject` SET `description` = 'Incendicite Mineral Vein - Wetlands - Group 4' WHERE `pool_entry` IN (6199, 6206, 6204, 6193);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4344+4, 1, 'Incendicite Mineral Vein - Wetlands - Group 4 - Master', 0, 0, 0, 10);

INSERT INTO `pool_pool` (`pool_id`, `mother_pool`, `chance`, `description`, `flags`) VALUES (6199, 4344+4, 0, 'Wetlands - Group 4 - Incendicite Mineral Vein', 0), (6206, 4344+4, 0, 'Wetlands - Group 4 - Incendicite Mineral Vein', 0), (6204, 4344+4, 0, 'Wetlands - Group 4 - Incendicite Mineral Vein', 0), (6193, 4344+4, 0, 'Wetlands - Group 4 - Incendicite Mineral Vein', 0);

DELETE FROM `pool_template` WHERE `entry` = 3257;

-- End of migration.
;
