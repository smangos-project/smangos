-- key: ID
SELECT d."ID" AS "ID",
       d."MinScale" AS "MinScale",
       d."MinScaleLevel" AS "MinScaleLevel",
       d."MaxScale" AS "MaxScale",
       d."MaxScaleLevel" AS "MaxScaleLevel",
       d."SkillLine1" AS "SkillLine1",
       d."SkillLine2" AS "SkillLine2",
       d."PetFoodMask" AS "PetFoodMask",
       d."Name_lang" AS "Name_lang",
       d."IconFileID" AS "IconFile"
FROM "dbc_creaturefamily" d
