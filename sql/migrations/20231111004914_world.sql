INSERT INTO `migrations` VALUES ('20231111004914');

-- Add your query below.
 -- Damage Variance
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =3104;

-- Makrura Shellhide (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =3111;

-- Razormane Quilboar (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =3119;

-- Kolkar Drudge (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.14 WHERE `entry` =3129;

-- Kul Tiras Marine (was 0.13)
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =3204;

-- Gazz'uz (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.12 WHERE `entry` =3293;

-- Rezlak (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =3989;

-- Venture Co. Logger (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4022;

-- Bloodfury Harpy (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4026;

-- Bloodfury Windcaller (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4051;

-- Cenarion Botanist (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4053;

-- Daughter of Cenarius (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4057;

-- Son of Cenarius (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =4658;

-- Maraudine Stormer (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.14 WHERE `entry` =5786;

-- Snagglespear (was 0.1)
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =5931;

-- Foreman Rigger (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =6199;

-- Blood Elf Reclaimer (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =6370;

-- Makrinni Scrabbler (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =11685;

-- Maraudine Priest (was 0.14)
 -- Damage Multipliers
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3103 AND `patch` IN (0);

-- Makrura Clacker (was 0.85)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3104 AND `patch` IN (0);

-- Makrura Shellhide (was 0.85)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3105 AND `patch` IN (0);

-- Makrura Snapclaw (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3111 AND `patch` IN (0);

-- Razormane Quilboar (was 0.85)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3112 AND `patch` IN (0);

-- Razormane Scout (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3115 AND `patch` IN (0);

-- Dustwind Harpy (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3116 AND `patch` IN (0);

-- Dustwind Pillager (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3118 AND `patch` IN (0);

-- Dustwind Storm Witch (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3119 AND `patch` IN (0);

-- Kolkar Drudge (was 0.85)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3141 AND `patch` IN (0);

-- Makrura Elder (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1.86 WHERE `entry` =3195 AND `patch` IN (0);

-- Burning Blade Thug (was 1.75)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3196 AND `patch` IN (0);

-- Burning Blade Neophyte (was 0.9)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3197 AND `patch` IN (0);

-- Burning Blade Fanatic (was 0.9)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3205 AND `patch` IN (0);

-- Zalazane (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3207 AND `patch` IN (0);

-- Hexed Troll (was 0.9)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3293 AND `patch` IN (0);

-- Rezlak (was 1.2)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4053 AND `patch` IN (0);

-- Daughter of Cenarius (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4658 AND `patch` IN (0);

-- Maraudine Stormer (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5345 AND `patch` IN (0);

-- Diamond Head (was 3.15)
 UPDATE `creature_template` SET `damage_multiplier` =2.625 WHERE `entry` =5786 AND `patch` IN (0);

-- Snagglespear (was 2.5)
 UPDATE `creature_template` SET `damage_multiplier` =2.88 WHERE `entry` =5931 AND `patch` IN (0);

-- Foreman Rigger (was 3.2)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6020 AND `patch` IN (0);

-- Slimeshell Makrura (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6351 AND `patch` IN (0);

-- Storm Bay Oracle (was 1.2)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6370 AND `patch` IN (0);

-- Makrinni Scrabbler (was 1.15)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6371 AND `patch` IN (0);

-- Storm Bay Warrior (was 1.2)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6372 AND `patch` IN (0);

-- Makrinni Snapclaw (was 0.15)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8420 AND `patch` IN (0);

-- Kim'jael (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11276 AND `patch` IN (0, 5);

-- Azshara Sentinel (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11752 AND `patch` IN (0);

-- Blaise Montgomery (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =12576 AND `patch` IN (0);

-- Grish Longrunner (was 0.9)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14301 AND `patch` IN (1);

-- Brinna Valanaar (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14373 AND `patch` IN (1);

-- Sage Korolusk (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14479 AND `patch` IN (2);

-- Twilight Lord Everun (was 5.6)
 -- Armor Multipliers
 UPDATE `creature_template` SET `armor_multiplier` =1.5 WHERE `entry` =3103 AND `patch` IN (0);

-- Makrura Clacker (was 1.3)
 UPDATE `creature_template` SET `armor_multiplier` =1.5 WHERE `entry` =3104 AND `patch` IN (0);

-- Makrura Shellhide (was 1.1)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3111 AND `patch` IN (0);

-- Razormane Quilboar (was 0.85)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3112 AND `patch` IN (0);

-- Razormane Scout (was 0.85)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3113 AND `patch` IN (0);

-- Razormane Dustrunner (was 0.8)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3114 AND `patch` IN (0);

-- Razormane Battleguard (was 0.9)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3115 AND `patch` IN (0);

-- Dustwind Harpy (was 0.9)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3116 AND `patch` IN (0);

-- Dustwind Pillager (was 0.9)
 UPDATE `creature_template` SET `armor_multiplier` =0.4 WHERE `entry` =3117 AND `patch` IN (0);

-- Dustwind Savage (was 0.35)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3119 AND `patch` IN (0);

-- Kolkar Drudge (was 0.8)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3205 AND `patch` IN (0);

-- Zalazane (was 0.9)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3206 AND `patch` IN (0);

-- Voodoo Troll (was 0.9)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3207 AND `patch` IN (0);

-- Hexed Troll (was 0.85)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5797 AND `patch` IN (0);

-- Aean Swiftriver (was 0.9)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5800 AND `patch` IN (0);

-- Marcus Bel (was 0.95)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =6371 AND `patch` IN (0);

-- Storm Bay Warrior (was 0.85)
 UPDATE `creature_template` SET `armor_multiplier` =1.5 WHERE `entry` =6372 AND `patch` IN (0);

-- Makrinni Snapclaw (was 0.15)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11752 AND `patch` IN (0);

-- Blaise Montgomery (was 0.8)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =14479 AND `patch` IN (2);

-- Twilight Lord Everun (was 1.1)
 -- nature resistance
 UPDATE `creature_template` SET `nature_res` =48 WHERE `entry` =4051 AND `nature_res` =0;

-- Cenarion Botanist
 -- arcane resistance
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =4053 AND `arcane_res` =25;

-- Daughter of Cenarius
 -- End of migration.
;
