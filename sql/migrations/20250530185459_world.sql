INSERT INTO `migrations` VALUES ('20250530185459');

-- Add your query below.
 -- Fix being able to train later ranks of Mortal Strike without having the talent.
 UPDATE `skill_line_ability` SET `superseded_by_spell` =21551 WHERE `spell_id` =12294;

UPDATE `skill_line_ability` SET `superseded_by_spell` =21552 WHERE `spell_id` =21551;

UPDATE `skill_line_ability` SET `superseded_by_spell` =21553 WHERE `spell_id` =21552;

-- Fix being able to train later ranks of Bloodthirst without having the talent.
 UPDATE `skill_line_ability` SET `superseded_by_spell` =23892 WHERE `spell_id` =23881;

-- Fix being able to train later ranks of Shield Slam without having the talent.
 UPDATE `skill_line_ability` SET `superseded_by_spell` =23923 WHERE `spell_id` =23922;

-- End of migration.
;
