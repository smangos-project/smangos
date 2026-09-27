INSERT INTO `migrations` VALUES ('20230923124032');

-- Add your query below.
 -- Do not check effect index for Mother's Milk aura condition.
 UPDATE `conditions` SET `value2` =-1 WHERE `type` =1 AND `value1` =16468;

-- End of migration.
;
