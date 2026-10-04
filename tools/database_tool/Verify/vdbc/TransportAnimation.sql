-- key: ID
SELECT d."ID" AS "ID",
       d."TransportID" AS "TransportID",
       d."TimeIndex" AS "TimeIndex",
       d."Pos1" AS "Pos1",
       d."Pos2" AS "Pos2",
       d."Pos3" AS "Pos3",
       d."SequenceID" AS "SequenceID"
FROM "dbc_transportanimation" d
