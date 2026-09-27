INSERT INTO `migrations` VALUES ('20241226193319');

-- Add your query below.

-- Remove empty pools.
DELETE FROM `pool_template` WHERE `entry` IN (417, 1858, 1864, 1873, 2528, 2530, 2549, 2868, 2869, 2870, 4292, 4293, 14228, 14249, 17005, 21400, 21422, 21424, 21450, 21486, 21526, 21550, 21600, 21650, 25620);
DELETE FROM `pool_pool` WHERE `pool_id` IN (417, 1858, 1864, 1873, 2528, 2530, 2549, 2868, 2869, 2870, 4292, 4293, 14228, 14249, 17005, 21400, 21422, 21424, 21450, 21486, 21526, 21550, 21600, 21650, 25620);

-- `pool_creature` has guid 40461 for invalid pool 5004, skipped.
DELETE FROM `creature` WHERE `guid`=40461;
DELETE FROM `pool_creature` WHERE `guid`=40461;
DELETE FROM `creature_movement` WHERE `id`=40461;

-- Pool ids are uint16 in core so make db reflect that.
-- Every CHANGE COLUMN below only restates a SMALLINT UNSIGNED size, which
-- already maps to SQLite's INTEGER, in a position each column already has.

-- End of migration.
