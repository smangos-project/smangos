INSERT INTO `migrations` VALUES ('20220629062443');

-- Add your query below.
 -- Correct base stats for some paladin levels.
 UPDATE `creature_classlevelstats` SET `melee_damage` = 33.00647062 WHERE `class` =2 AND `level` =21;

-- Seen 5 times.
 UPDATE `creature_classlevelstats` SET `melee_damage` = 67.49181572 WHERE `class` =2 AND `level` =42;

-- Seen 1 times.
 UPDATE `creature_classlevelstats` SET `melee_damage` = 99.62660217 WHERE `class` =2 AND `level` =54;

-- Seen 2 times.
 -- Correct base stats for some mage levels.
 UPDATE `creature_classlevelstats` SET `melee_damage` = 7.527945042 WHERE `class` =8 AND `level` =7;

-- Seen 1 times.
 UPDATE `creature_classlevelstats` SET `melee_damage` = 8.761404991 WHERE `class` =8 AND `level` =8;

-- Seen 1 times.
 UPDATE `creature_classlevelstats` SET `melee_damage` = 10.05632591 WHERE `class` =8 AND `level` =9;

-- Seen 5 times.
 UPDATE `creature_classlevelstats` SET `melee_damage` = 11.16280937 WHERE `class` =8 AND `level` =10;

-- Seen 6 times.
 UPDATE `creature_classlevelstats` SET `melee_damage` = 12.60544968 WHERE `class` =8 AND `level` =11;

-- Seen 4 times.
 UPDATE `creature_classlevelstats` SET `melee_damage` = 14.47480011 WHERE `class` =8 AND `level` =12;

-- Seen 4 times.
 UPDATE `creature_classlevelstats` SET `melee_damage` = 15.71164989 WHERE `class` =8 AND `level` =13;

-- Seen 2 times.
 UPDATE `creature_classlevelstats` SET `melee_damage` = 28.06985092 WHERE `class` =8 AND `level` =23;

-- Seen 1 times.
 UPDATE `creature_classlevelstats` SET `melee_damage` = 31.38304901 WHERE `class` =8 AND `level` =26;

-- Seen 2 times.
 UPDATE `creature_classlevelstats` SET `ranged_damage` = 4.708533951 WHERE `class` =8 AND `level` =5;

-- Seen 1 times.
 UPDATE `creature_classlevelstats` SET `ranged_damage` = 6.367605870 WHERE `class` =8 AND `level` =6;

-- Seen 1 times.
 UPDATE `creature_classlevelstats` SET `ranged_damage` = 8.177945137 WHERE `class` =8 AND `level` =7;

-- Seen 1 times.
 UPDATE `creature_classlevelstats` SET `ranged_damage` = 9.361405373 WHERE `class` =8 AND `level` =8;

-- Seen 1 times.
 UPDATE `creature_classlevelstats` SET `ranged_damage` = 10.59202957 WHERE `class` =8 AND `level` =9;

-- Seen 5 times.
 UPDATE `creature_classlevelstats` SET `ranged_damage` = 11.96280003 WHERE `class` =8 AND `level` =10;

-- Seen 6 times.
 UPDATE `creature_classlevelstats` SET `ranged_damage` = 13.33405018 WHERE `class` =8 AND `level` =11;

-- Seen 4 times.
 UPDATE `creature_classlevelstats` SET `ranged_damage` = 15.11769962 WHERE `class` =8 AND `level` =12;

-- Seen 4 times.
 UPDATE `creature_classlevelstats` SET `ranged_damage` = 16.25444984 WHERE `class` =8 AND `level` =13;

-- Seen 3 times.
 UPDATE `creature_classlevelstats` SET `ranged_damage` = 27.06985092 WHERE `class` =8 AND `level` =23;

-- Seen 2 times.
 UPDATE `creature_classlevelstats` SET `ranged_damage` = 30.09730148 WHERE `class` =8 AND `level` =26;

-- Seen 6 times.
 UPDATE `creature_classlevelstats` SET `armor` = 19 WHERE `class` =8 AND `level` =4;

-- Seen 1 times.
 UPDATE `creature_classlevelstats` SET `armor` = 111 WHERE `class` =8 AND `level` =7;

-- Seen 1 times.
 UPDATE `creature_classlevelstats` SET `armor` = 151 WHERE `class` =8 AND `level` =8;

-- Seen 1 times.
 UPDATE `creature_classlevelstats` SET `armor` = 198 WHERE `class` =8 AND `level` =9;

-- Seen 5 times.
 UPDATE `creature_classlevelstats` SET `armor` = 248 WHERE `class` =8 AND `level` =10;

-- Seen 6 times.
 UPDATE `creature_classlevelstats` SET `armor` = 264 WHERE `class` =8 AND `level` =11;

-- Seen 4 times.
 UPDATE `creature_classlevelstats` SET `armor` = 283 WHERE `class` =8 AND `level` =12;

-- Seen 4 times.
 UPDATE `creature_classlevelstats` SET `armor` = 303 WHERE `class` =8 AND `level` =13;

-- Seen 3 times.
 UPDATE `creature_classlevelstats` SET `armor` = 495 WHERE `class` =8 AND `level` =23;

-- Seen 2 times.
 UPDATE `creature_classlevelstats` SET `armor` = 552 WHERE `class` =8 AND `level` =26;

-- Seen 6 times.
 -- Damage Variance
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =6;

-- Kobold Vermin
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =30;

-- Forest Spider
 UPDATE `creature_template` SET `damage_variance` =0.1 WHERE `entry` =38;

-- Defias Thug
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =69;

-- Timber Wolf
 UPDATE `creature_template` SET `damage_variance` =0.1 WHERE `entry` =80;

-- Kobold Laborer
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =94;

-- Defias Cutpurse
 UPDATE `creature_template` SET `damage_variance` =0.12 WHERE `entry` =103;

-- Garrick Padfoot
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =113;

-- Stonetusk Boar
 UPDATE `creature_template` SET `damage_variance` =0.12 WHERE `entry` =241;

-- Remy "Two Times"
 UPDATE `creature_template` SET `damage_variance` =0.09 WHERE `entry` =257;

-- Kobold Worker
 UPDATE `creature_template` SET `damage_variance` =0.02 WHERE `entry` =299;

-- Young Wolf
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =397;

-- Morganth
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =436;

-- Blackrock Shadowcaster
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =474;

-- Defias Rogue Wizard
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =476;

-- Kobold Geomancer
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =525;

-- Mangy Wolf
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =619;

-- Defias Conjurer
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =704;

-- Ragged Timber Wolf
 UPDATE `creature_template` SET `damage_variance` =0.02 WHERE `entry` =705;

-- Ragged Young Wolf
 UPDATE `creature_template` SET `damage_variance` =0.09 WHERE `entry` =708;

-- Small Crag Boar
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =710;

-- Mosh'Ogg Spellcrafter
 UPDATE `creature_template` SET `damage_variance` =0.12 WHERE `entry` =808;

-- Grik'nir the Cold
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =881;

-- Surena Caledon
 UPDATE `creature_template` SET `damage_variance` =0.1 WHERE `entry` =946;

-- Frostmane Novice
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =1028;

-- Bluegill Muckdweller
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =1038;

-- Dragonmaw Shadowwarder
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =1057;

-- Dragonmaw Bonewarder
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =1124;

-- Frostmane Shadowcaster
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =1125;

-- Crag Boar
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =1128;

-- Young Black Bear
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =1129;

-- Black Bear
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =1165;

-- Stonesplinter Geomancer
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =1174;

-- Tunnel Rat Geomancer
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =1199;

-- Juvenile Snow Leopard
 UPDATE `creature_template` SET `damage_variance` =0.09 WHERE `entry` =1504;

-- Young Night Web Spider
 UPDATE `creature_template` SET `damage_variance` =0.1 WHERE `entry` =1505;

-- Night Web Spider
 UPDATE `creature_template` SET `damage_variance` =0.09 WHERE `entry` =1506;

-- Scarlet Convert
 UPDATE `creature_template` SET `damage_variance` =0.02 WHERE `entry` =1508;

-- Young Scavenger
 UPDATE `creature_template` SET `damage_variance` =0.09 WHERE `entry` =1509;

-- Ragged Scavenger
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =1512;

-- Duskbat
 UPDATE `creature_template` SET `damage_variance` =0.1 WHERE `entry` =1513;

-- Mangy Duskbat
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =1547;

-- Decrepit Darkhound
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =1562;

-- Bloodsail Mage
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =1564;

-- Bloodsail Warlock
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =1653;

-- Bloodsail Elder Magus
 UPDATE `creature_template` SET `damage_variance` =0.12 WHERE `entry` =1667;

-- Meven Korgal
 UPDATE `creature_template` SET `damage_variance` =0.12 WHERE `entry` =1688;

-- Night Web Matriarch
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =1726;

-- Defias Magician
 UPDATE `creature_template` SET `damage_variance` =0.12 WHERE `entry` =1733;

-- Zggi
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =1826;

-- Scarlet Mage
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =1832;

-- Scarlet Magus
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =1935;

-- Tirisfal Farmhand
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =1984;

-- Young Thistle Boar
 UPDATE `creature_template` SET `damage_variance` =0.09 WHERE `entry` =1985;

-- Thistle Boar
 UPDATE `creature_template` SET `damage_variance` =0.1 WHERE `entry` =1986;

-- Webwood Spider
 UPDATE `creature_template` SET `damage_variance` =0.09 WHERE `entry` =1988;

-- Grell
 UPDATE `creature_template` SET `damage_variance` =0.1 WHERE `entry` =1989;

-- Grellkin
 UPDATE `creature_template` SET `damage_variance` =0.12 WHERE `entry` =1994;

-- Githyiss the Vile
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =1995;

-- Strigid Owl
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =1998;

-- Webwood Lurker
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =2002;

-- Rascal Sprite
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =2003;

-- Shadow Sprite
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =2018;

-- Bloodfeather Sorceress
 UPDATE `creature_template` SET `damage_variance` =0.02 WHERE `entry` =2031;

-- Young Nightsaber
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =2032;

-- Mangy Nightsaber
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =2042;

-- Nightsaber
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =2182;

-- Stormscale Sorceress
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =2191;

-- Licillin
 UPDATE `creature_template` SET `damage_variance` =0.02 WHERE `entry` =2230;

-- Umpi
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =2244;

-- Syndicate Shadow Mage
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =2255;

-- Crushridge Mage
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =2306;

-- Baron Vardus
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =2318;

-- Argus Shadow Mage
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =2319;

-- Syndicate Wizard
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =2335;

-- Magistrate Burnside
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =2373;

-- Mudsnout Shaman
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =2387;

-- Hillsbrad Councilman
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =2553;

-- Witherbark Shadowcaster
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =2567;

-- Boulderfist Magus
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =2573;

-- Drywhisker Surveyor
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =2577;

-- Dark Iron Shadowcaster
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =2590;

-- Syndicate Conjuror
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =2591;

-- Syndicate Magus
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =2596;

-- Daggerspine Sorceress
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =2642;

-- Vilebranch Shadowcaster
 UPDATE `creature_template` SET `damage_variance` =0.1 WHERE `entry` =2953;

-- Bristleback Shaman
 UPDATE `creature_template` SET `damage_variance` =0.12 WHERE `entry` =2954;

-- Bristleback Battleboar
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =2955;

-- Plainstrider
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =2958;

-- Prairie Wolf
 UPDATE `creature_template` SET `damage_variance` =0.09 WHERE `entry` =2961;

-- Mountain Cougar
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =2964;

-- Windfury Sorceress
 UPDATE `creature_template` SET `damage_variance` =0.1 WHERE `entry` =2966;

-- Battleboar
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =2969;

-- Wiry Swoop
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =3098;

-- Mottled Boar
 UPDATE `creature_template` SET `damage_variance` =0.1 WHERE `entry` =3101;

-- Vile Familiar
 UPDATE `creature_template` SET `damage_variance` =0.1 WHERE `entry` =3102;

-- Felstalker
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =3106;

-- Pygmy Surf Crawler
 UPDATE `creature_template` SET `damage_variance` =0.09 WHERE `entry` =3124;

-- Scorpid Worker
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =3125;

-- Clattering Scorpid
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =3128;

-- Kul Tiras Sailor
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =3129;

-- Kul Tiras Marine
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =3198;

-- Burning Blade Apprentice
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =3199;

-- Burning Blade Cultist
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =3203;

-- Fizzle Darkstorm
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =3269;

-- Razormane Geomancer
 UPDATE `creature_template` SET `damage_variance` =0.1 WHERE `entry` =3281;

-- Sarkoth
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =3380;

-- Burning Blade Acolyte
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =3662;

-- Delmanis the Hated
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =3664;

-- Ilkrud Magthrull
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =3717;

-- Wrathtail Sorceress
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =3850;

-- Sorcerer Ashcrombe
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =3894;

-- Pelturas Whitemoon
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =3914;

-- Rethilgore
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =3953;

-- Tandaan Lightmane
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =3954;

-- Dalria
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =3958;

-- Lardan
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =3959;

-- Nantar
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =3961;

-- Maliynn
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =3962;

-- Haljan Oakheart
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =3967;

-- Aayndia Floralwind
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =3969;

-- Fahran Silentblade
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =3970;

-- Llana
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =3983;

-- Interrogator Vishas
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =3984;

-- Nancy Vishas
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =3985;

-- Grandpa Vishas
 UPDATE `creature_template` SET `damage_variance` =0.08 WHERE `entry` =3986;

-- Sarilus Foulborne
 UPDATE `creature_template` SET `damage_variance` =0.08 WHERE `entry` =3991;

-- Venture Co. Deforester
 UPDATE `creature_template` SET `damage_variance` =0.08 WHERE `entry` =4003;

-- Windshear Geomancer
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4004;

-- Windshear Overlord
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4005;

-- Deepmoss Creeper
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4006;

-- Deepmoss Webspinner
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4007;

-- Deepmoss Venomspitter
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4008;

-- Cliff Stormer
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4009;

-- Raging Cliff Stormer
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4011;

-- Young Pridewing
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4012;

-- Pridewing Wyvern
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4013;

-- Pridewing Skyhunter
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4014;

-- Pridewing Consort
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4015;

-- Pridewing Patriarch
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4018;

-- Antlered Courser
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4019;

-- Great Courser
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4031;

-- Fledgling Chimaera
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4032;

-- Young Chimaera
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4041;

-- Scorched Basilisk
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4042;

-- Singed Basilisk
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4044;

-- Blackened Basilisk
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =4063;

-- Feeboz
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4067;

-- Twilight Runner
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4263;

-- Deepmoss Hatchling
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4264;

-- Deepmoss Matriarch
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =4279;

-- Odo the Blindwatcher
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =4282;

-- Scarlet Magician
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =4289;

-- Scarlet Evoker
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =4297;

-- Scarlet Conjuror
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =4463;

-- Blackrock Summoner
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =4494;

-- Scarlet Spellbinder
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =4516;

-- Death's Head Adept
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =4684;

-- Nether Sorceress
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =4813;

-- Twilight Shadowmage
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =4814;

-- Twilight Elementalist
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =4844;

-- Shadowforge Surveyor
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =5403;

-- White Stallion
 UPDATE `creature_template` SET `damage_variance` =0.02 WHERE `entry` =5404;

-- Black Stallion
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =5405;

-- Pinto
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =5406;

-- Palomino
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =5617;

-- Wastewander Shadow Mage
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =5647;

-- Sandfury Firecaller
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =5648;

-- Sandfury Shadowcaster
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =5694;

-- High Sorcerer Andromath
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =5750;

-- Gina Lang
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =5774;

-- Riding Wolf
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =5839;

-- Dark Iron Geologist
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =5862;

-- Twilight Geomancer
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =5981;

-- Portal Seeker
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =6008;

-- Shadowsworn Warlock
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =6009;

-- Shadowsworn Dreadweaver
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =6127;

-- Haldarr Felsworn
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =6167;

-- Chimaera Matriarch
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =6202;

-- Legashi Hellcaller
 UPDATE `creature_template` SET `damage_variance` =0.12 WHERE `entry` =6367;

-- Donni Anthania
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =7111;

-- Jadefire Hellcaller
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =7335;

-- Death's Head Geomancer
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =7789;

-- Sandfury Cretin
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =7795;

-- Hydromancer Velratha
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =7872;

-- Death's Head Cultist
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =7874;

-- Razorfen Thornweaver
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =7995;

-- Vile Priestess Hexx
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =7996;

-- Qiaga the Keeper
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =8503;

-- Gibblewilt
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =8550;

-- Shadowmage
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =8636;

-- Morta'gya the Keeper
 UPDATE `creature_template` SET `damage_variance` =0.02 WHERE `entry` =8882;

-- Riding Tiger
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =8883;

-- Riding Horse
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =8885;

-- Riding Raptor
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =10760;

-- Grimtotem Geomancer
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =10802;

-- Hitah'ya the Keeper
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =11137;

-- Xai'ander
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =11324;

-- Searing Blade Warlock
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =11383;

-- High Priestess Hai'watna
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =11518;

-- Jergosh the Invoker
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =11683;

-- Horde Shaman
 UPDATE `creature_template` SET `damage_variance` =0.32 WHERE `entry` =11896;

-- Borelgore
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =11921;

-- Besseleth
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =11937;

-- Demon Portal Guardian
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =12346;

-- Emerald Raptor
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =12349;

-- Turquoise Raptor
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =12350;

-- Violet Raptor
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =12372;

-- Brown Ram
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =12373;

-- Gray Ram
 UPDATE `creature_template` SET `damage_variance` =0.02 WHERE `entry` =12374;

-- White Riding Ram
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =12375;

-- Chestnut Mare
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =12376;

-- Brown Horse
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =12579;

-- Bloodfury Ripper
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =14277;

-- Lady Zephris
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =14542;

-- Great White Kodo
 UPDATE `creature_template` SET `damage_variance` =0.02 WHERE `entry` =14543;

-- Swift Olive Raptor
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =14544;

-- Swift Orange Raptor
 UPDATE `creature_template` SET `damage_variance` =0.02 WHERE `entry` =14545;

-- Swift Blue Raptor
 UPDATE `creature_template` SET `damage_variance` =0.02 WHERE `entry` =14546;

-- Swift Brown Ram
 UPDATE `creature_template` SET `damage_variance` =0.02 WHERE `entry` =14547;

-- Swift White Ram
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =14548;

-- Swift Gray Ram
 UPDATE `creature_template` SET `damage_variance` =0.02 WHERE `entry` =14549;

-- Great Brown Kodo
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =14559;

-- Swift Palamino
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =14560;

-- Swift White Steed
 UPDATE `creature_template` SET `damage_variance` =0.02 WHERE `entry` =14561;

-- Swift Brown Steed
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =15191;

-- Windcaller Proudhorn
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =15308;

-- Twilight Prophet
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =16506;

-- Naxxramas Worshipper
 -- Damage Multipliers
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6 AND `patch` IN (0);

-- Kobold Vermin
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =30 AND `patch` IN (0);

-- Forest Spider
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =38 AND `patch` IN (0);

-- Defias Thug
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =43 AND `patch` IN (0);

-- Mine Spider
 UPDATE `creature_template` SET `damage_multiplier` =2 WHERE `entry` =68 AND `patch` IN (0);

-- Stormwind City Guard
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =69 AND `patch` IN (0);

-- Timber Wolf
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =80 AND `patch` IN (0);

-- Kobold Laborer
 UPDATE `creature_template` SET `damage_multiplier` =2.6 WHERE `entry` =89 AND `patch` IN (7);

-- Infernal
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =94 AND `patch` IN (0);

-- Defias Cutpurse
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =103 AND `patch` IN (0);

-- Garrick Padfoot
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =113 AND `patch` IN (0, 7);

-- Stonetusk Boar
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =116 AND `patch` IN (0);

-- Defias Bandit
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =118 AND `patch` IN (0);

-- Prowler
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =119 AND `patch` IN (0, 7);

-- Longsnout
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =154 AND `patch` IN (0, 5);

-- Greater Fleshripper
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =157 AND `patch` IN (0, 7);

-- Goretusk
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =213 AND `patch` IN (0);

-- Starving Dire Wolf
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =217 AND `patch` IN (0);

-- Venom Web Spider
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =240 AND `patch` IN (0);

-- Marshal Dughan
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =241 AND `patch` IN (0);

-- Remy "Two Times"
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =257 AND `patch` IN (0);

-- Kobold Worker
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =299 AND `patch` IN (0);

-- Young Wolf
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =330 AND `patch` IN (0, 7);

-- Princess
 UPDATE `creature_template` SET `damage_multiplier` =2.4 WHERE `entry` =334 AND `patch` IN (0);

-- Gath'Ilzogg
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =390 AND `patch` IN (0, 7);

-- Porcine Entourage
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =397 AND `patch` IN (0);

-- Morganth
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =428 AND `patch` IN (0);

-- Dire Condor
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =433 AND `patch` IN (0);

-- Shadowhide Gnoll
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =436 AND `patch` IN (0);

