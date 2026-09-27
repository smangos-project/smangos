INSERT INTO `migrations` VALUES ('20221013041810');

-- Add your query below.
 -- Revert changes made to Druid's Enrage.
 UPDATE `spell_template` SET `effect2` =3, `effectImplicitTargetA2` =0, `effectApplyAuraName2` =0, `effectMiscValue2` =0, `effectBonusCoefficient2` =1 WHERE `entry` =5229 AND `build` >= 5086;

-- End of migration.
;
