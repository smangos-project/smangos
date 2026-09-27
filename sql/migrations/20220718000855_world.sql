INSERT INTO `migrations` VALUES ('20220718000855');

-- Add your query below.
 -- Restore old period for Grom'Gol Base Camp to Undercity transport.
 UPDATE `transports` SET `period` =333044 WHERE `entry` =176495;

-- End of migration.
;
