-- key: ID
SELECT d."ID" AS "ID",
       d."SkillID" AS "SkillID",
       d."RaceMask" AS "RaceMask",
       d."ClassMask" AS "ClassMask",
       d."Flags" AS "Flags",
       d."MinLevel" AS "MinLevel",
       d."SkillTierID" AS "SkillTierID"
FROM "dbc_skillraceclassinfo" d
