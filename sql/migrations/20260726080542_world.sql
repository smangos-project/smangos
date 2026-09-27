INSERT INTO `migrations` VALUES ('20260726080542');

-- Add your query below.
 -- Correct position and movement of two critters.
 UPDATE `creature` SET `position_x` =2331.45, `position_y` =-2530.23, `position_z` =101.722, `orientation` =5.85511, `wander_distance` =15, `movement_type` =1 WHERE `guid` =32676;

UPDATE `creature` SET `position_x` =2287.69, `position_y` =313.116, `position_z` =35.2387, `orientation` =2.37553, `wander_distance` =15, `movement_type` =1 WHERE `guid` =44429;

-- End of migration.
;
