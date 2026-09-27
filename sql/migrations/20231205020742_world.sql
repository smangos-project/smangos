INSERT INTO `migrations` VALUES ('20231205020742');

-- Add your query below.
 -- Will of Shahram and Holy Strength should have separate aura slot per caster.
 UPDATE `spell_template` SET `customFlags` = (`customFlags` | 4096) WHERE `entry` IN (16598, 20007);

-- End of migration.
;
