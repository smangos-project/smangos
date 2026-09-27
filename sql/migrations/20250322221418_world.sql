INSERT INTO `migrations` VALUES ('20250322221418');

-- Add your query below.
 -- Allow assigning scripts in areatrigger template.
 CREATE TABLE `areatrigger_template__new` (
  `id` INTEGER NOT NULL DEFAULT 0,
  `build` INTEGER NOT NULL DEFAULT 0,
  `name` TEXT DEFAULT '' COLLATE NOCASE,
  `map_id` INTEGER NOT NULL DEFAULT 0,
  `x` REAL NOT NULL DEFAULT 0,
  `y` REAL NOT NULL DEFAULT 0,
  `z` REAL NOT NULL DEFAULT 0,
  `radius` REAL NOT NULL DEFAULT 0,
  `box_x` REAL NOT NULL DEFAULT 0,
  `box_y` REAL NOT NULL DEFAULT 0,
  `box_z` REAL NOT NULL DEFAULT 0,
  `box_orientation` REAL NOT NULL DEFAULT 0,
  `cooldown` INTEGER NOT NULL DEFAULT 0,
  `condition_id` INTEGER NOT NULL DEFAULT 0,
  `script_id` INTEGER NOT NULL DEFAULT 0,
  `script_name` TEXT NOT NULL DEFAULT '' COLLATE NOCASE,
  PRIMARY KEY (`id`, `build`)
);

INSERT INTO `areatrigger_template__new` (`id`, `build`, `name`, `map_id`, `x`, `y`, `z`, `radius`, `box_x`, `box_y`, `box_z`, `box_orientation`, `cooldown`, `condition_id`, `script_id`, `script_name`)
SELECT `id`, `build`, `name`, `map_id`, `x`, `y`, `z`, `radius`, `box_x`, `box_y`, `box_z`, `box_orientation`, 0, 0, 0, '' FROM `areatrigger_template`;

DROP TABLE `areatrigger_template`;
ALTER TABLE `areatrigger_template__new` RENAME TO `areatrigger_template`;

-- Move scripts from scripted_areatrigger table.
 UPDATE `areatrigger_template` SET `script_name` = 'at_huldar_miran' WHERE `id` =171;

UPDATE `areatrigger_template` SET `script_name` = 'at_twiggy_flathead' WHERE `id` =522;

UPDATE `areatrigger_template` SET `script_name` = 'at_zumrah' WHERE `id` =962;

UPDATE `areatrigger_template` SET `script_name` = 'at_antusul' WHERE `id` =1447;

UPDATE `areatrigger_template` SET `script_name` = 'at_ring_of_law' WHERE `id` =1526;

UPDATE `areatrigger_template` SET `script_name` = 'at_sentry_point' WHERE `id` =1667;

UPDATE `areatrigger_template` SET `script_name` = 'at_scent_larkorwi' WHERE `id` =1726;

UPDATE `areatrigger_template` SET `script_name` = 'at_scent_larkorwi' WHERE `id` =1727;

UPDATE `areatrigger_template` SET `script_name` = 'at_scent_larkorwi' WHERE `id` =1728;

UPDATE `areatrigger_template` SET `script_name` = 'at_scent_larkorwi' WHERE `id` =1729;

UPDATE `areatrigger_template` SET `script_name` = 'at_scent_larkorwi' WHERE `id` =1730;

UPDATE `areatrigger_template` SET `script_name` = 'at_scent_larkorwi' WHERE `id` =1731;

UPDATE `areatrigger_template` SET `script_name` = 'at_scent_larkorwi' WHERE `id` =1732;

UPDATE `areatrigger_template` SET `script_name` = 'at_scent_larkorwi' WHERE `id` =1733;

UPDATE `areatrigger_template` SET `script_name` = 'at_scent_larkorwi' WHERE `id` =1734;

UPDATE `areatrigger_template` SET `script_name` = 'at_scent_larkorwi' WHERE `id` =1735;

UPDATE `areatrigger_template` SET `script_name` = 'at_scent_larkorwi' WHERE `id` =1736;

UPDATE `areatrigger_template` SET `script_name` = 'at_scent_larkorwi' WHERE `id` =1737;

UPDATE `areatrigger_template` SET `script_name` = 'at_scent_larkorwi' WHERE `id` =1738;

UPDATE `areatrigger_template` SET `script_name` = 'at_scent_larkorwi' WHERE `id` =1739;

UPDATE `areatrigger_template` SET `script_name` = 'at_scent_larkorwi' WHERE `id` =1740;

UPDATE `areatrigger_template` SET `script_name` = 'at_scent_larkorwi' WHERE `id` =1766;

UPDATE `areatrigger_template` SET `script_name` = 'at_shadowforge_bridge' WHERE `id` =1786;

UPDATE `areatrigger_template` SET `script_name` = 'at_murloc_camp' WHERE `id` =1966;

UPDATE `areatrigger_template` SET `script_name` = 'at_blackrock_spire' WHERE `id` =2026;

UPDATE `areatrigger_template` SET `script_name` = 'at_blackrock_spire' WHERE `id` =2046;

UPDATE `areatrigger_template` SET `script_name` = 'at_ubrs_the_beast' WHERE `id` =2066;

UPDATE `areatrigger_template` SET `script_name` = 'at_stormwind_gates' WHERE `id` =2786;

