-- key: entry, item1, item2, item3
SELECT "entry", "probability", "item1", "item2", "item3", "patch_min", "patch_max" FROM "creature_equip_template" WHERE "patch_min" <= 10 AND 10 <= "patch_max"
