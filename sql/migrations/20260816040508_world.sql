INSERT INTO `migrations` VALUES ('20260816040508');

-- Add your query below.
 -- Correct Black Guard Swordsmith position to sniff
 UPDATE `gameobject_scripts` SET `x` =3820.96142578125, `y` =-3707.0078125, `z` =143.568634033203125, `o` =1.256637096405029296 WHERE `id` =42959;

-- End of migration.
;
