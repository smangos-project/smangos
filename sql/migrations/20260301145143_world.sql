INSERT INTO `migrations` VALUES ('20260301145143');

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Felfire Hill
 DELETE FROM `pool_gameobject` WHERE `guid` IN (16511, 42136, 42132, 42133, 68720, 68719);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (1763+1, 1, 'Ashenvale - Felfire Hill - Gromsblood', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (16511, 1763+1, 0, 'Gromsblood - Ashenvale - Felfire Hill', 0, 0, 10), (42136, 1763+1, 0, 'Gromsblood - Ashenvale - Felfire Hill', 0, 0, 10), (42132, 1763+1, 0, 'Gromsblood - Ashenvale - Felfire Hill', 0, 0, 10), (42133, 1763+1, 0, 'Gromsblood - Ashenvale - Felfire Hill', 0, 0, 10), (68720, 1763+1, 0, 'Gromsblood - Ashenvale - Felfire Hill', 0, 0, 10), (68719, 1763+1, 0, 'Gromsblood - Ashenvale - Felfire Hill', 0, 0, 10);

-- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 -- Demon Fall Ridge
 DELETE FROM `pool_gameobject` WHERE `guid` IN (625, 630, 631, 42134, 42135, 47871);

INSERT INTO `pool_template` (`entry`, `max_limit`, `description`, `flags`, `instance`, `patch_min`, `patch_max`) VALUES (1763+2, 1, 'Ashenvale - Demon Fall Ridge - Gromsblood', 0, 0, 0, 10);

INSERT INTO `pool_gameobject` (`guid`, `pool_entry`, `chance`, `description`, `flags`, `patch_min`, `patch_max`) VALUES (625, 1763+2, 0, 'Gromsblood - Ashenvale - Demon Fall Ridge', 0, 0, 10), (630, 1763+2, 0, 'Gromsblood - Ashenvale - Demon Fall Ridge', 0, 0, 10), (631, 1763+2, 0, 'Gromsblood - Ashenvale - Demon Fall Ridge', 0, 0, 10), (42134, 1763+2, 0, 'Gromsblood - Ashenvale - Demon Fall Ridge', 0, 0, 10), (42135, 1763+2, 0, 'Gromsblood - Ashenvale - Demon Fall Ridge', 0, 0, 10), (47871, 1763+2, 0, 'Gromsblood - Ashenvale - Demon Fall Ridge', 0, 0, 10);

DELETE FROM `pool_template` WHERE `entry` =1327 AND `patch_min` =0 AND `patch_max` =10;

-- End of migration.
;
