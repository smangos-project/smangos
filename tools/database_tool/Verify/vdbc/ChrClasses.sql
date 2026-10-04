-- key: ID
SELECT d."ID" AS "ID",
       d."DamageBonusStat" AS "DamageBonusStat",
       d."DisplayPower" AS "DisplayPower",
       d."PetNameToken" AS "PetNameToken",
       d."Name_lang" AS "Name_lang",
       d."Filename" AS "Filename",
       d."SpellClassSet" AS "SpellClassSet",
       d."Flags" AS "Flags"
FROM "dbc_chrclasses" d
