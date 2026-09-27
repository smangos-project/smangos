INSERT INTO `migrations` VALUES ('20260322232402');

-- Add your query below.
 -- Vol'jin should haev no subname.
 UPDATE `creature_template` SET `subname` =NULL WHERE `entry` =10540;

-- Nefarian should have no subname.
 UPDATE `creature_template` SET `subname` =NULL WHERE `entry` =11583;

-- [UNUSED] Xur'gyl should have no subname.
 UPDATE `creature_template` SET `subname` =NULL WHERE `entry` =10370;

-- Grunenstur Balindom should have a subname.
 UPDATE `creature_template` SET `subname` = 'Crocilisk Trainer' WHERE `entry` =2876;

-- End of migration.
;