-- Blackrock Shadowcaster
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =437 AND `patch` IN (0);

-- Blackrock Renegade
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =440 AND `patch` IN (0);

-- Blackrock Grunt
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =452 AND `patch` IN (0);

-- Riverpaw Bandit
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =453 AND `patch` IN (0);

-- Riverpaw Mystic
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =454 AND `patch` IN (0, 7);

-- Young Goretusk
 UPDATE `creature_template` SET `damage_multiplier` =1.25 WHERE `entry` =462 AND `patch` IN (0);

-- Vultros
 UPDATE `creature_template` SET `damage_multiplier` =7 WHERE `entry` =466 AND `patch` IN (0);

-- General Marcus Jonathan
 UPDATE `creature_template` SET `damage_multiplier` =10 WHERE `entry` =469 AND `patch` IN (0);

-- Lieutenant Doren
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =471 AND `patch` IN (0);

-- Mother Fang
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =473 AND `patch` IN (0);

-- Morgan the Collector
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =474 AND `patch` IN (0);

-- Defias Rogue Wizard
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =486 AND `patch` IN (0);

-- Tharil'zun
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =505 AND `patch` IN (0);

-- Greater Tarantula
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =521 AND `patch` IN (0);

-- Lupos
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =524 AND `patch` IN (0, 7);

-- Rockhide Boar
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =539 AND `patch` IN (0);

-- Pygmy Venom Web Spider
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =547 AND `patch` IN (0, 7);

-- Great Goretusk
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =569 AND `patch` IN (0);

-- Green Recluse
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =574 AND `patch` IN (0);

-- Naraxis
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =594 AND `patch` IN (0);

-- Defias Henchman
 UPDATE `creature_template` SET `damage_multiplier` =1.56 WHERE `entry` =597 AND `patch` IN (0);

-- Bloodscalp Berserker
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =616 AND `patch` IN (0);

-- Chatter
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =619 AND `patch` IN (0);

-- Defias Conjurer
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =659 AND `patch` IN (0);

-- El Pollo Grande
 UPDATE `creature_template` SET `damage_multiplier` =4.9 WHERE `entry` =678 AND `patch` IN (0);

-- Mosh'Ogg Mauler
 UPDATE `creature_template` SET `damage_multiplier` =2.4 WHERE `entry` =679 AND `patch` IN (0);

-- Mosh'Ogg Shaman
 UPDATE `creature_template` SET `damage_multiplier` =3.375 WHERE `entry` =680 AND `patch` IN (0);

-- Mosh'Ogg Lord
 UPDATE `creature_template` SET `damage_multiplier` =1.25 WHERE `entry` =685 AND `patch` IN (0);

-- Stranglethorn Raptor
 UPDATE `creature_template` SET `damage_multiplier` =1.2 WHERE `entry` =687 AND `patch` IN (0);

-- Jungle Stalker
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =704 AND `patch` IN (0);

-- Ragged Timber Wolf
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =705 AND `patch` IN (0);

-- Ragged Young Wolf
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =708 AND `patch` IN (0, 7);

-- Small Crag Boar
 UPDATE `creature_template` SET `damage_multiplier` =2.2 WHERE `entry` =709 AND `patch` IN (0);

-- Mosh'Ogg Warmonger
 UPDATE `creature_template` SET `damage_multiplier` =2.4 WHERE `entry` =710 AND `patch` IN (0);

-- Mosh'Ogg Spellcrafter
 UPDATE `creature_template` SET `damage_multiplier` =1.425 WHERE `entry` =728 AND `patch` IN (0);

-- Bhag'thera
 UPDATE `creature_template` SET `damage_multiplier` =0.91 WHERE `entry` =729 AND `patch` IN (0);

-- Sin'Dall
 UPDATE `creature_template` SET `damage_multiplier` =2 WHERE `entry` =730 AND `patch` IN (0);

-- Tethis
 UPDATE `creature_template` SET `damage_multiplier` =1.61 WHERE `entry` =731 AND `patch` IN (0);

-- King Bangalash
 UPDATE `creature_template` SET `damage_multiplier` =0.5 WHERE `entry` =733 AND `patch` IN (0);

-- Sergeant Yohwa
 UPDATE `creature_template` SET `damage_multiplier` =0.5 WHERE `entry` =734 AND `patch` IN (0);

-- Corporal Bluth
 UPDATE `creature_template` SET `damage_multiplier` =0.5 WHERE `entry` =738 AND `patch` IN (0);

-- Private Thorsen
 UPDATE `creature_template` SET `damage_multiplier` =0.5 WHERE `entry` =739 AND `patch` IN (0);

-- Brother Nimetz
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =756 AND `patch` IN (0);

-- Skullsplitter Panther
 UPDATE `creature_template` SET `damage_multiplier` =0.5 WHERE `entry` =770 AND `patch` IN (0);

-- Corporal Kaleb
 UPDATE `creature_template` SET `damage_multiplier` =1.5 WHERE `entry` =783 AND `patch` IN (0);

-- Skullsplitter Berserker
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =808 AND `patch` IN (0);

-- Grik'nir the Cold
 UPDATE `creature_template` SET `damage_multiplier` =2.3375 WHERE `entry` =813 AND `patch` IN (0);

-- Colonel Kurzen
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =814 AND `patch` IN (0);

-- Sergeant Malthus
 UPDATE `creature_template` SET `damage_multiplier` =3.65 WHERE `entry` =818 AND `patch` IN (0);

-- Mai'Zoth
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =819 AND `patch` IN (0);

-- Servant of Ilgalar
 UPDATE `creature_template` SET `damage_multiplier` =1.25 WHERE `entry` =822 AND `patch` IN (0);

-- Young Forest Bear
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =830 AND `patch` IN (0);

-- Sand Crawler
 UPDATE `creature_template` SET `damage_multiplier` =1.25 WHERE `entry` =856 AND `patch` IN (0);

-- Young Lashtail Raptor
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =861 AND `patch` IN (0);

-- Stonard Scout
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =862 AND `patch` IN (0);

-- Stonard Explorer
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =866 AND `patch` IN (0);

-- Stonard Grunt
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =871 AND `patch` IN (0);

-- Saltscale Warrior
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =873 AND `patch` IN (0);

-- Saltscale Oracle
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =875 AND `patch` IN (0);

-- Saltscale Tide Lord
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =877 AND `patch` IN (0);

-- Saltscale Forager
 UPDATE `creature_template` SET `damage_multiplier` =2.25 WHERE `entry` =879 AND `patch` IN (0);

-- Saltscale Hunter
 UPDATE `creature_template` SET `damage_multiplier` =1.625 WHERE `entry` =880 AND `patch` IN (0);

-- Erlan Drudgemoor
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =881 AND `patch` IN (0);

-- Surena Caledon
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =913 AND `patch` IN (0);

-- Lyria Du Lac
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =927 AND `patch` IN (0);

-- Brother Wilhelm
 UPDATE `creature_template` SET `damage_multiplier` =1.2 WHERE `entry` =1015 AND `patch` IN (0);

-- Highland Raptor
 UPDATE `creature_template` SET `damage_multiplier` =1.2 WHERE `entry` =1016 AND `patch` IN (0);

-- Highland Lashtail
 UPDATE `creature_template` SET `damage_multiplier` =1.2 WHERE `entry` =1020 AND `patch` IN (0);

-- Mottled Raptor
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1034 AND `patch` IN (0);

-- Dragonmaw Raider
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1035 AND `patch` IN (0);

-- Dragonmaw Swamprunner
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1036 AND `patch` IN (0);

-- Dragonmaw Centurion
 UPDATE `creature_template` SET `damage_multiplier` =0.96 WHERE `entry` =1037 AND `patch` IN (0);

-- Dragonmaw Battlemaster
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1038 AND `patch` IN (0);

-- Dragonmaw Shadowwarder
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1051 AND `patch` IN (0);

-- Dark Iron Dwarf
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1052 AND `patch` IN (0);

-- Dark Iron Saboteur
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1053 AND `patch` IN (0);

-- Dark Iron Tunneler
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1054 AND `patch` IN (0);

-- Dark Iron Demolitionist
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1057 AND `patch` IN (0);

-- Dragonmaw Bonewarder
 UPDATE `creature_template` SET `damage_multiplier` =1.68 WHERE `entry` =1059 AND `patch` IN (0);

-- Ana'thek the Cruel
 UPDATE `creature_template` SET `damage_multiplier` =2.4 WHERE `entry` =1060 AND `patch` IN (0);

-- Mogh the Undying
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1064 AND `patch` IN (0);

-- Grom'gol Grunt
 UPDATE `creature_template` SET `damage_multiplier` =1.1 WHERE `entry` =1087 AND `patch` IN (0);

-- Sawtooth Snapper
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1088 AND `patch` IN (0);

-- Monstrous Crawler
 UPDATE `creature_template` SET `damage_multiplier` =1.2 WHERE `entry` =1108 AND `patch` IN (0, 7);

-- Mistvale Gorilla
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1109 AND `patch` IN (0);

-- Fleshripper
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1111 AND `patch` IN (0);

-- Leech Stalker
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1112 AND `patch` IN (0);

-- Leech Widow
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1115 AND `patch` IN (0);

-- Rockjaw Skullthumper
 UPDATE `creature_template` SET `damage_multiplier` =1.25 WHERE `entry` =1118 AND `patch` IN (0);

-- Rockjaw Backbreaker
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1121 AND `patch` IN (0);

-- Frostmane Snowstrider
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1124 AND `patch` IN (0);

-- Frostmane Shadowcaster
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1125 AND `patch` IN (0, 7);

-- Crag Boar
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1126 AND `patch` IN (0, 7);

-- Large Crag Boar
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1127 AND `patch` IN (0, 7);

-- Elder Crag Boar
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1128 AND `patch` IN (0);

-- Young Black Bear
 UPDATE `creature_template` SET `damage_multiplier` =1.25 WHERE `entry` =1129 AND `patch` IN (0);

-- Black Bear
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1131 AND `patch` IN (0);

-- Winter Wolf
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1133 AND `patch` IN (0);

-- Starving Winter Wolf
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1138 AND `patch` IN (0);

-- Snow Tracker Wolf
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1140 AND `patch` IN (0);

-- Razormaw Matriarch
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1165 AND `patch` IN (0);

-- Stonesplinter Geomancer
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1167 AND `patch` IN (0);

-- Stonesplinter Digger
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1169 AND `patch` IN (0);

-- Dark Iron Insurgent
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1172 AND `patch` IN (0);

-- Tunnel Rat Vermin
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1174 AND `patch` IN (0);

-- Tunnel Rat Geomancer
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1178 AND `patch` IN (0);

-- Mo'grosh Ogre
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1179 AND `patch` IN (0);

-- Mo'grosh Enforcer
 UPDATE `creature_template` SET `damage_multiplier` =3.675 WHERE `entry` =1180 AND `patch` IN (0, 1);

-- Mo'grosh Brute
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1181 AND `patch` IN (0);

-- Mo'grosh Shaman
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1183 AND `patch` IN (0);

-- Mo'grosh Mystic
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1185 AND `patch` IN (0);

-- Wood Lurker
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1188 AND `patch` IN (0);

-- Grizzled Black Bear
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1189 AND `patch` IN (0);

-- Black Bear Patriarch
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1191 AND `patch` IN (0, 7);

-- Mangy Mountain Boar
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1192 AND `patch` IN (0, 7);

-- Elder Mountain Boar
 UPDATE `creature_template` SET `damage_multiplier` =0.165 WHERE `entry` =1193 AND `patch` IN (0);

-- Loch Frenzy
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1194 AND `patch` IN (0);

-- Mountain Buzzard
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1196 AND `patch` IN (0);

-- Ice Claw Bear
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1197 AND `patch` IN (0);

-- Stonesplinter Shaman
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =1199 AND `patch` IN (0);

-- Juvenile Snow Leopard
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1201 AND `patch` IN (0);

-- Snow Leopard
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1210 AND `patch` IN (0);

-- Chok'sul
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1224 AND `patch` IN (0);

-- Young Threshadon
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1225 AND `patch` IN (0);

-- Ol' Sooty
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1271 AND `patch` IN (0);

-- Old Icebeard
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1344 AND `patch` IN (0);

-- Prospector Ironband
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1388 AND `patch` IN (0);

-- Vagash
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1398 AND `patch` IN (0);

-- Boss Galgosh
 UPDATE `creature_template` SET `damage_multiplier` =1.1 WHERE `entry` =1400 AND `patch` IN (0);

-- Wetlands Crocolisk
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1422 AND `patch` IN (0);

-- Corporal Sethman
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1423 AND `patch` IN (0);

-- Stormwind Guard
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1490 AND `patch` IN (0);

-- Zanzil Witch Doctor
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1504 AND `patch` IN (0);

-- Young Night Web Spider
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1505 AND `patch` IN (0);

-- Night Web Spider
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1506 AND `patch` IN (0);

-- Scarlet Convert
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1507 AND `patch` IN (0);

-- Scarlet Initiate
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1508 AND `patch` IN (0);

-- Young Scavenger
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1509 AND `patch` IN (0);

-- Ragged Scavenger
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1511 AND `patch` IN (0);

-- Enraged Silverback Gorilla
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1512 AND `patch` IN (0);

-- Duskbat
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1513 AND `patch` IN (0);

-- Mangy Duskbat
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1514 AND `patch` IN (0);

-- Mokk the Savage
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1535 AND `patch` IN (0);

-- Scarlet Warrior
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1536 AND `patch` IN (0);

-- Scarlet Missionary
 UPDATE `creature_template` SET `damage_multiplier` =1.5 WHERE `entry` =1537 AND `patch` IN (0);

-- Scarlet Zealot
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1538 AND `patch` IN (0);

-- Scarlet Friar
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1539 AND `patch` IN (0);

-- Scarlet Neophyte
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1543 AND `patch` IN (0);

-- Vile Fin Puddlejumper
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1544 AND `patch` IN (0);

-- Vile Fin Minor Oracle
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1547 AND `patch` IN (0);

-- Decrepit Darkhound
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1548 AND `patch` IN (0);

-- Cursed Darkhound
 UPDATE `creature_template` SET `damage_multiplier` =2.24 WHERE `entry` =1551 AND `patch` IN (0);

-- Ironjaw Basilisk
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1552 AND `patch` IN (0);

-- Scale Belly
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1553 AND `patch` IN (0);

-- Greater Duskbat
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1554 AND `patch` IN (0);

-- Vampiric Duskbat
 UPDATE `creature_template` SET `damage_multiplier` =4.9 WHERE `entry` =1559 AND `patch` IN (0);

-- King Mukla
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1563 AND `patch` IN (0);

-- Bloodsail Swashbuckler
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1564 AND `patch` IN (0);

-- Bloodsail Warlock
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1565 AND `patch` IN (0);

-- Bloodsail Sea Dog
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1642 AND `patch` IN (0);

-- Northshire Guard
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1653 AND `patch` IN (0);

-- Bloodsail Elder Magus
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1662 AND `patch` IN (0);

-- Captain Perrine
 UPDATE `creature_template` SET `damage_multiplier` =2.04 WHERE `entry` =1663 AND `patch` IN (0);

-- Dextren Ward
 UPDATE `creature_template` SET `damage_multiplier` =1.5 WHERE `entry` =1665 AND `patch` IN (0);

-- Captain Melrache
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1666 AND `patch` IN (0);

-- Kam Deepfury
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1667 AND `patch` IN (0);

-- Meven Korgal
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1688 AND `patch` IN (0);

-- Night Web Matriarch
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1693 AND `patch` IN (0);

-- Loch Crocolisk
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1696 AND `patch` IN (0);

-- Targorr the Dread
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1706 AND `patch` IN (0);

-- Defias Prisoner
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1707 AND `patch` IN (0);

-- Defias Captive
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1708 AND `patch` IN (0);

-- Defias Inmate
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1711 AND `patch` IN (0);

-- Defias Convict
 UPDATE `creature_template` SET `damage_multiplier` =2.21 WHERE `entry` =1715 AND `patch` IN (0);

-- Defias Insurgent
 UPDATE `creature_template` SET `damage_multiplier` =2.4 WHERE `entry` =1716 AND `patch` IN (0);

-- Bazil Thredd
 UPDATE `creature_template` SET `damage_multiplier` =2.5 WHERE `entry` =1717 AND `patch` IN (0);

-- Hamhock
 UPDATE `creature_template` SET `damage_multiplier` =2.4 WHERE `entry` =1720 AND `patch` IN (0);

-- Bruegal Ironknuckle
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1725 AND `patch` IN (0);

-- Defias Watchman
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1726 AND `patch` IN (0);

-- Defias Magician
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1733 AND `patch` IN (0);

-- Zggi
 UPDATE `creature_template` SET `damage_multiplier` =1.1 WHERE `entry` =1765 AND `patch` IN (0);

-- Worg
 UPDATE `creature_template` SET `damage_multiplier` =1.1 WHERE `entry` =1766 AND `patch` IN (0);

-- Mottled Worg
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1767 AND `patch` IN (0);

-- Vile Fin Shredder
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1769 AND `patch` IN (0);

-- Moonrage Whitescalp
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =1770 AND `patch` IN (0);

-- Moonrage Darkrunner
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1779 AND `patch` IN (0);

-- Moonrage Glutton
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1780 AND `patch` IN (0);

-- Moss Stalker
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1781 AND `patch` IN (0);

-- Mist Creeper
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1782 AND `patch` IN (0);

-- Moonrage Darksoul
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1797 AND `patch` IN (0);

-- Giant Grizzled Bear
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1809 AND `patch` IN (0);

-- Carrion Vulture
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1826 AND `patch` IN (0);

-- Scarlet Mage
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1827 AND `patch` IN (0);

-- Scarlet Sentinel
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1832 AND `patch` IN (0);

-- Scarlet Magus
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1834 AND `patch` IN (0);

-- Scarlet Paladin
 UPDATE `creature_template` SET `damage_multiplier` =2.04 WHERE `entry` =1836 AND `patch` IN (0);

-- Scarlet Cavalier
 UPDATE `creature_template` SET `damage_multiplier` =4.5 WHERE `entry` =1838 AND `patch` IN (0);

-- Scarlet Interrogator
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1839 AND `patch` IN (0);

-- Scarlet High Clerist
 UPDATE `creature_template` SET `damage_multiplier` =3.375 WHERE `entry` =1843 AND `patch` IN (0);

-- Foreman Jerris
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1846 AND `patch` IN (0);

-- High Protector Lorik
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1854 AND `patch` IN (0);

-- High Priest Thel'danis
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1885 AND `patch` IN (0);

-- Scarlet Smith
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1909 AND `patch` IN (0);

-- Vile Fin Lakestalker
 UPDATE `creature_template` SET `damage_multiplier` =0.85 WHERE `entry` =1922 AND `patch` IN (0);

-- Gray Forest Wolf
 UPDATE `creature_template` SET `damage_multiplier` =1.1 WHERE `entry` =1923 AND `patch` IN (0);

-- Bloodsnout Worg
 UPDATE `creature_template` SET `damage_multiplier` =1.92 WHERE `entry` =1934 AND `patch` IN (0);

-- Tirisfal Farmer
 UPDATE `creature_template` SET `damage_multiplier` =1.92 WHERE `entry` =1935 AND `patch` IN (0);

-- Tirisfal Farmhand
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1936 AND `patch` IN (0);

-- Farmer Solliden
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1937 AND `patch` IN (0);

-- Apothecary Renferrel
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1938 AND `patch` IN (0);

-- Dalar Dawnweaver
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1950 AND `patch` IN (0);

-- Rane Yorick
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1951 AND `patch` IN (0);

-- Quinn Yorick
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1957 AND `patch` IN (0);

-- Vile Fin Shorecreeper
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1958 AND `patch` IN (0);

-- Vile Fin Tidecaller
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1961 AND `patch` IN (0);

-- Mangeclaw
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1972 AND `patch` IN (0);

-- Grimson the Pale
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1978 AND `patch` IN (0);

-- Deathstalker Erland
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1984 AND `patch` IN (0, 7);

-- Young Thistle Boar
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1985 AND `patch` IN (0, 7);

-- Thistle Boar
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1986 AND `patch` IN (0);

-- Webwood Spider
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1988 AND `patch` IN (0);

-- Grell
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1989 AND `patch` IN (0);

-- Grellkin
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1994 AND `patch` IN (0);

-- Githyiss the Vile
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1995 AND `patch` IN (0);

-- Strigid Owl
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1996 AND `patch` IN (0);

-- Strigid Screecher
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1997 AND `patch` IN (0);

-- Strigid Hunter
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1998 AND `patch` IN (0);

-- Webwood Lurker
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1999 AND `patch` IN (0);

-- Webwood Venomfang
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2000 AND `patch` IN (0);

-- Webwood Silkspinner
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2001 AND `patch` IN (0);

-- Giant Webwood Spider
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2002 AND `patch` IN (0);

-- Rascal Sprite
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2003 AND `patch` IN (0);

-- Shadow Sprite
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2004 AND `patch` IN (0);

-- Dark Sprite
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2005 AND `patch` IN (0);

