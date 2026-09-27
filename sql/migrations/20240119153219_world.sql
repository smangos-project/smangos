INSERT INTO `migrations` VALUES ('20240119153219');

-- Add your query below.
 -- Mark all totems as immune to aoe and taunt.
 UPDATE `creature_template` SET `immunity_flags` =3 WHERE `type` =11;

-- End of migration.
;
