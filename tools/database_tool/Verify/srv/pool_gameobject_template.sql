-- key: id
SELECT "id", "pool_entry", "chance", "description", "flags", "patch_min", "patch_max" FROM "pool_gameobject_template" WHERE "patch_min" <= 10 AND 10 <= "patch_max"
