-- key: id, target_map
SELECT "id", "target_map", "target_position_x", "target_position_y", "target_position_z", "target_orientation", "build_min", "build_max" FROM "spell_target_position" WHERE "build_min" <= 5875 AND 5875 <= "build_max"
