INSERT INTO `migrations` VALUES ('20260718130001');

DELETE FROM `pool_template` WHERE `entry` = 1252;

DELETE FROM `pool_gameobject` WHERE `pool_entry` = 1252;

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (1145+1, 1, 'Fadeleaf - Hinterlands - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (28974, 1145+1, 'Fadeleaf - Hinterlands - Group 1'), (28975, 1145+1, 'Fadeleaf - Hinterlands - Group 1'), (28978, 1145+1, 'Fadeleaf - Hinterlands - Group 1'), (28979, 1145+1, 'Fadeleaf - Hinterlands - Group 1'), (34959, 1145+1, 'Fadeleaf - Hinterlands - Group 1');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (1145+2, 1, 'Fadeleaf - Hinterlands - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (28972, 1145+2, 'Fadeleaf - Hinterlands - Group 2'), (28973, 1145+2, 'Fadeleaf - Hinterlands - Group 2'), (28976, 1145+2, 'Fadeleaf - Hinterlands - Group 2'), (28977, 1145+2, 'Fadeleaf - Hinterlands - Group 2'), (46399, 1145+2, 'Fadeleaf - Hinterlands - Group 2');

UPDATE `pool_gameobject` SET `pool_entry` = 1071 WHERE `guid` = 33201;

DELETE FROM `gameobject` WHERE `guid` = 7660;

DELETE FROM `pool_template` WHERE `entry` = 1143;

DELETE FROM `pool_gameobject` WHERE `pool_entry` = 1143;

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4979+1, 1, 'Gromsblood - Desolace - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (16523, 4979+1, 'Gromsblood - Desolace - Group 1'), (16544, 4979+1, 'Gromsblood - Desolace - Group 1'), (42137, 4979+1, 'Gromsblood - Desolace - Group 1'), (42138, 4979+1, 'Gromsblood - Desolace - Group 1'), (42141, 4979+1, 'Gromsblood - Desolace - Group 1');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4979+2, 1, 'Gromsblood - Desolace - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (16537, 4979+2, 'Gromsblood - Desolace - Group 2'), (16539, 4979+2, 'Gromsblood - Desolace - Group 2'), (42139, 4979+2, 'Gromsblood - Desolace - Group 2'), (42143, 4979+2, 'Gromsblood - Desolace - Group 2'), (42146, 4979+2, 'Gromsblood - Desolace - Group 2');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4979+3, 1, 'Gromsblood - Desolace - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (16458, 4979+3, 'Gromsblood - Desolace - Group 3'), (42140, 4979+3, 'Gromsblood - Desolace - Group 3'), (42142, 4979+3, 'Gromsblood - Desolace - Group 3'), (42144, 4979+3, 'Gromsblood - Desolace - Group 3'), (42145, 4979+3, 'Gromsblood - Desolace - Group 3');

DELETE FROM `gameobject` WHERE `guid` IN (16533, 16464);

DELETE FROM `pool_template` WHERE `entry` = 107;

DELETE FROM `pool_gameobject` WHERE `pool_entry` = 107;

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4982+1, 1, 'Grave Moss - Desolace - Group 1 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (18680, 4982+1, 'Grave Moss - Desolace - Group 1'), (29137, 4982+1, 'Grave Moss - Desolace - Group 1'), (29142, 4982+1, 'Grave Moss - Desolace - Group 1'), (29145, 4982+1, 'Grave Moss - Desolace - Group 1'), (68708, 4982+1, 'Grave Moss - Desolace - Group 1');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4982+2, 1, 'Grave Moss - Desolace - Group 2 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (18676, 4982+2, 'Grave Moss - Desolace - Group 2'), (29135, 4982+2, 'Grave Moss - Desolace - Group 2'), (29136, 4982+2, 'Grave Moss - Desolace - Group 2'), (29140, 4982+2, 'Grave Moss - Desolace - Group 2'), (29141, 4982+2, 'Grave Moss - Desolace - Group 2');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4982+3, 1, 'Grave Moss - Desolace - Group 3 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (1159, 4982+3, 'Grave Moss - Desolace - Group 3'), (18677, 4982+3, 'Grave Moss - Desolace - Group 3'), (29143, 4982+3, 'Grave Moss - Desolace - Group 3'), (29144, 4982+3, 'Grave Moss - Desolace - Group 3'), (68709, 4982+3, 'Grave Moss - Desolace - Group 3');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4982+4, 1, 'Grave Moss - Desolace - Group 4 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (4507, 4982+4, 'Grave Moss - Desolace - Group 4'), (29134, 4982+4, 'Grave Moss - Desolace - Group 4'), (29138, 4982+4, 'Grave Moss - Desolace - Group 4'), (29147, 4982+4, 'Grave Moss - Desolace - Group 4'), (29149, 4982+4, 'Grave Moss - Desolace - Group 4');

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (4982+5, 1, 'Grave Moss - Desolace - Group 5 - Master', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `description`) VALUES (4437, 4982+5, 'Grave Moss - Desolace - Group 5'), (4495, 4982+5, 'Grave Moss - Desolace - Group 5'), (29139, 4982+5, 'Grave Moss - Desolace - Group 5'), (29146, 4982+5, 'Grave Moss - Desolace - Group 5'), (29148, 4982+5, 'Grave Moss - Desolace - Group 5');

-- End of migration.
;
