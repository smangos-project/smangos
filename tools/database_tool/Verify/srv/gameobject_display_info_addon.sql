-- key: display_id
SELECT "ID" AS "display_id",
       CAST(printf('%.6g', "GeoBox1") AS REAL) AS "min_x",
       CAST(printf('%.6g', "GeoBox2") AS REAL) AS "min_y",
       CAST(printf('%.6g', "GeoBox3") AS REAL) AS "min_z",
       CAST(printf('%.6g', "GeoBox4") AS REAL) AS "max_x",
       CAST(printf('%.6g', "GeoBox5") AS REAL) AS "max_y",
       CAST(printf('%.6g', "GeoBox6") AS REAL) AS "max_z"
FROM "dbc_gameobjectdisplayinfo"
