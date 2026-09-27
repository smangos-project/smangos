INSERT INTO `migrations` VALUES ('20220714075708');

-- Add your query below.
 -- Battered Junkbox should not drop from mobs. It's only pickpocketed. 
 DELETE FROM `creature_loot_template` WHERE `item` =16882;

UPDATE `creature_template` SET `loot_id` =0 WHERE `loot_id` =12352;

-- End of migration.
;
