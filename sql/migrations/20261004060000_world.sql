INSERT INTO `migrations` VALUES ('20261004060000');

UPDATE `dbc_spellmisc` SET `SchoolMask` = `SchoolMask` & 255 WHERE `SchoolMask` > 255;
