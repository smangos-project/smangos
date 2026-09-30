-- key: id
SELECT a."ID" AS "id",
       COALESCE(e."name", '') AS "name",
       a."ContinentID" AS "map_id",
       CAST(printf('%.6g', a."Pos1") AS REAL) AS "x",
       CAST(printf('%.6g', a."Pos2") AS REAL) AS "y",
       CAST(printf('%.6g', a."Pos3") AS REAL) AS "z",
       a."Radius" AS "radius",
       CAST(printf('%.6g', a."Box_length") AS REAL) AS "box_x",
       CAST(printf('%.6g', a."Box_width") AS REAL) AS "box_y",
       CAST(printf('%.6g', a."Box_height") AS REAL) AS "box_z",
       CAST(printf('%.6g', a."Box_yaw") AS REAL) AS "box_orientation",
       COALESCE(e."cooldown", 0) AS "cooldown",
       COALESCE(e."condition_id", 0) AS "condition_id",
       COALESCE(e."script_id", 0) AS "script_id",
       COALESCE(e."script_name", '') AS "script_name"
FROM "dbc_areatrigger" a
LEFT JOIN "areatrigger_extra" e ON e."id" = a."ID"
