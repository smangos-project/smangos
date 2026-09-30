-- key: display_id
SELECT a."display_id" AS "display_id",
       a."bounding_radius" AS "bounding_radius",
       a."combat_reach" AS "combat_reach",
       a."speed_walk" AS "speed_walk",
       a."speed_run" AS "speed_run",
       d."Gender" AS "gender",
       a."display_id_other_gender" AS "display_id_other_gender"
FROM "creature_display_info_addon" a
JOIN "dbc_creaturedisplayinfo" d ON d."ID" = a."display_id"
