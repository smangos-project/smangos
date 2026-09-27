INSERT INTO `migrations` VALUES ('20240926142033');

-- Add your query below.
 -- Make the column which holds the spell class mask for effects be uint64.
 -- no-op: BIGINT already maps to INTEGER here, position unchanged

-- Transfer spell class masks from spell_affect table to spell_template.
 UPDATE `spell_template` SET `effectItemType1` = (
  SELECT `SpellFamilyMask` FROM `spell_affect`
  WHERE `spell_affect`.`entry` = `spell_template`.`entry`
    AND `spell_template`.`build` BETWEEN `spell_affect`.`build_min` AND `spell_affect`.`build_max`
    AND `spell_affect`.`effectId` = 0
)
WHERE EXISTS (
  SELECT 1 FROM `spell_affect`
  WHERE `spell_affect`.`entry` = `spell_template`.`entry`
    AND `spell_template`.`build` BETWEEN `spell_affect`.`build_min` AND `spell_affect`.`build_max`
    AND `spell_affect`.`effectId` = 0
);

UPDATE `spell_template` SET `effectItemType2` = (
  SELECT `SpellFamilyMask` FROM `spell_affect`
  WHERE `spell_affect`.`entry` = `spell_template`.`entry`
    AND `spell_template`.`build` BETWEEN `spell_affect`.`build_min` AND `spell_affect`.`build_max`
    AND `spell_affect`.`effectId` = 1
)
WHERE EXISTS (
  SELECT 1 FROM `spell_affect`
  WHERE `spell_affect`.`entry` = `spell_template`.`entry`
    AND `spell_template`.`build` BETWEEN `spell_affect`.`build_min` AND `spell_affect`.`build_max`
    AND `spell_affect`.`effectId` = 1
);

UPDATE `spell_template` SET `effectItemType3` = (
  SELECT `SpellFamilyMask` FROM `spell_affect`
  WHERE `spell_affect`.`entry` = `spell_template`.`entry`
    AND `spell_template`.`build` BETWEEN `spell_affect`.`build_min` AND `spell_affect`.`build_max`
    AND `spell_affect`.`effectId` = 2
)
WHERE EXISTS (
  SELECT 1 FROM `spell_affect`
  WHERE `spell_affect`.`entry` = `spell_template`.`entry`
    AND `spell_template`.`build` BETWEEN `spell_affect`.`build_min` AND `spell_affect`.`build_max`
    AND `spell_affect`.`effectId` = 2
);

-- Get rid of no longer needed separate table.
 DROP TABLE `spell_affect`;

-- Change the mod table to use int64 as well.
 -- no-op: BIGINT already maps to INTEGER here, position unchanged

-- Correct spell class masks for effects in cases where its missing in vanilla Spell.dbc but present in classic SpellEffect.db2.
 UPDATE `spell_template` SET `effectItemType1` =14684919 WHERE `entry` =15059 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =2233056;

-- Arcane Instability
 UPDATE `spell_template` SET `effectItemType1` =14684919 WHERE `entry` =15058 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =2233056;

-- Arcane Instability
 UPDATE `spell_template` SET `effectItemType1` =14684919 WHERE `entry` =15060 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =2233056;

-- Arcane Instability
 UPDATE `spell_template` SET `effectItemType1` =14684919 WHERE `entry` =12042 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =551557879;

-- Arcane Power
 UPDATE `spell_template` SET `effectItemType1` =176162304 WHERE `entry` =20104 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =176161280;

-- Benediction
 UPDATE `spell_template` SET `effectItemType1` =176162304 WHERE `entry` =20101 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =176161280;

-- Benediction
 UPDATE `spell_template` SET `effectItemType1` =176162304 WHERE `entry` =20102 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =176161280;

-- Benediction
 UPDATE `spell_template` SET `effectItemType1` =176162304 WHERE `entry` =20103 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =176161280;

-- Benediction
 UPDATE `spell_template` SET `effectItemType1` =176162304 WHERE `entry` =20105 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =176161280;

-- Benediction
 UPDATE `spell_template` SET `effectItemType1` =4194325 WHERE `entry` =12351 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =12714007;

-- Burning Soul
 UPDATE `spell_template` SET `effectItemType1` =4194325 WHERE `entry` =11083 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =12714007;

-- Burning Soul
 UPDATE `spell_template` SET `effectItemType1` =4096 WHERE `entry` =28787 AND `effect1` =6 AND `effectApplyAuraName1` =109 AND `effectItemType1` =8589934592;

