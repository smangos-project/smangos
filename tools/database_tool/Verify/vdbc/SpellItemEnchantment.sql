-- key: ID
SELECT d."ID" AS "ID",
       d."Effect1" AS "Effect1",
       d."Effect2" AS "Effect2",
       d."Effect3" AS "Effect3",
       d."EffectPointsMin1" AS "EffectPointsMin1",
       d."EffectPointsMin2" AS "EffectPointsMin2",
       d."EffectPointsMin3" AS "EffectPointsMin3",
       d."EffectPointsMax1" AS "EffectPointsMax1",
       d."EffectPointsMax2" AS "EffectPointsMax2",
       d."EffectPointsMax3" AS "EffectPointsMax3",
       d."EffectArg1" AS "EffectArg1",
       d."EffectArg2" AS "EffectArg2",
       d."EffectArg3" AS "EffectArg3",
       d."Name_lang" AS "Name_lang",
       d."ItemVisual" AS "ItemVisual",
       d."Flags" AS "Flags"
FROM "dbc_spellitemenchantment" d
