INSERT INTO `migrations` VALUES ('20241116214502');

-- Add your query below.
 -- Add reputation spillover mask column.
 CREATE TABLE `quest_template__new` (
  `entry` INTEGER NOT NULL DEFAULT 0,
  `patch` INTEGER NOT NULL DEFAULT 0,
  `Method` INTEGER NOT NULL DEFAULT 2,
  `ZoneOrSort` INTEGER NOT NULL DEFAULT 0,
  `MinLevel` INTEGER NOT NULL DEFAULT 0,
  `MaxLevel` INTEGER NOT NULL DEFAULT 0,
  `QuestLevel` INTEGER NOT NULL DEFAULT 0,
  `Type` INTEGER NOT NULL DEFAULT 0,
  `RequiredClasses` INTEGER NOT NULL DEFAULT 0,
  `RequiredRaces` INTEGER NOT NULL DEFAULT 0,
  `RequiredSkill` INTEGER NOT NULL DEFAULT 0,
  `RequiredSkillValue` INTEGER NOT NULL DEFAULT 0,
  `RequiredCondition` INTEGER NOT NULL DEFAULT 0,
  `RepObjectiveFaction` INTEGER NOT NULL DEFAULT 0,
  `RepObjectiveValue` INTEGER NOT NULL DEFAULT 0,
  `RequiredMinRepFaction` INTEGER NOT NULL DEFAULT 0,
  `RequiredMinRepValue` INTEGER NOT NULL DEFAULT 0,
  `RequiredMaxRepFaction` INTEGER NOT NULL DEFAULT 0,
  `RequiredMaxRepValue` INTEGER NOT NULL DEFAULT 0,
  `SuggestedPlayers` INTEGER NOT NULL DEFAULT 0,
  `LimitTime` INTEGER NOT NULL DEFAULT 0,
  `QuestFlags` INTEGER NOT NULL DEFAULT 0,
  `SpecialFlags` INTEGER NOT NULL DEFAULT 0,
  `PrevQuestId` INTEGER NOT NULL DEFAULT 0,
  `NextQuestId` INTEGER NOT NULL DEFAULT 0,
  `ExclusiveGroup` INTEGER NOT NULL DEFAULT 0,
  `BreadcrumbForQuestId` INTEGER NOT NULL DEFAULT 0,
  `NextQuestInChain` INTEGER NOT NULL DEFAULT 0,
  `SrcItemId` INTEGER NOT NULL DEFAULT 0,
  `SrcItemCount` INTEGER NOT NULL DEFAULT 0,
  `SrcSpell` INTEGER NOT NULL DEFAULT 0,
  `Title` TEXT COLLATE NOCASE,
  `Details` TEXT COLLATE NOCASE,
  `Objectives` TEXT COLLATE NOCASE,
  `OfferRewardText` TEXT COLLATE NOCASE,
  `RequestItemsText` TEXT COLLATE NOCASE,
  `EndText` TEXT COLLATE NOCASE,
  `ObjectiveText1` TEXT COLLATE NOCASE,
  `ObjectiveText2` TEXT COLLATE NOCASE,
  `ObjectiveText3` TEXT COLLATE NOCASE,
  `ObjectiveText4` TEXT COLLATE NOCASE,
  `ReqItemId1` INTEGER NOT NULL DEFAULT 0,
  `ReqItemId2` INTEGER NOT NULL DEFAULT 0,
  `ReqItemId3` INTEGER NOT NULL DEFAULT 0,
  `ReqItemId4` INTEGER NOT NULL DEFAULT 0,
  `ReqItemCount1` INTEGER NOT NULL DEFAULT 0,
  `ReqItemCount2` INTEGER NOT NULL DEFAULT 0,
  `ReqItemCount3` INTEGER NOT NULL DEFAULT 0,
  `ReqItemCount4` INTEGER NOT NULL DEFAULT 0,
  `ReqSourceId1` INTEGER NOT NULL DEFAULT 0,
  `ReqSourceId2` INTEGER NOT NULL DEFAULT 0,
  `ReqSourceId3` INTEGER NOT NULL DEFAULT 0,
  `ReqSourceId4` INTEGER NOT NULL DEFAULT 0,
  `ReqSourceCount1` INTEGER NOT NULL DEFAULT 0,
  `ReqSourceCount2` INTEGER NOT NULL DEFAULT 0,
  `ReqSourceCount3` INTEGER NOT NULL DEFAULT 0,
  `ReqSourceCount4` INTEGER NOT NULL DEFAULT 0,
  `ReqCreatureOrGOId1` INTEGER NOT NULL DEFAULT 0,
  `ReqCreatureOrGOId2` INTEGER NOT NULL DEFAULT 0,
  `ReqCreatureOrGOId3` INTEGER NOT NULL DEFAULT 0,
  `ReqCreatureOrGOId4` INTEGER NOT NULL DEFAULT 0,
  `ReqCreatureOrGOCount1` INTEGER NOT NULL DEFAULT 0,
  `ReqCreatureOrGOCount2` INTEGER NOT NULL DEFAULT 0,
  `ReqCreatureOrGOCount3` INTEGER NOT NULL DEFAULT 0,
  `ReqCreatureOrGOCount4` INTEGER NOT NULL DEFAULT 0,
  `ReqSpellCast1` INTEGER NOT NULL DEFAULT 0,
  `ReqSpellCast2` INTEGER NOT NULL DEFAULT 0,
  `ReqSpellCast3` INTEGER NOT NULL DEFAULT 0,
  `ReqSpellCast4` INTEGER NOT NULL DEFAULT 0,
  `RewChoiceItemId1` INTEGER NOT NULL DEFAULT 0,
  `RewChoiceItemId2` INTEGER NOT NULL DEFAULT 0,
  `RewChoiceItemId3` INTEGER NOT NULL DEFAULT 0,
  `RewChoiceItemId4` INTEGER NOT NULL DEFAULT 0,
  `RewChoiceItemId5` INTEGER NOT NULL DEFAULT 0,
  `RewChoiceItemId6` INTEGER NOT NULL DEFAULT 0,
  `RewChoiceItemCount1` INTEGER NOT NULL DEFAULT 0,
  `RewChoiceItemCount2` INTEGER NOT NULL DEFAULT 0,
  `RewChoiceItemCount3` INTEGER NOT NULL DEFAULT 0,
  `RewChoiceItemCount4` INTEGER NOT NULL DEFAULT 0,
  `RewChoiceItemCount5` INTEGER NOT NULL DEFAULT 0,
  `RewChoiceItemCount6` INTEGER NOT NULL DEFAULT 0,
  `RewItemId1` INTEGER NOT NULL DEFAULT 0,
  `RewItemId2` INTEGER NOT NULL DEFAULT 0,
  `RewItemId3` INTEGER NOT NULL DEFAULT 0,
  `RewItemId4` INTEGER NOT NULL DEFAULT 0,
  `RewItemCount1` INTEGER NOT NULL DEFAULT 0,
  `RewItemCount2` INTEGER NOT NULL DEFAULT 0,
  `RewItemCount3` INTEGER NOT NULL DEFAULT 0,
  `RewItemCount4` INTEGER NOT NULL DEFAULT 0,
  `RewRepFaction1` INTEGER NOT NULL DEFAULT 0,
  `RewRepFaction2` INTEGER NOT NULL DEFAULT 0,
  `RewRepFaction3` INTEGER NOT NULL DEFAULT 0,
  `RewRepFaction4` INTEGER NOT NULL DEFAULT 0,
  `RewRepFaction5` INTEGER NOT NULL DEFAULT 0,
  `RewRepValue1` INTEGER NOT NULL DEFAULT 0,
  `RewRepValue2` INTEGER NOT NULL DEFAULT 0,
  `RewRepValue3` INTEGER NOT NULL DEFAULT 0,
  `RewRepValue4` INTEGER NOT NULL DEFAULT 0,
  `RewRepValue5` INTEGER NOT NULL DEFAULT 0,
  `RewRepSpilloverMask` INTEGER NOT NULL DEFAULT 0,
  `RewXP` INTEGER NOT NULL DEFAULT 0,
  `RewOrReqMoney` INTEGER NOT NULL DEFAULT 0,
  `RewMoneyMaxLevel` INTEGER NOT NULL DEFAULT 0,
  `RewSpell` INTEGER NOT NULL DEFAULT 0,
  `RewSpellCast` INTEGER NOT NULL DEFAULT 0,
  `RewMailTemplateId` INTEGER NOT NULL DEFAULT 0,
  `RewMailDelaySecs` INTEGER NOT NULL DEFAULT 0,
  `RewMailMoney` INTEGER NOT NULL DEFAULT 0,
  `PointMapId` INTEGER NOT NULL DEFAULT 0,
  `PointX` REAL NOT NULL DEFAULT 0,
  `PointY` REAL NOT NULL DEFAULT 0,
  `PointOpt` INTEGER NOT NULL DEFAULT 0,
  `DetailsEmote1` INTEGER NOT NULL DEFAULT 0,
  `DetailsEmote2` INTEGER NOT NULL DEFAULT 0,
  `DetailsEmote3` INTEGER NOT NULL DEFAULT 0,
  `DetailsEmote4` INTEGER NOT NULL DEFAULT 0,
  `DetailsEmoteDelay1` INTEGER NOT NULL DEFAULT 0,
  `DetailsEmoteDelay2` INTEGER NOT NULL DEFAULT 0,
  `DetailsEmoteDelay3` INTEGER NOT NULL DEFAULT 0,
  `DetailsEmoteDelay4` INTEGER NOT NULL DEFAULT 0,
  `IncompleteEmote` INTEGER NOT NULL DEFAULT 0,
  `CompleteEmote` INTEGER NOT NULL DEFAULT 0,
  `OfferRewardEmote1` INTEGER NOT NULL DEFAULT 0,
  `OfferRewardEmote2` INTEGER NOT NULL DEFAULT 0,
  `OfferRewardEmote3` INTEGER NOT NULL DEFAULT 0,
  `OfferRewardEmote4` INTEGER NOT NULL DEFAULT 0,
  `OfferRewardEmoteDelay1` INTEGER NOT NULL DEFAULT 0,
  `OfferRewardEmoteDelay2` INTEGER NOT NULL DEFAULT 0,
  `OfferRewardEmoteDelay3` INTEGER NOT NULL DEFAULT 0,
  `OfferRewardEmoteDelay4` INTEGER NOT NULL DEFAULT 0,
  `StartScript` INTEGER NOT NULL DEFAULT 0,
  `CompleteScript` INTEGER NOT NULL DEFAULT 0,
  PRIMARY KEY (`entry`, `patch`)
);

