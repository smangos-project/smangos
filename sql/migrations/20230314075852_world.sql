INSERT INTO `migrations` VALUES ('20230314075852');

-- Add your query below.
 -- Correct stats of a few humanoids.
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =4668;

-- Burning Blade Summoner (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =4705;

-- Burning Blade Invoker (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =4814;

-- Twilight Elementalist (was 0.16)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4668 AND `patch` IN (0);

-- Burning Blade Summoner (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7727 AND `patch` IN (0);

-- Grimtotem Shaman (was 0.95)
 -- End of migration.
;
