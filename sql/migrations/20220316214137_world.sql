INSERT INTO `migrations` VALUES ('20220316214137');

-- Add your query below.
 -- Change the war effort stage condition into a generic saved variable check.
 UPDATE `conditions` SET `value3` = `value2`, `value2` = `value1`, `value1` =30050 WHERE `type` =11;

-- End of migration.
;
