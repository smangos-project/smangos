-- key: ID
SELECT e."ID" AS "ID",
       e."Name" AS "Name",
       e."EmoteID" AS "EmoteID",
       COALESCE(MAX(CASE WHEN t."RelationshipFlags" = 0 THEN t."ID" END), 0) AS "EmoteText1",
       COALESCE(MAX(CASE WHEN t."RelationshipFlags" = 1 THEN t."ID" END), 0) AS "EmoteText2",
       COALESCE(MAX(CASE WHEN t."RelationshipFlags" = 2 THEN t."ID" END), 0) AS "EmoteText3",
       COALESCE(MAX(CASE WHEN t."RelationshipFlags" = 3 THEN t."ID" END), 0) AS "EmoteText4",
       COALESCE(MAX(CASE WHEN t."RelationshipFlags" = 4 THEN t."ID" END), 0) AS "EmoteText5",
       COALESCE(MAX(CASE WHEN t."RelationshipFlags" = 5 THEN t."ID" END), 0) AS "EmoteText6",
       COALESCE(MAX(CASE WHEN t."RelationshipFlags" = 6 THEN t."ID" END), 0) AS "EmoteText7",
       COALESCE(MAX(CASE WHEN t."RelationshipFlags" = 7 THEN t."ID" END), 0) AS "EmoteText8",
       COALESCE(MAX(CASE WHEN t."RelationshipFlags" = 8 THEN t."ID" END), 0) AS "EmoteText9",
       COALESCE(MAX(CASE WHEN t."RelationshipFlags" = 9 THEN t."ID" END), 0) AS "EmoteText10",
       COALESCE(MAX(CASE WHEN t."RelationshipFlags" = 10 THEN t."ID" END), 0) AS "EmoteText11",
       COALESCE(MAX(CASE WHEN t."RelationshipFlags" = 11 THEN t."ID" END), 0) AS "EmoteText12",
       COALESCE(MAX(CASE WHEN t."RelationshipFlags" = 12 THEN t."ID" END), 0) AS "EmoteText13",
       COALESCE(MAX(CASE WHEN t."RelationshipFlags" = 13 THEN t."ID" END), 0) AS "EmoteText14",
       COALESCE(MAX(CASE WHEN t."RelationshipFlags" = 14 THEN t."ID" END), 0) AS "EmoteText15",
       COALESCE(MAX(CASE WHEN t."RelationshipFlags" = 15 THEN t."ID" END), 0) AS "EmoteText16"
FROM "dbc_emotestext" e
LEFT JOIN "dbc_emotestextdata" t ON t."EmotesTextID" = e."ID"
GROUP BY e."ID"