UPDATE `areatrigger_template` SET `script_name` = 'at_ravenholdt' WHERE `id` =3066;

UPDATE `areatrigger_template` SET `script_name` = 'at_childrens_week_spot' WHERE `id` =3546;

UPDATE `areatrigger_template` SET `script_name` = 'at_childrens_week_spot' WHERE `id` =3547;

UPDATE `areatrigger_template` SET `script_name` = 'at_childrens_week_spot' WHERE `id` =3548;

UPDATE `areatrigger_template` SET `script_name` = 'at_childrens_week_spot' WHERE `id` =3549;

UPDATE `areatrigger_template` SET `script_name` = 'at_childrens_week_spot' WHERE `id` =3550;

UPDATE `areatrigger_template` SET `script_name` = 'at_childrens_week_spot' WHERE `id` =3552;

UPDATE `areatrigger_template` SET `script_name` = 'at_irontree_wood' WHERE `id` =3587;

UPDATE `areatrigger_template` SET `script_name` = 'at_enter_vael_room' WHERE `id` =3626;

UPDATE `areatrigger_template` SET `script_name` = 'at_dmf_chest_dm' WHERE `id` =3746;

UPDATE `areatrigger_template` SET `script_name` = 'at_dmf_chest_wc' WHERE `id` =3766;

UPDATE `areatrigger_template` SET `script_name` = 'at_orb_of_command' WHERE `id` =3847;

UPDATE `areatrigger_template` SET `script_name` = 'at_shade_of_eranikus' WHERE `id` =4016;

UPDATE `areatrigger_template` SET `script_name` = 'at_twilight_grove' WHERE `id` =4017;

UPDATE `areatrigger_template` SET `script_name` = 'at_temple_ahnqiraj' WHERE `id` =4033;

UPDATE `areatrigger_template` SET `script_name` = 'at_temple_ahnqiraj' WHERE `id` =4034;

UPDATE `areatrigger_template` SET `script_name` = 'at_temple_ahnqiraj' WHERE `id` =4036;

UPDATE `areatrigger_template` SET `script_name` = 'at_temple_ahnqiraj' WHERE `id` =4047;

UPDATE `areatrigger_template` SET `script_name` = 'at_temple_ahnqiraj' WHERE `id` =4052;

UPDATE `areatrigger_template` SET `script_name` = 'at_cathedral_entrance' WHERE `id` =4089;

UPDATE `areatrigger_template` SET `script_name` = 'at_naxxramas' WHERE `id` =4112;

UPDATE `areatrigger_template` SET `script_name` = 'at_naxxramas' WHERE `id` =4113;

UPDATE `areatrigger_template` SET `script_name` = 'at_naxxramas' WHERE `id` =4115;

UPDATE `areatrigger_template` SET `script_name` = 'at_naxxramas' WHERE `id` =4117;

UPDATE `areatrigger_template` SET `script_name` = 'at_naxxramas' WHERE `id` =4156;

-- This table is no longer needed.
 DROP TABLE `scripted_areatrigger`;

CREATE TABLE `areatrigger_scripts` (
  `id` INTEGER NOT NULL DEFAULT 0,
  `delay` INTEGER NOT NULL DEFAULT 0,
  `priority` INTEGER NOT NULL DEFAULT 0,
  `command` INTEGER NOT NULL DEFAULT 0,
  `datalong` INTEGER NOT NULL DEFAULT 0,
  `datalong2` INTEGER NOT NULL DEFAULT 0,
  `datalong3` INTEGER NOT NULL DEFAULT 0,
  `datalong4` INTEGER NOT NULL DEFAULT 0,
  `target_param1` INTEGER NOT NULL DEFAULT 0,
  `target_param2` INTEGER NOT NULL DEFAULT 0,
  `target_type` INTEGER NOT NULL DEFAULT 0,
  `data_flags` INTEGER NOT NULL DEFAULT 0,
  `dataint` INTEGER NOT NULL DEFAULT 0,
  `dataint2` INTEGER NOT NULL DEFAULT 0,
  `dataint3` INTEGER NOT NULL DEFAULT 0,
  `dataint4` INTEGER NOT NULL DEFAULT 0,
  `x` REAL NOT NULL DEFAULT 0,
  `y` REAL NOT NULL DEFAULT 0,
  `z` REAL NOT NULL DEFAULT 0,
  `o` REAL NOT NULL DEFAULT 0,
  `condition_id` INTEGER NOT NULL DEFAULT 0,
  `comments` TEXT NOT NULL DEFAULT '' COLLATE NOCASE
);

-- Move Aurius greeting script from EventAI to areatrigger.
 DELETE FROM `creature_ai_events` WHERE `id` =1091702;

DELETE FROM `creature_ai_scripts` WHERE `id` =1091702;

UPDATE `areatrigger_template` SET `name` = 'Stratholme, Alonsus Chapel - Near Aurius', `cooldown` =30, `script_id` =2187 WHERE `id` =2187 AND `build` =4222;

DELETE FROM `areatrigger_scripts` WHERE `id` =2187;

INSERT INTO `areatrigger_scripts` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (2187, 0, 0, 39, 1091702, 0, 0, 0, 10917, 30, 10, 18, 100, 0, 0, 0, 0, 0, 0, 0, 0, 'Aurius - Start Script');

-- End of migration.
;
