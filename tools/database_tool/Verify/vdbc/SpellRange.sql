-- key: ID
SELECT d."ID" AS "ID",
       d."RangeMin1" AS "RangeMin",
       d."RangeMax1" AS "RangeMax",
       d."Flags" AS "Flags",
       d."DisplayName_lang" AS "DisplayName_lang",
       d."DisplayNameShort_lang" AS "DisplayNameShort_lang"
FROM "dbc_spellrange" d
