INSERT INTO `migrations` VALUES ('20210822215257');

-- Add your query below.
 -- Fix Wyvern Sting not being removed on taking damage.
 UPDATE `spell_template` SET `procCharges` =1 WHERE (`procFlags` & 1048576) AND (`auraInterruptFlags` & 2) AND (`procCharges` = 0);

-- End of migration.
;