-- Vicious Grell
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2015 AND `patch` IN (0);

-- Bloodfeather Harpy
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2018 AND `patch` IN (0);

-- Bloodfeather Sorceress
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2031 AND `patch` IN (0);

-- Young Nightsaber
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2032 AND `patch` IN (0);

-- Mangy Nightsaber
 UPDATE `creature_template` SET `damage_multiplier` =1.42857 WHERE `entry` =2038 AND `patch` IN (0);

-- Lord Melenas
 UPDATE `creature_template` SET `damage_multiplier` =0.7 WHERE `entry` =2042 AND `patch` IN (0);

-- Nightsaber
 UPDATE `creature_template` SET `damage_multiplier` =0.77 WHERE `entry` =2043 AND `patch` IN (0, 1);

-- Nightsaber Stalker
 UPDATE `creature_template` SET `damage_multiplier` =0.65 WHERE `entry` =2069 AND `patch` IN (0);

-- Moonstalker
 UPDATE `creature_template` SET `damage_multiplier` =0.65 WHERE `entry` =2070 AND `patch` IN (0);

-- Moonstalker Runt
 UPDATE `creature_template` SET `damage_multiplier` =0.65 WHERE `entry` =2071 AND `patch` IN (0);

-- Moonstalker Matriarch
 UPDATE `creature_template` SET `damage_multiplier` =2.4 WHERE `entry` =2091 AND `patch` IN (0);

-- Chieftain Nek'rosh
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2108 AND `patch` IN (0);

-- Garneg Charskull
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =2121 AND `patch` IN (0);

-- Shadow Priest Allister
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2140 AND `patch` IN (0);

-- Edwin Harly
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2164 AND `patch` IN (0);

-- Rabid Thistle Bear
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2165 AND `patch` IN (0, 1);

-- Grizzled Thistle Bear
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2172 AND `patch` IN (0);

-- Strider Clutchmother
 UPDATE `creature_template` SET `damage_multiplier` =1.2 WHERE `entry` =2175 AND `patch` IN (0);

-- Shadowclaw
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2182 AND `patch` IN (0);

-- Stormscale Sorceress
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2185 AND `patch` IN (0);

-- Darkshore Thresher
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2187 AND `patch` IN (0);

-- Elder Darkshore Thresher
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2191 AND `patch` IN (0);

-- Licillin
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2214 AND `patch` IN (0);

-- Deathstalker Lesh
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2216 AND `patch` IN (0);

-- Apothecary Lydon
 UPDATE `creature_template` SET `damage_multiplier` =3.5 WHERE `entry` =2226 AND `patch` IN (0);

-- Karos Razok
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2229 AND `patch` IN (0);

-- Krusk
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2230 AND `patch` IN (0);

-- Umpi
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2232 AND `patch` IN (0);

-- Tide Crawler
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2235 AND `patch` IN (0);

-- Reef Crawler
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2238 AND `patch` IN (0);

-- Tog'thar
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2241 AND `patch` IN (0);

-- Syndicate Thief
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2244 AND `patch` IN (0);

-- Syndicate Shadow Mage
 UPDATE `creature_template` SET `damage_multiplier` =1.875 WHERE `entry` =2253 AND `patch` IN (0);

-- Crushridge Brute
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =2254 AND `patch` IN (0);

-- Crushridge Mauler
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =2255 AND `patch` IN (0);

-- Crushridge Mage
 UPDATE `creature_template` SET `damage_multiplier` =1.9 WHERE `entry` =2256 AND `patch` IN (0);

-- Crushridge Enforcer
 UPDATE `creature_template` SET `damage_multiplier` =2.3 WHERE `entry` =2257 AND `patch` IN (0);

-- Mug'thol
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2260 AND `patch` IN (0);

-- Syndicate Rogue
 UPDATE `creature_template` SET `damage_multiplier` =2 WHERE `entry` =2287 AND `patch` IN (0);

-- Crushridge Warmonger
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2306 AND `patch` IN (0);

-- Baron Vardus
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2316 AND `patch` IN (0);

-- Gol'dir
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2318 AND `patch` IN (0);

-- Argus Shadow Mage
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2321 AND `patch` IN (0);

-- Foreststrider Fledgling
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2322 AND `patch` IN (0);

-- Foreststrider
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2351 AND `patch` IN (0);

-- Gray Bear
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2354 AND `patch` IN (0);

-- Vicious Gray Bear
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2371 AND `patch` IN (0);

-- Daggerspine Siren
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2372 AND `patch` IN (0);

-- Mudsnout Gnoll
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2373 AND `patch` IN (0);

-- Mudsnout Shaman
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2377 AND `patch` IN (0);

-- Torn Fin Tidehunter
 UPDATE `creature_template` SET `damage_multiplier` =0.6 WHERE `entry` =2385 AND `patch` IN (0);

-- Feral Mountain Lion
 UPDATE `creature_template` SET `damage_multiplier` =3.5 WHERE `entry` =2389 AND `patch` IN (0);

-- Zarise
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2390 AND `patch` IN (0);

-- Aranae Venomblood
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2399 AND `patch` IN (0);

-- Daryl Stack
 UPDATE `creature_template` SET `damage_multiplier` =2 WHERE `entry` =2405 AND `patch` IN (0);

-- Tarren Mill Deathguard
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2408 AND `patch` IN (0);

-- Snapjaw
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =2410 AND `patch` IN (0);

-- Magus Wordeen Voidglare
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =2416 AND `patch` IN (0);

-- Crushridge Plunderer
 UPDATE `creature_template` SET `damage_multiplier` =1.9 WHERE `entry` =2417 AND `patch` IN (0);

-- Grel'borg the Miser
 UPDATE `creature_template` SET `damage_multiplier` =2.1 WHERE `entry` =2420 AND `patch` IN (0);

-- Targ
 UPDATE `creature_template` SET `damage_multiplier` =2 WHERE `entry` =2421 AND `patch` IN (0);

-- Muckrake
 UPDATE `creature_template` SET `damage_multiplier` =1.9 WHERE `entry` =2422 AND `patch` IN (0);

-- Glommus
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2423 AND `patch` IN (0);

-- Lord Aliden Perenolde
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2434 AND `patch` IN (0);

-- Shadowy Assassin
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =2437 AND `patch` IN (0);

-- Keeper Bel'varil
 UPDATE `creature_template` SET `damage_multiplier` =1.2 WHERE `entry` =2440 AND `patch` IN (0);

-- Drunken Footpad
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2452 AND `patch` IN (0);

-- Skhowl
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2476 AND `patch` IN (0);

-- Large Loch Crocolisk
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =2477 AND `patch` IN (0);

-- Gradok
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =2478 AND `patch` IN (0);

-- Haren Swifthoof
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2521 AND `patch` IN (0);

-- Skymane Gorilla
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2522 AND `patch` IN (0);

-- Jaguero Stalker
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =2529 AND `patch` IN (0);

-- Son of Arugal
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2545 AND `patch` IN (0);

-- "Pretty Boy" Duncan
 UPDATE `creature_template` SET `damage_multiplier` =0.78 WHERE `entry` =2546 AND `patch` IN (0);

-- Fleet Master Firallon
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2548 AND `patch` IN (0);

-- Captain Keelhaul
 UPDATE `creature_template` SET `damage_multiplier` =2.295 WHERE `entry` =2549 AND `patch` IN (0);

-- Garr Salthoof
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2550 AND `patch` IN (0);

-- Captain Stillwater
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2553 AND `patch` IN (0);

-- Witherbark Shadowcaster
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =2558 AND `patch` IN (0);

-- Witherbark Berserker
 UPDATE `creature_template` SET `damage_multiplier` =1.2 WHERE `entry` =2560 AND `patch` IN (0);

-- Highland Thrasher
 UPDATE `creature_template` SET `damage_multiplier` =1.75 WHERE `entry` =2569 AND `patch` IN (0);

-- Boulderfist Mauler
 UPDATE `creature_template` SET `damage_multiplier` =1.9 WHERE `entry` =2570 AND `patch` IN (0);

-- Boulderfist Shaman
 UPDATE `creature_template` SET `damage_multiplier` =4.03 WHERE `entry` =2571 AND `patch` IN (0);

-- Boulderfist Lord
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2573 AND `patch` IN (0);

-- Drywhisker Surveyor
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2575 AND `patch` IN (0);

-- Dark Iron Supplier
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2578 AND `patch` IN (0);

-- Young Mesa Buzzard
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =2583 AND `patch` IN (0);

-- Stromgarde Troll Hunter
 UPDATE `creature_template` SET `damage_multiplier` =1.9 WHERE `entry` =2584 AND `patch` IN (0);

-- Stromgarde Defender
 UPDATE `creature_template` SET `damage_multiplier` =2 WHERE `entry` =2585 AND `patch` IN (0);

-- Stromgarde Vindicator
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =2588 AND `patch` IN (0);

-- Syndicate Prowler
 UPDATE `creature_template` SET `damage_multiplier` =0.78 WHERE `entry` =2589 AND `patch` IN (0);

-- Syndicate Mercenary
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =2590 AND `patch` IN (0);

-- Syndicate Conjuror
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =2591 AND `patch` IN (0);

-- Syndicate Magus
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2596 AND `patch` IN (0);

-- Daggerspine Sorceress
 UPDATE `creature_template` SET `damage_multiplier` =3.65 WHERE `entry` =2597 AND `patch` IN (0);

-- Lord Falconcrest
 UPDATE `creature_template` SET `damage_multiplier` =2.8 WHERE `entry` =2598 AND `patch` IN (0);

-- Darbel Montrose
 UPDATE `creature_template` SET `damage_multiplier` =4.59 WHERE `entry` =2599 AND `patch` IN (0);

-- Otto
 UPDATE `creature_template` SET `damage_multiplier` =0.6 WHERE `entry` =2600 AND `patch` IN (0);

-- Singer
 UPDATE `creature_template` SET `damage_multiplier` =2.2 WHERE `entry` =2601 AND `patch` IN (0);

-- Foulbelly
 UPDATE `creature_template` SET `damage_multiplier` =1.9 WHERE `entry` =2602 AND `patch` IN (0);

-- Ruul Onestone
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2603 AND `patch` IN (0);

-- Kovork
 UPDATE `creature_template` SET `damage_multiplier` =4.455 WHERE `entry` =2604 AND `patch` IN (0);

-- Molok the Crusher
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2605 AND `patch` IN (0);

-- Zalas Witherbark
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2606 AND `patch` IN (0);

-- Nimar the Slayer
 UPDATE `creature_template` SET `damage_multiplier` =3.5 WHERE `entry` =2607 AND `patch` IN (0);

-- Prince Galen Trollbane
 UPDATE `creature_template` SET `damage_multiplier` =1.8 WHERE `entry` =2612 AND `patch` IN (0);

-- Lieutenant Valorcall
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2618 AND `patch` IN (0);

-- Hammerfall Peon
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2619 AND `patch` IN (0);

-- Hammerfall Grunt
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2621 AND `patch` IN (0);

-- Hammerfall Guardian
 UPDATE `creature_template` SET `damage_multiplier` =1.5 WHERE `entry` =2635 AND `patch` IN (0);

-- Elder Saltwater Crocolisk
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2636 AND `patch` IN (0);

-- Blackwater Deckhand
 UPDATE `creature_template` SET `damage_multiplier` =2.7 WHERE `entry` =2641 AND `patch` IN (0);

-- Vilebranch Headhunter
 UPDATE `creature_template` SET `damage_multiplier` =2.8 WHERE `entry` =2642 AND `patch` IN (0);

-- Vilebranch Shadowcaster
 UPDATE `creature_template` SET `damage_multiplier` =2.8 WHERE `entry` =2643 AND `patch` IN (0);

-- Vilebranch Berserker
 UPDATE `creature_template` SET `damage_multiplier` =2.9 WHERE `entry` =2644 AND `patch` IN (0);

-- Vilebranch Hideskinner
 UPDATE `creature_template` SET `damage_multiplier` =2.9 WHERE `entry` =2645 AND `patch` IN (0);

-- Vilebranch Shadow Hunter
 UPDATE `creature_template` SET `damage_multiplier` =3 WHERE `entry` =2646 AND `patch` IN (0);

-- Vilebranch Blood Drinker
 UPDATE `creature_template` SET `damage_multiplier` =3 WHERE `entry` =2647 AND `patch` IN (0);

-- Vilebranch Soul Eater
 UPDATE `creature_template` SET `damage_multiplier` =3.1 WHERE `entry` =2648 AND `patch` IN (0);

-- Vilebranch Aman'zasi Guard
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2657 AND `patch` IN (0);

-- Trained Razorbeak
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2658 AND `patch` IN (0);

-- Razorbeak Gryphon
 UPDATE `creature_template` SET `damage_multiplier` =2.635 WHERE `entry` =2681 AND `patch` IN (0);

-- Vilebranch Raiding Wolf
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2686 AND `patch` IN (0);

-- Witherbark Broodguard
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2698 AND `patch` IN (0);

-- George Candarte
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2703 AND `patch` IN (0);

-- Zengu
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2706 AND `patch` IN (0);

-- Tor'gan
 UPDATE `creature_template` SET `damage_multiplier` =5 WHERE `entry` =2707 AND `patch` IN (0);

-- Shadra
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2718 AND `patch` IN (0);

-- Dustbelcher Shaman
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2721 AND `patch` IN (0);

-- Forsaken Bodyguard
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =2738 AND `patch` IN (0);

-- Stromgarde Cavalryman
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2764 AND `patch` IN (0);

-- Sleeby
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2765 AND `patch` IN (0);

-- Znort
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2768 AND `patch` IN (0);

-- Professor Phizzlethorpe
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2769 AND `patch` IN (0);

-- Captain Steelgut
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2770 AND `patch` IN (0);

-- Tallow
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2772 AND `patch` IN (0);

-- Korin Fel
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =2773 AND `patch` IN (0);

-- Or'Kalar
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2774 AND `patch` IN (0);

-- Doctor Draxlegauge
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2775 AND `patch` IN (0);

-- Daggerspine Marauder
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2779 AND `patch` IN (0);

-- Prince Nazjak
 UPDATE `creature_template` SET `damage_multiplier` =2.1 WHERE `entry` =2780 AND `patch` IN (0);

-- Caretaker Nevlin
 UPDATE `creature_template` SET `damage_multiplier` =2.1 WHERE `entry` =2781 AND `patch` IN (0);

-- Caretaker Weston
 UPDATE `creature_template` SET `damage_multiplier` =2.1 WHERE `entry` =2782 AND `patch` IN (0);

-- Caretaker Alaric
 UPDATE `creature_template` SET `damage_multiplier` =2 WHERE `entry` =2783 AND `patch` IN (0);

-- Marez Cowl
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2787 AND `patch` IN (0);

-- Zaruk
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2792 AND `patch` IN (0);

-- Gor'mul
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =2805 AND `patch` IN (0);

-- Deneb Walker
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2818 AND `patch` IN (0);

-- Slagg
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2820 AND `patch` IN (0);

-- Graud
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2830 AND `patch` IN (0);

-- Buzzard
 UPDATE `creature_template` SET `damage_multiplier` =0.625 WHERE `entry` =2850 AND `patch` IN (0);

-- Broken Tooth
 UPDATE `creature_template` SET `damage_multiplier` =2 WHERE `entry` =2892 AND `patch` IN (0);

-- Stonevault Seer
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2924 AND `patch` IN (0, 5);

-- Silvermane Wolf
 UPDATE `creature_template` SET `damage_multiplier` =3.5 WHERE `entry` =2931 AND `patch` IN (0);

-- Zaricotl
 UPDATE `creature_template` SET `damage_multiplier` =1.8 WHERE `entry` =2932 AND `patch` IN (0);

-- Magregan Deepshadow
 UPDATE `creature_template` SET `damage_multiplier` =2.3 WHERE `entry` =2937 AND `patch` IN (0);

-- Dagun the Ravenous
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2950 AND `patch` IN (0);

-- Palemane Skinner
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2954 AND `patch` IN (0, 7);

-- Bristleback Battleboar
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2955 AND `patch` IN (0);

-- Plainstrider
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2956 AND `patch` IN (0);

-- Adult Plainstrider
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2957 AND `patch` IN (0);

-- Elder Plainstrider
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =2958 AND `patch` IN (0);

-- Prairie Wolf
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2961 AND `patch` IN (0);

-- Mountain Cougar
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2966 AND `patch` IN (0, 7);

-- Battleboar
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2969 AND `patch` IN (0);

-- Wiry Swoop
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2970 AND `patch` IN (0);

-- Swoop
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2971 AND `patch` IN (0);

-- Taloned Swoop
 UPDATE `creature_template` SET `damage_multiplier` =1.5 WHERE `entry` =2972 AND `patch` IN (0);

-- Kodo Calf
 UPDATE `creature_template` SET `damage_multiplier` =1.5 WHERE `entry` =2973 AND `patch` IN (0);

-- Kodo Bull
 UPDATE `creature_template` SET `damage_multiplier` =1.5 WHERE `entry` =2974 AND `patch` IN (0);

-- Kodo Matriarch
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3035 AND `patch` IN (0);

-- Flatland Cougar
 UPDATE `creature_template` SET `damage_multiplier` =1.25 WHERE `entry` =3056 AND `patch` IN (0);

-- Ghost Howl
 UPDATE `creature_template` SET `damage_multiplier` =1.25 WHERE `entry` =3058 AND `patch` IN (0);

-- Arra'chea
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3068 AND `patch` IN (0);

-- Mazzranache
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3084 AND `patch` IN (0);

-- Bluffwatcher
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3098 AND `patch` IN (0, 7);

-- Mottled Boar
 UPDATE `creature_template` SET `damage_multiplier` =1.53847 WHERE `entry` =3099 AND `patch` IN (0, 7);

-- Dire Mottled Boar
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3100 AND `patch` IN (0, 7);

-- Elder Mottled Boar
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3102 AND `patch` IN (0);

-- Felstalker
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3106 AND `patch` IN (0);

-- Pygmy Surf Crawler
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3107 AND `patch` IN (0);

-- Surf Crawler
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3110 AND `patch` IN (0);

-- Dreadmaw Crocolisk
 UPDATE `creature_template` SET `damage_multiplier` =0.65 WHERE `entry` =3121 AND `patch` IN (0);

-- Durotar Tiger
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3122 AND `patch` IN (0);

-- Bloodtalon Taillasher
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3123 AND `patch` IN (0);

-- Bloodtalon Scythemaw
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3124 AND `patch` IN (0);

-- Scorpid Worker
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3125 AND `patch` IN (0);

-- Clattering Scorpid
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3126 AND `patch` IN (0);

-- Armored Scorpid
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3128 AND `patch` IN (0);

-- Kul Tiras Sailor
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3129 AND `patch` IN (0);

-- Kul Tiras Marine
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3149 AND `patch` IN (0);

-- Nez'raz
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3198 AND `patch` IN (0);

-- Burning Blade Apprentice
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3199 AND `patch` IN (0);

-- Burning Blade Cultist
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3203 AND `patch` IN (0);

-- Fizzle Darkstorm
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3210 AND `patch` IN (0);

-- Brave Proudsnout
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3211 AND `patch` IN (0);

-- Brave Lightninghorn
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3213 AND `patch` IN (0);

-- Brave Running Wolf
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3214 AND `patch` IN (0);

-- Brave Greathoof
 UPDATE `creature_template` SET `damage_multiplier` =1.35 WHERE `entry` =3234 AND `patch` IN (0);

-- Lost Barrens Kodo
 UPDATE `creature_template` SET `damage_multiplier` =1.35 WHERE `entry` =3235 AND `patch` IN (0);

-- Greater Barrens Kodo
 UPDATE `creature_template` SET `damage_multiplier` =1.35 WHERE `entry` =3236 AND `patch` IN (0);

-- Barrens Kodo
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3238 AND `patch` IN (0);

-- Stormhide
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3240 AND `patch` IN (0);

-- Stormsnout
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3242 AND `patch` IN (0);

-- Zhevra Runner
 UPDATE `creature_template` SET `damage_multiplier` =0.65 WHERE `entry` =3245 AND `patch` IN (0);

-- Ornery Plainstrider
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3246 AND `patch` IN (0);

-- Fleeting Plainstrider
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3247 AND `patch` IN (0, 6);

-- Thunderhawk Hatchling
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3251 AND `patch` IN (0);

-- Silithid Grub
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3254 AND `patch` IN (0);

-- Sunscale Lashtail
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3255 AND `patch` IN (0);

-- Sunscale Screecher
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3256 AND `patch` IN (0);

-- Sunscale Scytheclaw
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3257 AND `patch` IN (0);

-- Ishamuhale
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3269 AND `patch` IN (0);

-- Razormane Geomancer
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3296 AND `patch` IN (0);

-- Orgrimmar Grunt
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3380 AND `patch` IN (0);

-- Burning Blade Acolyte
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =3415 AND `patch` IN (0);

-- Savannah Huntress
 UPDATE `creature_template` SET `damage_multiplier` =0.85 WHERE `entry` =3416 AND `patch` IN (0);

