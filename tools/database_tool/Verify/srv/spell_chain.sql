-- key: spell_id
-- first_spell and rank follow prev_spell, also through the client SkillLineAbility chains
WITH RECURSIVE
links(spell, prev) AS (
    SELECT "spell_id", "prev_spell" FROM "spell_chain"
    UNION
    SELECT "Spell", "SupercedesSpell" FROM "dbc_skilllineability"
    WHERE "SupercedesSpell" <> 0 AND "Spell" NOT IN (SELECT "spell_id" FROM "spell_chain")
),
walk(spell, cur, depth) AS (
    SELECT "spell_id", "spell_id", 1 FROM "spell_chain"
    UNION ALL
    SELECT w.spell, l.prev, w.depth + 1 FROM walk w JOIN links l ON l.spell = w.cur
    WHERE l.prev <> 0 AND w.depth < 30
),
tops AS (SELECT spell, MAX(depth) AS depth FROM walk GROUP BY spell)
SELECT c."spell_id", c."prev_spell", w.cur AS "first_spell", t.depth AS "rank", c."req_spell"
FROM "spell_chain" c
JOIN tops t ON t.spell = c."spell_id"
JOIN walk w ON w.spell = t.spell AND w.depth = t.depth
