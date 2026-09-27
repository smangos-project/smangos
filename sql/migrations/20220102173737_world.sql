INSERT INTO `migrations` VALUES ('20220102173737');

-- Add your query below.

-- Add patch to this table. AFTER `guid` plus a primary key change needs a
-- table rebuild in SQLite (world-sql.md).
CREATE TABLE `game_event_creature_data__new` (
  `guid` INTEGER NOT NULL DEFAULT 0,
  `patch` INTEGER NOT NULL DEFAULT 0,
  `entry_id` INTEGER NOT NULL DEFAULT 0,
  `display_id` INTEGER NOT NULL DEFAULT 0,
  `equipment_id` INTEGER NOT NULL DEFAULT 0,
  `spell_start` INTEGER NOT NULL DEFAULT 0,
  `spell_end` INTEGER NOT NULL DEFAULT 0,
  `event` INTEGER NOT NULL DEFAULT 0,
  PRIMARY KEY (`guid`, `event`, `patch`)
);

INSERT INTO `game_event_creature_data__new` (`guid`, `patch`, `entry_id`, `display_id`, `equipment_id`, `spell_start`, `spell_end`, `event`)
SELECT `guid`, 0, `entry_id`, `display_id`, `equipment_id`, `spell_start`, `spell_end`, `event` FROM `game_event_creature_data`;

DROP TABLE `game_event_creature_data`;
ALTER TABLE `game_event_creature_data__new` RENAME TO `game_event_creature_data`;

-- Spells aded in patch 1.8.
UPDATE `game_event_creature_data` SET `patch`=6 WHERE `spell_start` IN (26231, 26215, 26212, 26209, 26210, 26115, 26214);

-- Spells aded in patch 1.9.
UPDATE `game_event_creature_data` SET `patch`=7 WHERE `spell_start` IN (26383, 26384, 27654, 26870, 26520);

-- End of migration.
