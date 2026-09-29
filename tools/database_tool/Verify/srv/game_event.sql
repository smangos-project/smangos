-- key: entry
SELECT "entry", "start_time", "end_time", "occurence", "length", "holiday", "description", "hardcoded", "disabled", "patch_min", "patch_max" FROM "game_event" WHERE "patch_min" <= 10 AND 10 <= "patch_max"
