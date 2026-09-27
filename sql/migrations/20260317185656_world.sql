INSERT INTO `migrations` VALUES ('20260317185656');

-- Add your query below.
 -- Scourge Archer should be immune to players.
 UPDATE `creature_template` SET `static_flags1` =292 WHERE `entry` =14489;

-- End of migration.
;
