-- key: id
SELECT "ID" AS "id",
       "ContinentID" AS "map_id",
       CAST(printf('%.6g', "Pos1") AS REAL) AS "x",
       CAST(printf('%.6g', "Pos2") AS REAL) AS "y",
       CAST(printf('%.6g', "Pos3") AS REAL) AS "z",
       "Name_lang" AS "name",
       "MountCreatureID1" AS "mount_creature_id1",
       "MountCreatureID2" AS "mount_creature_id2"
FROM "dbc_taxinodes"
