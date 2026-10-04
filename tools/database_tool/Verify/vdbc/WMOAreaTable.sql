-- key: ID
SELECT d."ID" AS "ID",
       d."WMOID" AS "WMOID",
       d."NameSetID" AS "NameSetID",
       CASE WHEN d."WMOGroupID" > 2147483647 THEN d."WMOGroupID" - 4294967296 ELSE d."WMOGroupID" END AS "WMOGroupID",
       d."SoundProviderPref" AS "SoundProviderPref",
       d."SoundProviderPrefUnderwater" AS "SoundProviderPrefUnderwater",
       d."AmbienceID" AS "AmbienceID",
       d."ZoneMusic" AS "ZoneMusic",
       d."IntroSound" AS "IntroSound",
       d."Flags" AS "Flags",
       d."AreaTableID" AS "AreaTableID",
       d."AreaName_lang" AS "AreaName_lang"
FROM "dbc_wmoareatable" d
