-- key: guid
SELECT "guid", "id", "map", "position_x", "position_y", "position_z", "orientation", "rotation0", "rotation1", "rotation2", "rotation3", "spawntimesecsmin", "spawntimesecsmax", "animprogress", "state", "spawn_flags", "visibility_mod", "patch_min", "patch_max" FROM "gameobject" WHERE "patch_min" <= 10 AND 10 <= "patch_max"
