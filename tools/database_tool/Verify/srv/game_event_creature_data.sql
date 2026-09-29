-- key: guid, event
SELECT "guid", "patch", "entry_id", "display_id", "equipment_id", "spell_start", "spell_end", "event" FROM (SELECT "guid", "patch", "entry_id", "display_id", "equipment_id", "spell_start", "spell_end", "event", ROW_NUMBER() OVER (PARTITION BY "guid", "event" ORDER BY "patch" DESC, rowid DESC) AS __rn FROM "game_event_creature_data" WHERE "patch" <= 10) WHERE __rn = 1