-- Savannah Matriarch
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3424 AND `patch` IN (0, 6);

-- Thunderhawk Cloudscraper
 UPDATE `creature_template` SET `damage_multiplier` =0.7 WHERE `entry` =3425 AND `patch` IN (0);

-- Savannah Prowler
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3463 AND `patch` IN (0);

-- Wandering Barrens Giraffe
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3470 AND `patch` IN (0);

-- Rathorian
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3475 AND `patch` IN (0);

-- Echeyakee
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3503 AND `patch` IN (0);

-- Silithid Protector
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3544 AND `patch` IN (0);

-- Jason Lemieux
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3551 AND `patch` IN (0);

-- Patrice Dwyer
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3552 AND `patch` IN (0);

-- Alexandre Lefevre
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3553 AND `patch` IN (0);

-- Sebastian Meloche
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3554 AND `patch` IN (0);

-- Andrea Boynton
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3555 AND `patch` IN (0);

-- Johan Focht
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3556 AND `patch` IN (0);

-- Andrew Hilbert
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3557 AND `patch` IN (0);

-- Guillaume Sorouy
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3566 AND `patch` IN (0);

-- Flatland Prowler
 UPDATE `creature_template` SET `damage_multiplier` =2.55 WHERE `entry` =3581 AND `patch` IN (0);

-- Sewer Beast
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =3630 AND `patch` IN (0, 6);

-- Deviate Coiler
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =3631 AND `patch` IN (0, 6);

-- Deviate Stinglash
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =3632 AND `patch` IN (0);

-- Deviate Creeper
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =3633 AND `patch` IN (0);

-- Deviate Slayer
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =3634 AND `patch` IN (0);

-- Deviate Stalker
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =3636 AND `patch` IN (0);

-- Deviate Ravager
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =3637 AND `patch` IN (0);

-- Deviate Guardian
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =3641 AND `patch` IN (0);

-- Deviate Lurker
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =3653 AND `patch` IN (0);

-- Kresh
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =3655 AND `patch` IN (0);

-- Mad Magglish
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3662 AND `patch` IN (0);

-- Delmanis the Hated
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3664 AND `patch` IN (0);

-- Ilkrud Magthrull
 UPDATE `creature_template` SET `damage_multiplier` =2.85 WHERE `entry` =3670 AND `patch` IN (0);

-- Lord Pythas
 UPDATE `creature_template` SET `damage_multiplier` =2.5 WHERE `entry` =3671 AND `patch` IN (0);

-- Lady Anacondra
 UPDATE `creature_template` SET `damage_multiplier` =2.85 WHERE `entry` =3673 AND `patch` IN (0);

-- Lord Serpentis
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3717 AND `patch` IN (0);

-- Wrathtail Sorceress
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3773 AND `patch` IN (0);

-- Akkrilus
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3809 AND `patch` IN (0);

-- Ashenvale Bear
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3812 AND `patch` IN (0);

-- Clattering Crawler
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3816 AND `patch` IN (0);

-- Wild Buck
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3817 AND `patch` IN (0);

-- Shadowhorn Stag
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3823 AND `patch` IN (0);

-- Ghostpaw Runner
 UPDATE `creature_template` SET `damage_multiplier` =0.78 WHERE `entry` =3825 AND `patch` IN (0);

-- Ghostpaw Alpha
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =3840 AND `patch` IN (0);

-- Druid of the Fang
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3845 AND `patch` IN (0);

-- Shindrell Swiftfire
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =3850 AND `patch` IN (0);

-- Sorcerer Ashcrombe
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =3851 AND `patch` IN (0);

-- Shadowfang Whitescalp
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =3853 AND `patch` IN (0);

-- Shadowfang Moonwalker
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =3857 AND `patch` IN (0);

-- Shadowfang Glutton
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =3859 AND `patch` IN (0);

-- Shadowfang Ragetooth
 UPDATE `creature_template` SET `damage_multiplier` =3.92 WHERE `entry` =3886 AND `patch` IN (0);

-- Razorclaw the Butcher
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3894 AND `patch` IN (0);

-- Pelturas Whitemoon
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =3914 AND `patch` IN (0);

-- Rethilgore
 UPDATE `creature_template` SET `damage_multiplier` =3.3 WHERE `entry` =3927 AND `patch` IN (0);

-- Wolf Master Nandos
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3936 AND `patch` IN (0);

-- Shandris Feathermoon
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =3937 AND `patch` IN (0);

-- Kira Songshine
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3939 AND `patch` IN (0);

-- Razormane Wolf
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3953 AND `patch` IN (0);

-- Tandaan Lightmane
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3954 AND `patch` IN (0);

-- Dalria
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3958 AND `patch` IN (0);

-- Lardan
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3959 AND `patch` IN (0);

-- Nantar
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3961 AND `patch` IN (0);

-- Maliynn
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3962 AND `patch` IN (0);

-- Haljan Oakheart
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3967 AND `patch` IN (0);

-- Aayndia Floralwind
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3969 AND `patch` IN (0);

-- Fahran Silentblade
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3970 AND `patch` IN (0);

-- Llana
 UPDATE `creature_template` SET `damage_multiplier` =2 WHERE `entry` =3983 AND `patch` IN (0);

-- Interrogator Vishas
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =3984 AND `patch` IN (0);

-- Nancy Vishas
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =3985 AND `patch` IN (0);

-- Grandpa Vishas
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3986 AND `patch` IN (0);

-- Sarilus Foulborne
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3991 AND `patch` IN (0);

-- Venture Co. Deforester
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4003 AND `patch` IN (0);

-- Windshear Geomancer
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4008 AND `patch` IN (0);

-- Cliff Stormer
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4009 AND `patch` IN (0);

-- Raging Cliff Stormer
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4032 AND `patch` IN (0);

-- Young Chimaera
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4040 AND `patch` IN (0);

-- Cave Stalker
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4041 AND `patch` IN (0);

-- Scorched Basilisk
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4063 AND `patch` IN (0);

-- Feeboz
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4064 AND `patch` IN (0);

-- Blackrock Scout
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4065 AND `patch` IN (0);

-- Blackrock Sentry
 UPDATE `creature_template` SET `damage_multiplier` =0.65 WHERE `entry` =4067 AND `patch` IN (0);

-- Twilight Runner
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4079 AND `patch` IN (0);

-- Sentinel Thenysil
 UPDATE `creature_template` SET `damage_multiplier` =1.69 WHERE `entry` =4095 AND `patch` IN (0);

-- Galak Mauler
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4096 AND `patch` IN (0);

-- Galak Windchaser
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4118 AND `patch` IN (0, 6);

-- Venomous Cloud Serpent
 UPDATE `creature_template` SET `damage_multiplier` =0.72 WHERE `entry` =4124 AND `patch` IN (0);

-- Needles Cougar
 UPDATE `creature_template` SET `damage_multiplier` =0.8 WHERE `entry` =4126 AND `patch` IN (0);

-- Crag Stalker
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4128 AND `patch` IN (0);

-- Hecklefang Stalker
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4129 AND `patch` IN (0);

-- Hecklefang Snarler
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4158 AND `patch` IN (0, 5);

-- Salt Flats Vulture
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4248 AND `patch` IN (0);

-- Pesterhide Hyena
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4263 AND `patch` IN (0);

-- Deepmoss Hatchling
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4264 AND `patch` IN (0);

-- Deepmoss Matriarch
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4277 AND `patch` IN (0);

-- Eye of Kilrogg
 UPDATE `creature_template` SET `damage_multiplier` =2 WHERE `entry` =4279 AND `patch` IN (0);

-- Odo the Blindwatcher
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4280 AND `patch` IN (0);

-- Scarlet Preserver
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4281 AND `patch` IN (0);

-- Scarlet Scout
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4282 AND `patch` IN (0);

-- Scarlet Magician
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4283 AND `patch` IN (0);

-- Scarlet Sentry
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4284 AND `patch` IN (0);

-- Scarlet Augur
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4285 AND `patch` IN (0);

-- Scarlet Disciple
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4286 AND `patch` IN (0);

-- Scarlet Soldier
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4287 AND `patch` IN (0);

-- Scarlet Gallant
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4288 AND `patch` IN (0);

-- Scarlet Beastmaster
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4289 AND `patch` IN (0);

-- Scarlet Evoker
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4291 AND `patch` IN (0);

-- Scarlet Diviner
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4292 AND `patch` IN (0);

-- Scarlet Protector
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4293 AND `patch` IN (0);

-- Scarlet Scryer
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4295 AND `patch` IN (0);

-- Scarlet Myrmidon
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4296 AND `patch` IN (0);

-- Scarlet Adept
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4297 AND `patch` IN (0);

-- Scarlet Conjuror
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4299 AND `patch` IN (0);

-- Scarlet Chaplain
 UPDATE `creature_template` SET `damage_multiplier` =1.275 WHERE `entry` =4304 AND `patch` IN (0);

-- Scarlet Tracking Hound
 UPDATE `creature_template` SET `damage_multiplier` =2 WHERE `entry` =4306 AND `patch` IN (0);

-- Scarlet Torturer
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4342 AND `patch` IN (0);

-- Drywallow Vicejaw
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4344 AND `patch` IN (0);

-- Mottled Drywallow Crocolisk
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4351 AND `patch` IN (0);

-- Bloodfen Raptor
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4355 AND `patch` IN (0);

-- Bloodfen Scytheclaw
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4356 AND `patch` IN (0);

-- Bloodfen Razormaw
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4357 AND `patch` IN (0);

-- Bloodfen Lashtail
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4360 AND `patch` IN (0);

-- Mirefin Warrior
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4363 AND `patch` IN (0);

-- Mirefin Oracle
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4376 AND `patch` IN (0);

-- Darkmist Spider
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4377 AND `patch` IN (0);

-- Darkmist Lurker
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4378 AND `patch` IN (0);

-- Darkmist Recluse
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4379 AND `patch` IN (0);

-- Darkmist Silkspinner
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4380 AND `patch` IN (0);

-- Darkmist Widow
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4388 AND `patch` IN (0);

-- Young Murk Thresher
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4389 AND `patch` IN (0);

-- Murk Thresher
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4390 AND `patch` IN (0);

-- Elder Murk Thresher
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4396 AND `patch` IN (0);

-- Mudrock Tortoise
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4397 AND `patch` IN (0);

-- Mudrock Spikeshell
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4398 AND `patch` IN (0);

-- Mudrock Burrower
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4399 AND `patch` IN (0);

-- Mudrock Borer
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4400 AND `patch` IN (0);

-- Mudrock Snapjaw
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4411 AND `patch` IN (0);

-- Darkfang Lurker
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4412 AND `patch` IN (0);

-- Darkfang Creeper
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4413 AND `patch` IN (0);

-- Darkfang Spider
 UPDATE `creature_template` SET `damage_multiplier` =1.2 WHERE `entry` =4415 AND `patch` IN (0);

-- Giant Darkfang Spider
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4427 AND `patch` IN (0);

-- Ward Guardian
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4428 AND `patch` IN (0);

-- Death Speaker Jargba
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4435 AND `patch` IN (0);

-- Razorfen Warrior
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4436 AND `patch` IN (0);

-- Razorfen Quilguard
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4437 AND `patch` IN (0);

-- Razorfen Warden
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4438 AND `patch` IN (0);

-- Razorfen Spearhide
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4440 AND `patch` IN (0);

-- Razorfen Totemic
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4442 AND `patch` IN (0);

-- Razorfen Defender
 UPDATE `creature_template` SET `damage_multiplier` =2.95 WHERE `entry` =4462 AND `patch` IN (0);

-- Blackrock Hunter
 UPDATE `creature_template` SET `damage_multiplier` =2.04 WHERE `entry` =4464 AND `patch` IN (0);

-- Blackrock Gladiator
 UPDATE `creature_template` SET `damage_multiplier` =2.6 WHERE `entry` =4465 AND `patch` IN (0);

-- Vilebranch Warrior
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4494 AND `patch` IN (0);

-- Scarlet Spellbinder
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4505 AND `patch` IN (0);

-- Bloodsail Deckhand
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4506 AND `patch` IN (0);

-- Bloodsail Swabby
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4510 AND `patch` IN (0);

-- Heralath Fallowbrook
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4515 AND `patch` IN (0);

-- Death's Head Acolyte
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4516 AND `patch` IN (0);

-- Death's Head Adept
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4517 AND `patch` IN (0);

-- Death's Head Priest
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4519 AND `patch` IN (0);

-- Death's Head Seer
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4520 AND `patch` IN (0);

-- Razorfen Geomancer
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4522 AND `patch` IN (0);

-- Razorfen Dustweaver
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4523 AND `patch` IN (0);

-- Razorfen Groundshaker
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4530 AND `patch` IN (0);

-- Razorfen Handler
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4532 AND `patch` IN (0);

-- Razorfen Beastmaster
 UPDATE `creature_template` SET `damage_multiplier` =2.16 WHERE `entry` =4540 AND `patch` IN (0);

-- Scarlet Monk
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4623 AND `patch` IN (0);

-- Quilguard Champion
 UPDATE `creature_template` SET `damage_multiplier` =2 WHERE `entry` =4624 AND `patch` IN (0, 2, 10);

-- Booty Bay Bruiser
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4625 AND `patch` IN (0);

-- Death's Head Ward Keeper
 UPDATE `creature_template` SET `damage_multiplier` =1.32 WHERE `entry` =4676 AND `patch` IN (0);

-- Lesser Infernal
 UPDATE `creature_template` SET `damage_multiplier` =1.25 WHERE `entry` =4677 AND `patch` IN (0);

-- Doomwarder
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4679 AND `patch` IN (0);

-- Nether Maiden
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4684 AND `patch` IN (0);

-- Nether Sorceress
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4685 AND `patch` IN (0);

-- Ley Hunter
 UPDATE `creature_template` SET `damage_multiplier` =1.5 WHERE `entry` =4702 AND `patch` IN (0);

-- Ancient Kodo
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4788 AND `patch` IN (0);

-- Fallenroot Satyr
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4789 AND `patch` IN (0);

-- Fallenroot Rogue
 UPDATE `creature_template` SET `damage_multiplier` =2.5 WHERE `entry` =4798 AND `patch` IN (0);

-- Fallenroot Shadowstalker
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4799 AND `patch` IN (0);

-- Fallenroot Hellcaller
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4802 AND `patch` IN (0);

-- Blackfathom Tide Priestess
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4803 AND `patch` IN (0);

-- Blackfathom Oracle
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4805 AND `patch` IN (0);

-- Blackfathom Sea Witch
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4807 AND `patch` IN (0, 7);

-- Blackfathom Myrmidon
 UPDATE `creature_template` SET `damage_multiplier` =1.2 WHERE `entry` =4809 AND `patch` IN (0);

-- Twilight Acolyte
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4810 AND `patch` IN (0);

-- Twilight Reaver
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4811 AND `patch` IN (0);

-- Twilight Aquamancer
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4812 AND `patch` IN (0);

-- Twilight Loreseeker
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4813 AND `patch` IN (0);

-- Twilight Shadowmage
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4814 AND `patch` IN (0);

-- Twilight Elementalist
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4815 AND `patch` IN (0);

-- Murkshallow Snapclaw
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4818 AND `patch` IN (0);

-- Blindlight Murloc
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4819 AND `patch` IN (0);

-- Blindlight Muckdweller
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4820 AND `patch` IN (0);

-- Blindlight Oracle
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4831 AND `patch` IN (0);

-- Lady Sarevess
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4841 AND `patch` IN (0);

-- Deadmire
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4842 AND `patch` IN (0);

-- Earthcaller Halmgar
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4844 AND `patch` IN (0);

-- Shadowforge Surveyor
 UPDATE `creature_template` SET `damage_multiplier` =1.955 WHERE `entry` =4845 AND `patch` IN (0);

-- Shadowforge Ruffian
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4846 AND `patch` IN (0);

-- Shadowforge Digger
 UPDATE `creature_template` SET `damage_multiplier` =2 WHERE `entry` =4847 AND `patch` IN (0);

-- Shadowforge Relic Hunter
 UPDATE `creature_template` SET `damage_multiplier` =1.9 WHERE `entry` =4850 AND `patch` IN (0);

-- Stonevault Cave Lurker
 UPDATE `creature_template` SET `damage_multiplier` =2.31251 WHERE `entry` =4851 AND `patch` IN (0);

-- Stonevault Rockchewer
 UPDATE `creature_template` SET `damage_multiplier` =1.8 WHERE `entry` =4852 AND `patch` IN (0);

-- Stonevault Oracle
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =4856 AND `patch` IN (0);

-- Stonevault Cave Hunter
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4954 AND `patch` IN (0);

-- Uttnar
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =5048 AND `patch` IN (0);

-- Deviate Adder
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5053 AND `patch` IN (0);

-- Deviate Crocolisk
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =5056 AND `patch` IN (0, 6);

-- Deviate Dreadfang
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5195 AND `patch` IN (0);

-- Brown Riding Wolf
 UPDATE `creature_template` SET `damage_multiplier` =2.6 WHERE `entry` =5224 AND `patch` IN (0);

-- Murk Slitherer
 UPDATE `creature_template` SET `damage_multiplier` =2.7 WHERE `entry` =5225 AND `patch` IN (0);

-- Murk Spitter
 UPDATE `creature_template` SET `damage_multiplier` =1.25 WHERE `entry` =5260 AND `patch` IN (0);

-- Groddoc Ape
 UPDATE `creature_template` SET `damage_multiplier` =1.25 WHERE `entry` =5262 AND `patch` IN (0, 7);

-- Groddoc Thunderer
 UPDATE `creature_template` SET `damage_multiplier` =2.7 WHERE `entry` =5269 AND `patch` IN (0);

-- Atal'ai Priest
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5272 AND `patch` IN (0);

-- Grizzled Ironfur Bear
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5286 AND `patch` IN (0);

-- Longtooth Runner
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5287 AND `patch` IN (0, 5);

-- Longtooth Howler
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5300 AND `patch` IN (0);

-- Frayfeather Hippogryph
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5304 AND `patch` IN (0);

-- Frayfeather Stagwing
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5307 AND `patch` IN (0);

-- Vale Screecher
 UPDATE `creature_template` SET `damage_multiplier` =1.25 WHERE `entry` =5347 AND `patch` IN (0);

-- Antilus the Soarer
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5349 AND `patch` IN (0);

-- Arash-ethis
 UPDATE `creature_template` SET `damage_multiplier` =1.15 WHERE `entry` =5352 AND `patch` IN (0);

-- Old Grizzlegut
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5356 AND `patch` IN (0);

-- Snarler
 UPDATE `creature_template` SET `damage_multiplier` =2.8 WHERE `entry` =5401 AND `patch` IN (0);

-- Kazkaz the Unholy
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5403 AND `patch` IN (0);

-- White Stallion
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5404 AND `patch` IN (0);

-- Black Stallion
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5405 AND `patch` IN (0);

-- Pinto
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5406 AND `patch` IN (0);

-- Palomino
 UPDATE `creature_template` SET `damage_multiplier` =0.5 WHERE `entry` =5409 AND `patch` IN (0);

-- Harvester Swarm
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5423 AND `patch` IN (0, 5);

-- Scorpid Tail Lasher
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5426 AND `patch` IN (0);

-- Blisterpaw Hyena
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5428 AND `patch` IN (0);

-- Roc
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5429 AND `patch` IN (0);

-- Fire Roc
 UPDATE `creature_template` SET `damage_multiplier` =3 WHERE `entry` =5434 AND `patch` IN (0);

-- Coral Shark
 UPDATE `creature_template` SET `damage_multiplier` =2.5 WHERE `entry` =5435 AND `patch` IN (0);

-- Sand Shark
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5546 AND `patch` IN (0);

-- Grunt Zuul
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5547 AND `patch` IN (0);

-- Grunt Tharlak
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5615 AND `patch` IN (0);

-- Wastewander Rogue
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5617 AND `patch` IN (0);

-- Wastewander Shadow Mage
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5623 AND `patch` IN (0);

-- Wastewander Assassin
 UPDATE `creature_template` SET `damage_multiplier` =2.3 WHERE `entry` =5645 AND `patch` IN (0);

-- Sandfury Hideskinner
 UPDATE `creature_template` SET `damage_multiplier` =2.4 WHERE `entry` =5646 AND `patch` IN (0);

-- Sandfury Axe Thrower
 UPDATE `creature_template` SET `damage_multiplier` =2.4 WHERE `entry` =5647 AND `patch` IN (0);

-- Sandfury Firecaller
 UPDATE `creature_template` SET `damage_multiplier` =2.4 WHERE `entry` =5648 AND `patch` IN (0);

-- Sandfury Shadowcaster
 UPDATE `creature_template` SET `damage_multiplier` =2.5 WHERE `entry` =5649 AND `patch` IN (0);

-- Sandfury Blood Drinker
 UPDATE `creature_template` SET `damage_multiplier` =2.5 WHERE `entry` =5650 AND `patch` IN (0);

