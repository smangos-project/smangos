INSERT INTO `migrations` VALUES ('20240705213916');

-- Add your query below.
 -- Fix Stormwind and Orgrimmar world buffs not applying to pets.
 UPDATE `generic_scripts` SET `datalong3` =1 WHERE `command` =92 AND `id` IN (4974, 7782, 7496);

UPDATE `creature_movement_scripts` SET `datalong3` =1 WHERE `command` =92 AND `id` IN (1439202, 1472002);

-- End of migration.
;
