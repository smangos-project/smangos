INSERT INTO `migrations` VALUES ('20240428090619');

-- Add your query below.
 -- Add No Threat List flag to Grobbulus Cloud and Scourge Summoning Crystal.
 UPDATE `creature_template` SET `flags_extra` = (`flags_extra` | (0x00000800 | 0x00020000)) WHERE `entry` IN (11623, 16363);

-- End of migration.
;
