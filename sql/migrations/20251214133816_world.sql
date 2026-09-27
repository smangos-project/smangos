INSERT INTO `migrations` VALUES ('20251214133816');

-- Add your query below.
 -- Add no unreachable evade flag to Razorgore. He summons players instead.
 UPDATE `creature_template` SET `flags_extra` = (`flags_extra` | 0x00000008) WHERE `entry` =12435;

-- End of migration.
;
