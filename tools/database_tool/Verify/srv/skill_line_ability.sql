-- key: id
-- where: "build" = 5875
SELECT a."ID" AS "id",
       a."SkillLine" AS "skill_id",
       a."Spell" AS "spell_id",
       a."RaceMask" AS "race_mask",
       a."ClassMask" AS "class_mask",
       a."MinSkillLineRank" AS "req_skill_value",
       COALESCE(e."superseded_by_spell", n."next_spell", 0) AS "superseded_by_spell",
       a."AcquireMethod" AS "learn_on_get_skill",
       a."TrivialSkillLineRankHigh" AS "max_value",
       a."TrivialSkillLineRankLow" AS "min_value",
       a."CharacterPoints2" AS "req_train_points"
FROM "dbc_skilllineability" a
LEFT JOIN "skill_line_ability_extra" e ON e."id" = a."ID"
LEFT JOIN (SELECT "SkillLine", "SupercedesSpell", MIN("Spell") AS "next_spell" FROM "dbc_skilllineability"
           WHERE "SupercedesSpell" <> 0 GROUP BY "SkillLine", "SupercedesSpell") n
  ON n."SkillLine" = a."SkillLine" AND n."SupercedesSpell" = a."Spell"
