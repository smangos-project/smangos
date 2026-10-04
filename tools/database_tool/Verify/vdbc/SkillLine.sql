-- key: ID
SELECT d."ID" AS "ID",
       d."CategoryID" AS "CategoryID",
       d."DisplayName_lang" AS "DisplayName_lang",
       d."Description_lang" AS "Description_lang",
       d."SpellIconFileID" AS "SpellIconID"
FROM "dbc_skillline" d
