INSERT INTO `migrations` VALUES ('20251010162006');

-- Add your query below.

-- Set unsigned value to zero to avoid error in mariadb.
UPDATE `gameobject_template` SET `data5`=0 WHERE `entry`=176084;

-- All gameobject data fields should be signed.
-- Every CHANGE COLUMN here only restates an INT size/position that already
-- maps to SQLite's INTEGER (which has no signedness), so it's a no-op.

-- Convert unsigned value to signed for Subway.
UPDATE `gameobject_template` SET `data5`=-1 WHERE `entry`=176084;


-- End of migration.
