INSERT INTO `migrations` VALUES ('20230221132045');

-- Add your query below.
 -- Correct faction for Larva Spewer.
 UPDATE `gameobject_template` SET `faction` =14, `script_name` = '' WHERE `entry` =178559;

-- End of migration.
;
