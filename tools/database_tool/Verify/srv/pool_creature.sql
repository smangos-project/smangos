-- key: guid
SELECT "guid", "pool_entry", "chance", "description", "flags", "patch_min", "patch_max" FROM "pool_creature" WHERE "patch_min" <= 10 AND 10 <= "patch_max"
