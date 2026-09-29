-- key: id, quest
SELECT "id", "quest", "patch_min", "patch_max" FROM "creature_involvedrelation" WHERE "patch_min" <= 10 AND 10 <= "patch_max"
