-- key: ID
SELECT d."ID" AS "ID",
       d."ModelID" AS "ModelID",
       d."SoundID" AS "SoundID",
       d."ExtendedDisplayInfoID" AS "ExtendedDisplayInfoID",
       d."CreatureModelScale" AS "CreatureModelScale",
       d."CreatureModelAlpha" AS "CreatureModelAlpha",
       d."TextureVariationFileDataID1" AS "TextureVariation1",
       d."TextureVariationFileDataID2" AS "TextureVariation2",
       d."TextureVariationFileDataID3" AS "TextureVariation3",
       d."SizeClass" AS "SizeClass",
       d."BloodID" AS "BloodID",
       d."NPCSoundID" AS "NPCSoundID"
FROM "dbc_creaturedisplayinfo" d
