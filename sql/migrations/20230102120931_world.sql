INSERT INTO `migrations` VALUES ('20230102120931');

-- Add your query below.
 -- Mini Pets.
 UPDATE `creature_template` SET `auras` =19230 WHERE `entry` =11325;

UPDATE `creature_template` SET `auras` =18873 WHERE `entry` =11326;

UPDATE `creature_template` SET `auras` =19226 WHERE `entry` =11327;

UPDATE `creature_template` SET `auras` =24983 WHERE `entry` IN (15186, 15361, 16069, 16445);

-- End of migration.
;
