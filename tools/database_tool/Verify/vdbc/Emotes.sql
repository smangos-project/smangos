-- key: ID
SELECT d."ID" AS "ID",
       d."EmoteSlashCommand" AS "EmoteSlashCommand",
       d."AnimID" AS "AnimID",
       d."EmoteFlags" AS "EmoteFlags",
       d."EmoteSpecProc" AS "EmoteSpecProc",
       d."EmoteSpecProcParam" AS "EmoteSpecProcParam",
       d."EventSoundID" AS "EventSoundID"
FROM "dbc_emotes" d
