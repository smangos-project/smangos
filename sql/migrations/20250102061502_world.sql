INSERT INTO `migrations` VALUES ('20250102061502');

-- Add your query below.
 UPDATE `quest_template` SET `RequestItemsText` = 'I must not interfere, $R.' WHERE `entry` =9269 AND `patch` =9;

UPDATE `quest_template` SET `RequestItemsText` = 'I must not interfere, $R.' WHERE `entry` =9270 AND `patch` =9;

UPDATE `quest_template` SET `RequestItemsText` = 'I must not interfere, $R.' WHERE `entry` =9271 AND `patch` =9;

UPDATE `quest_template` SET `RequestItemsText` = 'I must not interfere, $R.' WHERE `entry` =9257 AND `patch` =9;

UPDATE `quest_template` SET `OfferRewardText` = 'The magnitude of this accomplishment must not be understated, $N. You have done what most would have thought to be impossible. Alas, it was fated. The staff has made its choice...' WHERE `entry` =9269 AND `patch` =9;

UPDATE `quest_template` SET `OfferRewardText` = 'The magnitude of this accomplishment must not be understated, $N. You have done what most would have thought to be impossible. Alas, it was fated. The staff has made its choice...' WHERE `entry` =9270 AND `patch` =9;

UPDATE `quest_template` SET `OfferRewardText` = 'The magnitude of this accomplishment must not be understated, $N. You have done what most would have thought to be impossible. Alas, it was fated. The staff has made its choice...' WHERE `entry` =9271 AND `patch` =9;

UPDATE `quest_template` SET `OfferRewardText` = 'The magnitude of this accomplishment must not be understated, $N. You have done what most would have thought to be impossible. Alas, it was fated. The staff has made its choice...' WHERE `entry` =9257 AND `patch` =9;

-- End of migration.
;
