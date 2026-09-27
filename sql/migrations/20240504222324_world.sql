INSERT INTO `migrations` VALUES ('20240504222324');

-- Add your query below.
 -- Fix Detect Magic to have one debuff per caster
 UPDATE `spell_template` SET `customFlags` =4096 WHERE `entry` =2855;

-- End of migration.
;