INSERT INTO `quest_template__new` (`entry`, `patch`, `Method`, `ZoneOrSort`, `MinLevel`, `MaxLevel`, `QuestLevel`, `Type`, `RequiredClasses`, `RequiredRaces`, `RequiredSkill`, `RequiredSkillValue`, `RequiredCondition`, `RepObjectiveFaction`, `RepObjectiveValue`, `RequiredMinRepFaction`, `RequiredMinRepValue`, `RequiredMaxRepFaction`, `RequiredMaxRepValue`, `SuggestedPlayers`, `LimitTime`, `QuestFlags`, `SpecialFlags`, `PrevQuestId`, `NextQuestId`, `ExclusiveGroup`, `BreadcrumbForQuestId`, `NextQuestInChain`, `SrcItemId`, `SrcItemCount`, `SrcSpell`, `Title`, `Details`, `Objectives`, `OfferRewardText`, `RequestItemsText`, `EndText`, `ObjectiveText1`, `ObjectiveText2`, `ObjectiveText3`, `ObjectiveText4`, `ReqItemId1`, `ReqItemId2`, `ReqItemId3`, `ReqItemId4`, `ReqItemCount1`, `ReqItemCount2`, `ReqItemCount3`, `ReqItemCount4`, `ReqSourceId1`, `ReqSourceId2`, `ReqSourceId3`, `ReqSourceId4`, `ReqSourceCount1`, `ReqSourceCount2`, `ReqSourceCount3`, `ReqSourceCount4`, `ReqCreatureOrGOId1`, `ReqCreatureOrGOId2`, `ReqCreatureOrGOId3`, `ReqCreatureOrGOId4`, `ReqCreatureOrGOCount1`, `ReqCreatureOrGOCount2`, `ReqCreatureOrGOCount3`, `ReqCreatureOrGOCount4`, `ReqSpellCast1`, `ReqSpellCast2`, `ReqSpellCast3`, `ReqSpellCast4`, `RewChoiceItemId1`, `RewChoiceItemId2`, `RewChoiceItemId3`, `RewChoiceItemId4`, `RewChoiceItemId5`, `RewChoiceItemId6`, `RewChoiceItemCount1`, `RewChoiceItemCount2`, `RewChoiceItemCount3`, `RewChoiceItemCount4`, `RewChoiceItemCount5`, `RewChoiceItemCount6`, `RewItemId1`, `RewItemId2`, `RewItemId3`, `RewItemId4`, `RewItemCount1`, `RewItemCount2`, `RewItemCount3`, `RewItemCount4`, `RewRepFaction1`, `RewRepFaction2`, `RewRepFaction3`, `RewRepFaction4`, `RewRepFaction5`, `RewRepValue1`, `RewRepValue2`, `RewRepValue3`, `RewRepValue4`, `RewRepValue5`, `RewRepSpilloverMask`, `RewXP`, `RewOrReqMoney`, `RewMoneyMaxLevel`, `RewSpell`, `RewSpellCast`, `RewMailTemplateId`, `RewMailDelaySecs`, `RewMailMoney`, `PointMapId`, `PointX`, `PointY`, `PointOpt`, `DetailsEmote1`, `DetailsEmote2`, `DetailsEmote3`, `DetailsEmote4`, `DetailsEmoteDelay1`, `DetailsEmoteDelay2`, `DetailsEmoteDelay3`, `DetailsEmoteDelay4`, `IncompleteEmote`, `CompleteEmote`, `OfferRewardEmote1`, `OfferRewardEmote2`, `OfferRewardEmote3`, `OfferRewardEmote4`, `OfferRewardEmoteDelay1`, `OfferRewardEmoteDelay2`, `OfferRewardEmoteDelay3`, `OfferRewardEmoteDelay4`, `StartScript`, `CompleteScript`)
SELECT `entry`, `patch`, `Method`, `ZoneOrSort`, `MinLevel`, `MaxLevel`, `QuestLevel`, `Type`, `RequiredClasses`, `RequiredRaces`, `RequiredSkill`, `RequiredSkillValue`, `RequiredCondition`, `RepObjectiveFaction`, `RepObjectiveValue`, `RequiredMinRepFaction`, `RequiredMinRepValue`, `RequiredMaxRepFaction`, `RequiredMaxRepValue`, `SuggestedPlayers`, `LimitTime`, `QuestFlags`, `SpecialFlags`, `PrevQuestId`, `NextQuestId`, `ExclusiveGroup`, `BreadcrumbForQuestId`, `NextQuestInChain`, `SrcItemId`, `SrcItemCount`, `SrcSpell`, `Title`, `Details`, `Objectives`, `OfferRewardText`, `RequestItemsText`, `EndText`, `ObjectiveText1`, `ObjectiveText2`, `ObjectiveText3`, `ObjectiveText4`, `ReqItemId1`, `ReqItemId2`, `ReqItemId3`, `ReqItemId4`, `ReqItemCount1`, `ReqItemCount2`, `ReqItemCount3`, `ReqItemCount4`, `ReqSourceId1`, `ReqSourceId2`, `ReqSourceId3`, `ReqSourceId4`, `ReqSourceCount1`, `ReqSourceCount2`, `ReqSourceCount3`, `ReqSourceCount4`, `ReqCreatureOrGOId1`, `ReqCreatureOrGOId2`, `ReqCreatureOrGOId3`, `ReqCreatureOrGOId4`, `ReqCreatureOrGOCount1`, `ReqCreatureOrGOCount2`, `ReqCreatureOrGOCount3`, `ReqCreatureOrGOCount4`, `ReqSpellCast1`, `ReqSpellCast2`, `ReqSpellCast3`, `ReqSpellCast4`, `RewChoiceItemId1`, `RewChoiceItemId2`, `RewChoiceItemId3`, `RewChoiceItemId4`, `RewChoiceItemId5`, `RewChoiceItemId6`, `RewChoiceItemCount1`, `RewChoiceItemCount2`, `RewChoiceItemCount3`, `RewChoiceItemCount4`, `RewChoiceItemCount5`, `RewChoiceItemCount6`, `RewItemId1`, `RewItemId2`, `RewItemId3`, `RewItemId4`, `RewItemCount1`, `RewItemCount2`, `RewItemCount3`, `RewItemCount4`, `RewRepFaction1`, `RewRepFaction2`, `RewRepFaction3`, `RewRepFaction4`, `RewRepFaction5`, `RewRepValue1`, `RewRepValue2`, `RewRepValue3`, `RewRepValue4`, `RewRepValue5`, 0, `RewXP`, `RewOrReqMoney`, `RewMoneyMaxLevel`, `RewSpell`, `RewSpellCast`, `RewMailTemplateId`, `RewMailDelaySecs`, `RewMailMoney`, `PointMapId`, `PointX`, `PointY`, `PointOpt`, `DetailsEmote1`, `DetailsEmote2`, `DetailsEmote3`, `DetailsEmote4`, `DetailsEmoteDelay1`, `DetailsEmoteDelay2`, `DetailsEmoteDelay3`, `DetailsEmoteDelay4`, `IncompleteEmote`, `CompleteEmote`, `OfferRewardEmote1`, `OfferRewardEmote2`, `OfferRewardEmote3`, `OfferRewardEmote4`, `OfferRewardEmoteDelay1`, `OfferRewardEmoteDelay2`, `OfferRewardEmoteDelay3`, `OfferRewardEmoteDelay4`, `StartScript`, `CompleteScript` FROM `quest_template`;

