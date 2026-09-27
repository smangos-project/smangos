INSERT INTO `migrations` VALUES ('20260519041719');

-- Add your query below.
 -- Special bags should have bag family assigned.
 UPDATE `item_template` SET `bag_family` =3 WHERE `class` =1 AND `subclass` =1;

UPDATE `item_template` SET `bag_family` =6 WHERE `class` =1 AND `subclass` =2;

UPDATE `item_template` SET `bag_family` =7 WHERE `class` =1 AND `subclass` =3;

UPDATE `item_template` SET `bag_family` =1 WHERE `class` =11 AND `subclass` =2;

UPDATE `item_template` SET `bag_family` =2 WHERE `class` =11 AND `subclass` =3;

-- End of migration.
;
