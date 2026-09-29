-- key: id
SELECT "id", "build", "map_id", "x", "y", "z", "name", "mount_creature_id1", "mount_creature_id2" FROM (SELECT "id", "build", "map_id", "x", "y", "z", "name", "mount_creature_id1", "mount_creature_id2", ROW_NUMBER() OVER (PARTITION BY "id" ORDER BY "build" DESC, rowid DESC) AS __rn FROM "taxi_nodes" WHERE "build" <= 5875) WHERE __rn = 1