DROP TABLE `quest_template`;
ALTER TABLE `quest_template__new` RENAME TO `quest_template`;

-- Add data from classic 1.13 brute.
 UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =6;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =11;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =15;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =18;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =21;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =33;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =35;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =37;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =39;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =40;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =45;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =46;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =47;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =52;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =54;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =59;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =60;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =61;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =62;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =71;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =76;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =83;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =84;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =85;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =86;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =87;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =88;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =106;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =107;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =109;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =111;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =112;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =114;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =123;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =147;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =163;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =170;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =176;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =179;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =182;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =183;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =184;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =218;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =233;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =234;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =239;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =282;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =287;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =291;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =310;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =311;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =312;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =313;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =314;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =315;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =317;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =318;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =319;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =320;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =354;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =355;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =356;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =358;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =359;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =360;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =361;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =362;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =363;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =364;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =365;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =367;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =368;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =369;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =370;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =371;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =372;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =374;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =375;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =376;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =380;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =381;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =382;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =383;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =384;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =398;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =400;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =404;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =405;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =407;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =408;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =409;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =411;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =412;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =413;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =414;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =415;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =417;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =419;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =420;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =426;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =427;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =432;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =433;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =445;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =456;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =457;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =458;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =459;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =468;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =473;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =475;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =476;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =483;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =486;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =487;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =489;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =492;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =590;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =674;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =675;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =743;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =745;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =746;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =747;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =748;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =749;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =750;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =751;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =752;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =753;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =754;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =755;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =756;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =757;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =758;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =759;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =760;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =761;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =763;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =764;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =765;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =766;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =767;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =770;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =771;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =772;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =773;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =775;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =776;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =780;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =781;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =783;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =784;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =785;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =786;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =787;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =788;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =789;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =790;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =791;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =792;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =794;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =804;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =805;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =806;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =808;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =809;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =812;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =815;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =816;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =817;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =818;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =823;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =825;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =826;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =827;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =828;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =829;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =830;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =831;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =832;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =833;

