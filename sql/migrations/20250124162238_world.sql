INSERT INTO `migrations` VALUES ('20250124162238');

-- Add your query below.
 -- Player creation spells (paladin 'Seal of Righteousness')
 UPDATE `playercreateinfo_spell` SET `spell` =20154 WHERE `class` =2 AND `spell` =21084;

-- PLayer creation actions (paladin 'Seal of Righteousness')
 UPDATE `playercreateinfo_action` SET `action` =20154 WHERE `class` =2 AND `action` =21084 AND `type` =0;

-- End of migration.
;
