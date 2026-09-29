-- key: group_id
SELECT "group_id", "build", "stack_rule" FROM (SELECT "group_id", "build", "stack_rule", ROW_NUMBER() OVER (PARTITION BY "group_id" ORDER BY "build" DESC, rowid DESC) AS __rn FROM "spell_group_stack_rules" WHERE "build" <= 5875) WHERE __rn = 1
