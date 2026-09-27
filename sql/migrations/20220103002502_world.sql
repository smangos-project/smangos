INSERT INTO `migrations` VALUES ('20220103002502');

-- Add your query below.

-- Add priority field to scripts to allow defining order of execution of
-- commands with same delay. AFTER `delay` needs a table rebuild in SQLite
-- (world-sql.md), for each of these ten identically-shaped script tables.
CREATE TABLE `creature_ai_scripts__new` (
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

INSERT INTO `creature_ai_scripts__new` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`)
SELECT `id`, `delay`, 0, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments` FROM `creature_ai_scripts`;

DROP TABLE `creature_ai_scripts`;
ALTER TABLE `creature_ai_scripts__new` RENAME TO `creature_ai_scripts`;

CREATE TABLE `creature_movement_scripts__new` (
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

INSERT INTO `creature_movement_scripts__new` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`)
SELECT `id`, `delay`, 0, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments` FROM `creature_movement_scripts`;

DROP TABLE `creature_movement_scripts`;
ALTER TABLE `creature_movement_scripts__new` RENAME TO `creature_movement_scripts`;

CREATE TABLE `creature_spells_scripts__new` (
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

INSERT INTO `creature_spells_scripts__new` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`)
SELECT `id`, `delay`, 0, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments` FROM `creature_spells_scripts`;

DROP TABLE `creature_spells_scripts`;
ALTER TABLE `creature_spells_scripts__new` RENAME TO `creature_spells_scripts`;

CREATE TABLE `event_scripts__new` (
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

INSERT INTO `event_scripts__new` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`)
SELECT `id`, `delay`, 0, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments` FROM `event_scripts`;

DROP TABLE `event_scripts`;
ALTER TABLE `event_scripts__new` RENAME TO `event_scripts`;

CREATE TABLE `generic_scripts__new` (
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

INSERT INTO `generic_scripts__new` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`)
SELECT `id`, `delay`, 0, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments` FROM `generic_scripts`;

DROP TABLE `generic_scripts`;
ALTER TABLE `generic_scripts__new` RENAME TO `generic_scripts`;

CREATE TABLE `gossip_scripts__new` (
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

INSERT INTO `gossip_scripts__new` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`)
SELECT `id`, `delay`, 0, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments` FROM `gossip_scripts`;

DROP TABLE `gossip_scripts`;
ALTER TABLE `gossip_scripts__new` RENAME TO `gossip_scripts`;

CREATE TABLE `gameobject_scripts__new` (
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

INSERT INTO `gameobject_scripts__new` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`)
SELECT `id`, `delay`, 0, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments` FROM `gameobject_scripts`;

DROP TABLE `gameobject_scripts`;
ALTER TABLE `gameobject_scripts__new` RENAME TO `gameobject_scripts`;

CREATE TABLE `spell_scripts__new` (
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

INSERT INTO `spell_scripts__new` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`)
SELECT `id`, `delay`, 0, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments` FROM `spell_scripts`;

DROP TABLE `spell_scripts`;
ALTER TABLE `spell_scripts__new` RENAME TO `spell_scripts`;

CREATE TABLE `quest_end_scripts__new` (
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

INSERT INTO `quest_end_scripts__new` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`)
SELECT `id`, `delay`, 0, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments` FROM `quest_end_scripts`;

DROP TABLE `quest_end_scripts`;
ALTER TABLE `quest_end_scripts__new` RENAME TO `quest_end_scripts`;

CREATE TABLE `quest_start_scripts__new` (
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

INSERT INTO `quest_start_scripts__new` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`)
SELECT `id`, `delay`, 0, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments` FROM `quest_start_scripts`;

DROP TABLE `quest_start_scripts`;
ALTER TABLE `quest_start_scripts__new` RENAME TO `quest_start_scripts`;

-- Grand Inquisitor Isillien remove unit flags before starting attack.
UPDATE `creature_ai_scripts` SET `priority`=1 WHERE `id`=184002 AND `command`=26;
-- Crimson Elite remove unit flags before starting attack.
UPDATE `creature_ai_scripts` SET `priority`=1 WHERE `id`=1212801 AND `command`=26;

-- End of migration.
