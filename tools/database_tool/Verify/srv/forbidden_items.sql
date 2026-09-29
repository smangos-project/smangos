-- key: entry, after_or_before
SELECT "entry", "patch", "after_or_before" FROM (SELECT "entry", "patch", "after_or_before", ROW_NUMBER() OVER (PARTITION BY "entry", "after_or_before" ORDER BY "patch" DESC, rowid DESC) AS __rn FROM "forbidden_items" WHERE "patch" <= 10) WHERE __rn = 1