-- Sandfury Witch Doctor
 UPDATE `creature_template` SET `damage_multiplier` =0.5 WHERE `entry` =5694 AND `patch` IN (0);

-- High Sorcerer Andromath
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5748 AND `patch` IN (0);

-- Killian Sanatha
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =5755 AND `patch` IN (0);

-- Deviate Viper
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =5756 AND `patch` IN (0, 6);

-- Deviate Venomwing
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5757 AND `patch` IN (0);

-- Lilly
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5758 AND `patch` IN (0);

-- Leo Sarn
 UPDATE `creature_template` SET `damage_multiplier` =2 WHERE `entry` =5760 AND `patch` IN (0);

-- Lord Azrethoc
 UPDATE `creature_template` SET `damage_multiplier` =0.6 WHERE `entry` =5766 AND `patch` IN (0);

-- Savannah Cub
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5769 AND `patch` IN (0);

-- Arch Druid Hamuul Runetotem
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5774 AND `patch` IN (0);

-- Riding Wolf
 UPDATE `creature_template` SET `damage_multiplier` =0.6 WHERE `entry` =5807 AND `patch` IN (0);

-- The Rake
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =5822 AND `patch` IN (0);

-- Felweaver Scornn
 UPDATE `creature_template` SET `damage_multiplier` =0.96 WHERE `entry` =5823 AND `patch` IN (0);

-- Death Flayer
 UPDATE `creature_template` SET `damage_multiplier` =1.105 WHERE `entry` =5828 AND `patch` IN (0);

-- Humar the Pridelord
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5829 AND `patch` IN (0);

-- Snort the Heckler
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5832 AND `patch` IN (0);

-- Thunderstomp
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5834 AND `patch` IN (0);

-- Azzere the Skyblade
 UPDATE `creature_template` SET `damage_multiplier` =1.75 WHERE `entry` =5842 AND `patch` IN (0);

-- Takk the Leaper
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5843 AND `patch` IN (0);

-- Slave Worker
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5856 AND `patch` IN (0);

-- Glassweb Spider
 UPDATE `creature_template` SET `damage_multiplier` =2.8 WHERE `entry` =5860 AND `patch` IN (0);

-- Twilight Dark Shaman
 UPDATE `creature_template` SET `damage_multiplier` =2.9 WHERE `entry` =5861 AND `patch` IN (0);

-- Twilight Fire Guard
 UPDATE `creature_template` SET `damage_multiplier` =3 WHERE `entry` =5862 AND `patch` IN (0);

-- Twilight Geomancer
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =5865 AND `patch` IN (0);

-- Dishu
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5886 AND `patch` IN (0);

-- Gwyn Farrow
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5933 AND `patch` IN (0);

-- Achellios the Banished
 UPDATE `creature_template` SET `damage_multiplier` =2.35 WHERE `entry` =5934 AND `patch` IN (0);

-- Heartrazor
 UPDATE `creature_template` SET `damage_multiplier` =1.75 WHERE `entry` =5935 AND `patch` IN (0);

-- Ironeye the Invincible
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =5937 AND `patch` IN (0);

-- Vile Sting
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5952 AND `patch` IN (0);

-- Den Grunt
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5981 AND `patch` IN (0);

-- Portal Seeker
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5988 AND `patch` IN (0, 5);

-- Scorpok Stinger
 UPDATE `creature_template` SET `damage_multiplier` =1.32 WHERE `entry` =6007 AND `patch` IN (0);

-- Shadowsworn Enforcer
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6010 AND `patch` IN (0);

-- Felhound
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6013 AND `patch` IN (0, 2);

-- Wayward Buzzard
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =6035 AND `patch` IN (0);

-- Razorfen Stalker
 UPDATE `creature_template` SET `damage_multiplier` =0.3 WHERE `entry` =6071 AND `patch` IN (0);

-- Legion Hound
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6087 AND `patch` IN (0);

-- Astranaar Sentinel
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6126 AND `patch` IN (0);

-- Haldarr Trickster
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6127 AND `patch` IN (0);

-- Haldarr Felsworn
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =6132 AND `patch` IN (0);

-- Razorfen Servitor
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6202 AND `patch` IN (0);

-- Legashi Hellcaller
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6215 AND `patch` IN (0);

-- Chomper
 UPDATE `creature_template` SET `damage_multiplier` =1.44 WHERE `entry` =6243 AND `patch` IN (0);

-- Gelihast
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6250 AND `patch` IN (0);

-- Crawler
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =6367 AND `patch` IN (0);

-- Donni Anthania
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6389 AND `patch` IN (0);

-- Deathguard Podrig
 UPDATE `creature_template` SET `damage_multiplier` =4.08 WHERE `entry` =6498 AND `patch` IN (0);

-- Devilsaur
 UPDATE `creature_template` SET `damage_multiplier` =4.9 WHERE `entry` =6499 AND `patch` IN (0);

-- Ironhide Devilsaur
 UPDATE `creature_template` SET `damage_multiplier` =3.3 WHERE `entry` =6501 AND `patch` IN (0);

-- Stegodon
 UPDATE `creature_template` SET `damage_multiplier` =3.4 WHERE `entry` =6502 AND `patch` IN (0);

-- Plated Stegodon
 UPDATE `creature_template` SET `damage_multiplier` =3.4 WHERE `entry` =6503 AND `patch` IN (0);

-- Spiked Stegodon
 UPDATE `creature_template` SET `damage_multiplier` =3.5 WHERE `entry` =6504 AND `patch` IN (0);

-- Thunderstomp Stegodon
 UPDATE `creature_template` SET `damage_multiplier` =0.975 WHERE `entry` =6505 AND `patch` IN (0);

-- Ravasaur
 UPDATE `creature_template` SET `damage_multiplier` =0.975 WHERE `entry` =6506 AND `patch` IN (0);

-- Ravasaur Runner
 UPDATE `creature_template` SET `damage_multiplier` =0.975 WHERE `entry` =6507 AND `patch` IN (0);

-- Ravasaur Hunter
 UPDATE `creature_template` SET `damage_multiplier` =0.975 WHERE `entry` =6508 AND `patch` IN (0);

-- Venomhide Ravasaur
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =6523 AND `patch` IN (0);

-- Dark Iron Rifleman
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6570 AND `patch` IN (0);

-- Fenwick Thatros
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6574 AND `patch` IN (0);

-- Jun'ha
 UPDATE `creature_template` SET `damage_multiplier` =1.5 WHERE `entry` =6581 AND `patch` IN (0);

-- Ravasaur Matriarch
 UPDATE `creature_template` SET `damage_multiplier` =6.375 WHERE `entry` =6584 AND `patch` IN (0);

-- King Mosh
 UPDATE `creature_template` SET `damage_multiplier` =1.25 WHERE `entry` =6585 AND `patch` IN (0, 7);

-- Uhk'loc
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6707 AND `patch` IN (0);

-- Fahrad
 UPDATE `creature_template` SET `damage_multiplier` =2 WHERE `entry` =6733 AND `patch` IN (0);

-- Stonevault Basher
 UPDATE `creature_template` SET `damage_multiplier` =0.5 WHERE `entry` =6749 AND `patch` IN (0);

-- Erma
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6777 AND `patch` IN (0);

-- Zan Shivsproket
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6779 AND `patch` IN (0);

-- Smudge Thunderwood
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6788 AND `patch` IN (0);

-- Den Mother
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6789 AND `patch` IN (0);

-- Thistle Cub
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6911 AND `patch` IN (0);

-- Minion of Sethir
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7007 AND `patch` IN (0);

-- Tiev Mordune
 UPDATE `creature_template` SET `damage_multiplier` =4.25 WHERE `entry` =7051 AND `patch` IN (0);

-- Malformed Defias Drone
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7052 AND `patch` IN (0);

-- Defias Tower Patroller
 UPDATE `creature_template` SET `damage_multiplier` =3.3 WHERE `entry` =7053 AND `patch` IN (0);

-- Klaven Mortwake
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7111 AND `patch` IN (0);

-- Jadefire Hellcaller
 UPDATE `creature_template` SET `damage_multiplier` =3.4 WHERE `entry` =7135 AND `patch` IN (0);

-- Infernal Bodyguard
 UPDATE `creature_template` SET `damage_multiplier` =3.3 WHERE `entry` =7136 AND `patch` IN (0);

-- Infernal Sentry
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7167 AND `patch` IN (0);

-- Polly
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7168 AND `patch` IN (0);

--  Polly
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =7170 AND `patch` IN (0);

-- Thragomm
 UPDATE `creature_template` SET `damage_multiplier` =0.8 WHERE `entry` =7234 AND `patch` IN (0);

-- Ferocitas the Dream Eater
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7235 AND `patch` IN (0);

-- Gnarlpine Mystic
 UPDATE `creature_template` SET `damage_multiplier` =2.6 WHERE `entry` =7246 AND `patch` IN (0);

-- Sandfury Shadowhunter
 UPDATE `creature_template` SET `damage_multiplier` =1.3 WHERE `entry` =7269 AND `patch` IN (0);

-- Scarab
 UPDATE `creature_template` SET `damage_multiplier` =4.16 WHERE `entry` =7274 AND `patch` IN (0);

-- Sandfury Executioner
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7319 AND `patch` IN (0);

-- Lady Sathrah
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7324 AND `patch` IN (0);

-- Simone Cantrell
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7325 AND `patch` IN (0);

-- Master Kang
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =7335 AND `patch` IN (0);

-- Death's Head Geomancer
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =7337 AND `patch` IN (0);

-- Death's Head Necromancer
 UPDATE `creature_template` SET `damage_multiplier` =1.69 WHERE `entry` =7404 AND `patch` IN (0);

-- Galak Flame Guard
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7489 AND `patch` IN (0);

-- Silverpine Deathguard
 UPDATE `creature_template` SET `damage_multiplier` =1.3 WHERE `entry` =7671 AND `patch` IN (0);

-- Servant of Sevine
 UPDATE `creature_template` SET `damage_multiplier` =3.5 WHERE `entry` =7728 AND `patch` IN (0);

-- Kirith the Damned
 UPDATE `creature_template` SET `damage_multiplier` =1.25 WHERE `entry` =7788 AND `patch` IN (0);

-- Sandfury Drudge
 UPDATE `creature_template` SET `damage_multiplier` =2.5 WHERE `entry` =7789 AND `patch` IN (0, 10);

-- Sandfury Cretin
 UPDATE `creature_template` SET `damage_multiplier` =2.6 WHERE `entry` =7795 AND `patch` IN (0);

-- Hydromancer Velratha
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7803 AND `patch` IN (0, 5);

-- Scorpid Duneburrower
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7855 AND `patch` IN (0);

-- Southsea Pirate
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7856 AND `patch` IN (0);

-- Southsea Freebooter
 UPDATE `creature_template` SET `damage_multiplier` =1.5 WHERE `entry` =7872 AND `patch` IN (0);

-- Death's Head Cultist
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =7873 AND `patch` IN (0);

-- Razorfen Battleguard
 UPDATE `creature_template` SET `damage_multiplier` =1.5 WHERE `entry` =7874 AND `patch` IN (0);

-- Razorfen Thornweaver
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7877 AND `patch` IN (0);

-- Latronicus Moonspear
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7939 AND `patch` IN (0);

-- Feathermoon Sentinel
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7975 AND `patch` IN (0);

-- Mulgore Protector
 UPDATE `creature_template` SET `damage_multiplier` =2.8 WHERE `entry` =7977 AND `patch` IN (0);

-- Gammerita
 UPDATE `creature_template` SET `damage_multiplier` =4 WHERE `entry` =7995 AND `patch` IN (0);

-- Vile Priestess Hexx
 UPDATE `creature_template` SET `damage_multiplier` =3.835 WHERE `entry` =7996 AND `patch` IN (0);

-- Qiaga the Keeper
 UPDATE `creature_template` SET `damage_multiplier` =3.75 WHERE `entry` =8075 AND `patch` IN (0);

-- Edana Hatetalon
 UPDATE `creature_template` SET `damage_multiplier` =2.8 WHERE `entry` =8095 AND `patch` IN (10);

-- Sul'lithuz Sandcrawler
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8147 AND `patch` IN (0);

-- Camp Mojache Brave
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8155 AND `patch` IN (0);

-- Kargath Grunt
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8208 AND `patch` IN (0);

-- Murderous Blisterpaw
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8211 AND `patch` IN (0);

-- Old Cliff Jumper
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8213 AND `patch` IN (0);

-- Ironback
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =8216 AND `patch` IN (0);

-- Retherokk the Berserker
 UPDATE `creature_template` SET `damage_multiplier` =3.2 WHERE `entry` =8217 AND `patch` IN (0);

-- Mith'rethis the Enchanter
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8219 AND `patch` IN (0);

-- Zul'arek Hatefowler
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8236 AND `patch` IN (0);

-- Muck Frenzy
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8277 AND `patch` IN (0);

-- Rekk'tilac
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8280 AND `patch` IN (0);

-- Shleipnarr
 UPDATE `creature_template` SET `damage_multiplier` =3.1 WHERE `entry` =8282 AND `patch` IN (0);

-- Highlord Mastrogonde
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8283 AND `patch` IN (0);

-- Slave Master Blackheart
 UPDATE `creature_template` SET `damage_multiplier` =0.6 WHERE `entry` =8299 AND `patch` IN (0);

-- Spiteflayer
 UPDATE `creature_template` SET `damage_multiplier` =0.65 WHERE `entry` =8300 AND `patch` IN (0);

-- Ravage
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8301 AND `patch` IN (0);

-- Clack the Reaver
 UPDATE `creature_template` SET `damage_multiplier` =1.15 WHERE `entry` =8302 AND `patch` IN (0);

-- Deatheye
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8303 AND `patch` IN (0);

-- Grunter
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8385 AND `patch` IN (0);

-- Mura Runetotem
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8402 AND `patch` IN (0);

-- Enslaved Archaeologist
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8417 AND `patch` IN (0);

-- Dying Archaeologist
 UPDATE `creature_template` SET `damage_multiplier` =3.1 WHERE `entry` =8419 AND `patch` IN (0);

-- Twilight Idolater
 UPDATE `creature_template` SET `damage_multiplier` =2 WHERE `entry` =8444 AND `patch` IN (0);

-- Trade Master Kovic
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8503 AND `patch` IN (0);

-- Gibblewilt
 UPDATE `creature_template` SET `damage_multiplier` =4 WHERE `entry` =8504 AND `patch` IN (0);

-- Dark Iron Sentry
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8508 AND `patch` IN (0);

-- Gretta Ganter
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8550 AND `patch` IN (0);

-- Shadowmage
 UPDATE `creature_template` SET `damage_multiplier` =0.9375 WHERE `entry` =8596 AND `patch` IN (0);

-- Plaguehound Runt
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8602 AND `patch` IN (0);

-- Monstrous Plaguebat
 UPDATE `creature_template` SET `damage_multiplier` =3.835 WHERE `entry` =8636 AND `patch` IN (0);

-- Morta'gya the Keeper
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8637 AND `patch` IN (0);

-- Dark Iron Watchman
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8660 AND `patch` IN (0);

-- The Evalcharr
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8675 AND `patch` IN (0);

-- Felbeast
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8882 AND `patch` IN (0);

-- Riding Tiger
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8883 AND `patch` IN (0);

-- Riding Horse
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8885 AND `patch` IN (0);

-- Riding Raptor
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8886 AND `patch` IN (0);

-- Deviate Python
 UPDATE `creature_template` SET `damage_multiplier` =2.32 WHERE `entry` =8889 AND `patch` IN (0);

-- Anvilrage Overseer
 UPDATE `creature_template` SET `damage_multiplier` =3 WHERE `entry` =8890 AND `patch` IN (0);

-- Anvilrage Warden
 UPDATE `creature_template` SET `damage_multiplier` =2.9 WHERE `entry` =8917 AND `patch` IN (0);

-- Quarry Slave
 UPDATE `creature_template` SET `damage_multiplier` =4.5 WHERE `entry` =9042 AND `patch` IN (0);

-- Verek
 UPDATE `creature_template` SET `damage_multiplier` =3.4 WHERE `entry` =9043 AND `patch` IN (0);

-- Scarshield Grunt
 UPDATE `creature_template` SET `damage_multiplier` =3.4 WHERE `entry` =9044 AND `patch` IN (0);

-- Scarshield Sentry
 UPDATE `creature_template` SET `damage_multiplier` =3.5 WHERE `entry` =9045 AND `patch` IN (0);

-- Scarshield Acolyte
 UPDATE `creature_template` SET `damage_multiplier` =3.5 WHERE `entry` =9046 AND `patch` IN (0);

-- Scarshield Quartermaster
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =9077 AND `patch` IN (0);

-- Warlord Goretooth
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =9080 AND `patch` IN (0);

-- Lexlort
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =9081 AND `patch` IN (0);

-- Galamav the Marksman
 UPDATE `creature_template` SET `damage_multiplier` =3.5 WHERE `entry` =9097 AND `patch` IN (0);

-- Scarshield Legionnaire
 UPDATE `creature_template` SET `damage_multiplier` =3.5 WHERE `entry` =9098 AND `patch` IN (0);

-- Scarshield Spellbinder
 UPDATE `creature_template` SET `damage_multiplier` =3.5 WHERE `entry` =9297 AND `patch` IN (0);

-- Enraged Wyvern
 UPDATE `creature_template` SET `damage_multiplier` =2 WHERE `entry` =9460 AND `patch` IN (0, 2, 10);

-- Gadgetzan Bruiser
 UPDATE `creature_template` SET `damage_multiplier` =3.5 WHERE `entry` =9521 AND `patch` IN (0, 2);

-- Enraged Felbat
 UPDATE `creature_template` SET `damage_multiplier` =3.5 WHERE `entry` =9526 AND `patch` IN (0);

-- Enraged Gryphon
 UPDATE `creature_template` SET `damage_multiplier` =3.5 WHERE `entry` =9527 AND `patch` IN (0);

-- Enraged Hippogryph
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =9553 AND `patch` IN (0);

-- Nadia Vernon
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =9555 AND `patch` IN (0);

-- Mu'uta
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =9683 AND `patch` IN (0);

-- Lar'korwi Mate
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =9684 AND `patch` IN (0);

-- Lar'korwi
 UPDATE `creature_template` SET `damage_multiplier` =1.15 WHERE `entry` =9694 AND `patch` IN (0);

-- Slavering Ember Worg
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =10036 AND `patch` IN (0);

-- Brackenwall Enforcer
 UPDATE `creature_template` SET `damage_multiplier` =0.81 WHERE `entry` =10077 AND `patch` IN (0);

-- Deathmaw
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =10078 AND `patch` IN (0);

-- Terrorspark
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =10200 AND `patch` IN (0);

-- Rak'shiri
 UPDATE `creature_template` SET `damage_multiplier` =1.25 WHERE `entry` =10356 AND `patch` IN (0);

-- Bayne
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =10357 AND `patch` IN (0);

-- Ressan the Needler
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =10359 AND `patch` IN (0);

-- Sri'skulk
 UPDATE `creature_template` SET `damage_multiplier` =0.8125 WHERE `entry` =10644 AND `patch` IN (0, 5);

-- Mist Howler
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =10647 AND `patch` IN (0);

-- Prince Raze
 UPDATE `creature_template` SET `damage_multiplier` =1.9 WHERE `entry` =10696 AND `patch` IN (0);

-- Refuge Pointe Defender
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =10717 AND `patch` IN (0);

-- Temporal Parasite
 UPDATE `creature_template` SET `damage_multiplier` =2.1 WHERE `entry` =10738 AND `patch` IN (0);

-- High Chief Winterfall
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =10760 AND `patch` IN (0);

-- Grimtotem Geomancer
 UPDATE `creature_template` SET `damage_multiplier` =3.835 WHERE `entry` =10802 AND `patch` IN (0);

-- Hitah'ya the Keeper
 UPDATE `creature_template` SET `damage_multiplier` =4.8 WHERE `entry` =10806 AND `patch` IN (0);

-- Ursius
 UPDATE `creature_template` SET `damage_multiplier` =3.8 WHERE `entry` =10807 AND `patch` IN (0);

-- Brumeran
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =10882 AND `patch` IN (0);

-- Arikara
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =10979 AND `patch` IN (0);

-- Scarlet Hound
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11034 AND `patch` IN (0);

-- Lord Maxwell Tyrosus
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11036 AND `patch` IN (0);

-- Leonid Barthalomew the Revered
 UPDATE `creature_template` SET `damage_multiplier` =9.45 WHERE `entry` =11079 AND `patch` IN (0);

-- Wynd Nightchaser
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11137 AND `patch` IN (0);

-- Xai'ander
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11180 AND `patch` IN (0);

-- Bloodvenom Post Brave
 UPDATE `creature_template` SET `damage_multiplier` =2 WHERE `entry` =11190 AND `patch` IN (0, 2, 10);

-- Everlook Bruiser
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =11318 AND `patch` IN (0);

-- Ragefire Trogg
 UPDATE `creature_template` SET `damage_multiplier` =1.5 WHERE `entry` =11319 AND `patch` IN (0);

