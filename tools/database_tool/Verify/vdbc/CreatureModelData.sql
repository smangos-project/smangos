-- key: ID
SELECT d."ID" AS "ID",
       d."Flags" AS "Flags",
       d."SizeClass" AS "SizeClass",
       d."ModelScale" AS "ModelScale",
       d."BloodID" AS "BloodID",
       d."FootprintTextureID" AS "FootprintTextureID",
       d."FootprintTextureLength" AS "FootprintTextureLength",
       d."FootprintTextureWidth" AS "FootprintTextureWidth",
       d."FootprintParticleScale" AS "FootprintParticleScale",
       d."FoleyMaterialID" AS "FoleyMaterialID",
       d."SoundID" AS "SoundID",
       d."CollisionWidth" AS "CollisionWidth",
       d."CollisionHeight" AS "CollisionHeight"
FROM "dbc_creaturemodeldata" d
