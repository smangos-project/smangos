-- key: entry
SELECT "SpellID" AS "entry", "ConeDegrees" AS "cone_degrees" FROM "dbc_spelltargetrestrictions" WHERE "DifficultyID" = 0 AND "SpellID" IN (SELECT "entry" FROM ref."srv_spell_cone")