-- Ragefire Shaman
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =11322 AND `patch` IN (0);

-- Searing Blade Cultist
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =11323 AND `patch` IN (0);

-- Searing Blade Enforcer
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =11324 AND `patch` IN (0);

-- Searing Blade Warlock
 UPDATE `creature_template` SET `damage_multiplier` =4 WHERE `entry` =11346 AND `patch` IN (5);

-- Hakkari Oracle
 UPDATE `creature_template` SET `damage_multiplier` =3.5 WHERE `entry` =11355 AND `patch` IN (0);

-- Gurubashi Warrior
 UPDATE `creature_template` SET `damage_multiplier` =4.5 WHERE `entry` =11383 AND `patch` IN (0);

-- High Priestess Hai'watna
 UPDATE `creature_template` SET `damage_multiplier` =3.4 WHERE `entry` =11440 AND `patch` IN (0);

-- Gordok Enforcer
 UPDATE `creature_template` SET `damage_multiplier` =3.4 WHERE `entry` =11442 AND `patch` IN (0);

-- Gordok Mauler
 UPDATE `creature_template` SET `damage_multiplier` =3.3 WHERE `entry` =11443 AND `patch` IN (0);

-- Gordok Ogre-Mage
 UPDATE `creature_template` SET `damage_multiplier` =2.2 WHERE `entry` =11517 AND `patch` IN (0);

-- Oggleflint
 UPDATE `creature_template` SET `damage_multiplier` =2 WHERE `entry` =11518 AND `patch` IN (0);

-- Jergosh the Invoker
 UPDATE `creature_template` SET `damage_multiplier` =2.2 WHERE `entry` =11519 AND `patch` IN (0);

-- Bazzalan
 UPDATE `creature_template` SET `damage_multiplier` =2.4 WHERE `entry` =11520 AND `patch` IN (0);

-- Taragaman the Hungerer
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11564 AND `patch` IN (0);

-- Gizelton Caravan Kodo
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11611 AND `patch` IN (0);

-- Cavalier Durgen
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11656 AND `patch` IN (0);

-- Horde Peon
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11681 AND `patch` IN (0);

-- Horde Deforester
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11683 AND `patch` IN (0);

-- Horde Shaman
 UPDATE `creature_template` SET `damage_multiplier` =1.1 WHERE `entry` =11697 AND `patch` IN (0);

-- Mannoroc Lasher
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11739 AND `patch` IN (0);

-- Rock Stalker
 UPDATE `creature_template` SET `damage_multiplier` =2.5 WHERE `entry` =11787 AND `patch` IN (0);

-- Rock Borer
 UPDATE `creature_template` SET `damage_multiplier` =2.4 WHERE `entry` =11788 AND `patch` IN (0);

-- Rock Worm
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11822 AND `patch` IN (7, 10);

-- Moonglade Warden
 UPDATE `creature_template` SET `damage_multiplier` =3 WHERE `entry` =11859 AND `patch` IN (7);

-- Doomguard
 UPDATE `creature_template` SET `damage_multiplier` =7.1 WHERE `entry` =11871 AND `patch` IN (0);

-- Grinning Dog
 UPDATE `creature_template` SET `damage_multiplier` =5.5 WHERE `entry` =11885 AND `patch` IN (0);

-- Blighthound
 UPDATE `creature_template` SET `damage_multiplier` =6.5 WHERE `entry` =11896 AND `patch` IN (0);

-- Borelgore
 UPDATE `creature_template` SET `damage_multiplier` =5.5 WHERE `entry` =11897 AND `patch` IN (0);

-- Duskwing
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =11921 AND `patch` IN (0);

-- Besseleth
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =11937 AND `patch` IN (0);

-- Demon Portal Guardian
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11956 AND `patch` IN (0);

-- Great Bear Spirit
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11957 AND `patch` IN (0);

-- Great Cat Spirit
 UPDATE `creature_template` SET `damage_multiplier` =3 WHERE `entry` =12123 AND `patch` IN (0);

-- Reef Shark
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =12338 AND `patch` IN (0);

-- Shadowprey Guardian
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =12346 AND `patch` IN (0);

-- Emerald Raptor
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =12349 AND `patch` IN (0);

-- Turquoise Raptor
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =12350 AND `patch` IN (0);

-- Violet Raptor
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =12351 AND `patch` IN (0);

-- Dire Riding Wolf
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =12353 AND `patch` IN (0);

-- Timber Riding Wolf
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =12354 AND `patch` IN (0);

-- Brown Riding Kodo
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =12355 AND `patch` IN (0);

-- Gray Riding Kodo
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =12372 AND `patch` IN (0);

-- Brown Ram
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =12373 AND `patch` IN (0);

-- Gray Ram
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =12374 AND `patch` IN (0);

-- White Riding Ram
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =12375 AND `patch` IN (0);

-- Chestnut Mare
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =12376 AND `patch` IN (0);

-- Brown Horse
 UPDATE `creature_template` SET `damage_multiplier` =1.95 WHERE `entry` =12418 AND `patch` IN (0);

-- Gordok Hyena
 UPDATE `creature_template` SET `damage_multiplier` =4 WHERE `entry` =12425 AND `patch` IN (0);

-- Flint Shadowmore
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =12431 AND `patch` IN (0);

-- Gorefang
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =12432 AND `patch` IN (0);

-- Old Vicejaw
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =12433 AND `patch` IN (0);

-- Krethis Shadowspinner
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =12579 AND `patch` IN (0);

-- Bloodfury Ripper
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =12902 AND `patch` IN (0);

-- Lorgus Jett
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =12920 AND `patch` IN (0);

-- Doctor Gregory Victor
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =13085 AND `patch` IN (0);

-- Myrokos Silentform
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =13158 AND `patch` IN (0);

-- Lieutenant Sanders
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =13602 AND `patch` IN (0);

-- The Abominable Greench
 UPDATE `creature_template` SET `damage_multiplier` =1.25 WHERE `entry` =13896 AND `patch` IN (0, 7);

-- Scalebeard
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14221 AND `patch` IN (0);

-- Gravis Slipknot
 UPDATE `creature_template` SET `damage_multiplier` =0.7 WHERE `entry` =14222 AND `patch` IN (0);

-- Araga
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14223 AND `patch` IN (0);

-- Cranky Benj
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14232 AND `patch` IN (0);

-- Dart
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14233 AND `patch` IN (0);

-- Ripscale
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14268 AND `patch` IN (1);

-- Lord Condar
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14277 AND `patch` IN (0);

-- Lady Zephris
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14278 AND `patch` IN (0);

-- Ro'Bark
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14279 AND `patch` IN (0);

-- Creepthess
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14280 AND `patch` IN (0);

-- Big Samras
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14281 AND `patch` IN (0);

-- Jimmy the Bleeder
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14343 AND `patch` IN (0);

-- Olm the Wise
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14344 AND `patch` IN (1);

-- Mongress
 UPDATE `creature_template` SET `damage_multiplier` =12 WHERE `entry` =14394 AND `patch` IN (1);

-- Major Mattingly
 UPDATE `creature_template` SET `damage_multiplier` =0.77 WHERE `entry` =14430 AND `patch` IN (0);

-- Duskstalker
 UPDATE `creature_template` SET `damage_multiplier` =4 WHERE `entry` =14442 AND `patch` IN (2);

-- Hunter Thunderhorn
 UPDATE `creature_template` SET `damage_multiplier` =0.675 WHERE `entry` =14490 AND `patch` IN (2);

-- Rippa
 UPDATE `creature_template` SET `damage_multiplier` =1.25 WHERE `entry` =14491 AND `patch` IN (0);

-- Kurmokk
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14539 AND `patch` IN (2);

-- Swift Timber Wolf
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14540 AND `patch` IN (2);

-- Swift Brown Wolf
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14541 AND `patch` IN (2);

-- Swift Gray Wolf
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14542 AND `patch` IN (2);

-- Great White Kodo
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14543 AND `patch` IN (2);

-- Swift Olive Raptor
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14544 AND `patch` IN (2);

-- Swift Orange Raptor
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14545 AND `patch` IN (2);

-- Swift Blue Raptor
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14546 AND `patch` IN (2);

-- Swift Brown Ram
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14547 AND `patch` IN (2);

-- Swift White Ram
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14548 AND `patch` IN (2);

-- Swift Gray Ram
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14549 AND `patch` IN (2);

-- Great Brown Kodo
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14559 AND `patch` IN (2);

-- Swift Palamino
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14560 AND `patch` IN (2);

-- Swift White Steed
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14561 AND `patch` IN (2);

-- Swift Brown Steed
 UPDATE `creature_template` SET `damage_multiplier` =4 WHERE `entry` =14621 AND `patch` IN (3);

-- Overseer Maltorius
 UPDATE `creature_template` SET `damage_multiplier` =12 WHERE `entry` =14721 AND `patch` IN (3);

-- Field Marshal Afrasiabi
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14730 AND `patch` IN (3);

-- Revantusk Watcher
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14757 AND `patch` IN (3);

-- Elder Torntusk
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14984 AND `patch` IN (5);

-- Sergeant Maclear
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =15021 AND `patch` IN (5);

-- Deathmaster Dwire
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =15022 AND `patch` IN (5);

-- Deathstalker Mortis
 UPDATE `creature_template` SET `damage_multiplier` =2 WHERE `entry` =15138 AND `patch` IN (5);

-- Silverpine Elite
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =15174 AND `patch` IN (6);

-- Calandrath
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =15175 AND `patch` IN (6);

-- Khur Hornstriker
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =15176 AND `patch` IN (6);

-- Vargus
 UPDATE `creature_template` SET `damage_multiplier` =3 WHERE `entry` =15177 AND `patch` IN (6);

-- Cloud Skydancer
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =15179 AND `patch` IN (6);

-- Mishta
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =15180 AND `patch` IN (6);

-- Baristolth of the Shifting Sands
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =15183 AND `patch` IN (6);

-- Geologist Larksbane
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =15191 AND `patch` IN (6);

-- Windcaller Proudhorn
 UPDATE `creature_template` SET `damage_multiplier` =3 WHERE `entry` =15196 AND `patch` IN (6);

-- Deathclasp
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =15270 AND `patch` IN (6);

-- Huum Wildmane
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =15282 AND `patch` IN (6);

-- Aurel Goldleaf
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =15306 AND `patch` IN (6);

-- Bor Wildmane
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =15308 AND `patch` IN (6);

-- Twilight Prophet
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =15328 AND `patch` IN (8);

-- Steam Tonk
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =15350 AND `patch` IN (6);

-- Horde Warbringer
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =15419 AND `patch` IN (6);

-- Kania
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =15692 AND `patch` IN (6);

-- Dark Iron Kidnapper
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =15722 AND `patch` IN (7);

-- Squire Leoren Mal'derath
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =15760 AND `patch` IN (6);

-- Winter Reveler
 UPDATE `creature_template` SET `damage_multiplier` =3 WHERE `entry` =16096 AND `patch` IN (8);

-- Steamwheedle Bruiser
 UPDATE `creature_template` SET `damage_multiplier` =35 WHERE `entry` =16506 AND `patch` IN (9);

-- Naxxramas Worshipper
 UPDATE `creature_template` SET `damage_multiplier` =45.5 WHERE `entry` =16803 AND `patch` IN (9);

-- Deathknight Understudy
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =17804 AND `patch` IN (10);

-- Squire Rowe
 -- Armor Multipliers
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =30 AND `patch` IN (0);

-- Forest Spider
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =38 AND `patch` IN (0);

-- Defias Thug
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =43 AND `patch` IN (0);

-- Mine Spider
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =68 AND `patch` IN (0);

-- Stormwind City Guard
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =80 AND `patch` IN (0);

-- Kobold Laborer
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =94 AND `patch` IN (0);

-- Defias Cutpurse
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =103 AND `patch` IN (0);

-- Garrick Padfoot
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =116 AND `patch` IN (0);

-- Defias Bandit
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =118 AND `patch` IN (0);

-- Prowler
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =125 AND `patch` IN (0);

-- Riverpaw Overseer
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =257 AND `patch` IN (0);

-- Kobold Worker
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =466 AND `patch` IN (0);

-- General Marcus Jonathan
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =469 AND `patch` IN (0);

-- Lieutenant Doren
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =474 AND `patch` IN (0);

-- Defias Rogue Wizard
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =476 AND `patch` IN (0);

-- Kobold Geomancer
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =574 AND `patch` IN (0);

-- Naraxis
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =659 AND `patch` IN (0);

-- El Pollo Grande
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =686 AND `patch` IN (0);

-- Lashtail Raptor
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =708 AND `patch` IN (0, 7);

-- Small Crag Boar
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =754 AND `patch` IN (0);

-- Rebel Watchman
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =756 AND `patch` IN (0);

-- Skullsplitter Panther
 UPDATE `creature_template` SET `armor_multiplier` =1.5 WHERE `entry` =814 AND `patch` IN (0);

-- Sergeant Malthus
 UPDATE `creature_template` SET `armor_multiplier` =0 WHERE `entry` =818 AND `patch` IN (0);

-- Mai'Zoth
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =819 AND `patch` IN (0);

-- Servant of Ilgalar
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =854 AND `patch` IN (0);

-- Young Jungle Stalker
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =861 AND `patch` IN (0);

-- Stonard Scout
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =866 AND `patch` IN (0);

-- Stonard Grunt
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =875 AND `patch` IN (0);

-- Saltscale Tide Lord
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =881 AND `patch` IN (0);

-- Surena Caledon
 UPDATE `creature_template` SET `armor_multiplier` =1.5 WHERE `entry` =922 AND `patch` IN (0);

-- Silt Crawler
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =946 AND `patch` IN (0);

-- Frostmane Novice
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1020 AND `patch` IN (0);

-- Mottled Raptor
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1034 AND `patch` IN (0);

-- Dragonmaw Raider
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1035 AND `patch` IN (0);

-- Dragonmaw Swamprunner
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1036 AND `patch` IN (0);

-- Dragonmaw Centurion
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1037 AND `patch` IN (0);

-- Dragonmaw Battlemaster
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1038 AND `patch` IN (0);

-- Dragonmaw Shadowwarder
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1057 AND `patch` IN (0);

-- Dragonmaw Bonewarder
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1064 AND `patch` IN (0);

-- Grom'gol Grunt
 UPDATE `creature_template` SET `armor_multiplier` =1.5 WHERE `entry` =1088 AND `patch` IN (0);

-- Monstrous Crawler
 UPDATE `creature_template` SET `armor_multiplier` =0 WHERE `entry` =1108 AND `patch` IN (0, 7);

-- Mistvale Gorilla
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1115 AND `patch` IN (0);

-- Rockjaw Skullthumper
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1121 AND `patch` IN (0);

-- Frostmane Snowstrider
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1124 AND `patch` IN (0);

-- Frostmane Shadowcaster
 UPDATE `creature_template` SET `armor_multiplier` =0.3 WHERE `entry` =1129 AND `patch` IN (0);

-- Black Bear
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1140 AND `patch` IN (0);

-- Razormaw Matriarch
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1199 AND `patch` IN (0);

-- Juvenile Snow Leopard
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1201 AND `patch` IN (0);

-- Snow Leopard
 UPDATE `creature_template` SET `armor_multiplier` =1.2 WHERE `entry` =1423 AND `patch` IN (0);

-- Stormwind Guard
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1504 AND `patch` IN (0);

-- Young Night Web Spider
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1505 AND `patch` IN (0);

-- Night Web Spider
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1506 AND `patch` IN (0);

-- Scarlet Convert
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1507 AND `patch` IN (0);

-- Scarlet Initiate
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1509 AND `patch` IN (0);

-- Ragged Scavenger
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1511 AND `patch` IN (0);

-- Enraged Silverback Gorilla
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1513 AND `patch` IN (0);

-- Mangy Duskbat
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1514 AND `patch` IN (0);

-- Mokk the Savage
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1535 AND `patch` IN (0);

-- Scarlet Warrior
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1536 AND `patch` IN (0);

-- Scarlet Missionary
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1537 AND `patch` IN (0);

-- Scarlet Zealot
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1538 AND `patch` IN (0);

-- Scarlet Friar
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1543 AND `patch` IN (0);

-- Vile Fin Puddlejumper
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1545 AND `patch` IN (0);

-- Vile Fin Muckdweller
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1549 AND `patch` IN (0);

-- Ravenous Darkhound
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1551 AND `patch` IN (0);

-- Ironjaw Basilisk
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1555 AND `patch` IN (0);

-- Vicious Night Web Spider
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1559 AND `patch` IN (0);

-- King Mukla
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1563 AND `patch` IN (0);

-- Bloodsail Swashbuckler
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1564 AND `patch` IN (0);

-- Bloodsail Warlock
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1565 AND `patch` IN (0);

-- Bloodsail Sea Dog
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1642 AND `patch` IN (0);

-- Northshire Guard
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1653 AND `patch` IN (0);

-- Bloodsail Elder Magus
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1666 AND `patch` IN (0);

-- Kam Deepfury
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1688 AND `patch` IN (0);

-- Night Web Matriarch
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1708 AND `patch` IN (0);

-- Defias Inmate
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1711 AND `patch` IN (0);

-- Defias Convict
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1715 AND `patch` IN (0);

-- Defias Insurgent
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1716 AND `patch` IN (0);

-- Bazil Thredd
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1717 AND `patch` IN (0);

-- Hamhock
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1769 AND `patch` IN (0);

-- Moonrage Whitescalp
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1838 AND `patch` IN (0);

-- Scarlet Interrogator
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1839 AND `patch` IN (0);

-- Scarlet High Clerist
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1843 AND `patch` IN (0);

-- Foreman Jerris
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1846 AND `patch` IN (0);

-- High Protector Lorik
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1854 AND `patch` IN (0);

-- High Priest Thel'danis
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1985 AND `patch` IN (0, 7);

-- Thistle Boar
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1986 AND `patch` IN (0);

-- Webwood Spider
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1988 AND `patch` IN (0);

-- Grell
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1989 AND `patch` IN (0);

-- Grellkin
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1994 AND `patch` IN (0);

-- Githyiss the Vile
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1998 AND `patch` IN (0);

-- Webwood Lurker
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1999 AND `patch` IN (0);

-- Webwood Venomfang
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2000 AND `patch` IN (0);

-- Webwood Silkspinner
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2003 AND `patch` IN (0);

-- Shadow Sprite
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2018 AND `patch` IN (0);

-- Bloodfeather Sorceress
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2033 AND `patch` IN (0);

-- Elder Nightsaber
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2042 AND `patch` IN (0);

-- Nightsaber
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2043 AND `patch` IN (0, 1);

-- Nightsaber Stalker
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2091 AND `patch` IN (0);

-- Chieftain Nek'rosh
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2174 AND `patch` IN (0);

-- Coastal Frenzy
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2185 AND `patch` IN (0);

-- Darkshore Thresher
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2187 AND `patch` IN (0);

-- Elder Darkshore Thresher
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2226 AND `patch` IN (0);

-- Karos Razok
 UPDATE `creature_template` SET `armor_multiplier` =1.5 WHERE `entry` =2231 AND `patch` IN (0);

-- Pygmy Tide Crawler
 UPDATE `creature_template` SET `armor_multiplier` =1.5 WHERE `entry` =2232 AND `patch` IN (0);

-- Tide Crawler
 UPDATE `creature_template` SET `armor_multiplier` =1.5 WHERE `entry` =2233 AND `patch` IN (0);

-- Encrusted Tide Crawler
 UPDATE `creature_template` SET `armor_multiplier` =1.5 WHERE `entry` =2234 AND `patch` IN (0);

-- Young Reef Crawler
 UPDATE `creature_template` SET `armor_multiplier` =1.5 WHERE `entry` =2235 AND `patch` IN (0);

-- Reef Crawler
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2238 AND `patch` IN (0);

-- Tog'thar
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2319 AND `patch` IN (0);

-- Syndicate Wizard
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2321 AND `patch` IN (0);

-- Foreststrider Fledgling
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2322 AND `patch` IN (0);

-- Foreststrider
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2323 AND `patch` IN (0);

-- Giant Foreststrider
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2387 AND `patch` IN (0);

-- Hillsbrad Councilman
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2389 AND `patch` IN (0);

-- Zarise
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2391 AND `patch` IN (0);

-- Serge Hinott
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2392 AND `patch` IN (0);

-- Delia Verana
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2393 AND `patch` IN (0);

-- Christoph Jeffcoat
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2394 AND `patch` IN (0);

-- Mallen Swain
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2395 AND `patch` IN (0);

-- Vinna Wayne
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2396 AND `patch` IN (0);

-- Hans Zandin
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2397 AND `patch` IN (0);

-- Derak Nightfall
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2398 AND `patch` IN (0);

-- Tara Coldgaze
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2400 AND `patch` IN (0);

-- Craig Hewitt
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2402 AND `patch` IN (0);

-- Shara Blazen
 UPDATE `creature_template` SET `armor_multiplier` =1.2 WHERE `entry` =2405 AND `patch` IN (0);

