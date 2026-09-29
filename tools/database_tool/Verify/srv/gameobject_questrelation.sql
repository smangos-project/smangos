-- key: id, quest
SELECT "id", "quest", "patch_min", "patch_max" FROM "gameobject_questrelation" WHERE "patch_min" <= 10 AND 10 <= "patch_max"