UPDATE `quest_template` SET `RewRepSpilloverMask` =7 WHERE `entry` =834;

UPDATE `quest_template` SET `RewRepSpilloverMask` =7 WHERE `entry` =835;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =837;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =840;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =842;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =854;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =861;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =916;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =917;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =918;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =919;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =920;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =921;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =922;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =927;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =928;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =929;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =930;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =931;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =932;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =933;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =935;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =937;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =938;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =940;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =941;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =952;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =970;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =973;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =976;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =997;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =1021;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =1031;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =1032;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =1049;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =1056;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =1070;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =1075;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =1076;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =1078;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =1085;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =1140;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =1167;

UPDATE `quest_template` SET `RewRepSpilloverMask` =2 WHERE `entry` =1198;

UPDATE `quest_template` SET `RewRepSpilloverMask` =2 WHERE `entry` =1199;

UPDATE `quest_template` SET `RewRepSpilloverMask` =2 WHERE `entry` =1200;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =1372;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =1383;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =1423;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =1470;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =1499;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =1581;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =1598;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =1599;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =1656;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =1657;

UPDATE `quest_template` SET `RewRepSpilloverMask` =2 WHERE `entry` =2040;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =2158;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =2159;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =2160;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =2161;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =2383;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =2399;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =2438;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =2459;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =2498;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =2499;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =2518;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =2519;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =2520;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =2541;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =2561;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =2747;

