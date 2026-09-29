-- key: entry
SELECT "entry", "max_limit", "description", "flags", "instance", "patch_min", "patch_max" FROM "pool_template" WHERE "patch_min" <= 10 AND 10 <= "patch_max"
