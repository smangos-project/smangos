INSERT INTO `migrations` VALUES ('20220825210714');

-- Add your query below.
 -- Define targets for Activate Bomb A.
 INSERT INTO `spell_target_position` (`id`, `target_map`, `target_position_x`, `target_position_y`, `target_position_z`) VALUES (11511, 90, -530.754, 670.571, -313.784);

-- These critters fight back.
 UPDATE `creature_template` SET `ai_name` = 'BasicAI' WHERE `entry` IN (6145, 9600, 10510, 13321, 15010, 16998);

-- End of migration.
;
