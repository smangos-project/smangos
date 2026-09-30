-- key: entry
-- where: ("after_or_before" = 0 AND "patch" <= 10) OR ("after_or_before" = 1 AND "patch" >= 10)
SELECT "entry" FROM "forbidden_items"
