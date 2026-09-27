INSERT INTO `migrations` VALUES ('20240117173840');

-- Add your query below.
 -- Fix requirements for Shard of a Felhound and Shard of an Infernal.
 UPDATE `quest_template` SET `RequiredCondition` =1799, `PrevQuestId` =0 WHERE `entry` IN (4962, 4963);

-- End of migration.
;
