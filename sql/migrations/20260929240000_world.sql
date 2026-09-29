INSERT INTO `migrations` VALUES ('20260929240000');

-- SPELL_CUSTOM_MAGNET_REDIRECT
UPDATE `spell_template` SET `customFlags` = (`customFlags` | 0x4000) WHERE `entry` IN (24239, 24274, 24275, 25182);
