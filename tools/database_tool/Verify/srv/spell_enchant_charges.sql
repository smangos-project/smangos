-- key: entry
SELECT r."entry" AS "entry", (SELECT e."Charges" FROM "dbc_spellitemenchantment" e WHERE e."ID" = (SELECT f."EffectMiscValue1" FROM "dbc_spelleffect" f WHERE f."SpellID" = r."entry" AND f."DifficultyID" = 0 AND f."Effect" IN (53, 54, 92) ORDER BY f."EffectIndex" LIMIT 1)) AS "charges" FROM ref."srv_spell_enchant_charges" r