-- Cleanse
 UPDATE `spell_template` SET `effectItemType1` =12589815 WHERE `entry` =12536 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =549460727;

-- Clearcasting
 UPDATE `spell_template` SET `effectItemType1` =12900875263 WHERE `entry` =16870 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =4512331310824191;

-- Clearcasting
 UPDATE `spell_template` SET `effectItemType1` =12582935 WHERE `entry` =28682 AND `effect1` =6 AND `effectApplyAuraName1` =107 AND `effectItemType1` =4194327;

-- Combustion
 UPDATE `spell_template` SET `effectItemType1` =11813318656 WHERE `entry` =29197 AND `effect1` =6 AND `effectApplyAuraName1` =112 AND `effectItemType1` =0;

-- Corrupted Mind
 UPDATE `spell_template` SET `effectItemType1` =34359738816 WHERE `entry` =29199 AND `effect1` =6 AND `effectApplyAuraName1` =112 AND `effectItemType1` =0;

-- Corrupted Mind
 UPDATE `spell_template` SET `effectItemType1` =4450426437 WHERE `entry` =29184 AND `effect1` =6 AND `effectApplyAuraName1` =112 AND `effectItemType1` =0;

-- Corrupted Mind
 UPDATE `spell_template` SET `effectItemType1` =25769804016 WHERE `entry` =29195 AND `effect1` =6 AND `effectApplyAuraName1` =112 AND `effectItemType1` =0;

-- Corrupted Mind
 UPDATE `spell_template` SET `effectItemType1` =12582935 WHERE `entry` =11115 AND `effect1` =6 AND `effectApplyAuraName1` =107 AND `effectItemType1` =146931735;

-- Critical Mass
 UPDATE `spell_template` SET `effectItemType1` =12582935 WHERE `entry` =11367 AND `effect1` =6 AND `effectApplyAuraName1` =107 AND `effectItemType1` =146931735;

-- Critical Mass
 UPDATE `spell_template` SET `effectItemType1` =12582935 WHERE `entry` =11369 AND `effect1` =6 AND `effectApplyAuraName1` =107 AND `effectItemType1` =146931735;

-- Critical Mass
 UPDATE `spell_template` SET `effectItemType1` =12582935 WHERE `entry` =11370 AND `effect1` =6 AND `effectApplyAuraName1` =107 AND `effectItemType1` =146931735;

-- Critical Mass
 UPDATE `spell_template` SET `effectItemType1` =12582935 WHERE `entry` =11368 AND `effect1` =6 AND `effectApplyAuraName1` =107 AND `effectItemType1` =146931735;

-- Critical Mass
 UPDATE `spell_template` SET `effectItemType1` =8388608 WHERE `entry` =29588 AND `effect1` =6 AND `effectApplyAuraName1` =112 AND `effectItemType1` =0;

-- Defensive Tactics
 UPDATE `spell_template` SET `effectItemType1` =8388608 WHERE `entry` =29589 AND `effect1` =6 AND `effectApplyAuraName1` =112 AND `effectItemType1` =0;

-- Defensive Tactics
 UPDATE `spell_template` SET `effectItemType1` =8388608 WHERE `entry` =29559 AND `effect1` =6 AND `effectApplyAuraName1` =112 AND `effectItemType1` =0;

-- Defensive Tactics
 UPDATE `spell_template` SET `effectItemType1` =3509583875 WHERE `entry` =16089 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =20723008519;

-- Elemental Fury
 UPDATE `spell_template` SET `effectItemType1` =12582935 WHERE `entry` =12399 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =146931735;

-- Fire Power
 UPDATE `spell_template` SET `effectItemType1` =12582935 WHERE `entry` =12400 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =146931735;

-- Fire Power
 UPDATE `spell_template` SET `effectItemType1` =12582935 WHERE `entry` =11124 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =146931735;

-- Fire Power
 UPDATE `spell_template` SET `effectItemType1` =12582935 WHERE `entry` =12378 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =146931735;

-- Fire Power
 UPDATE `spell_template` SET `effectItemType1` =12582935 WHERE `entry` =12398 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =146931735;

-- Fire Power
 UPDATE `spell_template` SET `effectItemType1` =6447219738 WHERE `entry` =18218 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =6447547418;

-- Grim Reach
 UPDATE `spell_template` SET `effectItemType1` =6447219738 WHERE `entry` =18219 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =6447547418;

