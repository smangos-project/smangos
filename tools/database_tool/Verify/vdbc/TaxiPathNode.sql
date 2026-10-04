-- key: ID
SELECT d."ID" AS "ID",
       d."PathID" AS "PathID",
       d."NodeIndex" AS "NodeIndex",
       d."ContinentID" AS "ContinentID",
       d."Loc1" AS "Loc1",
       d."Loc2" AS "Loc2",
       d."Loc3" AS "Loc3",
       d."Flags" AS "Flags",
       d."Delay" AS "Delay"
FROM "dbc_taxipathnode" d
