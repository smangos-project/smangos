-- key: ID
SELECT d."ID" AS "ID",
       d."Flags" AS "Flags",
       d."FactionID" AS "FactionID",
       d."MaleDisplayID" AS "MaleDisplayID",
       d."FemaleDisplayID" AS "FemaleDisplayID",
       d."ClientPrefix" AS "ClientPrefix",
       d."BaseLanguage" AS "BaseLanguage",
       d."CreatureType" AS "CreatureType",
       836 AS "LoginEffectSpellID",
       1604 AS "CombatStunSpellID",
       d."ResSicknessSpellID" AS "ResSicknessSpellID",
       d."SplashSoundID" AS "SplashSoundID",
       t."taxi_mask" AS "StartingTaxiNodes",
       d."ClientFileString" AS "ClientFileString",
       d."CinematicSequenceID" AS "CinematicSequenceID",
       d."Name_lang" AS "Name_lang"
FROM "dbc_chrraces" d
LEFT JOIN "race_taxi_mask" t ON t."race" = d."ID"