UPDATE `quest_template` SET `RewRepSpilloverMask` =2 WHERE `entry` =2928;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =2948;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =2973;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =2974;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =2976;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3065;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3082;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3083;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3084;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3085;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3086;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3087;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3088;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3089;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3090;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3091;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3092;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3093;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3094;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3095;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3096;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3097;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3098;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3099;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3100;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3101;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3102;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3103;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3104;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3105;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3106;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3107;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3108;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3109;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3110;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3112;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3113;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3114;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3115;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3116;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3117;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3118;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3119;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3120;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =3361;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =3364;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =3365;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3376;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3519;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3521;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3522;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3741;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3792;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3803;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3804;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3901;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3902;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3903;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3904;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =3905;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =4161;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =4402;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =4495;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =4641;

UPDATE `quest_template` SET `RewRepSpilloverMask` =15 WHERE `entry` =4974;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =5261;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =5441;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =5481;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =5482;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =5541;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =5881;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =6181;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =6261;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =6281;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =6285;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =6321;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =6322;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =6323;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =6324;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =6341;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =6342;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =6343;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =6344;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =6361;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =6362;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =6363;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =6364;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =6365;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =6384;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =6385;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =6386;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =6387;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =6388;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =6391;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =6392;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =6394;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =6395;

UPDATE `quest_template` SET `RewRepSpilloverMask` =2 WHERE `entry` =6561;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7383;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7701;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7791;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7792;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7793;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7794;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7795;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7796;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7798;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7799;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7800;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7801;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7802;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7803;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7804;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7805;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7806;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7807;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7808;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7809;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7811;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7812;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7813;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7814;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7817;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7818;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7819;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7820;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7821;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7822;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7823;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7824;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7825;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7826;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7827;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7831;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7832;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7833;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7834;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7835;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7836;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7837;

