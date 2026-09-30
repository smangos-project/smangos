-- key: entry
SELECT m."entry" AS "entry",
       m."patch" AS "patch",
       CASE WHEN d."ParentMapID" = -1 THEN 0 ELSE d."ParentMapID" END AS "parent",
       d."InstanceType" AS "map_type",
       m."linked_zone" AS "linked_zone",
       d."MaxPlayers" AS "player_limit",
       m."reset_delay" AS "reset_delay",
       d."CorpseMapID" AS "ghost_entrance_map",
       m."ghost_entrance_x" AS "ghost_entrance_x",
       m."ghost_entrance_y" AS "ghost_entrance_y",
       d."MapName_lang" AS "map_name",
       m."script_name" AS "script_name"
FROM (SELECT *, ROW_NUMBER() OVER (PARTITION BY "entry" ORDER BY "patch" DESC, rowid DESC) AS __rn
      FROM "map_template" WHERE "patch" <= 10) m
JOIN "dbc_map" d ON d."ID" = m."entry"
WHERE m.__rn = 1
