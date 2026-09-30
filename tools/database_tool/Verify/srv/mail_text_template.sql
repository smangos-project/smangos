-- key: entry
SELECT d."ID" AS "entry",
       d."Body_lang" AS "content_default",
       l."content_loc1" AS "content_loc1",
       l."content_loc2" AS "content_loc2",
       l."content_loc3" AS "content_loc3",
       l."content_loc4" AS "content_loc4",
       l."content_loc5" AS "content_loc5",
       l."content_loc6" AS "content_loc6",
       l."content_loc7" AS "content_loc7"
FROM "dbc_mailtemplate" d
JOIN "mail_text_template" l ON l."entry" = d."ID"
