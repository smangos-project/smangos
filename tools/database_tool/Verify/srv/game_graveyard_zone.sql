-- key: id, ghost_zone
SELECT "id", "ghost_zone", "faction", "patch_min", "patch_max" FROM "game_graveyard_zone" WHERE "patch_min" <= 10 AND 10 <= "patch_max"