-- Grim Reach
 UPDATE `spell_template` SET `effectItemType1` =12288 WHERE `entry` =26118 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =67108864;

-- Implements of Unspoken Names Pet Bonus
 UPDATE `spell_template` SET `effectItemType1` =37448586264 WHERE `entry` =30814 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =2954368024;

-- Mental Quickness
 UPDATE `spell_template` SET `effectItemType1` =37448586264 WHERE `entry` =30812 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =2954368024;

-- Mental Quickness
 UPDATE `spell_template` SET `effectItemType1` =37448586264 WHERE `entry` =30813 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =2954368024;

-- Mental Quickness
 UPDATE `spell_template` SET `effectItemType1` =151264832 WHERE `entry` =28808 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =17465351744;

-- Reduced Threat
 UPDATE `spell_template` SET `effectItemType1` =33685510 WHERE `entry` =28811 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =8388608;

-- Reduced Threat
 UPDATE `spell_template` SET `effectItemType1` =64 WHERE `entry` =28744 AND `effect1` =6 AND `effectApplyAuraName1` =112 AND `effectItemType1` =0;

-- Regrowth
 UPDATE `spell_template` SET `effectItemType1` =16 WHERE `entry` =28716 AND `effect1` =6 AND `effectApplyAuraName1` =112 AND `effectItemType1` =0;

-- Rejuvenation
 UPDATE `spell_template` SET `effectItemType1` =145 WHERE `entry` =18271 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =0;

-- Shadow Mastery
 UPDATE `spell_template` SET `effectItemType1` =145 WHERE `entry` =18272 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =0;

-- Shadow Mastery
 UPDATE `spell_template` SET `effectItemType1` =145 WHERE `entry` =18275 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =0;

-- Shadow Mastery
 UPDATE `spell_template` SET `effectItemType1` =145 WHERE `entry` =18274 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =0;

-- Shadow Mastery
 UPDATE `spell_template` SET `effectItemType1` =145 WHERE `entry` =18273 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =0;

-- Shadow Mastery
 UPDATE `spell_template` SET `effectItemType1` =15208471 WHERE `entry` =11170 AND `effect1` =6 AND `effectApplyAuraName1` =112 AND `effectItemType1` =1090394871;

-- Shatter
 UPDATE `spell_template` SET `effectItemType1` =15208471 WHERE `entry` =12985 AND `effect1` =6 AND `effectApplyAuraName1` =112 AND `effectItemType1` =1090394871;

-- Shatter
 UPDATE `spell_template` SET `effectItemType1` =15208471 WHERE `entry` =12982 AND `effect1` =6 AND `effectApplyAuraName1` =112 AND `effectItemType1` =1090394871;

-- Shatter
 UPDATE `spell_template` SET `effectItemType1` =15208471 WHERE `entry` =12983 AND `effect1` =6 AND `effectApplyAuraName1` =112 AND `effectItemType1` =1090394871;

-- Shatter
 UPDATE `spell_template` SET `effectItemType1` =15208471 WHERE `entry` =12984 AND `effect1` =6 AND `effectApplyAuraName1` =112 AND `effectItemType1` =1090394871;

-- Shatter
 UPDATE `spell_template` SET `effectItemType1` =6447219738 WHERE `entry` =18174 AND `effect1` =6 AND `effectApplyAuraName1` =107 AND `effectItemType1` =6447531034;

-- Suppression
 UPDATE `spell_template` SET `effectItemType1` =6447219738 WHERE `entry` =18175 AND `effect1` =6 AND `effectApplyAuraName1` =107 AND `effectItemType1` =6447531034;

-- Suppression
 UPDATE `spell_template` SET `effectItemType1` =6447219738 WHERE `entry` =18177 AND `effect1` =6 AND `effectApplyAuraName1` =107 AND `effectItemType1` =6447531034;

-- Suppression
 UPDATE `spell_template` SET `effectItemType1` =6447219738 WHERE `entry` =18176 AND `effect1` =6 AND `effectApplyAuraName1` =107 AND `effectItemType1` =6447531034;

-- Suppression
 UPDATE `spell_template` SET `effectItemType1` =6447219738 WHERE `entry` =18178 AND `effect1` =6 AND `effectApplyAuraName1` =107 AND `effectItemType1` =6447531034;

-- Suppression
 UPDATE `spell_template` SET `effectItemType1` =537399320 WHERE `entry` =28818 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =537395200;

