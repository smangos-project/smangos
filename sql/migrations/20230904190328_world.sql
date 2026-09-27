INSERT INTO `migrations` VALUES ('20230904190328');

CREATE TABLE `creature_groups_entry_limit` (
  `leader_guid` INTEGER NOT NULL DEFAULT 0,
  `creature_id` INTEGER NOT NULL DEFAULT 0,
  `min_count` INTEGER NOT NULL DEFAULT 0,
  `max_count` INTEGER NOT NULL DEFAULT 1,
  PRIMARY KEY (`leader_guid`, `creature_id`)
);

-- End of migration.
;