UPDATE `quest_template` SET `RewRepSpilloverMask` =15 WHERE `entry` =7871;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7907;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7927;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7928;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7929;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8156;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8161;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8241;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8242;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =8249;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8308;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8318;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8811;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8812;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8813;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8814;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8815;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8816;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8817;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8818;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8819;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8820;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8821;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8822;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8823;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8824;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8825;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8826;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8830;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8831;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8832;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8833;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8834;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8835;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8836;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8837;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8838;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8839;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8840;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8841;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8842;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8843;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8844;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8845;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8856;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8867;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8873;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8874;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8875;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9121;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9124;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9126;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9128;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9131;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9136;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9141;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9165;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9178;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9179;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9181;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9182;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9183;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9184;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9185;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9186;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9187;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9188;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9190;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9191;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9194;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9195;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9196;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9197;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9198;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9200;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9201;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9202;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9203;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9204;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9205;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9206;

-- Add data from 1.13 mass parse for quests that dont appear in the brute.
 UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =934;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =7893;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8049;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8051;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8052;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8101;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8116;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8123;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8160;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8162;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8185;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8188;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8189;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8191;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8286;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8288;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8290;

UPDATE `quest_template` SET `RewRepSpilloverMask` =15 WHERE `entry` =8294;

UPDATE `quest_template` SET `RewRepSpilloverMask` =15 WHERE `entry` =8295;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8299;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8301;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8304;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8306;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8320;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8331;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8332;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8341;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8343;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8348;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8361;