-- Tarren Mill Deathguard
 UPDATE `creature_template` SET `armor_multiplier` =1.2 WHERE `entry` =2418 AND `patch` IN (0);

-- Deathguard Samsa
 UPDATE `creature_template` SET `armor_multiplier` =1.2 WHERE `entry` =2419 AND `patch` IN (0);

-- Deathguard Humbert
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2440 AND `patch` IN (0);

-- Drunken Footpad
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2478 AND `patch` IN (0);

-- Haren Swifthoof
 UPDATE `creature_template` SET `armor_multiplier` =1.2 WHERE `entry` =2518 AND `patch` IN (0);

-- Mountaineer Swarth
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2521 AND `patch` IN (0);

-- Skymane Gorilla
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2522 AND `patch` IN (0);

-- Jaguero Stalker
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2529 AND `patch` IN (0);

-- Son of Arugal
 UPDATE `creature_template` SET `armor_multiplier` =1.5 WHERE `entry` =2544 AND `patch` IN (0);

-- Southern Sand Crawler
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2546 AND `patch` IN (0);

-- Fleet Master Firallon
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2548 AND `patch` IN (0);

-- Captain Keelhaul
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2549 AND `patch` IN (0);

-- Garr Salthoof
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2550 AND `patch` IN (0);

-- Captain Stillwater
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2561 AND `patch` IN (0);

-- Highland Fleshstalker
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2578 AND `patch` IN (0);

-- Young Mesa Buzzard
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2605 AND `patch` IN (0);

-- Zalas Witherbark
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2636 AND `patch` IN (0);

-- Blackwater Deckhand
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2647 AND `patch` IN (0);

-- Vilebranch Soul Eater
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2657 AND `patch` IN (0);

-- Trained Razorbeak
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2732 AND `patch` IN (0);

-- Ridge Huntress
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2734 AND `patch` IN (0);

-- Ridge Stalker Patriarch
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2738 AND `patch` IN (0);

-- Stromgarde Cavalryman
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2787 AND `patch` IN (0);

-- Zaruk
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2821 AND `patch` IN (0);

-- Keena
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2829 AND `patch` IN (0);

-- Starving Buzzard
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2830 AND `patch` IN (0);

-- Buzzard
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2831 AND `patch` IN (0);

-- Giant Buzzard
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2892 AND `patch` IN (0);

-- Stonevault Seer
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2932 AND `patch` IN (0);

-- Magregan Deepshadow
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2954 AND `patch` IN (0, 7);

-- Bristleback Battleboar
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2956 AND `patch` IN (0);

-- Adult Plainstrider
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2957 AND `patch` IN (0);

-- Elder Plainstrider
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2961 AND `patch` IN (0);

-- Mountain Cougar
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2966 AND `patch` IN (0, 7);

-- Battleboar
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2969 AND `patch` IN (0);

-- Wiry Swoop
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2970 AND `patch` IN (0);

-- Swoop
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2971 AND `patch` IN (0);

-- Taloned Swoop
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2972 AND `patch` IN (0);

-- Kodo Calf
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2974 AND `patch` IN (0);

-- Kodo Matriarch
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3035 AND `patch` IN (0);

-- Flatland Cougar
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3084 AND `patch` IN (0);

-- Bluffwatcher
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3106 AND `patch` IN (0);

-- Pygmy Surf Crawler
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3107 AND `patch` IN (0);

-- Surf Crawler
 UPDATE `creature_template` SET `armor_multiplier` =1.15 WHERE `entry` =3108 AND `patch` IN (0);

-- Encrusted Surf Crawler
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3110 AND `patch` IN (0);

-- Dreadmaw Crocolisk
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3120 AND `patch` IN (0);

-- Kolkar Outrunner
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3121 AND `patch` IN (0);

-- Durotar Tiger
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3122 AND `patch` IN (0);

-- Bloodtalon Taillasher
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3123 AND `patch` IN (0);

-- Bloodtalon Scythemaw
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3124 AND `patch` IN (0);

-- Scorpid Worker
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3125 AND `patch` IN (0);

-- Clattering Scorpid
 UPDATE `creature_template` SET `armor_multiplier` =1.5 WHERE `entry` =3126 AND `patch` IN (0);

-- Armored Scorpid
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3127 AND `patch` IN (0);

-- Venomtail Scorpid
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3130 AND `patch` IN (0);

-- Thunder Lizard
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3210 AND `patch` IN (0);

-- Brave Proudsnout
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3211 AND `patch` IN (0);

-- Brave Lightninghorn
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3213 AND `patch` IN (0);

-- Brave Running Wolf
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3214 AND `patch` IN (0);

-- Brave Greathoof
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3234 AND `patch` IN (0);

-- Lost Barrens Kodo
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3246 AND `patch` IN (0);

-- Fleeting Plainstrider
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3251 AND `patch` IN (0);

-- Silithid Grub
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3254 AND `patch` IN (0);

-- Sunscale Lashtail
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3255 AND `patch` IN (0);

-- Sunscale Screecher
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3269 AND `patch` IN (0);

-- Razormane Geomancer
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3281 AND `patch` IN (0);

-- Sarkoth
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3296 AND `patch` IN (0);

-- Orgrimmar Grunt
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3537 AND `patch` IN (0);

-- Zixil
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3539 AND `patch` IN (0);

-- Ott
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3581 AND `patch` IN (0);

-- Sewer Beast
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3634 AND `patch` IN (0);

-- Deviate Stalker
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3655 AND `patch` IN (0);

-- Mad Magglish
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3857 AND `patch` IN (0);

-- Shadowfang Glutton
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3859 AND `patch` IN (0);

-- Shadowfang Ragetooth
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3886 AND `patch` IN (0);

-- Razorclaw the Butcher
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3927 AND `patch` IN (0);

-- Wolf Master Nandos
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3936 AND `patch` IN (0);

-- Shandris Feathermoon
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3939 AND `patch` IN (0);

-- Razormane Wolf
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3962 AND `patch` IN (0);

-- Haljan Oakheart
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3967 AND `patch` IN (0);

-- Aayndia Floralwind
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3986 AND `patch` IN (0);

-- Sarilus Foulborne
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4008 AND `patch` IN (0);

-- Cliff Stormer
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4044 AND `patch` IN (0);

-- Blackened Basilisk
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4124 AND `patch` IN (0);

-- Needles Cougar
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4158 AND `patch` IN (0, 5);

-- Salt Flats Vulture
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4248 AND `patch` IN (0);

-- Pesterhide Hyena
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4277 AND `patch` IN (0);

-- Eye of Kilrogg
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4279 AND `patch` IN (0);

-- Odo the Blindwatcher
 UPDATE `creature_template` SET `armor_multiplier` =1.5 WHERE `entry` =4286 AND `patch` IN (0);

-- Scarlet Soldier
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4289 AND `patch` IN (0);

-- Scarlet Evoker
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4290 AND `patch` IN (0);

-- Scarlet Guardsman
 UPDATE `creature_template` SET `armor_multiplier` =1.5 WHERE `entry` =4292 AND `patch` IN (0);

-- Scarlet Protector
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4295 AND `patch` IN (0);

-- Scarlet Myrmidon
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4297 AND `patch` IN (0);

-- Scarlet Conjuror
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4344 AND `patch` IN (0);

-- Mottled Drywallow Crocolisk
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4351 AND `patch` IN (0);

-- Bloodfen Raptor
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4356 AND `patch` IN (0);

-- Bloodfen Razormaw
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4357 AND `patch` IN (0);

-- Bloodfen Lashtail
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4363 AND `patch` IN (0);

-- Mirefin Oracle
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4376 AND `patch` IN (0);

-- Darkmist Spider
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4377 AND `patch` IN (0);

-- Darkmist Lurker
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4379 AND `patch` IN (0);

-- Darkmist Silkspinner
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4380 AND `patch` IN (0);

-- Darkmist Widow
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4388 AND `patch` IN (0);

-- Young Murk Thresher
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4389 AND `patch` IN (0);

-- Murk Thresher
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4390 AND `patch` IN (0);

-- Elder Murk Thresher
 UPDATE `creature_template` SET `armor_multiplier` =1.5 WHERE `entry` =4396 AND `patch` IN (0);

-- Mudrock Tortoise
 UPDATE `creature_template` SET `armor_multiplier` =1.5 WHERE `entry` =4397 AND `patch` IN (0);

-- Mudrock Spikeshell
 UPDATE `creature_template` SET `armor_multiplier` =1.5 WHERE `entry` =4398 AND `patch` IN (0);

-- Mudrock Burrower
 UPDATE `creature_template` SET `armor_multiplier` =1.5 WHERE `entry` =4399 AND `patch` IN (0);

-- Mudrock Borer
 UPDATE `creature_template` SET `armor_multiplier` =1.5 WHERE `entry` =4400 AND `patch` IN (0);

-- Mudrock Snapjaw
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4412 AND `patch` IN (0);

-- Darkfang Creeper
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4414 AND `patch` IN (0);

-- Darkfang Venomspitter
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4415 AND `patch` IN (0);

-- Giant Darkfang Spider
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4427 AND `patch` IN (0);

-- Ward Guardian
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4428 AND `patch` IN (0);

-- Death Speaker Jargba
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4438 AND `patch` IN (0);

-- Razorfen Spearhide
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4440 AND `patch` IN (0);

-- Razorfen Totemic
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4442 AND `patch` IN (0);

-- Razorfen Defender
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4505 AND `patch` IN (0);

-- Bloodsail Deckhand
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4506 AND `patch` IN (0);

-- Bloodsail Swabby
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4510 AND `patch` IN (0);

-- Heralath Fallowbrook
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4515 AND `patch` IN (0);

-- Death's Head Acolyte
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4516 AND `patch` IN (0);

-- Death's Head Adept
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4517 AND `patch` IN (0);

-- Death's Head Priest
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4519 AND `patch` IN (0);

-- Death's Head Seer
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4522 AND `patch` IN (0);

-- Razorfen Dustweaver
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4523 AND `patch` IN (0);

-- Razorfen Groundshaker
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4532 AND `patch` IN (0);

-- Razorfen Beastmaster
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4623 AND `patch` IN (0);

-- Quilguard Champion
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4624 AND `patch` IN (0, 2, 10);

-- Booty Bay Bruiser
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4685 AND `patch` IN (0);

-- Ley Hunter
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4690 AND `patch` IN (0);

-- Rabid Bonepaw
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4695 AND `patch` IN (0);

-- Carrion Horror
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4701 AND `patch` IN (0);

-- Dying Kodo
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4702 AND `patch` IN (0);

-- Ancient Kodo
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4798 AND `patch` IN (0);

-- Fallenroot Shadowstalker
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4799 AND `patch` IN (0);

-- Fallenroot Hellcaller
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4805 AND `patch` IN (0);

-- Blackfathom Sea Witch
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4809 AND `patch` IN (0);

-- Twilight Acolyte
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4810 AND `patch` IN (0);

-- Twilight Reaver
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4811 AND `patch` IN (0);

-- Twilight Aquamancer
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4812 AND `patch` IN (0);

-- Twilight Loreseeker
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4813 AND `patch` IN (0);

-- Twilight Shadowmage
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4814 AND `patch` IN (0);

-- Twilight Elementalist
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4819 AND `patch` IN (0);

-- Blindlight Muckdweller
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4820 AND `patch` IN (0);

-- Blindlight Oracle
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4831 AND `patch` IN (0);

-- Lady Sarevess
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4841 AND `patch` IN (0);

-- Deadmire
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4842 AND `patch` IN (0);

-- Earthcaller Halmgar
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4844 AND `patch` IN (0);

-- Shadowforge Surveyor
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4845 AND `patch` IN (0);

-- Shadowforge Ruffian
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4847 AND `patch` IN (0);

-- Shadowforge Relic Hunter
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4850 AND `patch` IN (0);

-- Stonevault Cave Lurker
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4852 AND `patch` IN (0);

-- Stonevault Oracle
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5056 AND `patch` IN (0, 6);

-- Deviate Dreadfang
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5185 AND `patch` IN (0);

-- Hammerhead Shark
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5307 AND `patch` IN (0);

-- Vale Screecher
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5308 AND `patch` IN (0);

-- Rogue Vale Screecher
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5426 AND `patch` IN (0);

-- Blisterpaw Hyena
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5428 AND `patch` IN (0);

-- Roc
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5429 AND `patch` IN (0);

-- Fire Roc
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5434 AND `patch` IN (0);

-- Coral Shark
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5546 AND `patch` IN (0);

-- Grunt Zuul
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5547 AND `patch` IN (0);

-- Grunt Tharlak
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5617 AND `patch` IN (0);

-- Wastewander Shadow Mage
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5646 AND `patch` IN (0);

-- Sandfury Axe Thrower
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5647 AND `patch` IN (0);

-- Sandfury Firecaller
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5694 AND `patch` IN (0);

-- High Sorcerer Andromath
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5755 AND `patch` IN (0);

-- Deviate Viper
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5756 AND `patch` IN (0, 6);

-- Deviate Venomwing
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5766 AND `patch` IN (0);

-- Savannah Cub
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5769 AND `patch` IN (0);

-- Arch Druid Hamuul Runetotem
 UPDATE `creature_template` SET `armor_multiplier` =1.5 WHERE `entry` =5809 AND `patch` IN (0);

-- Watch Commander Zalaphil
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5843 AND `patch` IN (0);

-- Slave Worker
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5860 AND `patch` IN (0);

-- Twilight Dark Shaman
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5861 AND `patch` IN (0);

-- Twilight Fire Guard
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5862 AND `patch` IN (0);

-- Twilight Geomancer
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5952 AND `patch` IN (0);

-- Den Grunt
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5981 AND `patch` IN (0);

-- Portal Seeker
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5988 AND `patch` IN (0, 5);

-- Scorpok Stinger
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =6013 AND `patch` IN (0, 2);

-- Wayward Buzzard
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =6035 AND `patch` IN (0);

-- Razorfen Stalker
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =6123 AND `patch` IN (0);

-- Dark Iron Spy
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =6126 AND `patch` IN (0);

-- Haldarr Trickster
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =6167 AND `patch` IN (0);

-- Chimaera Matriarch
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =6215 AND `patch` IN (0);

-- Chomper
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =6243 AND `patch` IN (0);

-- Gelihast
 UPDATE `creature_template` SET `armor_multiplier` =1.5 WHERE `entry` =6352 AND `patch` IN (0);

-- Coralshell Lurker
 UPDATE `creature_template` SET `armor_multiplier` =1.2 WHERE `entry` =6389 AND `patch` IN (0);

-- Deathguard Podrig
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =6584 AND `patch` IN (0);

-- King Mosh
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =6707 AND `patch` IN (0);

-- Fahrad
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =6733 AND `patch` IN (0);

-- Stonevault Basher
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =6768 AND `patch` IN (0);

-- Lord Jorach Ravenholdt
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =6777 AND `patch` IN (0);

-- Zan Shivsproket
 UPDATE `creature_template` SET `armor_multiplier` =0.35 WHERE `entry` =6789 AND `patch` IN (0);

-- Thistle Cub
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =6911 AND `patch` IN (0);

-- Minion of Sethir
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =7051 AND `patch` IN (0);

-- Malformed Defias Drone
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =7246 AND `patch` IN (0);

-- Sandfury Shadowhunter
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =7269 AND `patch` IN (0);

-- Scarab
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =7323 AND `patch` IN (0);

-- Winstone Wolfe
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =7325 AND `patch` IN (0);

-- Master Kang
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =7335 AND `patch` IN (0);

-- Death's Head Geomancer
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =7337 AND `patch` IN (0);

-- Death's Head Necromancer
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =7432 AND `patch` IN (0);

-- Frostsaber Stalker
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =7434 AND `patch` IN (0);

-- Frostsaber Pride Watcher
 UPDATE `creature_template` SET `armor_multiplier` =0.75 WHERE `entry` =7446 AND `patch` IN (0);

-- Rabid Shardtooth
 UPDATE `creature_template` SET `armor_multiplier` =1.2 WHERE `entry` =7489 AND `patch` IN (0);

-- Silverpine Deathguard
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =7856 AND `patch` IN (0);

-- Southsea Freebooter
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =7872 AND `patch` IN (0);

-- Death's Head Cultist
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =7874 AND `patch` IN (0);

-- Razorfen Thornweaver
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =7939 AND `patch` IN (0);

-- Feathermoon Sentinel
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =7975 AND `patch` IN (0);

-- Mulgore Protector
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =8095 AND `patch` IN (0, 10);

-- Sul'lithuz Sandcrawler
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =8147 AND `patch` IN (0);

-- Camp Mojache Brave
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =8155 AND `patch` IN (0);

-- Kargath Grunt
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =8208 AND `patch` IN (0);

-- Murderous Blisterpaw
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =8419 AND `patch` IN (0);

-- Twilight Idolater
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =8444 AND `patch` IN (0);

-- Trade Master Kovic
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =8550 AND `patch` IN (0);

-- Shadowmage
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =9077 AND `patch` IN (0);

-- Warlord Goretooth
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =9297 AND `patch` IN (0);

-- Enraged Wyvern
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =9318 AND `patch` IN (0);

-- Incendosaur
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =9460 AND `patch` IN (0, 2, 10);

-- Gadgetzan Bruiser
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =9526 AND `patch` IN (0);

-- Enraged Gryphon
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =9683 AND `patch` IN (0);

-- Lar'korwi Mate
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =9684 AND `patch` IN (0);

-- Lar'korwi
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =10036 AND `patch` IN (0);

-- Brackenwall Enforcer
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =10717 AND `patch` IN (0);

-- Temporal Parasite
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =10882 AND `patch` IN (0);

-- Arikara
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =10979 AND `patch` IN (0);

-- Scarlet Hound
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11034 AND `patch` IN (0);

-- Lord Maxwell Tyrosus
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11036 AND `patch` IN (0);

-- Leonid Barthalomew the Revered
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11079 AND `patch` IN (0);

-- Wynd Nightchaser
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11180 AND `patch` IN (0);

-- Bloodvenom Post Brave
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11190 AND `patch` IN (0, 2, 10);

-- Everlook Bruiser
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11318 AND `patch` IN (0);

-- Ragefire Trogg
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11322 AND `patch` IN (0);

-- Searing Blade Cultist
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11323 AND `patch` IN (0);

-- Searing Blade Enforcer
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11324 AND `patch` IN (0);

-- Searing Blade Warlock
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11521 AND `patch` IN (0);

-- Kodo Apparition
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11564 AND `patch` IN (0);

-- Gizelton Caravan Kodo
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11683 AND `patch` IN (0);

-- Horde Shaman
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11804 AND `patch` IN (0);

-- Twilight Keeper Havunth
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11822 AND `patch` IN (0, 7, 10);

-- Moonglade Warden
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11876 AND `patch` IN (0);

-- Demon Spirit
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11881 AND `patch` IN (0);

-- Twilight Geolord
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11882 AND `patch` IN (0);

-- Twilight Stonecaller
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11883 AND `patch` IN (0);

-- Twilight Master
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11885 AND `patch` IN (0);

-- Blighthound
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11896 AND `patch` IN (0);

-- Borelgore
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11897 AND `patch` IN (0);

-- Duskwing
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11937 AND `patch` IN (0);

-- Demon Portal Guardian
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11956 AND `patch` IN (0);

-- Great Bear Spirit
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11957 AND `patch` IN (0);

-- Great Cat Spirit
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =12123 AND `patch` IN (0);

-- Reef Shark
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =12338 AND `patch` IN (0);

-- Shadowprey Guardian
 UPDATE `creature_template` SET `armor_multiplier` =1.5 WHERE `entry` =12347 AND `patch` IN (0);

-- Enraged Reef Crawler
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =12425 AND `patch` IN (0);

-- Flint Shadowmore
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =12902 AND `patch` IN (0);

-- Lorgus Jett
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =12920 AND `patch` IN (0);

-- Doctor Gregory Victor
 UPDATE `creature_template` SET `armor_multiplier` =1.2 WHERE `entry` =13076 AND `patch` IN (0);

-- Dun Morogh Mountaineer
 UPDATE `creature_template` SET `armor_multiplier` =1.5 WHERE `entry` =14123 AND `patch` IN (0);

-- Steeljaw Snapper
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =14222 AND `patch` IN (0);

-- Araga
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =14233 AND `patch` IN (0);

-- Ripscale
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =14356 AND `patch` IN (1);

-- Sawfin Frenzy
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =14394 AND `patch` IN (1);

-- Major Mattingly
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =14442 AND `patch` IN (2);

-- Hunter Thunderhorn
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =14721 AND `patch` IN (3);

-- Field Marshal Afrasiabi
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =14730 AND `patch` IN (3);

-- Revantusk Watcher
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =14757 AND `patch` IN (3);

-- Elder Torntusk
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =14984 AND `patch` IN (5);

-- Sergeant Maclear
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =15021 AND `patch` IN (5);

-- Deathmaster Dwire
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =15022 AND `patch` IN (5);

-- Deathstalker Mortis
 UPDATE `creature_template` SET `armor_multiplier` =1.2 WHERE `entry` =15138 AND `patch` IN (5);

