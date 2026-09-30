-- key: entry
SELECT "ID" AS "entry",
       "ContinentID" AS "map_id",
       "ParentAreaID" AS "zone_id",
       "AreaBit" AS "explore_flag",
       "Flags1" & ~1073741824 AS "flags",
       "ExplorationLevel" AS "area_level",
       "AreaName_lang" AS "name",
       "FactionGroupMask" AS "team",
       "LiquidTypeID4" AS "liquid_type"
FROM "dbc_areatable"
