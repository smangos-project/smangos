INSERT INTO `migrations` VALUES ('20230608162700');

-- Add your query below.
 -- Add some scans on I found on private servers when connecting with Hermes.
 INSERT INTO `warden_scans` (`id`, `type`, `str`, `data`, `address`, `length`, `result`, `flags`, `penalty`, `comment`) VALUES (95, 0, NULL, NULL, 7119406, 8, 'A37889CE00C70574', 2, -1, 'ActivateNextModule change');

INSERT INTO `warden_scans` (`id`, `type`, `str`, `data`, `address`, `length`, `result`, `flags`, `penalty`, `comment`) VALUES (96, 0, NULL, NULL, 6392088, 8, '24DA1A005DC20800', 2, -1, 'InitMovementStatus change');

-- More accurate names for some scans by Blu.
 UPDATE `warden_scans` SET `comment` = 'Anti-Afk' WHERE `id` =30 AND `address` =4730584;

UPDATE `warden_scans` SET `comment` = 'Wall climb' WHERE `id` =23 AND `address` =6502300;

UPDATE `warden_scans` SET `comment` = 'WMO collision' WHERE `id` =22 AND `address` =6993044;

UPDATE `warden_scans` SET `comment` = 'No fog hack' WHERE `id` =19 AND `address` =7153475;

UPDATE `warden_scans` SET `comment` = 'No fog hack' WHERE `id` =20 AND `address` =7138894;

UPDATE `warden_scans` SET `comment` = 'No fog hack' WHERE `id` =21 AND `address` =7138907;

UPDATE `warden_scans` SET `comment` = 'Water jump height hack' WHERE `id` =26 AND `address` =8151657;

UPDATE `warden_scans` SET `comment` = 'M2 collision' WHERE `id` =27 AND `address` =6992319;

UPDATE `warden_scans` SET `comment` = 'Looting hack' WHERE `id` =24 AND `address` =6340512;

UPDATE `warden_scans` SET `comment` = 'Looting hack' WHERE `id` =28 AND `address` =6340529;

UPDATE `warden_scans` SET `comment` = 'Generic movement hack' WHERE `id` =25 AND `address` =6380455;

-- End of migration.
;