-- Silverpine Elite
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =15177 AND `patch` IN (6);

-- Cloud Skydancer
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =15180 AND `patch` IN (6);

-- Baristolth of the Shifting Sands
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =15183 AND `patch` IN (6);

-- Geologist Larksbane
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =15201 AND `patch` IN (6);

-- Twilight Flamereaver
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =15213 AND `patch` IN (6);

-- Twilight Overlord
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =15270 AND `patch` IN (6);

-- Huum Wildmane
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =15282 AND `patch` IN (6);

-- Aurel Goldleaf
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =15308 AND `patch` IN (6);

-- Twilight Prophet
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =15350 AND `patch` IN (6);

-- Horde Warbringer
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =15466 AND `patch` IN (7);

-- Minion of Omen
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =15722 AND `patch` IN (7);

-- Squire Leoren Mal'derath
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =16117 AND `patch` IN (8, 9);

-- Plagued Swine
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =16506 AND `patch` IN (9);

-- Naxxramas Worshipper
 UPDATE `creature_template` SET `armor_multiplier` =2 WHERE `entry` =16803 AND `patch` IN (9);

-- Deathknight Understudy
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =17804 AND `patch` IN (10);

-- Squire Rowe
 -- fire resistance
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =521 AND `fire_res` =80;

-- Lupos
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =594 AND `fire_res` =10;

-- Defias Henchman
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =619 AND `fire_res` =10;

-- Defias Conjurer
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =628 AND `fire_res` =4;

-- Black Ravager
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =1225 AND `fire_res` =40;

-- Ol' Sooty
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =1258 AND `fire_res` =5;

-- Black Ravager Mastiff
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =1706 AND `fire_res` =10;

-- Defias Prisoner
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =1707 AND `fire_res` =10;

-- Defias Captive
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =1708 AND `fire_res` =10;

-- Defias Inmate
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =1711 AND `fire_res` =10;

-- Defias Convict
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =1715 AND `fire_res` =10;

-- Defias Insurgent
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =1725 AND `fire_res` =10;

-- Defias Watchman
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =1726 AND `fire_res` =10;

-- Defias Magician
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =1838 AND `fire_res` =10;

-- Scarlet Interrogator
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =1839 AND `fire_res` =15;

-- Scarlet High Clerist
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =1843 AND `fire_res` =15;

-- Foreman Jerris
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =1846 AND `fire_res` =5;

-- High Protector Lorik
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =1854 AND `fire_res` =10;

-- High Priest Thel'danis
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =3936 AND `fire_res` =10;

-- Shandris Feathermoon
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =4111 AND `fire_res` =81;

-- Gravelsnout Kobold
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =4112 AND `fire_res` =80;

-- Gravelsnout Vermin
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =4114 AND `fire_res` =82;

-- Gravelsnout Forager
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =4396 AND `fire_res` =74;

-- Mudrock Tortoise
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =4398 AND `fire_res` =80;

-- Mudrock Burrower
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =4399 AND `fire_res` =86;

-- Mudrock Borer
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =4400 AND `fire_res` =84;

-- Mudrock Snapjaw
 UPDATE `creature_template` SET `fire_res` =90 WHERE `entry` =4676 AND `fire_res` =0;

-- Lesser Infernal
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =4787 AND `fire_res` =30;

-- Argent Guard Thaelrid
 UPDATE `creature_template` SET `fire_res` =75 WHERE `entry` =6010 AND `fire_res` =0;

-- Felhound
 UPDATE `creature_template` SET `fire_res` =45 WHERE `entry` =6073 AND `fire_res` =0;

-- Searing Infernal
 UPDATE `creature_template` SET `fire_res` =54 WHERE `entry` =6498 AND `fire_res` =53;

-- Devilsaur
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =7325 AND `fire_res` =10;

-- Master Kang
 UPDATE `creature_template` SET `fire_res` =100 WHERE `entry` =8302 AND `fire_res` =0;

-- Deatheye
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =8889 AND `fire_res` =100;

-- Anvilrage Overseer
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =8890 AND `fire_res` =100;

-- Anvilrage Warden
 UPDATE `creature_template` SET `fire_res` =50 WHERE `entry` =9862 AND `fire_res` =0;

-- Jaedenar Legionnaire
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =11034 AND `fire_res` =30;

-- Lord Maxwell Tyrosus
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =11036 AND `fire_res` =30;

-- Leonid Barthalomew the Revered
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =11079 AND `fire_res` =10;

-- Wynd Nightchaser
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =11440 AND `fire_res` =54;

-- Gordok Enforcer
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =11442 AND `fire_res` =54;

-- Gordok Mauler
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =11443 AND `fire_res` =53;

-- Gordok Ogre-Mage
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =12432 AND `fire_res` =28;

-- Old Vicejaw
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =14232 AND `fire_res` =60;

-- Dart
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =14721 AND `fire_res` =10;

-- Field Marshal Afrasiabi
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =14984 AND `fire_res` =10;

-- Sergeant Maclear
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =15021 AND `fire_res` =10;

-- Deathmaster Dwire
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =15022 AND `fire_res` =10;

-- Deathstalker Mortis
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =16506 AND `fire_res` =5;

-- Naxxramas Worshipper
 -- nature resistance
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =521 AND `nature_res` =80;

-- Lupos
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =594 AND `nature_res` =10;

-- Defias Henchman
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =619 AND `nature_res` =10;

-- Defias Conjurer
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =628 AND `nature_res` =7;

-- Black Ravager
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =1199 AND `nature_res` =10;

-- Juvenile Snow Leopard
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =1201 AND `nature_res` =10;

-- Snow Leopard
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =1258 AND `nature_res` =8;

-- Black Ravager Mastiff
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =1509 AND `nature_res` =1;

-- Ragged Scavenger
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =1706 AND `nature_res` =10;

-- Defias Prisoner
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =1707 AND `nature_res` =10;

-- Defias Captive
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =1708 AND `nature_res` =10;

-- Defias Inmate
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =1711 AND `nature_res` =10;

-- Defias Convict
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =1715 AND `nature_res` =10;

-- Defias Insurgent
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =1725 AND `nature_res` =10;

-- Defias Watchman
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =1726 AND `nature_res` =10;

-- Defias Magician
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =1838 AND `nature_res` =10;

-- Scarlet Interrogator
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =1839 AND `nature_res` =15;

-- Scarlet High Clerist
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =1843 AND `nature_res` =15;

-- Foreman Jerris
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =1846 AND `nature_res` =5;

-- High Protector Lorik
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =1854 AND `nature_res` =10;

-- High Priest Thel'danis
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =2241 AND `nature_res` =100;

-- Syndicate Thief
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =2252 AND `nature_res` =35;

-- Crushridge Ogre
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =2253 AND `nature_res` =36;

-- Crushridge Brute
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =2254 AND `nature_res` =37;

-- Crushridge Mauler
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =2255 AND `nature_res` =38;

-- Crushridge Mage
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =2256 AND `nature_res` =39;

-- Crushridge Enforcer
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =2287 AND `nature_res` =40;

-- Crushridge Warmonger
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =2416 AND `nature_res` =37;

-- Crushridge Plunderer
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =2597 AND `nature_res` =100;

-- Lord Falconcrest
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =2605 AND `nature_res` =40;

-- Zalas Witherbark
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =3936 AND `nature_res` =10;

-- Shandris Feathermoon
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =5269 AND `nature_res` =30;

-- Atal'ai Priest
 UPDATE `creature_template` SET `nature_res` =75 WHERE `entry` =6010 AND `nature_res` =0;

-- Felhound
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =7325 AND `nature_res` =10;

-- Master Kang
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =11034 AND `nature_res` =10;

-- Lord Maxwell Tyrosus
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =11079 AND `nature_res` =10;

-- Wynd Nightchaser
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =11440 AND `nature_res` =54;

-- Gordok Enforcer
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =11442 AND `nature_res` =54;

-- Gordok Mauler
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =11443 AND `nature_res` =53;

-- Gordok Ogre-Mage
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =14232 AND `nature_res` =60;

-- Dart
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =14721 AND `nature_res` =10;

-- Field Marshal Afrasiabi
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =14984 AND `nature_res` =10;

-- Sergeant Maclear
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =15021 AND `nature_res` =10;

-- Deathmaster Dwire
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =15022 AND `nature_res` =10;

-- Deathstalker Mortis
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =16506 AND `nature_res` =5;

-- Naxxramas Worshipper
 -- frost resistance
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =521 AND `frost_res` =80;

-- Lupos
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =594 AND `frost_res` =10;

-- Defias Henchman
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =619 AND `frost_res` =10;

-- Defias Conjurer
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =1035 AND `frost_res` =25;

-- Dragonmaw Swamprunner
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =1225 AND `frost_res` =40;

-- Ol' Sooty
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =1706 AND `frost_res` =10;

-- Defias Prisoner
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =1707 AND `frost_res` =10;

-- Defias Captive
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =1708 AND `frost_res` =10;

-- Defias Inmate
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =1711 AND `frost_res` =10;

-- Defias Convict
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =1715 AND `frost_res` =10;

-- Defias Insurgent
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =1725 AND `frost_res` =10;

-- Defias Watchman
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =1726 AND `frost_res` =10;

-- Defias Magician
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =1838 AND `frost_res` =10;

-- Scarlet Interrogator
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =1839 AND `frost_res` =15;

-- Scarlet High Clerist
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =1843 AND `frost_res` =15;

-- Foreman Jerris
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =1846 AND `frost_res` =5;

-- High Protector Lorik
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =1854 AND `frost_res` =10;

-- High Priest Thel'danis
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =3936 AND `frost_res` =10;

-- Shandris Feathermoon
 UPDATE `creature_template` SET `frost_res` =75 WHERE `entry` =6010 AND `frost_res` =0;

-- Felhound
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =7325 AND `frost_res` =10;

-- Master Kang
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =11034 AND `frost_res` =10;

-- Lord Maxwell Tyrosus
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =11079 AND `frost_res` =10;

-- Wynd Nightchaser
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =11440 AND `frost_res` =54;

-- Gordok Enforcer
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =11442 AND `frost_res` =54;

-- Gordok Mauler
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =11443 AND `frost_res` =53;

-- Gordok Ogre-Mage
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =12432 AND `frost_res` =28;

-- Old Vicejaw
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =14232 AND `frost_res` =60;

-- Dart
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =14721 AND `frost_res` =10;

-- Field Marshal Afrasiabi
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =14984 AND `frost_res` =10;

-- Sergeant Maclear
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =15021 AND `frost_res` =10;

-- Deathmaster Dwire
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =15022 AND `frost_res` =10;

-- Deathstalker Mortis
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =16506 AND `frost_res` =5;

-- Naxxramas Worshipper
 -- shadow resistance
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =119 AND `shadow_res` =20;

-- Longsnout
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =157 AND `shadow_res` =40;

-- Goretusk
 UPDATE `creature_template` SET `shadow_res` =50 WHERE `entry` =434 AND `shadow_res` =0;

-- Rabid Shadowhide Gnoll
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =521 AND `shadow_res` =80;

-- Lupos
 UPDATE `creature_template` SET `shadow_res` =50 WHERE `entry` =568 AND `shadow_res` =0;

-- Shadowhide Warrior
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =594 AND `shadow_res` =10;

-- Defias Henchman
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =619 AND `shadow_res` =10;

-- Defias Conjurer
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =1035 AND `shadow_res` =25;

-- Dragonmaw Swamprunner
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =1706 AND `shadow_res` =10;

-- Defias Prisoner
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =1707 AND `shadow_res` =10;

-- Defias Captive
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =1708 AND `shadow_res` =10;

-- Defias Inmate
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =1711 AND `shadow_res` =10;

-- Defias Convict
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =1715 AND `shadow_res` =10;

-- Defias Insurgent
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =1725 AND `shadow_res` =10;

-- Defias Watchman
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =1726 AND `shadow_res` =10;

-- Defias Magician
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =1838 AND `shadow_res` =10;

-- Scarlet Interrogator
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =1839 AND `shadow_res` =15;

-- Scarlet High Clerist
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =1843 AND `shadow_res` =15;

-- Foreman Jerris
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =1846 AND `shadow_res` =5;

-- High Protector Lorik
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =1854 AND `shadow_res` =10;

-- High Priest Thel'danis
 UPDATE `creature_template` SET `shadow_res` =100 WHERE `entry` =2318 AND `shadow_res` =50;

-- Argus Shadow Mage
 UPDATE `creature_template` SET `shadow_res` =250 WHERE `entry` =2647 AND `shadow_res` =0;

-- Vilebranch Soul Eater
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =3936 AND `shadow_res` =10;

-- Shandris Feathermoon
 UPDATE `creature_template` SET `shadow_res` =90 WHERE `entry` =4679 AND `shadow_res` =0;

-- Nether Maiden
 UPDATE `creature_template` SET `shadow_res` =105 WHERE `entry` =5617 AND `shadow_res` =50;

-- Wastewander Shadow Mage
 UPDATE `creature_template` SET `shadow_res` =75 WHERE `entry` =6010 AND `shadow_res` =0;

-- Felhound
 UPDATE `creature_template` SET `shadow_res` =45 WHERE `entry` =6073 AND `shadow_res` =0;

-- Searing Infernal
 UPDATE `creature_template` SET `shadow_res` =76 WHERE `entry` =7135 AND `shadow_res` =0;

-- Infernal Bodyguard
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =7325 AND `shadow_res` =10;

-- Master Kang
 UPDATE `creature_template` SET `shadow_res` =90 WHERE `entry` =8550 AND `shadow_res` =0;

-- Shadowmage
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =8889 AND `shadow_res` =20;

-- Anvilrage Overseer
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =8890 AND `shadow_res` =20;

-- Anvilrage Warden
 UPDATE `creature_template` SET `shadow_res` =50 WHERE `entry` =9862 AND `shadow_res` =0;

-- Jaedenar Legionnaire
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =11034 AND `shadow_res` =10;

-- Lord Maxwell Tyrosus
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =11079 AND `shadow_res` =10;

-- Wynd Nightchaser
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =11440 AND `shadow_res` =54;

-- Gordok Enforcer
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =11442 AND `shadow_res` =54;

-- Gordok Mauler
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =11443 AND `shadow_res` =53;

-- Gordok Ogre-Mage
 UPDATE `creature_template` SET `shadow_res` =45 WHERE `entry` =11697 AND `shadow_res` =0;

-- Mannoroc Lasher
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =14232 AND `shadow_res` =60;

-- Dart
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =14721 AND `shadow_res` =10;

-- Field Marshal Afrasiabi
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =14984 AND `shadow_res` =10;

-- Sergeant Maclear
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =15021 AND `shadow_res` =10;

-- Deathmaster Dwire
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =15022 AND `shadow_res` =10;

-- Deathstalker Mortis
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =16506 AND `shadow_res` =5;

-- Naxxramas Worshipper
 -- arcane resistance
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =157 AND `arcane_res` =10;

-- Goretusk
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =334 AND `arcane_res` =100;

-- Gath'Ilzogg
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =521 AND `arcane_res` =80;

-- Lupos
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =594 AND `arcane_res` =10;

-- Defias Henchman
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =619 AND `arcane_res` =10;

-- Defias Conjurer
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =628 AND `arcane_res` =1;

-- Black Ravager
 UPDATE `creature_template` SET `arcane_res` =75 WHERE `entry` =819 AND `arcane_res` =0;

-- Servant of Ilgalar
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1035 AND `arcane_res` =25;

-- Dragonmaw Swamprunner
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1051 AND `arcane_res` =100;

-- Dark Iron Dwarf
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1052 AND `arcane_res` =100;

-- Dark Iron Saboteur
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1053 AND `arcane_res` =100;

-- Dark Iron Tunneler
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1054 AND `arcane_res` =100;

-- Dark Iron Demolitionist
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1169 AND `arcane_res` =100;

-- Dark Iron Insurgent
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1222 AND `arcane_res` =100;

-- Dark Iron Sapper
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1225 AND `arcane_res` =40;

-- Ol' Sooty
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1258 AND `arcane_res` =2;

-- Black Ravager Mastiff
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1706 AND `arcane_res` =10;

-- Defias Prisoner
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1707 AND `arcane_res` =10;

-- Defias Captive
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1708 AND `arcane_res` =10;

-- Defias Inmate
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1711 AND `arcane_res` =10;

-- Defias Convict
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1715 AND `arcane_res` =10;

-- Defias Insurgent
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1725 AND `arcane_res` =10;

-- Defias Watchman
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1726 AND `arcane_res` =10;

-- Defias Magician
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1838 AND `arcane_res` =10;

-- Scarlet Interrogator
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1839 AND `arcane_res` =15;

-- Scarlet High Clerist
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1843 AND `arcane_res` =15;

-- Foreman Jerris
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1846 AND `arcane_res` =5;

-- High Protector Lorik
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1854 AND `arcane_res` =10;

-- High Priest Thel'danis
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =2038 AND `arcane_res` =32;

-- Lord Melenas
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =2405 AND `arcane_res` =10;

-- Tarren Mill Deathguard
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =2418 AND `arcane_res` =10;

-- Deathguard Samsa
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =2419 AND `arcane_res` =10;

-- Deathguard Humbert
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =2575 AND `arcane_res` =100;

-- Dark Iron Supplier
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =2577 AND `arcane_res` =100;

-- Dark Iron Shadowcaster
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =3662 AND `arcane_res` =68;

-- Delmanis the Hated
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =3773 AND `arcane_res` =104;

-- Akkrilus
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =3936 AND `arcane_res` =10;

-- Shandris Feathermoon
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =4062 AND `arcane_res` =100;

-- Dark Iron Bombardier
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =4064 AND `arcane_res` =100;

-- Blackrock Scout
 UPDATE `creature_template` SET `arcane_res` =45 WHERE `entry` =4515 AND `arcane_res` =0;

-- Death's Head Acolyte
 UPDATE `creature_template` SET `arcane_res` =45 WHERE `entry` =4516 AND `arcane_res` =0;

-- Death's Head Adept
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =4619 AND `arcane_res` =128;

-- Geltharis
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =4787 AND `arcane_res` =70;

-- Argent Guard Thaelrid
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =5269 AND `arcane_res` =70;

-- Atal'ai Priest
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =5760 AND `arcane_res` =160;

-- Lord Azrethoc
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =5839 AND `arcane_res` =100;

-- Dark Iron Geologist
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =5840 AND `arcane_res` =100;

-- Dark Iron Steamsmith
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =5844 AND `arcane_res` =100;

-- Dark Iron Slaver
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =5846 AND `arcane_res` =100;

-- Dark Iron Taskmaster
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =6123 AND `arcane_res` =100;

-- Dark Iron Spy
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =6200 AND `arcane_res` =204;

-- Legashi Satyr
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =6201 AND `arcane_res` =209;

-- Legashi Rogue
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =6202 AND `arcane_res` =214;

-- Legashi Hellcaller
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =6389 AND `arcane_res` =10;

-- Deathguard Podrig
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =6523 AND `arcane_res` =100;

-- Dark Iron Rifleman
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =7106 AND `arcane_res` =200;

-- Jadefire Rogue
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =7107 AND `arcane_res` =204;

-- Jadefire Trickster
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =7108 AND `arcane_res` =208;

-- Jadefire Betrayer
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =7109 AND `arcane_res` =200;

-- Jadefire Felsworn
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =7110 AND `arcane_res` =205;

-- Jadefire Shadowstalker
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =7111 AND `arcane_res` =212;

-- Jadefire Hellcaller
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =7325 AND `arcane_res` =10;

-- Master Kang
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =7489 AND `arcane_res` =10;

-- Silverpine Deathguard
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =8504 AND `arcane_res` =100;

-- Dark Iron Sentry
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =8566 AND `arcane_res` =100;

-- Dark Iron Lookout
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =8637 AND `arcane_res` =100;

-- Dark Iron Watchman
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =11034 AND `arcane_res` =70;

-- Lord Maxwell Tyrosus
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =11036 AND `arcane_res` =70;

-- Leonid Barthalomew the Revered
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =11079 AND `arcane_res` =10;

-- Wynd Nightchaser
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =11440 AND `arcane_res` =54;

-- Gordok Enforcer
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =11442 AND `arcane_res` =54;

-- Gordok Mauler
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =11443 AND `arcane_res` =53;

-- Gordok Ogre-Mage
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =11876 AND `arcane_res` =148;

-- Demon Spirit
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =12432 AND `arcane_res` =28;

-- Old Vicejaw
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =14232 AND `arcane_res` =60;

-- Dart
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =14721 AND `arcane_res` =10;

-- Field Marshal Afrasiabi
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =14984 AND `arcane_res` =10;

-- Sergeant Maclear
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =15021 AND `arcane_res` =10;

-- Deathmaster Dwire
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =15022 AND `arcane_res` =10;

-- Deathstalker Mortis
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =15692 AND `arcane_res` =100;

-- Dark Iron Kidnapper
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =16506 AND `arcane_res` =5;

-- Naxxramas Worshipper
 -- End of migration.
;
