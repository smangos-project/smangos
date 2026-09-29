-- key: entry
SELECT "entry", "build", "name", "period" FROM (SELECT "entry", "build", "name", "period", ROW_NUMBER() OVER (PARTITION BY "entry" ORDER BY "build" DESC, rowid DESC) AS __rn FROM "transports" WHERE "build" <= 5875) WHERE __rn = 1
