-- key:
SELECT "id", "type", "str", "data", "address", "length", "result", "flags", "penalty", "build_min", "build_max", "comment" FROM "warden_scans" WHERE "build_min" <= 5875 AND 5875 <= "build_max"
