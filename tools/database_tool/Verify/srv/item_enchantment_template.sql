-- key: entry, ench
SELECT "entry", "ench", "chance", "patch_min", "patch_max" FROM "item_enchantment_template" WHERE "patch_min" <= 10 AND 10 <= "patch_max"