UPDATE `quest_template` SET `RewRepSpilloverMask` =7 WHERE `entry` =8367;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8368;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8369;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8370;

UPDATE `quest_template` SET `RewRepSpilloverMask` =7 WHERE `entry` =8371;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8372;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8374;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8375;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8383;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8384;

UPDATE `quest_template` SET `RewRepSpilloverMask` =7 WHERE `entry` =8385;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8386;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8387;

UPDATE `quest_template` SET `RewRepSpilloverMask` =7 WHERE `entry` =8388;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8389;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8390;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8391;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8392;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8393;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8397;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8398;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8399;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8400;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8401;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8402;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8403;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8404;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8405;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8406;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8408;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8414;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8416;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8418;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8426;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8427;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8428;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8429;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8430;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8431;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8432;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8433;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8434;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8437;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8439;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8440;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8442;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8443;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8446;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8497;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8498;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8501;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8534;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8535;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8536;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8537;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8539;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8540;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8548;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8556;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8558;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8562;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8572;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8573;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8575;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8593;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8594;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8596;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8603;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8622;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8627;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8629;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8630;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8631;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8638;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8655;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8656;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8687;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8697;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8698;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8699;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8700;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8701;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8702;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8703;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8704;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8705;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8706;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8707;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8708;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8709;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8710;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8711;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8712;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8728;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8730;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =8733;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =8735;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8737;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8738;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8739;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8740;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8747;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8752;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8756;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8757;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8761;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8770;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8771;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8772;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8774;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8775;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8776;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8777;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8778;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8779;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8780;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8781;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8782;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8783;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8785;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8786;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8787;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8789;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8790;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =8791;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8800;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =8801;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8802;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8804;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8805;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8806;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8807;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8808;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8809;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8829;

UPDATE `quest_template` SET `RewRepSpilloverMask` =15 WHERE `entry` =8858;

UPDATE `quest_template` SET `RewRepSpilloverMask` =15 WHERE `entry` =8859;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8871;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =8872;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9033;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9034;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9036;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9037;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9038;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9039;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9040;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9041;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9042;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9043;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9044;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9045;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9046;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9047;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9048;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9049;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9050;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9054;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9055;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9056;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9057;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9058;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9060;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9061;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9068;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9069;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9072;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9073;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9074;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9075;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9077;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9078;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9079;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9080;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9081;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9095;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9096;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9097;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9098;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9100;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9101;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9102;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9104;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9106;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9108;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9111;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9112;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9113;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9114;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9115;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9116;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9117;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9118;

UPDATE `quest_template` SET `RewRepSpilloverMask` =15 WHERE `entry` =9120;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9122;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9123;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9125;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9127;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9137;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9142;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9208;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9209;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9210;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9211;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9213;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9221;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9222;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9223;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9224;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9225;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9226;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9227;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9228;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9229;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9230;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9232;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9248;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =9250;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =9251;

UPDATE `quest_template` SET `RewRepSpilloverMask` =3 WHERE `entry` =9269;

UPDATE `quest_template` SET `RewRepSpilloverMask` =1 WHERE `entry` =9338;

-- End of migration.
;
