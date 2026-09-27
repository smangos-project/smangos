INSERT INTO `migrations` VALUES ('20240212124141');

-- Add your query below.
 -- Do not remove Amorous auras on evade.
 UPDATE `spell_template` SET `customFlags` = (`customFlags` | 1024) WHERE `entry` IN (26870, 27742, 26869);

-- End of migration.
;