-- Totemic Energy
 UPDATE `spell_template` SET `effectItemType1` =67624960 WHERE `entry` =16189 AND `effect1` =6 AND `effectApplyAuraName1` =107 AND `effectItemType1` =67362816;

-- Totemic Mastery
 UPDATE `spell_template` SET `effectItemType1` =73728 WHERE `entry` =16515 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =34359861248;

-- Vile Poisons
 UPDATE `spell_template` SET `effectItemType1` =73728 WHERE `entry` =16719 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =34359861248;

-- Vile Poisons
 UPDATE `spell_template` SET `effectItemType1` =8192 WHERE `entry` =16513 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =34359861248;

-- Vile Poisons
 UPDATE `spell_template` SET `effectItemType1` =73728 WHERE `entry` =16720 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =34359861248;

-- Vile Poisons
 UPDATE `spell_template` SET `effectItemType1` =73728 WHERE `entry` =16514 AND `effect1` =6 AND `effectApplyAuraName1` =108 AND `effectItemType1` =34359861248;

-- Vile Poisons
 UPDATE `spell_template` SET `effectItemType2` =12589815 WHERE `entry` =12042 AND `effect2` =6 AND `effectApplyAuraName2` =108 AND `effectItemType2` =551557879;

-- Arcane Power
 UPDATE `spell_template` SET `effectItemType2` =12288 WHERE `entry` =20575 AND `effect2` =6 AND `effectApplyAuraName2` =108 AND `effectItemType2` =67108864;

-- Command
 UPDATE `spell_template` SET `effectItemType2` =100 WHERE `entry` =17955 AND `effect2` =6 AND `effectApplyAuraName2` =108 AND `effectItemType2` =824633725796;

-- Emberstorm
 UPDATE `spell_template` SET `effectItemType2` =100 WHERE `entry` =17954 AND `effect2` =6 AND `effectApplyAuraName2` =108 AND `effectItemType2` =824633725796;

-- Emberstorm
 UPDATE `spell_template` SET `effectItemType2` =100 WHERE `entry` =17957 AND `effect2` =6 AND `effectApplyAuraName2` =108 AND `effectItemType2` =824633725796;

-- Emberstorm 
 UPDATE `spell_template` SET `effectItemType2` =100 WHERE `entry` =17956 AND `effect2` =6 AND `effectApplyAuraName2` =108 AND `effectItemType2` =824633725796;

-- Emberstorm 
 UPDATE `spell_template` SET `effectItemType2` =100 WHERE `entry` =17958 AND `effect2` =6 AND `effectApplyAuraName2` =108 AND `effectItemType2` =824633725796;

-- Emberstorm 
 UPDATE `spell_template` SET `effectItemType2` =0 WHERE `entry` =21942 AND `effect2` =6 AND `effectApplyAuraName2` =107 AND `effectItemType2` =134217728;

-- Hunter Giant Stalker
 UPDATE `spell_template` SET `effectItemType2` =3221225472 WHERE `entry` =20215 AND `effect2` =6 AND `effectApplyAuraName2` =112 AND `effectItemType2` =0;

-- Illumination
 UPDATE `spell_template` SET `effectItemType2` =3221225472 WHERE `entry` =20210 AND `effect2` =6 AND `effectApplyAuraName2` =112 AND `effectItemType2` =0;

-- Illumination
 UPDATE `spell_template` SET `effectItemType2` =3221225472 WHERE `entry` =20212 AND `effect2` =6 AND `effectApplyAuraName2` =112 AND `effectItemType2` =0;

-- Illumination
 UPDATE `spell_template` SET `effectItemType2` =3221225472 WHERE `entry` =20213 AND `effect2` =6 AND `effectApplyAuraName2` =112 AND `effectItemType2` =0;

-- Illumination
 UPDATE `spell_template` SET `effectItemType2` =3221225472 WHERE `entry` =20214 AND `effect2` =6 AND `effectApplyAuraName2` =112 AND `effectItemType2` =0;

-- Illumination
 UPDATE `spell_template` SET `effectItemType2` =17179869184 WHERE `entry` =29192 AND `effect2` =6 AND `effectApplyAuraName2` =108 AND `effectItemType2` =2097152;

-- Improved Weapon Totems
 UPDATE `spell_template` SET `effectItemType2` =17179869184 WHERE `entry` =29193 AND `effect2` =6 AND `effectApplyAuraName2` =108 AND `effectItemType2` =2097152;

