INSERT INTO `migrations` VALUES ('20230210082850');

-- Add your query below.
 -- Damage Variance
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4073;

-- XT:4 (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4074;

-- XT:9 (was 0.14)
 -- Damage Multipliers
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =114 AND `patch` IN (0);

-- Harvest Watcher (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =573 AND `patch` IN (0);

-- Foe Reaper 4000 (was 3.25)
 UPDATE `creature_template` SET `damage_multiplier` =3.3 WHERE `entry` =642 AND `patch` IN (0);

-- Sneed's Shredder (was 2.7)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4073 AND `patch` IN (0);

-- XT:4 (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4074 AND `patch` IN (0);

-- XT:9 (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1.8 WHERE `entry` =4872 AND `patch` IN (0);

-- Obsidian Golem (was 3.75)
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =6225 AND `patch` IN (0);

-- Mechano-Tank (was 3.25)
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =6226 AND `patch` IN (0);

-- Mechano-Flamewalker (was 4.4)
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =6227 AND `patch` IN (0);

-- Mechano-Frostwalker (was 4.6)
 UPDATE `creature_template` SET `damage_multiplier` =2.4 WHERE `entry` =6229 AND `patch` IN (0);

-- Crowd Pummeler 9-60 (was 4.25)
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =6230 AND `patch` IN (0);

-- Peacekeeper Security Suit (was 4.4)
 UPDATE `creature_template` SET `damage_multiplier` =2.4 WHERE `entry` =6232 AND `patch` IN (0);

-- Arcane Nullifier X-21 (was 4.15)
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =6233 AND `patch` IN (0);

-- Mechanized Sentry (was 3.1)
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =6234 AND `patch` IN (0);

-- Mechanized Guardian (was 4.25)
 UPDATE `creature_template` SET `damage_multiplier` =2.4 WHERE `entry` =6235 AND `patch` IN (0);

-- Electrocutioner 6000 (was 4.25)
 UPDATE `creature_template` SET `damage_multiplier` =3.5 WHERE `entry` =7023 AND `patch` IN (0);

-- Obsidian Sentinel (was 4.25)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7166 AND `patch` IN (0);

-- Wrenix's Gizmotronic Apparatus (was 0.85)
 UPDATE `creature_template` SET `damage_multiplier` =2.4 WHERE `entry` =7800 AND `patch` IN (0);

-- Mekgineer Thermaplugg (was 4.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7849 AND `patch` IN (0);

-- Mobile Alert System (was 1.3)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7915 AND `patch` IN (0);

-- Walking Bomb (was 1.45)
 UPDATE `creature_template` SET `damage_multiplier` =2.8 WHERE `entry` =8447 AND `patch` IN (0);

-- Clunk (was 3.3)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8856 AND `patch` IN (0);

-- Tyrion's Spybot (was 0.9)
 UPDATE `creature_template` SET `damage_multiplier` =6 WHERE `entry` =12337 AND `patch` IN (0);

-- Crimson Courier (was 5.55)
 UPDATE `creature_template` SET `damage_multiplier` =3.8 WHERE `entry` =13118 AND `patch` IN (0);

-- Crimson Bodyguard (was 3.25)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14224 AND `patch` IN (0);

-- 7:XT (was 3.7)
 -- Armor Multipliers
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =480 AND `patch` IN (0);

-- Rusty Harvest Golem (was 0.9)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4872 AND `patch` IN (0);

-- Obsidian Golem (was 0.9)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =6225 AND `patch` IN (0);

-- Mechano-Tank (was 0.95)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =6226 AND `patch` IN (0);

-- Mechano-Flamewalker (was 0)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =6227 AND `patch` IN (0);

-- Mechano-Frostwalker (was 0)
 UPDATE `creature_template` SET `armor_multiplier` =2 WHERE `entry` =6229 AND `patch` IN (0);

-- Crowd Pummeler 9-60 (was 1.35)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =6230 AND `patch` IN (0);

-- Peacekeeper Security Suit (was 0.9)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =6232 AND `patch` IN (0);

-- Arcane Nullifier X-21 (was 0.85)
 UPDATE `creature_template` SET `armor_multiplier` =1.5 WHERE `entry` =6233 AND `patch` IN (0);

-- Mechanized Sentry (was 0)
 UPDATE `creature_template` SET `armor_multiplier` =1.5 WHERE `entry` =6234 AND `patch` IN (0);

-- Mechanized Guardian (was 1.35)
 UPDATE `creature_template` SET `armor_multiplier` =1.5 WHERE `entry` =6235 AND `patch` IN (0);

-- Electrocutioner 6000 (was 1.35)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =7023 AND `patch` IN (0);

-- Obsidian Sentinel (was 0.9)
 UPDATE `creature_template` SET `armor_multiplier` =2 WHERE `entry` =7800 AND `patch` IN (0);

-- Mekgineer Thermaplugg (was 1.7)
 UPDATE `creature_template` SET `armor_multiplier` =2 WHERE `entry` =7849 AND `patch` IN (0);

-- Mobile Alert System (was 0)
 UPDATE `creature_template` SET `armor_multiplier` =2 WHERE `entry` =7915 AND `patch` IN (0);

-- Walking Bomb (was 1.8)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =8447 AND `patch` IN (0);

-- Clunk (was 0.9)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =8856 AND `patch` IN (0);

-- Tyrion's Spybot (was 0)
 -- fire resistance
 UPDATE `creature_template` SET `fire_res` =15 WHERE `entry` =6226 AND `fire_res` =0;

-- Mechano-Flamewalker
 -- frost resistance
 UPDATE `creature_template` SET `frost_res` =75 WHERE `entry` =6227 AND `frost_res` =0;

-- Mechano-Frostwalker
 -- End of migration.
;
