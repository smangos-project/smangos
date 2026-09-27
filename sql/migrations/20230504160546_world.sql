INSERT INTO `migrations` VALUES ('20230504160546');

-- Add your query below.
 -- Remove AV start texts from mangos string table.
 DELETE FROM `mangos_string` WHERE `entry` IN (784, 785, 786);

-- End of migration.
;