-- Improved Weapon Totems
 UPDATE `spell_template` SET `effectItemType2` =4294968326 WHERE `entry` =28746 AND `effect2` =6 AND `effectApplyAuraName2` =108 AND `effectItemType2` =66566;

-- Plagueheart
 UPDATE `spell_template` SET `effectItemType2` =1048576 WHERE `entry` =14909 AND `effect2` =6 AND `effectApplyAuraName2` =108 AND `effectItemType2` =1048704;

-- Searing Light
 UPDATE `spell_template` SET `effectItemType2` =1048576 WHERE `entry` =15017 AND `effect2` =6 AND `effectApplyAuraName2` =108 AND `effectItemType2` =1048704;

-- Searing Light
 UPDATE `spell_template` SET `effectItemType2` =65536 WHERE `entry` =16515 AND `effect2` =6 AND `effectApplyAuraName2` =108 AND `effectItemType2` =122880;

-- Vile Poisons
 UPDATE `spell_template` SET `effectItemType2` =65536 WHERE `entry` =16719 AND `effect2` =6 AND `effectApplyAuraName2` =108 AND `effectItemType2` =122880;

-- Vile Poisons
 UPDATE `spell_template` SET `effectItemType2` =65536 WHERE `entry` =16513 AND `effect2` =6 AND `effectApplyAuraName2` =108 AND `effectItemType2` =122880;

-- Vile Poisons
 UPDATE `spell_template` SET `effectItemType2` =65536 WHERE `entry` =16720 AND `effect2` =6 AND `effectApplyAuraName2` =108 AND `effectItemType2` =122880;

-- Vile Poisons
 UPDATE `spell_template` SET `effectItemType2` =65536 WHERE `entry` =16514 AND `effect2` =6 AND `effectApplyAuraName2` =108 AND `effectItemType2` =122880;

-- Vile Poisons
 UPDATE `spell_template` SET `effectItemType3` =4194437 WHERE `entry` =12042 AND `effect3` =6 AND `effectApplyAuraName3` =108 AND `effectItemType3` =551557879;

-- Arcane Power
 UPDATE `spell_template` SET `effectItemType3` =4295509000 WHERE `entry` =18271 AND `effect3` =6 AND `effectApplyAuraName3` =108 AND `effectItemType3` =0;

-- Shadow Mastery
 UPDATE `spell_template` SET `effectItemType3` =4295509000 WHERE `entry` =18272 AND `effect3` =6 AND `effectApplyAuraName3` =108 AND `effectItemType3` =0;

-- Shadow Mastery
 UPDATE `spell_template` SET `effectItemType3` =4295509000 WHERE `entry` =18275 AND `effect3` =6 AND `effectApplyAuraName3` =108 AND `effectItemType3` =0;

-- Shadow Mastery
 UPDATE `spell_template` SET `effectItemType3` =4295509000 WHERE `entry` =18274 AND `effect3` =6 AND `effectApplyAuraName3` =108 AND `effectItemType3` =0;

-- Shadow Mastery
 UPDATE `spell_template` SET `effectItemType3` =4295509000 WHERE `entry` =18273 AND `effect3` =6 AND `effectApplyAuraName3` =108 AND `effectItemType3` =0;

-- Shadow Mastery
 UPDATE `spell_template` SET `effectItemType3` =268550144 WHERE `entry` =16515 AND `effect3` =6 AND `effectApplyAuraName3` =107 AND `effectItemType3` =122880;

-- Vile Poisons
 UPDATE `spell_template` SET `effectItemType3` =268550144 WHERE `entry` =16719 AND `effect3` =6 AND `effectApplyAuraName3` =107 AND `effectItemType3` =122880;

-- Vile Poisons
 UPDATE `spell_template` SET `effectItemType3` =268550144 WHERE `entry` =16513 AND `effect3` =6 AND `effectApplyAuraName3` =107 AND `effectItemType3` =122880;

-- Vile Poisons
 UPDATE `spell_template` SET `effectItemType3` =268550144 WHERE `entry` =16720 AND `effect3` =6 AND `effectApplyAuraName3` =107 AND `effectItemType3` =122880;

-- Vile Poisons
 UPDATE `spell_template` SET `effectItemType3` =268550144 WHERE `entry` =16514 AND `effect3` =6 AND `effectApplyAuraName3` =107 AND `effectItemType3` =122880;

-- Vile Poisons
 -- End of migration.
;
