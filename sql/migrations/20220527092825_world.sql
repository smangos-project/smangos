INSERT INTO `migrations` VALUES ('20220527092825');

-- Add your query below.
 -- These mobs have their min and max gold reversed.
 UPDATE `creature_template` SET `gold_min` =0, `gold_max` =0 WHERE `entry` IN (16194, 16215, 16216);

UPDATE `creature_template` SET `gold_min` =1404186, `gold_max` =11012303 WHERE `entry` =15547;

-- Update gold dropped by mobs based on classic sniffs.
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =36 AND `gold_min` =13;

-- Harvest Golem (sample size 30)
 UPDATE `creature_template` SET `gold_max` =24 WHERE `entry` =36 AND `gold_max` =21;

-- Harvest Golem (sample size 30)
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =38 AND `gold_min` =2;

-- Defias Thug (sample size 134)
 UPDATE `creature_template` SET `gold_max` =5 WHERE `entry` =38 AND `gold_max` =7;

-- Defias Thug (sample size 134)
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =40 AND `gold_min` =5;

-- Kobold Miner (sample size 65)
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =46 AND `gold_min` =9;

-- Murloc Forager (sample size 26)
 UPDATE `creature_template` SET `gold_max` =15 WHERE `entry` =46 AND `gold_max` =17;

-- Murloc Forager (sample size 26)
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =80 AND `gold_min` =2;

-- Kobold Laborer (sample size 47)
 UPDATE `creature_template` SET `gold_max` =5 WHERE `entry` =80 AND `gold_max` =8;

-- Kobold Laborer (sample size 47)
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =94 AND `gold_min` =5;

-- Defias Cutpurse
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =95 AND `gold_min` =12;

-- Defias Smuggler (sample size 60)
 UPDATE `creature_template` SET `gold_max` =24 WHERE `entry` =95 AND `gold_max` =20;

-- Defias Smuggler (sample size 60)
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =97 AND `gold_min` =8;

-- Riverpaw Runt
 UPDATE `creature_template` SET `gold_max` =52 WHERE `entry` =98 AND `gold_max` =32;

-- Riverpaw Taskmaster
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =100 AND `gold_min` =13;

-- Gruff Swiftbite
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =114 AND `gold_min` =16;

-- Harvest Watcher (sample size 88)
 UPDATE `creature_template` SET `gold_max` =34 WHERE `entry` =114 AND `gold_max` =26;

-- Harvest Watcher (sample size 88)
 UPDATE `creature_template` SET `gold_max` =42 WHERE `entry` =115 AND `gold_max` =32;

-- Harvest Reaper
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =116 AND `gold_min` =8;

-- Defias Bandit (sample size 48)
 UPDATE `creature_template` SET `gold_max` =14 WHERE `entry` =116 AND `gold_max` =15;

-- Defias Bandit (sample size 48)
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =117 AND `gold_min` =12;

-- Riverpaw Gnoll (sample size 26)
 UPDATE `creature_template` SET `gold_max` =24 WHERE `entry` =117 AND `gold_max` =20;

-- Riverpaw Gnoll (sample size 26)
 UPDATE `creature_template` SET `gold_min` =5 WHERE `entry` =121 AND `gold_min` =18;

-- Defias Pathstalker
 UPDATE `creature_template` SET `gold_max` =38 WHERE `entry` =121 AND `gold_max` =28;

-- Defias Pathstalker
 UPDATE `creature_template` SET `gold_min` =8 WHERE `entry` =122 AND `gold_min` =21;

-- Defias Highwayman (sample size 48)
 UPDATE `creature_template` SET `gold_max` =54 WHERE `entry` =122 AND `gold_max` =32;

-- Defias Highwayman (sample size 48)
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =123 AND `gold_min` =14;

-- Riverpaw Mongrel
 UPDATE `creature_template` SET `gold_max` =28 WHERE `entry` =123 AND `gold_max` =24;

-- Riverpaw Mongrel
 UPDATE `creature_template` SET `gold_max` =35 WHERE `entry` =124 AND `gold_max` =28;

-- Riverpaw Brute
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =126 AND `gold_min` =13;

-- Murloc Coastrunner
 UPDATE `creature_template` SET `gold_max` =25 WHERE `entry` =126 AND `gold_max` =22;

-- Murloc Coastrunner
 UPDATE `creature_template` SET `gold_min` =13 WHERE `entry` =127 AND `gold_min` =21;

-- Murloc Tidehunter
 UPDATE `creature_template` SET `gold_max` =47 WHERE `entry` =127 AND `gold_max` =32;

-- Murloc Tidehunter
 UPDATE `creature_template` SET `gold_min` =7 WHERE `entry` =171 AND `gold_min` =17;

-- Murloc Warrior
 UPDATE `creature_template` SET `gold_max` =36 WHERE `entry` =171 AND `gold_max` =27;

-- Murloc Warrior
 UPDATE `creature_template` SET `gold_min` =19 WHERE `entry` =202 AND `gold_min` =31;

-- Skeletal Horror
 UPDATE `creature_template` SET `gold_max` =63 WHERE `entry` =203 AND `gold_max` =40;

-- Skeletal Mage
 UPDATE `creature_template` SET `gold_min` =28 WHERE `entry` =205 AND `gold_min` =41;

-- Nightbane Dark Runner
 UPDATE `creature_template` SET `gold_max` =144 WHERE `entry` =205 AND `gold_max` =58;

-- Nightbane Dark Runner
 UPDATE `creature_template` SET `gold_max` =169 WHERE `entry` =206 AND `gold_max` =61;

-- Nightbane Vile Fang
 UPDATE `creature_template` SET `gold_min` =22 WHERE `entry` =210 AND `gold_min` =37;

-- Bone Chewer
 UPDATE `creature_template` SET `gold_max` =143 WHERE `entry` =212 AND `gold_max` =61;

-- Splinter Fist Warrior
 UPDATE `creature_template` SET `gold_max` =110 WHERE `entry` =215 AND `gold_max` =48;

-- Defias Night Runner
 UPDATE `creature_template` SET `gold_max` =75 WHERE `entry` =218 AND `gold_max` =47;

-- Grave Robber
 UPDATE `creature_template` SET `gold_min` =17 WHERE `entry` =232 AND `gold_min` =30;

-- Farmer Ray
 UPDATE `creature_template` SET `gold_max` =103 WHERE `entry` =232 AND `gold_max` =44;

-- Farmer Ray
 UPDATE `creature_template` SET `gold_max` =5 WHERE `entry` =257 AND `gold_max` =7;

-- Kobold Worker (sample size 64)
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =285 AND `gold_min` =5;

-- Murloc (sample size 23)
 UPDATE `creature_template` SET `gold_max` =118 WHERE `entry` =300 AND `gold_max` =62;

-- Zzarc' Vul
 UPDATE `creature_template` SET `gold_min` =42 WHERE `entry` =315 AND `gold_min` =58;

-- Stalvan Mistmantle
 UPDATE `creature_template` SET `gold_max` =249 WHERE `entry` =315 AND `gold_max` =81;

-- Stalvan Mistmantle
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =327 AND `gold_min` =7;

-- Goldtooth
 UPDATE `creature_template` SET `gold_max` =141 WHERE `entry` =391 AND `gold_max` =40;

-- Old Murk-Eye
 UPDATE `creature_template` SET `gold_max` =1265 WHERE `entry` =412 AND `gold_max` =961;

-- Stitches
 UPDATE `creature_template` SET `gold_min` =8 WHERE `entry` =422 AND `gold_min` =22;

-- Murloc Flesheater
 UPDATE `creature_template` SET `gold_max` =58 WHERE `entry` =422 AND `gold_max` =33;

-- Murloc Flesheater
 UPDATE `creature_template` SET `gold_min` =7 WHERE `entry` =423 AND `gold_min` =17;

-- Redridge Mongrel (sample size 21)
 UPDATE `creature_template` SET `gold_max` =38 WHERE `entry` =423 AND `gold_max` =28;

-- Redridge Mongrel (sample size 21)
 UPDATE `creature_template` SET `gold_min` =9 WHERE `entry` =424 AND `gold_min` =19;

-- Redridge Poacher
 UPDATE `creature_template` SET `gold_max` =48 WHERE `entry` =424 AND `gold_max` =29;

-- Redridge Poacher
 UPDATE `creature_template` SET `gold_min` =7 WHERE `entry` =426 AND `gold_min` =20;

-- Redridge Brute (sample size 22)
 UPDATE `creature_template` SET `gold_max` =52 WHERE `entry` =426 AND `gold_max` =32;

-- Redridge Brute (sample size 22)
 UPDATE `creature_template` SET `gold_max` =82 WHERE `entry` =429 AND `gold_max` =48;

-- Shadowhide Darkweaver
 UPDATE `creature_template` SET `gold_min` =8 WHERE `entry` =430 AND `gold_min` =21;

-- Redridge Mystic (sample size 21)
 UPDATE `creature_template` SET `gold_max` =59 WHERE `entry` =430 AND `gold_max` =32;

-- Redridge Mystic (sample size 21)
 UPDATE `creature_template` SET `gold_min` =28 WHERE `entry` =431 AND `gold_min` =37;

-- Shadowhide Slayer
 UPDATE `creature_template` SET `gold_max` =94 WHERE `entry` =431 AND `gold_max` =53;

-- Shadowhide Slayer
 UPDATE `creature_template` SET `gold_max` =86 WHERE `entry` =432 AND `gold_max` =45;

-- Shadowhide Brute
 UPDATE `creature_template` SET `gold_min` =14 WHERE `entry` =433 AND `gold_min` =29;

-- Shadowhide Gnoll
 UPDATE `creature_template` SET `gold_max` =86 WHERE `entry` =433 AND `gold_max` =43;

-- Shadowhide Gnoll
 UPDATE `creature_template` SET `gold_min` =15 WHERE `entry` =434 AND `gold_min` =27;

-- Rabid Shadowhide Gnoll (sample size 37)
 UPDATE `creature_template` SET `gold_max` =87 WHERE `entry` =434 AND `gold_max` =40;

-- Rabid Shadowhide Gnoll (sample size 37)
 UPDATE `creature_template` SET `gold_min` =17 WHERE `entry` =435 AND `gold_min` =33;

-- Blackrock Champion (sample size 38)
 UPDATE `creature_template` SET `gold_max` =113 WHERE `entry` =435 AND `gold_max` =48;

-- Blackrock Champion (sample size 38)
 UPDATE `creature_template` SET `gold_min` =17 WHERE `entry` =437 AND `gold_min` =27;

-- Blackrock Renegade
 UPDATE `creature_template` SET `gold_max` =79 WHERE `entry` =437 AND `gold_max` =40;

-- Blackrock Renegade
 UPDATE `creature_template` SET `gold_min` =15 WHERE `entry` =440 AND `gold_min` =24;

-- Blackrock Grunt
 UPDATE `creature_template` SET `gold_max` =59 WHERE `entry` =440 AND `gold_max` =36;

-- Blackrock Grunt
 UPDATE `creature_template` SET `gold_min` =7 WHERE `entry` =441 AND `gold_min` =19;

-- Black Dragon Whelp (sample size 26)
 UPDATE `creature_template` SET `gold_max` =54 WHERE `entry` =441 AND `gold_max` =30;

-- Black Dragon Whelp (sample size 26)
 UPDATE `creature_template` SET `gold_min` =13 WHERE `entry` =446 AND `gold_min` =23;

-- Redridge Basher
 UPDATE `creature_template` SET `gold_min` =24 WHERE `entry` =448 AND `gold_min` =45;

-- Hogger
 UPDATE `creature_template` SET `gold_min` =10 WHERE `entry` =449 AND `gold_min` =19;

-- Defias Knuckleduster
 UPDATE `creature_template` SET `gold_max` =47 WHERE `entry` =449 AND `gold_max` =30;

-- Defias Knuckleduster
 UPDATE `creature_template` SET `gold_max` =48 WHERE `entry` =450 AND `gold_max` =29;

-- Defias Renegade Mage
 UPDATE `creature_template` SET `gold_max` =48 WHERE `entry` =452 AND `gold_max` =31;

-- Riverpaw Bandit
 UPDATE `creature_template` SET `gold_min` =15 WHERE `entry` =453 AND `gold_min` =22;

-- Riverpaw Mystic
 UPDATE `creature_template` SET `gold_min` =8 WHERE `entry` =456 AND `gold_min` =14;

-- Murloc Minor Oracle
 UPDATE `creature_template` SET `gold_min` =14 WHERE `entry` =458 AND `gold_min` =19;

-- Murloc Hunter
 UPDATE `creature_template` SET `gold_max` =45 WHERE `entry` =458 AND `gold_max` =30;

-- Murloc Hunter
 UPDATE `creature_template` SET `gold_min` =5 WHERE `entry` =473 AND `gold_min` =10;

-- Morgan the Collector
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =474 AND `gold_min` =10;

-- Defias Rogue Wizard
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =475 AND `gold_min` =4;

-- Kobold Tunneler (sample size 87)
 UPDATE `creature_template` SET `gold_max` =11 WHERE `entry` =475 AND `gold_max` =10;

-- Kobold Tunneler (sample size 87)
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =476 AND `gold_min` =6;

-- Kobold Geomancer
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =478 AND `gold_min` =10;

-- Riverpaw Outrunner (sample size 39)
 UPDATE `creature_template` SET `gold_max` =15 WHERE `entry` =478 AND `gold_max` =18;

-- Riverpaw Outrunner (sample size 39)
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =480 AND `gold_min` =10;

-- Rusty Harvest Golem
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =481 AND `gold_min` =10;

-- Defias Footpad
 UPDATE `creature_template` SET `gold_min` =14 WHERE `entry` =485 AND `gold_min` =26;

-- Blackrock Outrunner
 UPDATE `creature_template` SET `gold_max` =70 WHERE `entry` =485 AND `gold_max` =39;

-- Blackrock Outrunner
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =500 AND `gold_min` =13;

-- Riverpaw Scout
 UPDATE `creature_template` SET `gold_max` =27 WHERE `entry` =500 AND `gold_max` =22;

-- Riverpaw Scout
 UPDATE `creature_template` SET `gold_max` =27 WHERE `entry` =501 AND `gold_max` =26;

-- Riverpaw Herbalist
 UPDATE `creature_template` SET `gold_min` =7 WHERE `entry` =502 AND `gold_min` =17;

-- Benny Blaanco
 UPDATE `creature_template` SET `gold_max` =110 WHERE `entry` =503 AND `gold_max` =61;

-- Lord Malathrom
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =504 AND `gold_min` =13;

-- Defias Trapper (sample size 55)
 UPDATE `creature_template` SET `gold_max` =27 WHERE `entry` =504 AND `gold_max` =22;

-- Defias Trapper (sample size 55)
 UPDATE `creature_template` SET `gold_min` =8 WHERE `entry` =506 AND `gold_min` =21;

-- Sergeant Brashclaw
 UPDATE `creature_template` SET `gold_min` =8 WHERE `entry` =513 AND `gold_min` =16;

-- Murloc Netter
 UPDATE `creature_template` SET `gold_max` =27 WHERE `entry` =513 AND `gold_max` =26;

-- Murloc Netter
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =515 AND `gold_min` =12;

-- Murloc Raider
 UPDATE `creature_template` SET `gold_max` =21 WHERE `entry` =515 AND `gold_max` =20;

-- Murloc Raider
 UPDATE `creature_template` SET `gold_min` =10 WHERE `entry` =517 AND `gold_min` =20;

-- Murloc Oracle
 UPDATE `creature_template` SET `gold_max` =53 WHERE `entry` =517 AND `gold_max` =31;

-- Murloc Oracle
 UPDATE `creature_template` SET `gold_min` =6 WHERE `entry` =519 AND `gold_min` =17;

-- Slark
 UPDATE `creature_template` SET `gold_min` =21 WHERE `entry` =520 AND `gold_min` =23;

-- Brack
 UPDATE `creature_template` SET `gold_min` =24 WHERE `entry` =533 AND `gold_min` =37;

-- Nightbane Shadow Weaver
 UPDATE `creature_template` SET `gold_max` =136 WHERE `entry` =533 AND `gold_max` =53;

-- Nightbane Shadow Weaver
 UPDATE `creature_template` SET `gold_min` =12 WHERE `entry` =544 AND `gold_min` =28;

-- Murloc Nightcrawler
 UPDATE `creature_template` SET `gold_max` =72 WHERE `entry` =544 AND `gold_max` =41;

-- Murloc Nightcrawler
 UPDATE `creature_template` SET `gold_min` =13 WHERE `entry` =545 AND `gold_min` =23;

-- Murloc Tidecaller
 UPDATE `creature_template` SET `gold_max` =59 WHERE `entry` =545 AND `gold_max` =35;

-- Murloc Tidecaller
 UPDATE `creature_template` SET `gold_min` =7 WHERE `entry` =548 AND `gold_min` =20;

-- Murloc Minor Tidecaller
 UPDATE `creature_template` SET `gold_max` =52 WHERE `entry` =548 AND `gold_max` =30;

-- Murloc Minor Tidecaller
 UPDATE `creature_template` SET `gold_min` =19 WHERE `entry` =568 AND `gold_min` =33;

-- Shadowhide Warrior
 UPDATE `creature_template` SET `gold_max` =95 WHERE `entry` =568 AND `gold_max` =48;

-- Shadowhide Warrior
 UPDATE `creature_template` SET `gold_max` =106 WHERE `entry` =570 AND `gold_max` =59;

-- Brain Eater
 UPDATE `creature_template` SET `gold_min` =9 WHERE `entry` =572 AND `gold_min` =23;

-- Leprithus
 UPDATE `creature_template` SET `gold_min` =10 WHERE `entry` =578 AND `gold_min` =24;

-- Murloc Scout
 UPDATE `creature_template` SET `gold_max` =64 WHERE `entry` =578 AND `gold_max` =36;

-- Murloc Scout
 UPDATE `creature_template` SET `gold_max` =48 WHERE `entry` =579 AND `gold_max` =45;

-- Shadowhide Assassin
 UPDATE `creature_template` SET `gold_max` =69 WHERE `entry` =580 AND `gold_max` =38;

-- Redridge Drudger
 UPDATE `creature_template` SET `gold_min` =33 WHERE `entry` =584 AND `gold_min` =50;

-- Kazon
 UPDATE `creature_template` SET `gold_max` =127 WHERE `entry` =584 AND `gold_max` =80;

-- Kazon
 UPDATE `creature_template` SET `gold_min` =39 WHERE `entry` =587 AND `gold_min` =55;

-- Bloodscalp Warrior (sample size 50)
 UPDATE `creature_template` SET `gold_max` =226 WHERE `entry` =587 AND `gold_max` =76;

-- Bloodscalp Warrior (sample size 50)
 UPDATE `creature_template` SET `gold_max` =199 WHERE `entry` =588 AND `gold_max` =78;

-- Bloodscalp Scout
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =589 AND `gold_min` =16;

-- Defias Pillager (sample size 40)
 UPDATE `creature_template` SET `gold_max` =34 WHERE `entry` =589 AND `gold_max` =26;

-- Defias Pillager (sample size 40)
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =590 AND `gold_min` =15;

-- Defias Looter (sample size 41)
 UPDATE `creature_template` SET `gold_max` =29 WHERE `entry` =590 AND `gold_max` =25;

-- Defias Looter (sample size 41)
 UPDATE `creature_template` SET `gold_max` =243 WHERE `entry` =595 AND `gold_max` =75;

-- Bloodscalp Hunter
 UPDATE `creature_template` SET `gold_max` =280 WHERE `entry` =597 AND `gold_max` =83;

-- Bloodscalp Berserker
 UPDATE `creature_template` SET `gold_min` =7 WHERE `entry` =598 AND `gold_min` =23;

-- Defias Miner (sample size 48)
 UPDATE `creature_template` SET `gold_max` =54 WHERE `entry` =598 AND `gold_max` =35;

-- Defias Miner (sample size 48)
 UPDATE `creature_template` SET `gold_max` =128 WHERE `entry` =604 AND `gold_max` =57;

-- Plague Spreader
 UPDATE `creature_template` SET `gold_min` =18 WHERE `entry` =615 AND `gold_min` =31;

-- Blackrock Tracker (sample size 21)
 UPDATE `creature_template` SET `gold_max` =102 WHERE `entry` =615 AND `gold_max` =45;

-- Blackrock Tracker (sample size 21)
 UPDATE `creature_template` SET `gold_min` =49 WHERE `entry` =619 AND `gold_min` =62;

-- Defias Conjurer
 UPDATE `creature_template` SET `gold_max` =178 WHERE `entry` =622 AND `gold_max` =119;

-- Goblin Engineer
 UPDATE `creature_template` SET `gold_min` =26 WHERE `entry` =634 AND `gold_min` =77;

-- Defias Overseer
 UPDATE `creature_template` SET `gold_max` =158 WHERE `entry` =634 AND `gold_max` =105;

-- Defias Overseer
 UPDATE `creature_template` SET `gold_min` =49 WHERE `entry` =636 AND `gold_min` =89;

-- Defias Blackguard
 UPDATE `creature_template` SET `gold_min` =51 WHERE `entry` =639 AND `gold_min` =318;

-- Edwin VanCleef
 UPDATE `creature_template` SET `gold_min` =32 WHERE `entry` =641 AND `gold_min` =87;

-- Goblin Woodcarver
 UPDATE `creature_template` SET `gold_max` =161 WHERE `entry` =641 AND `gold_max` =119;

-- Goblin Woodcarver
 UPDATE `creature_template` SET `gold_min` =159 WHERE `entry` =642 AND `gold_min` =197;

-- Sneed's Shredder
 UPDATE `creature_template` SET `gold_max` =144 WHERE `entry` =643 AND `gold_max` =70;

-- Sneed
 UPDATE `creature_template` SET `gold_min` =108 WHERE `entry` =644 AND `gold_min` =225;

-- Rhahk'Zor
 UPDATE `creature_template` SET `gold_min` =58 WHERE `entry` =645 AND `gold_min` =159;

-- Cookie
 UPDATE `creature_template` SET `gold_min` =69 WHERE `entry` =646 AND `gold_min` =201;

-- Mr. Smite
 UPDATE `creature_template` SET `gold_min` =76 WHERE `entry` =647 AND `gold_min` =159;

-- Captain Greenskin
 UPDATE `creature_template` SET `gold_min` =44 WHERE `entry` =657 AND `gold_min` =93;

-- Defias Pirate (sample size 26)
 UPDATE `creature_template` SET `gold_max` =196 WHERE `entry` =657 AND `gold_max` =126;

-- Defias Pirate (sample size 26)
 UPDATE `creature_template` SET `gold_min` =43 WHERE `entry` =660 AND `gold_min` =54;

-- Bloodscalp Witch Doctor
 UPDATE `creature_template` SET `gold_max` =289 WHERE `entry` =660 AND `gold_max` =75;

-- Bloodscalp Witch Doctor
 UPDATE `creature_template` SET `gold_min` =48 WHERE `entry` =667 AND `gold_min` =66;

-- Skullsplitter Warrior (sample size 39)
 UPDATE `creature_template` SET `gold_max` =325 WHERE `entry` =667 AND `gold_max` =91;

-- Skullsplitter Warrior (sample size 39)
 UPDATE `creature_template` SET `gold_max` =374 WHERE `entry` =669 AND `gold_max` =105;

-- Skullsplitter Hunter
 UPDATE `creature_template` SET `gold_min` =58 WHERE `entry` =670 AND `gold_min` =68;

-- Skullsplitter Witch Doctor
 UPDATE `creature_template` SET `gold_max` =358 WHERE `entry` =670 AND `gold_max` =94;

-- Skullsplitter Witch Doctor
 UPDATE `creature_template` SET `gold_min` =41 WHERE `entry` =671 AND `gold_min` =59;

-- Bloodscalp Headhunter (sample size 31)
 UPDATE `creature_template` SET `gold_max` =281 WHERE `entry` =671 AND `gold_max` =83;

-- Bloodscalp Headhunter (sample size 31)
 UPDATE `creature_template` SET `gold_max` =448 WHERE `entry` =672 AND `gold_max` =97;

-- Skullsplitter Spiritchaser
 UPDATE `creature_template` SET `gold_min` =55 WHERE `entry` =674 AND `gold_min` =71;

-- Venture Co. Strip Miner (sample size 38)
 UPDATE `creature_template` SET `gold_max` =360 WHERE `entry` =674 AND `gold_max` =98;

-- Venture Co. Strip Miner (sample size 38)
 UPDATE `creature_template` SET `gold_max` =249 WHERE `entry` =675 AND `gold_max` =103;

-- Venture Co. Foreman
 UPDATE `creature_template` SET `gold_max` =340 WHERE `entry` =676 AND `gold_max` =91;

-- Venture Co. Surveyor
 UPDATE `creature_template` SET `gold_max` =264 WHERE `entry` =677 AND `gold_max` =84;

-- Venture Co. Tinkerer
 UPDATE `creature_template` SET `gold_max` =1040 WHERE `entry` =678 AND `gold_max` =391;

-- Mosh'Ogg Mauler
 UPDATE `creature_template` SET `gold_max` =832 WHERE `entry` =679 AND `gold_max` =341;

-- Mosh'Ogg Shaman
 UPDATE `creature_template` SET `gold_max` =230 WHERE `entry` =694 AND `gold_max` =74;

-- Bloodscalp Axe Thrower (sample size 38)
 UPDATE `creature_template` SET `gold_min` =53 WHERE `entry` =696 AND `gold_min` =66;

-- Skullsplitter Axe Thrower (sample size 33)
 UPDATE `creature_template` SET `gold_max` =323 WHERE `entry` =696 AND `gold_max` =91;

-- Skullsplitter Axe Thrower (sample size 33)
 UPDATE `creature_template` SET `gold_min` =33 WHERE `entry` =697 AND `gold_min` =51;

-- Bloodscalp Shaman (sample size 47)
 UPDATE `creature_template` SET `gold_max` =229 WHERE `entry` =697 AND `gold_max` =71;

-- Bloodscalp Shaman (sample size 47)
 UPDATE `creature_template` SET `gold_max` =215 WHERE `entry` =699 AND `gold_max` =78;

-- Bloodscalp Beastmaster
 UPDATE `creature_template` SET `gold_max` =211 WHERE `entry` =701 AND `gold_max` =72;

-- Bloodscalp Mystic
 UPDATE `creature_template` SET `gold_min` =45 WHERE `entry` =702 AND `gold_min` =55;

-- Bloodscalp Scavenger
 UPDATE `creature_template` SET `gold_max` =183 WHERE `entry` =702 AND `gold_max` =76;

-- Bloodscalp Scavenger
 UPDATE `creature_template` SET `gold_max` =5 WHERE `entry` =706 AND `gold_max` =7;

-- Frostmane Troll Whelp (sample size 67)
 UPDATE `creature_template` SET `gold_max` =934 WHERE `entry` =709 AND `gold_max` =364;

-- Mosh'Ogg Warmonger
 UPDATE `creature_template` SET `gold_max` =1098 WHERE `entry` =710 AND `gold_max` =307;

-- Mosh'Ogg Spellcrafter
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =712 AND `gold_min` =16;

-- Redridge Thrasher
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =732 AND `gold_min` =9;

-- Murloc Lurker (sample size 39)
 UPDATE `creature_template` SET `gold_max` =15 WHERE `entry` =732 AND `gold_max` =17;

-- Murloc Lurker (sample size 39)
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =735 AND `gold_min` =5;

-- Murloc Streamrunner
 UPDATE `creature_template` SET `gold_min` =42 WHERE `entry` =740 AND `gold_min` =47;

-- Adolescent Whelp (sample size 36)
 UPDATE `creature_template` SET `gold_max` =239 WHERE `entry` =740 AND `gold_max` =66;

-- Adolescent Whelp (sample size 36)
 UPDATE `creature_template` SET `gold_min` =45 WHERE `entry` =741 AND `gold_min` =50;

-- Dreaming Whelp (sample size 40)
 UPDATE `creature_template` SET `gold_max` =264 WHERE `entry` =741 AND `gold_max` =70;

-- Dreaming Whelp (sample size 40)
 UPDATE `creature_template` SET `gold_max` =362 WHERE `entry` =746 AND `gold_max` =115;

-- Elder Dragonkin
 UPDATE `creature_template` SET `gold_min` =54 WHERE `entry` =747 AND `gold_min` =74;

-- Marsh Murloc (sample size 22)
 UPDATE `creature_template` SET `gold_max` =346 WHERE `entry` =747 AND `gold_max` =102;

-- Marsh Murloc (sample size 22)
 UPDATE `creature_template` SET `gold_min` =59 WHERE `entry` =750 AND `gold_min` =75;

-- Marsh Inkspewer (sample size 87)
 UPDATE `creature_template` SET `gold_max` =404 WHERE `entry` =750 AND `gold_max` =103;

-- Marsh Inkspewer (sample size 87)
 UPDATE `creature_template` SET `gold_min` =63 WHERE `entry` =751 AND `gold_min` =78;

-- Marsh Flesheater (sample size 95)
 UPDATE `creature_template` SET `gold_max` =421 WHERE `entry` =751 AND `gold_max` =107;

-- Marsh Flesheater (sample size 95)
 UPDATE `creature_template` SET `gold_min` =86 WHERE `entry` =752 AND `gold_min` =73;

-- Marsh Oracle (sample size 34)
 UPDATE `creature_template` SET `gold_max` =453 WHERE `entry` =752 AND `gold_max` =100;

-- Marsh Oracle (sample size 34)
 UPDATE `creature_template` SET `gold_max` =145 WHERE `entry` =754 AND `gold_max` =83;

-- Rebel Watchman
 UPDATE `creature_template` SET `gold_max` =228 WHERE `entry` =755 AND `gold_max` =103;

-- Lost One Mudlurker
 UPDATE `creature_template` SET `gold_min` =51 WHERE `entry` =757 AND `gold_min` =61;

-- Lost One Fisherman
 UPDATE `creature_template` SET `gold_max` =254 WHERE `entry` =757 AND `gold_max` =84;

-- Lost One Fisherman
 UPDATE `creature_template` SET `gold_min` =45 WHERE `entry` =759 AND `gold_min` =59;

-- Lost One Hunter
 UPDATE `creature_template` SET `gold_max` =276 WHERE `entry` =759 AND `gold_max` =83;

-- Lost One Hunter
 UPDATE `creature_template` SET `gold_min` =40 WHERE `entry` =760 AND `gold_min` =59;

-- Lost One Muckdweller (sample size 22)
 UPDATE `creature_template` SET `gold_max` =275 WHERE `entry` =760 AND `gold_max` =83;

-- Lost One Muckdweller (sample size 22)
 UPDATE `creature_template` SET `gold_max` =252 WHERE `entry` =761 AND `gold_max` =78;

-- Lost One Seer
 UPDATE `creature_template` SET `gold_min` =50 WHERE `entry` =762 AND `gold_min` =52;

-- Lost One Riftseeker
 UPDATE `creature_template` SET `gold_max` =298 WHERE `entry` =762 AND `gold_max` =73;

-- Lost One Riftseeker
 UPDATE `creature_template` SET `gold_max` =243 WHERE `entry` =763 AND `gold_max` =89;

-- Lost One Chieftain
 UPDATE `creature_template` SET `gold_max` =294 WHERE `entry` =764 AND `gold_max` =82;

-- Swampwalker
 UPDATE `creature_template` SET `gold_max` =332 WHERE `entry` =765 AND `gold_max` =83;

-- Swampwalker Elder
 UPDATE `creature_template` SET `gold_min` =54 WHERE `entry` =766 AND `gold_min` =63;

-- Tangled Horror
 UPDATE `creature_template` SET `gold_max` =241 WHERE `entry` =766 AND `gold_max` =87;

-- Tangled Horror
 UPDATE `creature_template` SET `gold_max` =155 WHERE `entry` =771 AND `gold_max` =70;

-- Commander Felstrom
 UPDATE `creature_template` SET `gold_max` =176 WHERE `entry` =775 AND `gold_max` =145;

-- Kurzen's Agent
 UPDATE `creature_template` SET `gold_min` =50 WHERE `entry` =780 AND `gold_min` =60;

-- Skullsplitter Mystic (sample size 32)
 UPDATE `creature_template` SET `gold_max` =337 WHERE `entry` =780 AND `gold_max` =83;

-- Skullsplitter Mystic (sample size 32)
 UPDATE `creature_template` SET `gold_max` =388 WHERE `entry` =781 AND `gold_max` =107;

-- Skullsplitter Headhunter
 UPDATE `creature_template` SET `gold_min` =61 WHERE `entry` =782 AND `gold_min` =76;

-- Skullsplitter Scout (sample size 21)
 UPDATE `creature_template` SET `gold_max` =375 WHERE `entry` =782 AND `gold_max` =105;

-- Skullsplitter Scout (sample size 21)
 UPDATE `creature_template` SET `gold_min` =75 WHERE `entry` =783 AND `gold_min` =78;

-- Skullsplitter Berserker
 UPDATE `creature_template` SET `gold_max` =393 WHERE `entry` =783 AND `gold_max` =107;

-- Skullsplitter Berserker
 UPDATE `creature_template` SET `gold_max` =324 WHERE `entry` =784 AND `gold_max` =105;

-- Skullsplitter Beastmaster
 UPDATE `creature_template` SET `gold_min` =35 WHERE `entry` =785 AND `gold_min` =39;

-- Skeletal Warder
 UPDATE `creature_template` SET `gold_max` =100 WHERE `entry` =785 AND `gold_max` =55;

-- Skeletal Warder
 UPDATE `creature_template` SET `gold_max` =99 WHERE `entry` =787 AND `gold_max` =50;

-- Skeletal Healer
 UPDATE `creature_template` SET `gold_max` =1025 WHERE `entry` =813 AND `gold_max` =345;

-- Colonel Kurzen
 UPDATE `creature_template` SET `gold_min` =46 WHERE `entry` =814 AND `gold_min` =57;

-- Sergeant Malthus
 UPDATE `creature_template` SET `gold_max` =21 WHERE `entry` =846 AND `gold_max` =19;

-- Ghoul
 UPDATE `creature_template` SET `gold_max` =264 WHERE `entry` =861 AND `gold_max` =96;

-- Stonard Scout
 UPDATE `creature_template` SET `gold_max` =215 WHERE `entry` =862 AND `gold_max` =86;

-- Stonard Explorer
 UPDATE `creature_template` SET `gold_max` =399 WHERE `entry` =867 AND `gold_max` =142;

-- Stonard Cartographer
 UPDATE `creature_template` SET `gold_min` =213 WHERE `entry` =871 AND `gold_min` =218;

-- Saltscale Warrior
 UPDATE `creature_template` SET `gold_max` =741 WHERE `entry` =871 AND `gold_max` =290;

-- Saltscale Warrior
 UPDATE `creature_template` SET `gold_max` =586 WHERE `entry` =877 AND `gold_max` =290;

-- Saltscale Forager
 UPDATE `creature_template` SET `gold_min` =132 WHERE `entry` =879 AND `gold_min` =218;

-- Saltscale Hunter
 UPDATE `creature_template` SET `gold_max` =733 WHERE `entry` =879 AND `gold_max` =290;

-- Saltscale Hunter
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =880 AND `gold_min` =7;

-- Erlan Drudgemoor
 UPDATE `creature_template` SET `gold_max` =59 WHERE `entry` =889 AND `gold_max` =52;

-- Splinter Fist Ogre
 UPDATE `creature_template` SET `gold_min` =22 WHERE `entry` =892 AND `gold_min` =39;

-- Splinter Fist Taskmaster
 UPDATE `creature_template` SET `gold_min` =26 WHERE `entry` =909 AND `gold_min` =35;

-- Defias Night Blade
 UPDATE `creature_template` SET `gold_max` =79 WHERE `entry` =909 AND `gold_max` =51;

-- Defias Night Blade
 UPDATE `creature_template` SET `gold_min` =21 WHERE `entry` =910 AND `gold_min` =33;

-- Defias Enchanter
 UPDATE `creature_template` SET `gold_max` =126 WHERE `entry` =910 AND `gold_max` =48;

-- Defias Enchanter
 UPDATE `creature_template` SET `gold_min` =26 WHERE `entry` =920 AND `gold_min` =42;

-- Nightbane Tainted One
 UPDATE `creature_template` SET `gold_max` =180 WHERE `entry` =920 AND `gold_max` =60;

-- Nightbane Tainted One
 UPDATE `creature_template` SET `gold_min` =42 WHERE `entry` =921 AND `gold_min` =54;

-- Venture Co. Lumberjack
 UPDATE `creature_template` SET `gold_max` =219 WHERE `entry` =921 AND `gold_max` =75;

-- Venture Co. Lumberjack
 UPDATE `creature_template` SET `gold_min` =34 WHERE `entry` =937 AND `gold_min` =50;

-- Kurzen Jungle Fighter (sample size 35)
 UPDATE `creature_template` SET `gold_max` =189 WHERE `entry` =937 AND `gold_max` =70;

-- Kurzen Jungle Fighter (sample size 35)
 UPDATE `creature_template` SET `gold_min` =36 WHERE `entry` =938 AND `gold_min` =54;

-- Kurzen Commando (sample size 54)
 UPDATE `creature_template` SET `gold_max` =242 WHERE `entry` =938 AND `gold_max` =75;

-- Kurzen Commando (sample size 54)
 UPDATE `creature_template` SET `gold_min` =45 WHERE `entry` =939 AND `gold_min` =59;

-- Kurzen Elite (sample size 25)
 UPDATE `creature_template` SET `gold_max` =281 WHERE `entry` =939 AND `gold_max` =83;

-- Kurzen Elite (sample size 25)
 UPDATE `creature_template` SET `gold_min` =42 WHERE `entry` =940 AND `gold_min` =46;

-- Kurzen Medicine Man (sample size 25)
 UPDATE `creature_template` SET `gold_max` =195 WHERE `entry` =940 AND `gold_max` =65;

-- Kurzen Medicine Man (sample size 25)
 UPDATE `creature_template` SET `gold_min` =41 WHERE `entry` =941 AND `gold_min` =49;

-- Kurzen Headshrinker (sample size 37)
 UPDATE `creature_template` SET `gold_max` =246 WHERE `entry` =941 AND `gold_max` =69;

-- Kurzen Headshrinker (sample size 37)
 UPDATE `creature_template` SET `gold_min` =42 WHERE `entry` =942 AND `gold_min` =53;

-- Kurzen Witch Doctor
 UPDATE `creature_template` SET `gold_max` =268 WHERE `entry` =942 AND `gold_max` =74;

-- Kurzen Witch Doctor
 UPDATE `creature_template` SET `gold_max` =130 WHERE `entry` =947 AND `gold_max` =52;

-- Rohh the Silent
 UPDATE `creature_template` SET `gold_max` =51 WHERE `entry` =948 AND `gold_max` =50;

-- Rotted One
 UPDATE `creature_template` SET `gold_max` =640 WHERE `entry` =950 AND `gold_max` =140;

-- Swamp Talker
 UPDATE `creature_template` SET `gold_min` =52 WHERE `entry` =978 AND `gold_min` =57;

-- Kurzen Subchief (sample size 36)
 UPDATE `creature_template` SET `gold_max` =318 WHERE `entry` =978 AND `gold_max` =79;

-- Kurzen Subchief (sample size 36)
 UPDATE `creature_template` SET `gold_max` =249 WHERE `entry` =979 AND `gold_max` =90;

-- Kurzen Shadow Hunter
 UPDATE `creature_template` SET `gold_min` =10 WHERE `entry` =1007 AND `gold_min` =25;

-- Mosshide Gnoll (sample size 25)
 UPDATE `creature_template` SET `gold_max` =69 WHERE `entry` =1007 AND `gold_max` =38;

-- Mosshide Gnoll (sample size 25)
 UPDATE `creature_template` SET `gold_min` =16 WHERE `entry` =1008 AND `gold_min` =27;

-- Mosshide Mongrel (sample size 25)
 UPDATE `creature_template` SET `gold_max` =75 WHERE `entry` =1008 AND `gold_max` =40;

-- Mosshide Mongrel (sample size 25)
 UPDATE `creature_template` SET `gold_max` =67 WHERE `entry` =1009 AND `gold_max` =40;

-- Mosshide Mistweaver
 UPDATE `creature_template` SET `gold_min` =18 WHERE `entry` =1010 AND `gold_min` =29;

-- Mosshide Fenrunner
 UPDATE `creature_template` SET `gold_max` =86 WHERE `entry` =1010 AND `gold_max` =42;

-- Mosshide Fenrunner
 UPDATE `creature_template` SET `gold_min` =15 WHERE `entry` =1011 AND `gold_min` =31;

-- Mosshide Trapper
 UPDATE `creature_template` SET `gold_max` =99 WHERE `entry` =1011 AND `gold_max` =45;

-- Mosshide Trapper
 UPDATE `creature_template` SET `gold_min` =32 WHERE `entry` =1012 AND `gold_min` =33;

-- Mosshide Brute
 UPDATE `creature_template` SET `gold_max` =163 WHERE `entry` =1012 AND `gold_max` =48;

-- Mosshide Brute
 UPDATE `creature_template` SET `gold_max` =110 WHERE `entry` =1013 AND `gold_max` =49;

-- Mosshide Mystic
 UPDATE `creature_template` SET `gold_max` =125 WHERE `entry` =1014 AND `gold_max` =56;

-- Mosshide Alpha
 UPDATE `creature_template` SET `gold_min` =10 WHERE `entry` =1024 AND `gold_min` =25;

-- Bluegill Murloc (sample size 35)
 UPDATE `creature_template` SET `gold_max` =71 WHERE `entry` =1024 AND `gold_max` =38;

-- Bluegill Murloc (sample size 35)
 UPDATE `creature_template` SET `gold_min` =15 WHERE `entry` =1025 AND `gold_min` =27;

-- Bluegill Puddlejumper
 UPDATE `creature_template` SET `gold_max` =86 WHERE `entry` =1025 AND `gold_max` =40;

-- Bluegill Puddlejumper
 UPDATE `creature_template` SET `gold_max` =76 WHERE `entry` =1026 AND `gold_max` =43;

-- Bluegill Forager
 UPDATE `creature_template` SET `gold_max` =133 WHERE `entry` =1027 AND `gold_max` =48;

-- Bluegill Warrior
 UPDATE `creature_template` SET `gold_min` =18 WHERE `entry` =1028 AND `gold_min` =28;

-- Bluegill Muckdweller
 UPDATE `creature_template` SET `gold_min` =14 WHERE `entry` =1030 AND `gold_min` =26;

-- Black Slime
 UPDATE `creature_template` SET `gold_min` =10 WHERE `entry` =1031 AND `gold_min` =33;

-- Crimson Ooze
 UPDATE `creature_template` SET `gold_min` =6 WHERE `entry` =1032 AND `gold_min` =31;

-- Black Ooze (sample size 31)
 UPDATE `creature_template` SET `gold_max` =38 WHERE `entry` =1032 AND `gold_max` =45;

-- Black Ooze (sample size 31)
 UPDATE `creature_template` SET `gold_min` =23 WHERE `entry` =1034 AND `gold_min` =37;

-- Dragonmaw Raider (sample size 42)
 UPDATE `creature_template` SET `gold_max` =133 WHERE `entry` =1034 AND `gold_max` =53;

-- Dragonmaw Raider (sample size 42)
 UPDATE `creature_template` SET `gold_max` =113 WHERE `entry` =1035 AND `gold_max` =57;

-- Dragonmaw Swamprunner
 UPDATE `creature_template` SET `gold_min` =28 WHERE `entry` =1036 AND `gold_min` =42;

-- Dragonmaw Centurion
 UPDATE `creature_template` SET `gold_max` =145 WHERE `entry` =1036 AND `gold_max` =59;

-- Dragonmaw Centurion
 UPDATE `creature_template` SET `gold_min` =34 WHERE `entry` =1038 AND `gold_min` =37;

-- Dragonmaw Shadowwarder
 UPDATE `creature_template` SET `gold_max` =148 WHERE `entry` =1038 AND `gold_max` =53;

-- Dragonmaw Shadowwarder
 UPDATE `creature_template` SET `gold_max` =43 WHERE `entry` =1039 AND `gold_max` =38;

-- Fen Dweller
 UPDATE `creature_template` SET `gold_min` =32 WHERE `entry` =1040 AND `gold_min` =33;

-- Fen Creeper
 UPDATE `creature_template` SET `gold_max` =164 WHERE `entry` =1040 AND `gold_max` =48;

-- Fen Creeper
 UPDATE `creature_template` SET `gold_min` =20 WHERE `entry` =1042 AND `gold_min` =31;

-- Red Whelp
 UPDATE `creature_template` SET `gold_min` =72 WHERE `entry` =1051 AND `gold_min` =153;

-- Dark Iron Dwarf
 UPDATE `creature_template` SET `gold_max` =423 WHERE `entry` =1051 AND `gold_max` =205;

-- Dark Iron Dwarf
 UPDATE `creature_template` SET `gold_min` =147 WHERE `entry` =1052 AND `gold_min` =159;

-- Dark Iron Saboteur
 UPDATE `creature_template` SET `gold_max` =455 WHERE `entry` =1052 AND `gold_max` =213;

-- Dark Iron Saboteur
 UPDATE `creature_template` SET `gold_min` =119 WHERE `entry` =1053 AND `gold_min` =170;

-- Dark Iron Tunneler
 UPDATE `creature_template` SET `gold_max` =496 WHERE `entry` =1053 AND `gold_max` =226;

-- Dark Iron Tunneler
 UPDATE `creature_template` SET `gold_max` =308 WHERE `entry` =1054 AND `gold_max` =234;

-- Dark Iron Demolitionist
 UPDATE `creature_template` SET `gold_min` =20 WHERE `entry` =1057 AND `gold_min` =35;

-- Dragonmaw Bonewarder
 UPDATE `creature_template` SET `gold_max` =145 WHERE `entry` =1057 AND `gold_max` =50;

-- Dragonmaw Bonewarder
 UPDATE `creature_template` SET `gold_max` =438 WHERE `entry` =1059 AND `gold_max` =115;

-- Ana'thek the Cruel
 UPDATE `creature_template` SET `gold_min` =236 WHERE `entry` =1060 AND `gold_min` =259;

-- Mogh the Undying
 UPDATE `creature_template` SET `gold_min` =62 WHERE `entry` =1061 AND `gold_min` =74;

-- Gan'zulah
 UPDATE `creature_template` SET `gold_max` =189 WHERE `entry` =1061 AND `gold_max` =102;

-- Gan'zulah
 UPDATE `creature_template` SET `gold_min` =56 WHERE `entry` =1062 AND `gold_min` =61;

-- Nezzliok the Dire
 UPDATE `creature_template` SET `gold_max` =337 WHERE `entry` =1062 AND `gold_max` =84;

-- Nezzliok the Dire
 UPDATE `creature_template` SET `gold_min` =6 WHERE `entry` =1065 AND `gold_min` =13;

-- Riverpaw Shaman
 UPDATE `creature_template` SET `gold_max` =171 WHERE `entry` =1081 AND `gold_max` =117;

-- Mire Lord
 UPDATE `creature_template` SET `gold_min` =8 WHERE `entry` =1083 AND `gold_min` =19;

-- Murloc Shorestriker
 UPDATE `creature_template` SET `gold_max` =37 WHERE `entry` =1083 AND `gold_max` =30;

-- Murloc Shorestriker
 UPDATE `creature_template` SET `gold_max` =208 WHERE `entry` =1094 AND `gold_max` =75;

-- Venture Co. Miner
 UPDATE `creature_template` SET `gold_max` =175 WHERE `entry` =1095 AND `gold_max` =83;

-- Venture Co. Workboss
 UPDATE `creature_template` SET `gold_min` =50 WHERE `entry` =1096 AND `gold_min` =52;

-- Venture Co. Geologist (sample size 39)
 UPDATE `creature_template` SET `gold_max` =263 WHERE `entry` =1096 AND `gold_max` =73;

-- Venture Co. Geologist (sample size 39)
 UPDATE `creature_template` SET `gold_min` =44 WHERE `entry` =1097 AND `gold_min` =56;

-- Venture Co. Mechanic
 UPDATE `creature_template` SET `gold_max` =229 WHERE `entry` =1097 AND `gold_max` =78;

-- Venture Co. Mechanic
 UPDATE `creature_template` SET `gold_max` =152 WHERE `entry` =1106 AND `gold_max` =81;

-- Lost One Cook
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =1115 AND `gold_min` =8;

-- Rockjaw Skullthumper (sample size 29)
 UPDATE `creature_template` SET `gold_max` =14 WHERE `entry` =1115 AND `gold_max` =15;

-- Rockjaw Skullthumper (sample size 29)
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =1116 AND `gold_min` =9;

-- Rockjaw Ambusher
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =1117 AND `gold_min` =10;

-- Rockjaw Bonesnapper (sample size 22)
 UPDATE `creature_template` SET `gold_min` =7 WHERE `entry` =1118 AND `gold_min` =13;

-- Rockjaw Backbreaker
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =1120 AND `gold_min` =7;

-- Frostmane Troll
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =1121 AND `gold_min` =12;

-- Frostmane Snowstrider
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =1122 AND `gold_min` =9;

-- Frostmane Hideskinner
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =1123 AND `gold_min` =8;

-- Frostmane Headhunter
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =1134 AND `gold_min` =4;

-- Young Wendigo
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =1135 AND `gold_min` =6;

-- Wendigo (sample size 24)
 UPDATE `creature_template` SET `gold_max` =12 WHERE `entry` =1135 AND `gold_max` =13;

-- Wendigo (sample size 24)
 UPDATE `creature_template` SET `gold_min` =44 WHERE `entry` =1142 AND `gold_min` =62;

-- Mosh'Ogg Brute (sample size 21)
 UPDATE `creature_template` SET `gold_max` =275 WHERE `entry` =1142 AND `gold_max` =85;

-- Mosh'Ogg Brute (sample size 21)
 UPDATE `creature_template` SET `gold_max` =272 WHERE `entry` =1144 AND `gold_max` =74;

-- Mosh'Ogg Witch Doctor
 UPDATE `creature_template` SET `gold_min` =19 WHERE `entry` =1157 AND `gold_min` =37;

-- Cursed Sailor (sample size 35)
 UPDATE `creature_template` SET `gold_max` =132 WHERE `entry` =1157 AND `gold_max` =53;

-- Cursed Sailor (sample size 35)
 UPDATE `creature_template` SET `gold_min` =24 WHERE `entry` =1158 AND `gold_min` =39;

-- Cursed Marine (sample size 21)
 UPDATE `creature_template` SET `gold_max` =130 WHERE `entry` =1158 AND `gold_max` =56;

-- Cursed Marine (sample size 21)
 UPDATE `creature_template` SET `gold_max` =145 WHERE `entry` =1159 AND `gold_max` =60;

-- First Mate Snellig
 UPDATE `creature_template` SET `gold_min` =27 WHERE `entry` =1160 AND `gold_min` =44;

-- Captain Halyndor
 UPDATE `creature_template` SET `gold_max` =165 WHERE `entry` =1160 AND `gold_max` =62;

-- Captain Halyndor
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =1161 AND `gold_min` =12;

-- Stonesplinter Trogg (sample size 117)
 UPDATE `creature_template` SET `gold_max` =24 WHERE `entry` =1161 AND `gold_max` =20;

-- Stonesplinter Trogg (sample size 117)
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =1162 AND `gold_min` =12;

-- Stonesplinter Scout (sample size 142)
 UPDATE `creature_template` SET `gold_max` =24 WHERE `entry` =1162 AND `gold_max` =20;

-- Stonesplinter Scout (sample size 142)
 UPDATE `creature_template` SET `gold_min` =5 WHERE `entry` =1163 AND `gold_min` =15;

-- Stonesplinter Skullthumper (sample size 92)
 UPDATE `creature_template` SET `gold_max` =29 WHERE `entry` =1163 AND `gold_max` =24;

-- Stonesplinter Skullthumper (sample size 92)
 UPDATE `creature_template` SET `gold_min` =5 WHERE `entry` =1164 AND `gold_min` =17;

-- Stonesplinter Bonesnapper (sample size 61)
 UPDATE `creature_template` SET `gold_max` =38 WHERE `entry` =1164 AND `gold_max` =27;

-- Stonesplinter Bonesnapper (sample size 61)
 UPDATE `creature_template` SET `gold_min` =7 WHERE `entry` =1165 AND `gold_min` =20;

-- Stonesplinter Geomancer (sample size 23)
 UPDATE `creature_template` SET `gold_max` =54 WHERE `entry` =1165 AND `gold_max` =30;

-- Stonesplinter Geomancer (sample size 23)
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =1166 AND `gold_min` =14;

-- Stonesplinter Seer (sample size 68)
 UPDATE `creature_template` SET `gold_max` =29 WHERE `entry` =1166 AND `gold_max` =23;

-- Stonesplinter Seer (sample size 68)
 UPDATE `creature_template` SET `gold_min` =8 WHERE `entry` =1167 AND `gold_min` =22;

-- Stonesplinter Digger (sample size 21)
 UPDATE `creature_template` SET `gold_max` =60 WHERE `entry` =1167 AND `gold_max` =33;

-- Stonesplinter Digger (sample size 21)
 UPDATE `creature_template` SET `gold_min` =14 WHERE `entry` =1169 AND `gold_min` =22;

-- Dark Iron Insurgent
 UPDATE `creature_template` SET `gold_max` =60 WHERE `entry` =1169 AND `gold_max` =34;

-- Dark Iron Insurgent
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =1172 AND `gold_min` =10;

-- Tunnel Rat Vermin (sample size 28)
 UPDATE `creature_template` SET `gold_max` =17 WHERE `entry` =1172 AND `gold_max` =18;

-- Tunnel Rat Vermin (sample size 28)
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =1173 AND `gold_min` =10;

-- Tunnel Rat Scout
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =1174 AND `gold_min` =12;

-- Tunnel Rat Geomancer
 UPDATE `creature_template` SET `gold_max` =25 WHERE `entry` =1174 AND `gold_max` =21;

-- Tunnel Rat Geomancer
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =1175 AND `gold_min` =13;

-- Tunnel Rat Digger
 UPDATE `creature_template` SET `gold_max` =27 WHERE `entry` =1175 AND `gold_max` =22;

-- Tunnel Rat Digger
 UPDATE `creature_template` SET `gold_min` =7 WHERE `entry` =1176 AND `gold_min` =12;

-- Tunnel Rat Forager
 UPDATE `creature_template` SET `gold_min` =34 WHERE `entry` =1178 AND `gold_min` =83;

-- Mo'grosh Ogre
 UPDATE `creature_template` SET `gold_max` =169 WHERE `entry` =1179 AND `gold_max` =108;

-- Mo'grosh Enforcer
 UPDATE `creature_template` SET `gold_min` =56 WHERE `entry` =1180 AND `gold_min` =89;

-- Mo'grosh Brute
 UPDATE `creature_template` SET `gold_min` =13 WHERE `entry` =1181 AND `gold_min` =79;

-- Mo'grosh Shaman
 UPDATE `creature_template` SET `gold_min` =5 WHERE `entry` =1197 AND `gold_min` =17;

-- Stonesplinter Shaman (sample size 39)
 UPDATE `creature_template` SET `gold_max` =37 WHERE `entry` =1197 AND `gold_max` =27;

-- Stonesplinter Shaman (sample size 39)
 UPDATE `creature_template` SET `gold_min` =8 WHERE `entry` =1202 AND `gold_min` =12;

-- Tunnel Rat Kobold
 UPDATE `creature_template` SET `gold_min` =19 WHERE `entry` =1205 AND `gold_min` =20;

-- Grawmug
 UPDATE `creature_template` SET `gold_max` =37 WHERE `entry` =1206 AND `gold_max` =29;

-- Gnasher
 UPDATE `creature_template` SET `gold_min` =13 WHERE `entry` =1207 AND `gold_min` =18;

-- Brawler
 UPDATE `creature_template` SET `gold_max` =46 WHERE `entry` =1207 AND `gold_max` =29;

-- Brawler
 UPDATE `creature_template` SET `gold_min` =57 WHERE `entry` =1210 AND `gold_min` =111;

-- Chok'sul
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =1211 AND `gold_min` =9;

-- Leper Gnome (sample size 54)
 UPDATE `creature_template` SET `gold_max` =14 WHERE `entry` =1211 AND `gold_max` =17;

-- Leper Gnome (sample size 54)
 UPDATE `creature_template` SET `gold_max` =50 WHERE `entry` =1222 AND `gold_max` =31;

-- Dark Iron Sapper
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =1236 AND `gold_min` =13;

-- Kobold Digger
 UPDATE `creature_template` SET `gold_max` =56 WHERE `entry` =1251 AND `gold_max` =54;

-- Splinter Fist Firemonger
 UPDATE `creature_template` SET `gold_max` =91 WHERE `entry` =1259 AND `gold_max` =42;

-- Gobbler
 UPDATE `creature_template` SET `gold_max` =145 WHERE `entry` =1270 AND `gold_max` =61;

-- Fetid Corpse
 UPDATE `creature_template` SET `gold_min` =22 WHERE `entry` =1271 AND `gold_min` =44;

-- Old Icebeard
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =1380 AND `gold_min` =3;

-- Saean
 UPDATE `creature_template` SET `gold_min` =27 WHERE `entry` =1388 AND `gold_min` =45;

-- Vagash
 UPDATE `creature_template` SET `gold_max` =63 WHERE `entry` =1393 AND `gold_max` =36;

-- Berserk Trogg
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =1397 AND `gold_min` =8;

-- Frostmane Seer (sample size 21)
 UPDATE `creature_template` SET `gold_max` =14 WHERE `entry` =1397 AND `gold_max` =15;

-- Frostmane Seer (sample size 21)
 UPDATE `creature_template` SET `gold_max` =107 WHERE `entry` =1421 AND `gold_max` =69;

-- Private Merle
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =1426 AND `gold_min` =16;

-- Riverpaw Miner
 UPDATE `creature_template` SET `gold_max` =33 WHERE `entry` =1426 AND `gold_max` =26;

-- Riverpaw Miner
 UPDATE `creature_template` SET `gold_max` =118 WHERE `entry` =1487 AND `gold_max` =65;

-- Splinter Fist Enslaver
 UPDATE `creature_template` SET `gold_min` =85 WHERE `entry` =1488 AND `gold_min` =80;

-- Zanzil Zombie (sample size 27)
 UPDATE `creature_template` SET `gold_max` =419 WHERE `entry` =1488 AND `gold_max` =110;

-- Zanzil Zombie (sample size 27)
 UPDATE `creature_template` SET `gold_min` =74 WHERE `entry` =1489 AND `gold_min` =78;

-- Zanzil Hunter
 UPDATE `creature_template` SET `gold_max` =413 WHERE `entry` =1489 AND `gold_max` =107;

-- Zanzil Hunter
 UPDATE `creature_template` SET `gold_max` =428 WHERE `entry` =1490 AND `gold_max` =97;

-- Zanzil Witch Doctor
 UPDATE `creature_template` SET `gold_min` =77 WHERE `entry` =1491 AND `gold_min` =79;

-- Zanzil Naga
 UPDATE `creature_template` SET `gold_max` =376 WHERE `entry` =1491 AND `gold_max` =108;

-- Zanzil Naga
 UPDATE `creature_template` SET `gold_max` =1181 WHERE `entry` =1492 AND `gold_max` =549;

-- Gorlash
 UPDATE `creature_template` SET `gold_max` =4 WHERE `entry` =1501 AND `gold_max` =3;

-- Mindless Zombie (sample size 45)
 UPDATE `creature_template` SET `gold_max` =5 WHERE `entry` =1506 AND `gold_max` =6;

-- Scarlet Convert (sample size 52)
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =1520 AND `gold_min` =5;

-- Rattlecage Soldier (sample size 22)
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =1522 AND `gold_min` =6;

-- Darkeye Bonecaster (sample size 58)
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =1523 AND `gold_min` =7;

-- Cracked Skull Soldier (sample size 26)
 UPDATE `creature_template` SET `gold_max` =14 WHERE `entry` =1523 AND `gold_max` =15;

-- Cracked Skull Soldier (sample size 26)
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =1525 AND `gold_min` =4;

-- Rotting Dead (sample size 21)
 UPDATE `creature_template` SET `gold_max` =7 WHERE `entry` =1525 AND `gold_max` =10;

-- Rotting Dead (sample size 21)
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =1526 AND `gold_min` =5;

-- Ravaged Corpse
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =1527 AND `gold_min` =6;

-- Hungering Dead
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =1528 AND `gold_min` =8;

-- Shambling Horror
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =1529 AND `gold_min` =10;

-- Bleeding Horror
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =1530 AND `gold_min` =10;

-- Rotting Ancestor (sample size 22)
 UPDATE `creature_template` SET `gold_max` =16 WHERE `entry` =1530 AND `gold_max` =18;

-- Rotting Ancestor (sample size 22)
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =1531 AND `gold_min` =5;

-- Lost Soul
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =1532 AND `gold_min` =11;

-- Wandering Spirit
 UPDATE `creature_template` SET `gold_min` =6 WHERE `entry` =1533 AND `gold_min` =8;

-- Tormented Spirit
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =1534 AND `gold_min` =9;

-- Wailing Ancestor
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =1535 AND `gold_min` =5;

-- Scarlet Warrior (sample size 38)
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =1536 AND `gold_min` =7;

-- Scarlet Missionary
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =1537 AND `gold_min` =9;

-- Scarlet Zealot
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =1538 AND `gold_min` =9;

-- Scarlet Friar
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =1539 AND `gold_min` =10;

-- Scarlet Neophyte
 UPDATE `creature_template` SET `gold_min` =6 WHERE `entry` =1540 AND `gold_min` =11;

-- Scarlet Vanguard
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =1543 AND `gold_min` =6;

-- Vile Fin Puddlejumper
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =1544 AND `gold_min` =7;

-- Vile Fin Minor Oracle
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =1545 AND `gold_min` =9;

-- Vile Fin Muckdweller
 UPDATE `creature_template` SET `gold_min` =76 WHERE `entry` =1561 AND `gold_min` =74;

-- Bloodsail Raider (sample size 30)
 UPDATE `creature_template` SET `gold_max` =350 WHERE `entry` =1561 AND `gold_max` =101;

-- Bloodsail Raider (sample size 30)
 UPDATE `creature_template` SET `gold_min` =61 WHERE `entry` =1562 AND `gold_min` =59;

-- Bloodsail Mage (sample size 31)
 UPDATE `creature_template` SET `gold_max` =314 WHERE `entry` =1562 AND `gold_max` =81;

-- Bloodsail Mage (sample size 31)
 UPDATE `creature_template` SET `gold_min` =61 WHERE `entry` =1563 AND `gold_min` =75;

-- Bloodsail Swashbuckler (sample size 57)
 UPDATE `creature_template` SET `gold_max` =404 WHERE `entry` =1563 AND `gold_max` =103;

-- Bloodsail Swashbuckler (sample size 57)
 UPDATE `creature_template` SET `gold_max` =401 WHERE `entry` =1564 AND `gold_max` =86;

-- Bloodsail Warlock (sample size 57)
 UPDATE `creature_template` SET `gold_min` =73 WHERE `entry` =1565 AND `gold_min` =83;

-- Bloodsail Sea Dog (sample size 26)
 UPDATE `creature_template` SET `gold_max` =452 WHERE `entry` =1565 AND `gold_max` =114;

-- Bloodsail Sea Dog (sample size 26)
 UPDATE `creature_template` SET `gold_min` =66 WHERE `entry` =1653 AND `gold_min` =68;

-- Bloodsail Elder Magus (sample size 26)
 UPDATE `creature_template` SET `gold_max` =444 WHERE `entry` =1653 AND `gold_max` =94;

-- Bloodsail Elder Magus (sample size 26)
 UPDATE `creature_template` SET `gold_min` =6 WHERE `entry` =1654 AND `gold_min` =10;

-- Gregor Agamand
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =1655 AND `gold_min` =10;

-- Nissa Agamand
 UPDATE `creature_template` SET `gold_min` =9 WHERE `entry` =1656 AND `gold_min` =10;

-- Thurman Agamand
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =1657 AND `gold_min` =8;

-- Devlin Agamand
 UPDATE `creature_template` SET `gold_max` =27 WHERE `entry` =1658 AND `gold_max` =23;

-- Captain Dargol
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =1660 AND `gold_min` =7;

-- Scarlet Bodyguard
 UPDATE `creature_template` SET `gold_min` =5 WHERE `entry` =1662 AND `gold_min` =8;

-- Captain Perrine
 UPDATE `creature_template` SET `gold_max` =20 WHERE `entry` =1664 AND `gold_max` =19;

-- Captain Vachon
 UPDATE `creature_template` SET `gold_min` =10 WHERE `entry` =1665 AND `gold_min` =13;

-- Captain Melrache
 UPDATE `creature_template` SET `gold_min` =31 WHERE `entry` =1666 AND `gold_min` =153;

-- Kam Deepfury
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =1667 AND `gold_min` =3;

-- Meven Korgal
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =1674 AND `gold_min` =5;

-- Rot Hide Gnoll
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =1675 AND `gold_min` =7;

-- Rot Hide Mongrel (sample size 35)
 UPDATE `creature_template` SET `gold_max` =266 WHERE `entry` =1696 AND `gold_max` =165;

-- Targorr the Dread
 UPDATE `creature_template` SET `gold_min` =64 WHERE `entry` =1706 AND `gold_min` =121;

-- Defias Prisoner (sample size 22)
 UPDATE `creature_template` SET `gold_max` =323 WHERE `entry` =1706 AND `gold_max` =163;

-- Defias Prisoner (sample size 22)
 UPDATE `creature_template` SET `gold_min` =60 WHERE `entry` =1707 AND `gold_min` =121;

-- Defias Captive
 UPDATE `creature_template` SET `gold_max` =325 WHERE `entry` =1707 AND `gold_max` =163;

-- Defias Captive
 UPDATE `creature_template` SET `gold_min` =64 WHERE `entry` =1708 AND `gold_min` =131;

-- Defias Inmate
 UPDATE `creature_template` SET `gold_max` =369 WHERE `entry` =1708 AND `gold_max` =176;

-- Defias Inmate
 UPDATE `creature_template` SET `gold_min` =74 WHERE `entry` =1711 AND `gold_min` =127;

-- Defias Convict
 UPDATE `creature_template` SET `gold_max` =348 WHERE `entry` =1711 AND `gold_max` =171;

-- Defias Convict
 UPDATE `creature_template` SET `gold_min` =69 WHERE `entry` =1715 AND `gold_min` =133;

-- Defias Insurgent
 UPDATE `creature_template` SET `gold_max` =402 WHERE `entry` =1715 AND `gold_max` =179;

-- Defias Insurgent
 UPDATE `creature_template` SET `gold_max` =368 WHERE `entry` =1716 AND `gold_max` =226;

-- Bazil Thredd
 UPDATE `creature_template` SET `gold_max` =286 WHERE `entry` =1717 AND `gold_max` =196;

-- Hamhock
 UPDATE `creature_template` SET `gold_min` =34 WHERE `entry` =1725 AND `gold_min` =75;

-- Defias Watchman
 UPDATE `creature_template` SET `gold_max` =98 WHERE `entry` =1726 AND `gold_max` =91;

-- Defias Magician
 UPDATE `creature_template` SET `gold_min` =17 WHERE `entry` =1727 AND `gold_min` =20;

-- Defias Worker
 UPDATE `creature_template` SET `gold_max` =33 WHERE `entry` =1727 AND `gold_max` =31;

-- Defias Worker
 UPDATE `creature_template` SET `gold_min` =23 WHERE `entry` =1729 AND `gold_min` =77;

-- Defias Evoker
 UPDATE `creature_template` SET `gold_max` =156 WHERE `entry` =1729 AND `gold_max` =105;

-- Defias Evoker
 UPDATE `creature_template` SET `gold_min` =41 WHERE `entry` =1731 AND `gold_min` =87;

-- Goblin Craftsman
 UPDATE `creature_template` SET `gold_max` =174 WHERE `entry` =1731 AND `gold_max` =119;

-- Goblin Craftsman
 UPDATE `creature_template` SET `gold_min` =30 WHERE `entry` =1732 AND `gold_min` =80;

-- Defias Squallshaper
 UPDATE `creature_template` SET `gold_max` =156 WHERE `entry` =1732 AND `gold_max` =110;

-- Defias Squallshaper
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =1753 AND `gold_min` =10;

-- Maggot Eye
 UPDATE `creature_template` SET `gold_min` =76 WHERE `entry` =1763 AND `gold_min` =163;

-- Gilnid
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =1767 AND `gold_min` =14;

-- Vile Fin Shredder (sample size 212)
 UPDATE `creature_template` SET `gold_max` =27 WHERE `entry` =1767 AND `gold_max` =23;

-- Vile Fin Shredder (sample size 212)
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =1768 AND `gold_min` =16;

-- Vile Fin Tidehunter (sample size 99)
 UPDATE `creature_template` SET `gold_max` =29 WHERE `entry` =1768 AND `gold_max` =26;

-- Vile Fin Tidehunter (sample size 99)
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =1769 AND `gold_min` =10;

-- Moonrage Whitescalp (sample size 47)
 UPDATE `creature_template` SET `gold_max` =17 WHERE `entry` =1769 AND `gold_max` =18;

-- Moonrage Whitescalp (sample size 47)
 UPDATE `creature_template` SET `gold_min` =5 WHERE `entry` =1770 AND `gold_min` =12;

-- Moonrage Darkrunner (sample size 21)
 UPDATE `creature_template` SET `gold_max` =24 WHERE `entry` =1770 AND `gold_max` =20;

-- Moonrage Darkrunner (sample size 21)
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =1772 AND `gold_min` =12;

-- Rot Hide Gladerunner
 UPDATE `creature_template` SET `gold_max` =24 WHERE `entry` =1772 AND `gold_max` =20;

-- Rot Hide Gladerunner
 UPDATE `creature_template` SET `gold_max` =27 WHERE `entry` =1773 AND `gold_max` =22;

-- Rot Hide Mystic
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =1779 AND `gold_min` =13;

-- Moonrage Glutton (sample size 63)
 UPDATE `creature_template` SET `gold_max` =27 WHERE `entry` =1779 AND `gold_max` =22;

-- Moonrage Glutton (sample size 63)
 UPDATE `creature_template` SET `gold_min` =5 WHERE `entry` =1782 AND `gold_min` =15;

-- Moonrage Darksoul (sample size 34)
 UPDATE `creature_template` SET `gold_max` =28 WHERE `entry` =1782 AND `gold_max` =24;

-- Moonrage Darksoul (sample size 34)
 UPDATE `creature_template` SET `gold_min` =93 WHERE `entry` =1783 AND `gold_min` =101;

-- Skeletal Flayer (sample size 203)
 UPDATE `creature_template` SET `gold_max` =638 WHERE `entry` =1783 AND `gold_max` =137;

-- Skeletal Flayer (sample size 203)
 UPDATE `creature_template` SET `gold_min` =120 WHERE `entry` =1784 AND `gold_min` =93;

-- Skeletal Sorcerer (sample size 28)
 UPDATE `creature_template` SET `gold_max` =618 WHERE `entry` =1784 AND `gold_max` =127;

-- Skeletal Sorcerer (sample size 28)
 UPDATE `creature_template` SET `gold_max` =716 WHERE `entry` =1785 AND `gold_max` =181;

-- Skeletal Terror
 UPDATE `creature_template` SET `gold_max` =789 WHERE `entry` =1787 AND `gold_max` =158;

-- Skeletal Executioner (sample size 43)
 UPDATE `creature_template` SET `gold_max` =1236 WHERE `entry` =1788 AND `gold_max` =586;

-- Skeletal Warlord
 UPDATE `creature_template` SET `gold_min` =139 WHERE `entry` =1789 AND `gold_min` =108;

-- Skeletal Acolyte (sample size 51)
 UPDATE `creature_template` SET `gold_max` =813 WHERE `entry` =1789 AND `gold_max` =145;

-- Skeletal Acolyte (sample size 51)
 UPDATE `creature_template` SET `gold_max` =643 WHERE `entry` =1791 AND `gold_max` =140;

-- Slavering Ghoul (sample size 122)
 UPDATE `creature_template` SET `gold_max` =722 WHERE `entry` =1793 AND `gold_max` =158;

-- Rotting Ghoul
 UPDATE `creature_template` SET `gold_min` =127 WHERE `entry` =1794 AND `gold_min` =102;

-- Soulless Ghoul (sample size 45)
 UPDATE `creature_template` SET `gold_max` =791 WHERE `entry` =1794 AND `gold_max` =139;

-- Soulless Ghoul (sample size 45)
 UPDATE `creature_template` SET `gold_min` =121 WHERE `entry` =1795 AND `gold_min` =108;

-- Searing Ghoul (sample size 33)
 UPDATE `creature_template` SET `gold_max` =825 WHERE `entry` =1795 AND `gold_max` =145;

-- Searing Ghoul (sample size 33)
 UPDATE `creature_template` SET `gold_max` =807 WHERE `entry` =1796 AND `gold_max` =165;

-- Freezing Ghoul
 UPDATE `creature_template` SET `gold_max` =632 WHERE `entry` =1802 AND `gold_max` =171;

-- Hungering Wraith
 UPDATE `creature_template` SET `gold_max` =492 WHERE `entry` =1804 AND `gold_max` =169;

-- Wailing Death
 UPDATE `creature_template` SET `gold_min` =102 WHERE `entry` =1806 AND `gold_min` =117;

-- Vile Slime
 UPDATE `creature_template` SET `gold_max` =395 WHERE `entry` =1806 AND `gold_max` =158;

-- Vile Slime
 UPDATE `creature_template` SET `gold_max` =470 WHERE `entry` =1808 AND `gold_max` =165;

-- Devouring Ooze
 UPDATE `creature_template` SET `gold_max` =807 WHERE `entry` =1812 AND `gold_max` =168;

-- Rotting Behemoth
 UPDATE `creature_template` SET `gold_max` =866 WHERE `entry` =1813 AND `gold_max` =169;

-- Decaying Horror
 UPDATE `creature_template` SET `gold_max` =426 WHERE `entry` =1826 AND `gold_max` =134;

-- Scarlet Mage
 UPDATE `creature_template` SET `gold_min` =400 WHERE `entry` =1827 AND `gold_min` =435;

-- Scarlet Sentinel
 UPDATE `creature_template` SET `gold_max` =2407 WHERE `entry` =1827 AND `gold_max` =573;

-- Scarlet Sentinel
 UPDATE `creature_template` SET `gold_max` =700 WHERE `entry` =1831 AND `gold_max` =148;

-- Scarlet Hunter
 UPDATE `creature_template` SET `gold_max` =685 WHERE `entry` =1833 AND `gold_max` =158;

-- Scarlet Knight
 UPDATE `creature_template` SET `gold_max` =2067 WHERE `entry` =1834 AND `gold_max` =484;

-- Scarlet Paladin
 UPDATE `creature_template` SET `gold_max` =745 WHERE `entry` =1835 AND `gold_max` =135;

-- Scarlet Invoker
 UPDATE `creature_template` SET `gold_max` =1198 WHERE `entry` =1836 AND `gold_max` =621;

-- Scarlet Cavalier
 UPDATE `creature_template` SET `gold_max` =558 WHERE `entry` =1845 AND `gold_max` =155;

-- High Protector Tarsen
 UPDATE `creature_template` SET `gold_max` =2874 WHERE `entry` =1846 AND `gold_max` =558;

-- High Protector Lorik
 UPDATE `creature_template` SET `gold_max` =302 WHERE `entry` =1848 AND `gold_max` =146;

-- Lord Maldazzar
 UPDATE `creature_template` SET `gold_max` =1364 WHERE `entry` =1852 AND `gold_max` =603;

-- Araj the Summoner
 UPDATE `creature_template` SET `gold_max` =2625 WHERE `entry` =1853 AND `gold_max` =2517;

-- Darkmaster Gandling
 UPDATE `creature_template` SET `gold_min` =7 WHERE `entry` =1865 AND `gold_min` =14;

-- Ravenclaw Raider
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =1866 AND `gold_min` =12;

-- Ravenclaw Slave
 UPDATE `creature_template` SET `gold_max` =24 WHERE `entry` =1866 AND `gold_max` =20;

-- Ravenclaw Slave
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =1867 AND `gold_min` =14;

-- Dalaran Apprentice (sample size 26)
 UPDATE `creature_template` SET `gold_max` =28 WHERE `entry` =1867 AND `gold_max` =24;

-- Dalaran Apprentice (sample size 26)
 UPDATE `creature_template` SET `gold_min` =8 WHERE `entry` =1868 AND `gold_min` =14;

-- Ravenclaw Servant
 UPDATE `creature_template` SET `gold_max` =25 WHERE `entry` =1868 AND `gold_max` =23;

-- Ravenclaw Servant
 UPDATE `creature_template` SET `gold_min` =5 WHERE `entry` =1869 AND `gold_min` =16;

-- Ravenclaw Champion
 UPDATE `creature_template` SET `gold_max` =28 WHERE `entry` =1869 AND `gold_max` =26;

-- Ravenclaw Champion
 UPDATE `creature_template` SET `gold_max` =38 WHERE `entry` =1870 AND `gold_max` =28;

-- Hand of Ravenclaw
 UPDATE `creature_template` SET `gold_max` =664 WHERE `entry` =1883 AND `gold_max` =168;

-- Scarlet Worker
 UPDATE `creature_template` SET `gold_max` =769 WHERE `entry` =1884 AND `gold_max` =164;

-- Scarlet Lumberjack
 UPDATE `creature_template` SET `gold_min` =8 WHERE `entry` =1888 AND `gold_min` =21;

-- Dalaran Watcher
 UPDATE `creature_template` SET `gold_max` =55 WHERE `entry` =1888 AND `gold_max` =32;

-- Dalaran Watcher
 UPDATE `creature_template` SET `gold_max` =48 WHERE `entry` =1889 AND `gold_max` =33;

-- Dalaran Wizard
 UPDATE `creature_template` SET `gold_min` =14 WHERE `entry` =1891 AND `gold_min` =59;

-- Pyrewood Watcher
 UPDATE `creature_template` SET `gold_max` =87 WHERE `entry` =1891 AND `gold_max` =81;

-- Pyrewood Watcher
 UPDATE `creature_template` SET `gold_max` =86 WHERE `entry` =1892 AND `gold_max` =50;

-- Moonrage Watcher
 UPDATE `creature_template` SET `gold_max` =95 WHERE `entry` =1893 AND `gold_max` =47;

-- Moonrage Sentry
 UPDATE `creature_template` SET `gold_min` =27 WHERE `entry` =1894 AND `gold_min` =63;

-- Pyrewood Sentry
 UPDATE `creature_template` SET `gold_max` =102 WHERE `entry` =1894 AND `gold_max` =88;

-- Pyrewood Sentry
 UPDATE `creature_template` SET `gold_min` =24 WHERE `entry` =1895 AND `gold_min` =59;

-- Pyrewood Elder
 UPDATE `creature_template` SET `gold_max` =86 WHERE `entry` =1895 AND `gold_max` =81;

-- Pyrewood Elder
 UPDATE `creature_template` SET `gold_max` =101 WHERE `entry` =1896 AND `gold_max` =47;

-- Moonrage Elder
 UPDATE `creature_template` SET `gold_min` =74 WHERE `entry` =1907 AND `gold_min` =78;

-- Naga Explorer (sample size 86)
 UPDATE `creature_template` SET `gold_max` =425 WHERE `entry` =1907 AND `gold_max` =107;

-- Naga Explorer (sample size 86)
 UPDATE `creature_template` SET `gold_max` =62 WHERE `entry` =1908 AND `gold_max` =43;

-- Vile Fin Oracle
 UPDATE `creature_template` SET `gold_min` =19 WHERE `entry` =1909 AND `gold_min` =23;

-- Vile Fin Lakestalker
 UPDATE `creature_template` SET `gold_min` =5 WHERE `entry` =1912 AND `gold_min` =16;

-- Dalaran Protector (sample size 41)
 UPDATE `creature_template` SET `gold_max` =34 WHERE `entry` =1912 AND `gold_max` =26;

-- Dalaran Protector (sample size 41)
 UPDATE `creature_template` SET `gold_min` =8 WHERE `entry` =1913 AND `gold_min` =19;

-- Dalaran Warder
 UPDATE `creature_template` SET `gold_max` =49 WHERE `entry` =1913 AND `gold_max` =30;

-- Dalaran Warder
 UPDATE `creature_template` SET `gold_min` =12 WHERE `entry` =1914 AND `gold_min` =16;

-- Dalaran Mage
 UPDATE `creature_template` SET `gold_max` =38 WHERE `entry` =1914 AND `gold_max` =26;

-- Dalaran Mage
 UPDATE `creature_template` SET `gold_min` =12 WHERE `entry` =1915 AND `gold_min` =19;

-- Dalaran Conjuror
 UPDATE `creature_template` SET `gold_max` =50 WHERE `entry` =1915 AND `gold_max` =30;

-- Dalaran Conjuror
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =1919 AND `gold_min` =3;

-- Samuel Fipps
 UPDATE `creature_template` SET `gold_min` =13 WHERE `entry` =1924 AND `gold_min` =18;

-- Moonrage Bloodhowler
 UPDATE `creature_template` SET `gold_max` =37 WHERE `entry` =1924 AND `gold_max` =29;

-- Moonrage Bloodhowler
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =1934 AND `gold_min` =5;

-- Tirisfal Farmer
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =1935 AND `gold_min` =4;

-- Tirisfal Farmhand
 UPDATE `creature_template` SET `gold_min` =7 WHERE `entry` =1939 AND `gold_min` =20;

-- Rot Hide Brute (sample size 127)
 UPDATE `creature_template` SET `gold_max` =49 WHERE `entry` =1939 AND `gold_max` =31;

-- Rot Hide Brute (sample size 127)
 UPDATE `creature_template` SET `gold_min` =7 WHERE `entry` =1940 AND `gold_min` =20;

-- Rot Hide Plague Weaver (sample size 72)
 UPDATE `creature_template` SET `gold_max` =54 WHERE `entry` =1940 AND `gold_max` =30;

-- Rot Hide Plague Weaver (sample size 72)
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =1941 AND `gold_min` =5;

-- Rot Hide Graverobber (sample size 31)
 UPDATE `creature_template` SET `gold_min` =8 WHERE `entry` =1942 AND `gold_min` =22;

-- Rot Hide Savage (sample size 40)
 UPDATE `creature_template` SET `gold_max` =59 WHERE `entry` =1942 AND `gold_max` =33;

-- Rot Hide Savage (sample size 40)
 UPDATE `creature_template` SET `gold_min` =10 WHERE `entry` =1943 AND `gold_min` =22;

-- Raging Rot Hide (sample size 29)
 UPDATE `creature_template` SET `gold_max` =58 WHERE `entry` =1943 AND `gold_max` =34;

-- Raging Rot Hide (sample size 29)
 UPDATE `creature_template` SET `gold_max` =79 WHERE `entry` =1944 AND `gold_max` =42;

-- Rot Hide Bruiser
 UPDATE `creature_template` SET `gold_max` =175 WHERE `entry` =1947 AND `gold_max` =143;

-- Thule Ravenclaw
 UPDATE `creature_template` SET `gold_max` =83 WHERE `entry` =1948 AND `gold_max` =47;

-- Snarlmane
 UPDATE `creature_template` SET `gold_min` =5 WHERE `entry` =1949 AND `gold_min` =12;

-- Servant of Azora
 UPDATE `creature_template` SET `gold_min` =8 WHERE `entry` =1953 AND `gold_min` =18;

-- Lake Skulker (sample size 25)
 UPDATE `creature_template` SET `gold_max` =38 WHERE `entry` =1953 AND `gold_max` =28;

-- Lake Skulker (sample size 25)
 UPDATE `creature_template` SET `gold_min` =8 WHERE `entry` =1954 AND `gold_min` =18;

-- Elder Lake Skulker
 UPDATE `creature_template` SET `gold_max` =48 WHERE `entry` =1954 AND `gold_max` =28;

-- Elder Lake Skulker
 UPDATE `creature_template` SET `gold_min` =10 WHERE `entry` =1955 AND `gold_min` =20;

-- Lake Creeper
 UPDATE `creature_template` SET `gold_max` =53 WHERE `entry` =1955 AND `gold_max` =32;

-- Lake Creeper
 UPDATE `creature_template` SET `gold_min` =13 WHERE `entry` =1956 AND `gold_min` =22;

-- Elder Lake Creeper
 UPDATE `creature_template` SET `gold_max` =58 WHERE `entry` =1956 AND `gold_max` =34;

-- Elder Lake Creeper
 UPDATE `creature_template` SET `gold_max` =54 WHERE `entry` =1958 AND `gold_max` =33;

-- Vile Fin Tidecaller
 UPDATE `creature_template` SET `gold_min` =11 WHERE `entry` =1971 AND `gold_min` =14;

-- Ivar the Foul
 UPDATE `creature_template` SET `gold_max` =26 WHERE `entry` =1971 AND `gold_max` =23;

-- Ivar the Foul
 UPDATE `creature_template` SET `gold_min` =14 WHERE `entry` =1972 AND `gold_min` =17;

-- Grimson the Pale
 UPDATE `creature_template` SET `gold_min` =10 WHERE `entry` =1973 AND `gold_min` =26;

-- Ravenclaw Guardian (sample size 22)
 UPDATE `creature_template` SET `gold_max` =70 WHERE `entry` =1973 AND `gold_max` =38;

-- Ravenclaw Guardian (sample size 22)
 UPDATE `creature_template` SET `gold_min` =10 WHERE `entry` =1974 AND `gold_min` =24;

-- Ravenclaw Drudger
 UPDATE `creature_template` SET `gold_max` =62 WHERE `entry` =1974 AND `gold_max` =36;

-- Ravenclaw Drudger
 UPDATE `creature_template` SET `gold_min` =20 WHERE `entry` =1983 AND `gold_min` =23;

-- Nightlash
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =2002 AND `gold_min` =4;

-- Rascal Sprite
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =2003 AND `gold_min` =4;

-- Shadow Sprite (sample size 21)
 UPDATE `creature_template` SET `gold_max` =7 WHERE `entry` =2003 AND `gold_max` =10;

-- Shadow Sprite (sample size 21)
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =2004 AND `gold_min` =5;

-- Dark Sprite
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =2005 AND `gold_min` =6;

-- Vicious Grell
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =2006 AND `gold_min` =4;

-- Gnarlpine Ursa
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =2007 AND `gold_min` =4;

-- Gnarlpine Gardener
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =2008 AND `gold_min` =5;

-- Gnarlpine Warrior (sample size 30)
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =2009 AND `gold_min` =6;

-- Gnarlpine Shaman (sample size 105)
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =2010 AND `gold_min` =7;

-- Gnarlpine Defender (sample size 66)
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =2011 AND `gold_min` =8;

-- Gnarlpine Augur
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =2012 AND `gold_min` =9;

-- Gnarlpine Pathfinder
 UPDATE `creature_template` SET `gold_min` =6 WHERE `entry` =2013 AND `gold_min` =9;

-- Gnarlpine Avenger
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =2014 AND `gold_min` =10;

-- Gnarlpine Totemic
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =2015 AND `gold_min` =8;

-- Bloodfeather Harpy
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =2017 AND `gold_min` =8;

-- Bloodfeather Rogue
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =2018 AND `gold_min` =9;

-- Bloodfeather Sorceress
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =2019 AND `gold_min` =9;

-- Bloodfeather Fury
 UPDATE `creature_template` SET `gold_min` =6 WHERE `entry` =2020 AND `gold_min` =9;

-- Bloodfeather Wind Witch
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =2021 AND `gold_min` =12;

-- Bloodfeather Matriarch
 UPDATE `creature_template` SET `gold_max` =24 WHERE `entry` =2021 AND `gold_max` =20;

-- Bloodfeather Matriarch
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =2022 AND `gold_min` =4;

-- Timberling (sample size 68)
 UPDATE `creature_template` SET `gold_max` =7 WHERE `entry` =2022 AND `gold_max` =10;

-- Timberling (sample size 68)
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =2025 AND `gold_min` =6;

-- Timberling Bark Ripper
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =2027 AND `gold_min` =8;

-- Timberling Trampler
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =2029 AND `gold_min` =9;

-- Timberling Mire Beast
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =2030 AND `gold_min` =10;

-- Elder Timberling
 UPDATE `creature_template` SET `gold_min` =6 WHERE `entry` =2038 AND `gold_min` =7;

-- Lord Melenas
 UPDATE `creature_template` SET `gold_min` =9 WHERE `entry` =2039 AND `gold_min` =13;

-- Ursal the Mauler
 UPDATE `creature_template` SET `gold_min` =12 WHERE `entry` =2044 AND `gold_min` =0;

-- Forlorn Spirit (sample size 6)
 UPDATE `creature_template` SET `gold_max` =64 WHERE `entry` =2044 AND `gold_max` =0;

-- Forlorn Spirit (sample size 6)
 UPDATE `creature_template` SET `gold_max` =48 WHERE `entry` =2053 AND `gold_max` =34;

-- Haggard Refugee
 UPDATE `creature_template` SET `gold_max` =51 WHERE `entry` =2054 AND `gold_max` =34;

-- Sickly Refugee
 UPDATE `creature_template` SET `gold_min` =9 WHERE `entry` =2066 AND `gold_min` =10;

-- Councilman Higarth
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =2067 AND `gold_min` =10;

-- Councilman Brunswick
 UPDATE `creature_template` SET `gold_max` =69 WHERE `entry` =2068 AND `gold_max` =55;

-- Lord Mayor Morrison
 UPDATE `creature_template` SET `gold_max` =63 WHERE `entry` =2090 AND `gold_max` =44;

-- Ma'ruk Wyrmscale
 UPDATE `creature_template` SET `gold_min` =133 WHERE `entry` =2091 AND `gold_min` =187;

-- Chieftain Nek'rosh
 UPDATE `creature_template` SET `gold_min` =10 WHERE `entry` =2102 AND `gold_min` =26;

-- Dragonmaw Grunt
 UPDATE `creature_template` SET `gold_max` =67 WHERE `entry` =2102 AND `gold_max` =38;

-- Dragonmaw Grunt
 UPDATE `creature_template` SET `gold_min` =9 WHERE `entry` =2103 AND `gold_min` =23;

-- Dragonmaw Scout (sample size 34)
 UPDATE `creature_template` SET `gold_max` =65 WHERE `entry` =2103 AND `gold_max` =35;

-- Dragonmaw Scout (sample size 34)
 UPDATE `creature_template` SET `gold_max` =145 WHERE `entry` =2106 AND `gold_max` =103;

-- Apothecary Berard
 UPDATE `creature_template` SET `gold_max` =87 WHERE `entry` =2120 AND `gold_max` =39;

-- Archmage Ataeric
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =2152 AND `gold_min` =6;

-- Gnarlpine Ambusher
 UPDATE `creature_template` SET `gold_min` =19 WHERE `entry` =2156 AND `gold_min` =21;

-- Cracked Golem
 UPDATE `creature_template` SET `gold_max` =58 WHERE `entry` =2156 AND `gold_max` =32;

-- Cracked Golem
 UPDATE `creature_template` SET `gold_min` =9 WHERE `entry` =2157 AND `gold_min` =22;

-- Stone Behemoth
 UPDATE `creature_template` SET `gold_max` =57 WHERE `entry` =2157 AND `gold_max` =33;

-- Stone Behemoth
 UPDATE `creature_template` SET `gold_max` =52 WHERE `entry` =2158 AND `gold_max` =27;

-- Gravelflint Scout
 UPDATE `creature_template` SET `gold_max` =62 WHERE `entry` =2160 AND `gold_max` =34;

-- Gravelflint Geomancer
 UPDATE `creature_template` SET `gold_min` =31 WHERE `entry` =2166 AND `gold_min` =35;

-- Oakenscowl
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =2167 AND `gold_min` =13;

-- Blackwood Pathfinder (sample size 23)
 UPDATE `creature_template` SET `gold_max` =26 WHERE `entry` =2167 AND `gold_max` =21;

-- Blackwood Pathfinder (sample size 23)
 UPDATE `creature_template` SET `gold_min` =9 WHERE `entry` =2168 AND `gold_min` =19;

-- Blackwood Warrior (sample size 22)
 UPDATE `creature_template` SET `gold_max` =49 WHERE `entry` =2168 AND `gold_max` =29;

-- Blackwood Warrior (sample size 22)
 UPDATE `creature_template` SET `gold_min` =9 WHERE `entry` =2169 AND `gold_min` =20;

-- Blackwood Totemic
 UPDATE `creature_template` SET `gold_max` =51 WHERE `entry` =2169 AND `gold_max` =31;

-- Blackwood Totemic
 UPDATE `creature_template` SET `gold_min` =8 WHERE `entry` =2170 AND `gold_min` =21;

-- Blackwood Ursa (sample size 28)
 UPDATE `creature_template` SET `gold_max` =59 WHERE `entry` =2170 AND `gold_max` =32;

-- Blackwood Ursa (sample size 28)
 UPDATE `creature_template` SET `gold_min` =9 WHERE `entry` =2171 AND `gold_min` =23;

-- Blackwood Shaman
 UPDATE `creature_template` SET `gold_max` =59 WHERE `entry` =2171 AND `gold_max` =34;

-- Blackwood Shaman
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =2176 AND `gold_min` =10;

-- Cursed Highborne
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =2177 AND `gold_min` =12;

-- Writhing Highborne (sample size 27)
 UPDATE `creature_template` SET `gold_max` =24 WHERE `entry` =2177 AND `gold_max` =20;

-- Writhing Highborne (sample size 27)
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =2178 AND `gold_min` =13;

-- Wailing Highborne
 UPDATE `creature_template` SET `gold_max` =27 WHERE `entry` =2178 AND `gold_max` =22;

-- Wailing Highborne
 UPDATE `creature_template` SET `gold_min` =5 WHERE `entry` =2179 AND `gold_min` =17;

-- Stormscale Wave Rider
 UPDATE `creature_template` SET `gold_max` =35 WHERE `entry` =2179 AND `gold_max` =27;

-- Stormscale Wave Rider
 UPDATE `creature_template` SET `gold_min` =9 WHERE `entry` =2180 AND `gold_min` =18;

-- Stormscale Siren
 UPDATE `creature_template` SET `gold_max` =49 WHERE `entry` =2180 AND `gold_max` =29;

-- Stormscale Siren
 UPDATE `creature_template` SET `gold_min` =8 WHERE `entry` =2181 AND `gold_min` =22;

-- Stormscale Myrmidon (sample size 23)
 UPDATE `creature_template` SET `gold_max` =53 WHERE `entry` =2181 AND `gold_max` =33;

-- Stormscale Myrmidon (sample size 23)
 UPDATE `creature_template` SET `gold_min` =9 WHERE `entry` =2182 AND `gold_min` =22;

-- Stormscale Sorceress
 UPDATE `creature_template` SET `gold_max` =59 WHERE `entry` =2182 AND `gold_max` =33;

-- Stormscale Sorceress
 UPDATE `creature_template` SET `gold_min` =12 WHERE `entry` =2183 AND `gold_min` =25;

-- Stormscale Warrior
 UPDATE `creature_template` SET `gold_max` =69 WHERE `entry` =2183 AND `gold_max` =38;

-- Stormscale Warrior
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =2189 AND `gold_min` =10;

-- Vile Sprite (sample size 31)
 UPDATE `creature_template` SET `gold_max` =17 WHERE `entry` =2189 AND `gold_max` =18;

-- Vile Sprite (sample size 31)
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =2190 AND `gold_min` =12;

-- Wild Grell
 UPDATE `creature_template` SET `gold_max` =24 WHERE `entry` =2190 AND `gold_max` =20;

-- Wild Grell
 UPDATE `creature_template` SET `gold_min` =5 WHERE `entry` =2201 AND `gold_min` =12;

-- Greymist Raider
 UPDATE `creature_template` SET `gold_max` =22 WHERE `entry` =2201 AND `gold_max` =20;

-- Greymist Raider
 UPDATE `creature_template` SET `gold_min` =6 WHERE `entry` =2202 AND `gold_min` =13;

-- Greymist Coastrunner
 UPDATE `creature_template` SET `gold_max` =25 WHERE `entry` =2202 AND `gold_max` =22;

-- Greymist Coastrunner
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =2203 AND `gold_min` =14;

-- Greymist Seer
 UPDATE `creature_template` SET `gold_max` =28 WHERE `entry` =2203 AND `gold_max` =24;

-- Greymist Seer
 UPDATE `creature_template` SET `gold_min` =7 WHERE `entry` =2204 AND `gold_min` =16;

-- Greymist Netter
 UPDATE `creature_template` SET `gold_max` =31 WHERE `entry` =2204 AND `gold_max` =26;

-- Greymist Netter
 UPDATE `creature_template` SET `gold_min` =5 WHERE `entry` =2205 AND `gold_min` =17;

-- Greymist Warrior (sample size 24)
 UPDATE `creature_template` SET `gold_max` =37 WHERE `entry` =2205 AND `gold_max` =28;

-- Greymist Warrior (sample size 24)
 UPDATE `creature_template` SET `gold_min` =11 WHERE `entry` =2206 AND `gold_min` =19;

-- Greymist Hunter
 UPDATE `creature_template` SET `gold_max` =48 WHERE `entry` =2206 AND `gold_max` =30;

-- Greymist Hunter
 UPDATE `creature_template` SET `gold_max` =45 WHERE `entry` =2207 AND `gold_max` =32;

-- Greymist Oracle
 UPDATE `creature_template` SET `gold_min` =14 WHERE `entry` =2208 AND `gold_min` =23;

-- Greymist Tidehunter
 UPDATE `creature_template` SET `gold_max` =62 WHERE `entry` =2208 AND `gold_max` =34;

-- Greymist Tidehunter
 UPDATE `creature_template` SET `gold_min` =6 WHERE `entry` =2212 AND `gold_min` =13;

-- Deth'ryll Satyr
 UPDATE `creature_template` SET `gold_max` =26 WHERE `entry` =2212 AND `gold_max` =22;

-- Deth'ryll Satyr
 UPDATE `creature_template` SET `gold_min` =28 WHERE `entry` =2240 AND `gold_min` =50;

-- Syndicate Footpad (sample size 34)
 UPDATE `creature_template` SET `gold_max` =190 WHERE `entry` =2240 AND `gold_max` =70;

-- Syndicate Footpad (sample size 34)
 UPDATE `creature_template` SET `gold_min` =47 WHERE `entry` =2241 AND `gold_min` =53;

-- Syndicate Thief (sample size 30)
 UPDATE `creature_template` SET `gold_max` =222 WHERE `entry` =2241 AND `gold_max` =74;

-- Syndicate Thief (sample size 30)
 UPDATE `creature_template` SET `gold_min` =39 WHERE `entry` =2242 AND `gold_min` =61;

-- Syndicate Spy (sample size 26)
 UPDATE `creature_template` SET `gold_max` =263 WHERE `entry` =2242 AND `gold_max` =84;

-- Syndicate Spy (sample size 26)
 UPDATE `creature_template` SET `gold_max` =225 WHERE `entry` =2243 AND `gold_max` =83;

-- Syndicate Sentry
 UPDATE `creature_template` SET `gold_max` =84 WHERE `entry` =2244 AND `gold_max` =37;

-- Syndicate Shadow Mage
 UPDATE `creature_template` SET `gold_max` =289 WHERE `entry` =2245 AND `gold_max` =86;

-- Syndicate Saboteur
 UPDATE `creature_template` SET `gold_max` =317 WHERE `entry` =2246 AND `gold_max` =93;

-- Syndicate Assassin
 UPDATE `creature_template` SET `gold_max` =308 WHERE `entry` =2247 AND `gold_max` =94;

-- Syndicate Enforcer
 UPDATE `creature_template` SET `gold_min` =27 WHERE `entry` =2248 AND `gold_min` =46;

-- Cave Yeti (sample size 64)
 UPDATE `creature_template` SET `gold_max` =182 WHERE `entry` =2248 AND `gold_max` =65;

-- Cave Yeti (sample size 64)
 UPDATE `creature_template` SET `gold_min` =32 WHERE `entry` =2249 AND `gold_min` =49;

-- Ferocious Yeti (sample size 33)
 UPDATE `creature_template` SET `gold_max` =197 WHERE `entry` =2249 AND `gold_max` =69;

-- Ferocious Yeti (sample size 33)
 UPDATE `creature_template` SET `gold_min` =28 WHERE `entry` =2250 AND `gold_min` =50;

-- Mountain Yeti (sample size 29)
 UPDATE `creature_template` SET `gold_max` =196 WHERE `entry` =2250 AND `gold_max` =70;

-- Mountain Yeti (sample size 29)
 UPDATE `creature_template` SET `gold_min` =55 WHERE `entry` =2251 AND `gold_min` =53;

-- Giant Yeti (sample size 56)
 UPDATE `creature_template` SET `gold_max` =227 WHERE `entry` =2251 AND `gold_max` =74;

-- Giant Yeti (sample size 56)
 UPDATE `creature_template` SET `gold_min` =39 WHERE `entry` =2252 AND `gold_min` =54;

-- Crushridge Ogre (sample size 27)
 UPDATE `creature_template` SET `gold_max` =244 WHERE `entry` =2252 AND `gold_max` =75;

-- Crushridge Ogre (sample size 27)
 UPDATE `creature_template` SET `gold_min` =52 WHERE `entry` =2253 AND `gold_min` =59;

-- Crushridge Brute
 UPDATE `creature_template` SET `gold_max` =257 WHERE `entry` =2253 AND `gold_max` =81;

-- Crushridge Brute
 UPDATE `creature_template` SET `gold_min` =126 WHERE `entry` =2254 AND `gold_min` =228;

-- Crushridge Mauler (sample size 30)
 UPDATE `creature_template` SET `gold_max` =847 WHERE `entry` =2254 AND `gold_max` =303;

-- Crushridge Mauler (sample size 30)
 UPDATE `creature_template` SET `gold_max` =894 WHERE `entry` =2255 AND `gold_max` =255;

-- Crushridge Mage
 UPDATE `creature_template` SET `gold_min` =159 WHERE `entry` =2256 AND `gold_min` =249;

-- Crushridge Enforcer (sample size 29)
 UPDATE `creature_template` SET `gold_max` =913 WHERE `entry` =2256 AND `gold_max` =330;

-- Crushridge Enforcer (sample size 29)
 UPDATE `creature_template` SET `gold_min` =281 WHERE `entry` =2257 AND `gold_min` =291;

-- Mug'thol
 UPDATE `creature_template` SET `gold_max` =487 WHERE `entry` =2257 AND `gold_max` =385;

-- Mug'thol
 UPDATE `creature_template` SET `gold_min` =21 WHERE `entry` =2260 AND `gold_min` =27;

-- Syndicate Rogue
 UPDATE `creature_template` SET `gold_max` =85 WHERE `entry` =2260 AND `gold_max` =40;

-- Syndicate Rogue
 UPDATE `creature_template` SET `gold_min` =10 WHERE `entry` =2261 AND `gold_min` =26;

-- Syndicate Watchman (sample size 45)
 UPDATE `creature_template` SET `gold_max` =71 WHERE `entry` =2261 AND `gold_max` =38;

-- Syndicate Watchman (sample size 45)
 UPDATE `creature_template` SET `gold_max` =95 WHERE `entry` =2265 AND `gold_max` =48;

-- Hillsbrad Apprentice Blacksmith
 UPDATE `creature_template` SET `gold_min` =21 WHERE `entry` =2266 AND `gold_min` =31;

-- Hillsbrad Farmer
 UPDATE `creature_template` SET `gold_max` =102 WHERE `entry` =2266 AND `gold_max` =45;

-- Hillsbrad Farmer
 UPDATE `creature_template` SET `gold_min` =19 WHERE `entry` =2267 AND `gold_min` =32;

-- Hillsbrad Peasant
 UPDATE `creature_template` SET `gold_max` =106 WHERE `entry` =2267 AND `gold_max` =47;

-- Hillsbrad Peasant
 UPDATE `creature_template` SET `gold_min` =21 WHERE `entry` =2268 AND `gold_min` =35;

-- Hillsbrad Footman (sample size 28)
 UPDATE `creature_template` SET `gold_max` =120 WHERE `entry` =2268 AND `gold_max` =51;

-- Hillsbrad Footman (sample size 28)
 UPDATE `creature_template` SET `gold_min` =19 WHERE `entry` =2269 AND `gold_min` =37;

-- Hillsbrad Miner (sample size 32)
 UPDATE `creature_template` SET `gold_max` =131 WHERE `entry` =2269 AND `gold_max` =53;

-- Hillsbrad Miner (sample size 32)
 UPDATE `creature_template` SET `gold_min` =38 WHERE `entry` =2270 AND `gold_min` =39;

-- Hillsbrad Sentry
 UPDATE `creature_template` SET `gold_max` =136 WHERE `entry` =2270 AND `gold_max` =56;

-- Hillsbrad Sentry
 UPDATE `creature_template` SET `gold_min` =29 WHERE `entry` =2271 AND `gold_min` =49;

-- Dalaran Shield Guard (sample size 46)
 UPDATE `creature_template` SET `gold_max` =193 WHERE `entry` =2271 AND `gold_max` =69;

-- Dalaran Shield Guard (sample size 46)
 UPDATE `creature_template` SET `gold_min` =28 WHERE `entry` =2272 AND `gold_min` =44;

-- Dalaran Theurgist
 UPDATE `creature_template` SET `gold_max` =194 WHERE `entry` =2272 AND `gold_max` =62;

-- Dalaran Theurgist
 UPDATE `creature_template` SET `gold_min` =158 WHERE `entry` =2287 AND `gold_min` =259;

-- Crushridge Warmonger (sample size 32)
 UPDATE `creature_template` SET `gold_max` =1007 WHERE `entry` =2287 AND `gold_max` =343;

-- Crushridge Warmonger (sample size 32)
 UPDATE `creature_template` SET `gold_max` =96 WHERE `entry` =2305 AND `gold_max` =58;

-- Foreman Bonds
 UPDATE `creature_template` SET `gold_max` =188 WHERE `entry` =2306 AND `gold_max` =79;

-- Baron Vardus
 UPDATE `creature_template` SET `gold_max` =258 WHERE `entry` =2318 AND `gold_max` =70;

-- Argus Shadow Mage
 UPDATE `creature_template` SET `gold_max` =232 WHERE `entry` =2319 AND `gold_max` =66;

-- Syndicate Wizard
 UPDATE `creature_template` SET `gold_min` =58 WHERE `entry` =2320 AND `gold_min` =67;

-- Nagaz
 UPDATE `creature_template` SET `gold_max` =309 WHERE `entry` =2320 AND `gold_max` =93;

-- Nagaz
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =2324 AND `gold_min` =15;

-- Blackwood Windtalker (sample size 25)
 UPDATE `creature_template` SET `gold_max` =29 WHERE `entry` =2324 AND `gold_max` =24;

-- Blackwood Windtalker (sample size 25)
 UPDATE `creature_template` SET `gold_max` =40 WHERE `entry` =2332 AND `gold_max` =39;

-- Valdred Moray
 UPDATE `creature_template` SET `gold_max` =174 WHERE `entry` =2335 AND `gold_max` =57;

-- Magistrate Burnside
 UPDATE `creature_template` SET `gold_min` =7 WHERE `entry` =2336 AND `gold_min` =19;

-- Dark Strand Fanatic (sample size 31)
 UPDATE `creature_template` SET `gold_max` =47 WHERE `entry` =2336 AND `gold_max` =30;

-- Dark Strand Fanatic (sample size 31)
 UPDATE `creature_template` SET `gold_max` =153 WHERE `entry` =2337 AND `gold_max` =53;

-- Dark Strand Voidcaller
 UPDATE `creature_template` SET `gold_min` =10 WHERE `entry` =2338 AND `gold_min` =18;

-- Twilight Disciple
 UPDATE `creature_template` SET `gold_max` =40 WHERE `entry` =2338 AND `gold_max` =29;

-- Twilight Disciple
 UPDATE `creature_template` SET `gold_min` =19 WHERE `entry` =2339 AND `gold_min` =21;

-- Twilight Thug
 UPDATE `creature_template` SET `gold_max` =52 WHERE `entry` =2339 AND `gold_max` =32;

-- Twilight Thug
 UPDATE `creature_template` SET `gold_min` =126 WHERE `entry` =2344 AND `gold_min` =142;

-- Dun Garok Mountaineer
 UPDATE `creature_template` SET `gold_max` =422 WHERE `entry` =2344 AND `gold_max` =191;

-- Dun Garok Mountaineer
 UPDATE `creature_template` SET `gold_min` =132 WHERE `entry` =2345 AND `gold_min` =170;

-- Dun Garok Rifleman
 UPDATE `creature_template` SET `gold_max` =470 WHERE `entry` =2345 AND `gold_max` =226;

-- Dun Garok Rifleman
 UPDATE `creature_template` SET `gold_min` =77 WHERE `entry` =2346 AND `gold_min` =153;

-- Dun Garok Priest
 UPDATE `creature_template` SET `gold_max` =318 WHERE `entry` =2346 AND `gold_max` =204;

-- Dun Garok Priest
 UPDATE `creature_template` SET `gold_min` =36 WHERE `entry` =2358 AND `gold_min` =47;

-- Dalaran Summoner (sample size 42)
 UPDATE `creature_template` SET `gold_max` =234 WHERE `entry` =2358 AND `gold_max` =67;

-- Dalaran Summoner (sample size 42)
 UPDATE `creature_template` SET `gold_min` =23 WHERE `entry` =2360 AND `gold_min` =29;

-- Hillsbrad Farmhand
 UPDATE `creature_template` SET `gold_max` =95 WHERE `entry` =2360 AND `gold_max` =43;

-- Hillsbrad Farmhand
 UPDATE `creature_template` SET `gold_max` =68 WHERE `entry` =2368 AND `gold_max` =60;

-- Daggerspine Shorestalker
 UPDATE `creature_template` SET `gold_min` =30 WHERE `entry` =2369 AND `gold_min` =46;

-- Daggerspine Shorehunter (sample size 32)
 UPDATE `creature_template` SET `gold_max` =181 WHERE `entry` =2369 AND `gold_max` =65;

-- Daggerspine Shorehunter (sample size 32)
 UPDATE `creature_template` SET `gold_max` =118 WHERE `entry` =2370 AND `gold_max` =58;

-- Daggerspine Screamer
 UPDATE `creature_template` SET `gold_min` =28 WHERE `entry` =2371 AND `gold_min` =45;

-- Daggerspine Siren (sample size 43)
 UPDATE `creature_template` SET `gold_max` =197 WHERE `entry` =2371 AND `gold_max` =63;

-- Daggerspine Siren (sample size 43)
 UPDATE `creature_template` SET `gold_min` =19 WHERE `entry` =2372 AND `gold_min` =37;

-- Mudsnout Gnoll
 UPDATE `creature_template` SET `gold_max` =134 WHERE `entry` =2372 AND `gold_max` =53;

-- Mudsnout Gnoll
 UPDATE `creature_template` SET `gold_min` =28 WHERE `entry` =2373 AND `gold_min` =35;

-- Mudsnout Shaman
 UPDATE `creature_template` SET `gold_max` =145 WHERE `entry` =2373 AND `gold_max` =51;

-- Mudsnout Shaman
 UPDATE `creature_template` SET `gold_max` =149 WHERE `entry` =2374 AND `gold_max` =59;

-- Torn Fin Muckdweller
 UPDATE `creature_template` SET `gold_min` =26 WHERE `entry` =2375 AND `gold_min` =43;

-- Torn Fin Coastrunner (sample size 25)
 UPDATE `creature_template` SET `gold_max` =147 WHERE `entry` =2375 AND `gold_max` =61;

-- Torn Fin Coastrunner (sample size 25)
 UPDATE `creature_template` SET `gold_min` =28 WHERE `entry` =2376 AND `gold_min` =42;

-- Torn Fin Oracle (sample size 43)
 UPDATE `creature_template` SET `gold_max` =177 WHERE `entry` =2376 AND `gold_max` =60;

-- Torn Fin Oracle (sample size 43)
 UPDATE `creature_template` SET `gold_min` =36 WHERE `entry` =2377 AND `gold_min` =44;

-- Torn Fin Tidehunter (sample size 52)
 UPDATE `creature_template` SET `gold_max` =197 WHERE `entry` =2377 AND `gold_max` =63;

-- Torn Fin Tidehunter (sample size 52)
 UPDATE `creature_template` SET `gold_max` =122 WHERE `entry` =2387 AND `gold_max` =47;

-- Hillsbrad Councilman
 UPDATE `creature_template` SET `gold_min` =29 WHERE `entry` =2403 AND `gold_min` =32;

-- Farmer Getz
 UPDATE `creature_template` SET `gold_max` =95 WHERE `entry` =2403 AND `gold_max` =47;

-- Farmer Getz
 UPDATE `creature_template` SET `gold_max` =80 WHERE `entry` =2404 AND `gold_max` =53;

-- Blacksmith Verringtan
 UPDATE `creature_template` SET `gold_max` =231 WHERE `entry` =2411 AND `gold_max` =71;

-- Ricter
 UPDATE `creature_template` SET `gold_max` =190 WHERE `entry` =2412 AND `gold_max` =71;

-- Alina
 UPDATE `creature_template` SET `gold_max` =202 WHERE `entry` =2413 AND `gold_max` =75;

-- Dermot
 UPDATE `creature_template` SET `gold_min` =63 WHERE `entry` =2414 AND `gold_min` =100;

-- Kegan Darkmar
 UPDATE `creature_template` SET `gold_max` =209 WHERE `entry` =2414 AND `gold_max` =150;

-- Kegan Darkmar
 UPDATE `creature_template` SET `gold_min` =42 WHERE `entry` =2415 AND `gold_min` =51;

-- Warden Belamoore
 UPDATE `creature_template` SET `gold_max` =154 WHERE `entry` =2415 AND `gold_max` =71;

-- Warden Belamoore
 UPDATE `creature_template` SET `gold_min` =158 WHERE `entry` =2416 AND `gold_min` =232;

-- Crushridge Plunderer
 UPDATE `creature_template` SET `gold_max` =633 WHERE `entry` =2416 AND `gold_max` =308;

-- Crushridge Plunderer
 UPDATE `creature_template` SET `gold_max` =942 WHERE `entry` =2417 AND `gold_max` =287;

-- Grel'borg the Miser
 UPDATE `creature_template` SET `gold_min` =202 WHERE `entry` =2420 AND `gold_min` =402;

-- Targ
 UPDATE `creature_template` SET `gold_max` =681 WHERE `entry` =2420 AND `gold_max` =530;

-- Targ
 UPDATE `creature_template` SET `gold_max` =764 WHERE `entry` =2421 AND `gold_max` =345;

-- Muckrake
 UPDATE `creature_template` SET `gold_max` =958 WHERE `entry` =2422 AND `gold_max` =332;

-- Glommus
 UPDATE `creature_template` SET `gold_max` =320 WHERE `entry` =2423 AND `gold_max` =91;

-- Lord Aliden Perenolde
 UPDATE `creature_template` SET `gold_max` =56 WHERE `entry` =2427 AND `gold_max` =47;

-- Jailor Eston
 UPDATE `creature_template` SET `gold_max` =102 WHERE `entry` =2428 AND `gold_max` =46;

-- Jailor Marlgen
 UPDATE `creature_template` SET `gold_max` =224 WHERE `entry` =2431 AND `gold_max` =84;

-- Jailor Borhuin
 UPDATE `creature_template` SET `gold_max` =154 WHERE `entry` =2440 AND `gold_max` =70;

-- Drunken Footpad
 UPDATE `creature_template` SET `gold_min` =20 WHERE `entry` =2448 AND `gold_min` =37;

-- Clerk Horrace Whitesteed
 UPDATE `creature_template` SET `gold_max` =80 WHERE `entry` =2448 AND `gold_max` =54;

-- Clerk Horrace Whitesteed
 UPDATE `creature_template` SET `gold_max` =71 WHERE `entry` =2449 AND `gold_max` =50;

-- Citizen Wilkes
 UPDATE `creature_template` SET `gold_max` =114 WHERE `entry` =2450 AND `gold_max` =60;

-- Miner Hackett
 UPDATE `creature_template` SET `gold_max` =98 WHERE `entry` =2451 AND `gold_max` =50;

-- Farmer Kalaba
 UPDATE `creature_template` SET `gold_max` =223 WHERE `entry` =2452 AND `gold_max` =83;

-- Skhowl
 UPDATE `creature_template` SET `gold_min` =27 WHERE `entry` =2503 AND `gold_min` =40;

-- Hillsbrad Foreman
 UPDATE `creature_template` SET `gold_max` =145 WHERE `entry` =2503 AND `gold_max` =57;

-- Hillsbrad Foreman
 UPDATE `creature_template` SET `gold_min` =63 WHERE `entry` =2529 AND `gold_min` =131;

-- Son of Arugal
 UPDATE `creature_template` SET `gold_max` =328 WHERE `entry` =2529 AND `gold_max` =176;

-- Son of Arugal
 UPDATE `creature_template` SET `gold_max` =363 WHERE `entry` =2530 AND `gold_max` =102;

-- Yenniku
 UPDATE `creature_template` SET `gold_max` =343 WHERE `entry` =2534 AND `gold_max` =108;

-- Zanzil the Outcast
 UPDATE `creature_template` SET `gold_max` =392 WHERE `entry` =2535 AND `gold_max` =110;

-- Maury "Club Foot" Wilkins
 UPDATE `creature_template` SET `gold_max` =288 WHERE `entry` =2536 AND `gold_max` =110;

-- Jon-Jon the Crow
 UPDATE `creature_template` SET `gold_max` =427 WHERE `entry` =2537 AND `gold_max` =110;

-- Chucky "Ten Thumbs"
 UPDATE `creature_template` SET `gold_max` =316 WHERE `entry` =2541 AND `gold_max` =101;

-- Lord Sakrasis
 UPDATE `creature_template` SET `gold_max` =237 WHERE `entry` =2545 AND `gold_max` =94;

-- "Pretty Boy" Duncan
 UPDATE `creature_template` SET `gold_min` =88 WHERE `entry` =2546 AND `gold_min` =91;

-- Fleet Master Firallon
 UPDATE `creature_template` SET `gold_max` =408 WHERE `entry` =2546 AND `gold_max` =124;

-- Fleet Master Firallon
 UPDATE `creature_template` SET `gold_max` =323 WHERE `entry` =2547 AND `gold_max` =107;

-- Ironpatch
 UPDATE `creature_template` SET `gold_max` =479 WHERE `entry` =2548 AND `gold_max` =125;

-- Captain Keelhaul
 UPDATE `creature_template` SET `gold_max` =390 WHERE `entry` =2549 AND `gold_max` =104;

-- Garr Salthoof
 UPDATE `creature_template` SET `gold_max` =385 WHERE `entry` =2550 AND `gold_max` =108;

-- Captain Stillwater
 UPDATE `creature_template` SET `gold_min` =72 WHERE `entry` =2551 AND `gold_min` =93;

-- Brutus
 UPDATE `creature_template` SET `gold_max` =253 WHERE `entry` =2551 AND `gold_max` =126;

-- Brutus
 UPDATE `creature_template` SET `gold_min` =36 WHERE `entry` =2552 AND `gold_min` =46;

-- Witherbark Troll
 UPDATE `creature_template` SET `gold_max` =170 WHERE `entry` =2552 AND `gold_max` =65;

-- Witherbark Troll
 UPDATE `creature_template` SET `gold_min` =33 WHERE `entry` =2553 AND `gold_min` =43;

-- Witherbark Shadowcaster
 UPDATE `creature_template` SET `gold_max` =161 WHERE `entry` =2553 AND `gold_max` =61;

-- Witherbark Shadowcaster
 UPDATE `creature_template` SET `gold_min` =30 WHERE `entry` =2554 AND `gold_min` =50;

-- Witherbark Axe Thrower (sample size 56)
 UPDATE `creature_template` SET `gold_max` =193 WHERE `entry` =2554 AND `gold_max` =70;

-- Witherbark Axe Thrower (sample size 56)
 UPDATE `creature_template` SET `gold_min` =35 WHERE `entry` =2555 AND `gold_min` =48;

-- Witherbark Witch Doctor (sample size 66)
 UPDATE `creature_template` SET `gold_max` =222 WHERE `entry` =2555 AND `gold_max` =68;

-- Witherbark Witch Doctor (sample size 66)
 UPDATE `creature_template` SET `gold_min` =37 WHERE `entry` =2556 AND `gold_min` =54;

-- Witherbark Headhunter (sample size 80)
 UPDATE `creature_template` SET `gold_max` =245 WHERE `entry` =2556 AND `gold_max` =75;

-- Witherbark Headhunter (sample size 80)
 UPDATE `creature_template` SET `gold_min` =43 WHERE `entry` =2557 AND `gold_min` =54;

-- Witherbark Shadow Hunter
 UPDATE `creature_template` SET `gold_max` =262 WHERE `entry` =2557 AND `gold_max` =76;

-- Witherbark Shadow Hunter
 UPDATE `creature_template` SET `gold_max` =763 WHERE `entry` =2558 AND `gold_max` =314;

-- Witherbark Berserker
 UPDATE `creature_template` SET `gold_min` =29 WHERE `entry` =2562 AND `gold_min` =50;

-- Boulderfist Ogre (sample size 26)
 UPDATE `creature_template` SET `gold_max` =193 WHERE `entry` =2562 AND `gold_max` =70;

-- Boulderfist Ogre (sample size 26)
 UPDATE `creature_template` SET `gold_min` =51 WHERE `entry` =2564 AND `gold_min` =53;

-- Boulderfist Enforcer (sample size 29)
 UPDATE `creature_template` SET `gold_max` =227 WHERE `entry` =2564 AND `gold_max` =74;

-- Boulderfist Enforcer (sample size 29)
 UPDATE `creature_template` SET `gold_min` =54 WHERE `entry` =2566 AND `gold_min` =59;

-- Boulderfist Brute (sample size 33)
 UPDATE `creature_template` SET `gold_max` =264 WHERE `entry` =2566 AND `gold_max` =81;

-- Boulderfist Brute (sample size 33)
 UPDATE `creature_template` SET `gold_min` =46 WHERE `entry` =2567 AND `gold_min` =51;

-- Boulderfist Magus
 UPDATE `creature_template` SET `gold_max` =281 WHERE `entry` =2567 AND `gold_max` =71;

-- Boulderfist Magus
 UPDATE `creature_template` SET `gold_min` =208 WHERE `entry` =2569 AND `gold_min` =238;

-- Boulderfist Mauler (sample size 24)
 UPDATE `creature_template` SET `gold_max` =860 WHERE `entry` =2569 AND `gold_max` =316;

-- Boulderfist Mauler (sample size 24)
 UPDATE `creature_template` SET `gold_max` =919 WHERE `entry` =2570 AND `gold_max` =285;

-- Boulderfist Shaman
 UPDATE `creature_template` SET `gold_min` =186 WHERE `entry` =2571 AND `gold_min` =259;

-- Boulderfist Lord
 UPDATE `creature_template` SET `gold_max` =904 WHERE `entry` =2571 AND `gold_max` =343;

-- Boulderfist Lord
 UPDATE `creature_template` SET `gold_min` =40 WHERE `entry` =2572 AND `gold_min` =61;

-- Drywhisker Kobold (sample size 79)
 UPDATE `creature_template` SET `gold_max` =264 WHERE `entry` =2572 AND `gold_max` =84;

-- Drywhisker Kobold (sample size 79)
 UPDATE `creature_template` SET `gold_min` =60 WHERE `entry` =2573 AND `gold_min` =52;

-- Drywhisker Surveyor (sample size 25)
 UPDATE `creature_template` SET `gold_max` =295 WHERE `entry` =2573 AND `gold_max` =73;

-- Drywhisker Surveyor (sample size 25)
 UPDATE `creature_template` SET `gold_min` =40 WHERE `entry` =2574 AND `gold_min` =62;

-- Drywhisker Digger (sample size 50)
 UPDATE `creature_template` SET `gold_max` =276 WHERE `entry` =2574 AND `gold_max` =85;

-- Drywhisker Digger (sample size 50)
 UPDATE `creature_template` SET `gold_max` =146 WHERE `entry` =2575 AND `gold_max` =63;

-- Dark Iron Supplier
 UPDATE `creature_template` SET `gold_max` =134 WHERE `entry` =2577 AND `gold_max` =61;

-- Dark Iron Shadowcaster
 UPDATE `creature_template` SET `gold_min` =32 WHERE `entry` =2581 AND `gold_min` =51;

-- Dabyrie Militia
 UPDATE `creature_template` SET `gold_max` =191 WHERE `entry` =2581 AND `gold_max` =71;

-- Dabyrie Militia
 UPDATE `creature_template` SET `gold_min` =40 WHERE `entry` =2582 AND `gold_min` =46;

-- Dabyrie Laborer
 UPDATE `creature_template` SET `gold_max` =170 WHERE `entry` =2582 AND `gold_max` =65;

-- Dabyrie Laborer
 UPDATE `creature_template` SET `gold_max` =429 WHERE `entry` =2583 AND `gold_max` =272;

-- Stromgarde Troll Hunter
 UPDATE `creature_template` SET `gold_max` =855 WHERE `entry` =2584 AND `gold_max` =330;

-- Stromgarde Defender
 UPDATE `creature_template` SET `gold_min` =256 WHERE `entry` =2585 AND `gold_min` =259;

-- Stromgarde Vindicator
 UPDATE `creature_template` SET `gold_max` =569 WHERE `entry` =2585 AND `gold_max` =343;

-- Stromgarde Vindicator
 UPDATE `creature_template` SET `gold_min` =27 WHERE `entry` =2586 AND `gold_min` =46;

-- Syndicate Highwayman (sample size 58)
 UPDATE `creature_template` SET `gold_max` =181 WHERE `entry` =2586 AND `gold_max` =65;

-- Syndicate Highwayman (sample size 58)
 UPDATE `creature_template` SET `gold_min` =35 WHERE `entry` =2587 AND `gold_min` =50;

-- Syndicate Pathstalker (sample size 32)
 UPDATE `creature_template` SET `gold_max` =192 WHERE `entry` =2587 AND `gold_max` =70;

-- Syndicate Pathstalker (sample size 32)
 UPDATE `creature_template` SET `gold_min` =160 WHERE `entry` =2588 AND `gold_min` =228;

-- Syndicate Prowler (sample size 35)
 UPDATE `creature_template` SET `gold_max` =815 WHERE `entry` =2588 AND `gold_max` =303;

-- Syndicate Prowler (sample size 35)
 UPDATE `creature_template` SET `gold_min` =34 WHERE `entry` =2589 AND `gold_min` =49;

-- Syndicate Mercenary (sample size 56)
 UPDATE `creature_template` SET `gold_max` =197 WHERE `entry` =2589 AND `gold_max` =69;

-- Syndicate Mercenary (sample size 56)
 UPDATE `creature_template` SET `gold_max` =774 WHERE `entry` =2590 AND `gold_max` =240;

-- Syndicate Conjuror
 UPDATE `creature_template` SET `gold_max` =847 WHERE `entry` =2591 AND `gold_max` =255;

-- Syndicate Magus
 UPDATE `creature_template` SET `gold_min` =46 WHERE `entry` =2595 AND `gold_min` =65;

-- Daggerspine Raider (sample size 53)
 UPDATE `creature_template` SET `gold_max` =312 WHERE `entry` =2595 AND `gold_max` =90;

-- Daggerspine Raider (sample size 53)
 UPDATE `creature_template` SET `gold_max` =287 WHERE `entry` =2596 AND `gold_max` =78;

-- Daggerspine Sorceress
 UPDATE `creature_template` SET `gold_min` =185 WHERE `entry` =2597 AND `gold_min` =533;

-- Lord Falconcrest
 UPDATE `creature_template` SET `gold_max` =783 WHERE `entry` =2597 AND `gold_max` =700;

-- Lord Falconcrest
 UPDATE `creature_template` SET `gold_max` =808 WHERE `entry` =2598 AND `gold_max` =383;

-- Darbel Montrose
 UPDATE `creature_template` SET `gold_max` =179 WHERE `entry` =2600 AND `gold_max` =69;

-- Singer
 UPDATE `creature_template` SET `gold_max` =780 WHERE `entry` =2602 AND `gold_max` =590;

-- Ruul Onestone
 UPDATE `creature_template` SET `gold_max` =504 WHERE `entry` =2607 AND `gold_max` =343;

-- Prince Galen Trollbane
 UPDATE `creature_template` SET `gold_max` =1122 WHERE `entry` =2611 AND `gold_max` =366;

-- Fozruk
 UPDATE `creature_template` SET `gold_max` =611 WHERE `entry` =2612 AND `gold_max` =285;

-- Lieutenant Valorcall
 UPDATE `creature_template` SET `gold_min` =45 WHERE `entry` =2618 AND `gold_min` =53;

-- Hammerfall Peon
 UPDATE `creature_template` SET `gold_max` =188 WHERE `entry` =2618 AND `gold_max` =74;

-- Hammerfall Peon
 UPDATE `creature_template` SET `gold_min` =39 WHERE `entry` =2619 AND `gold_min` =54;

-- Hammerfall Grunt (sample size 22)
 UPDATE `creature_template` SET `gold_max` =244 WHERE `entry` =2619 AND `gold_max` =75;

-- Hammerfall Grunt (sample size 22)
 UPDATE `creature_template` SET `gold_min` =33 WHERE `entry` =2628 AND `gold_min` =53;

-- Dalaran Worker (sample size 22)
 UPDATE `creature_template` SET `gold_max` =222 WHERE `entry` =2628 AND `gold_max` =74;

-- Dalaran Worker (sample size 22)
 UPDATE `creature_template` SET `gold_min` =71 WHERE `entry` =2639 AND `gold_min` =86;

-- Vilebranch Axe Thrower
 UPDATE `creature_template` SET `gold_max` =456 WHERE `entry` =2639 AND `gold_max` =118;

-- Vilebranch Axe Thrower
 UPDATE `creature_template` SET `gold_max` =476 WHERE `entry` =2640 AND `gold_max` =108;

-- Vilebranch Witch Doctor
 UPDATE `creature_template` SET `gold_max` =1484 WHERE `entry` =2641 AND `gold_max` =436;

-- Vilebranch Headhunter
 UPDATE `creature_template` SET `gold_max` =1382 WHERE `entry` =2642 AND `gold_max` =355;

-- Vilebranch Shadowcaster
 UPDATE `creature_template` SET `gold_min` =245 WHERE `entry` =2643 AND `gold_min` =336;

-- Vilebranch Berserker
 UPDATE `creature_template` SET `gold_max` =1545 WHERE `entry` =2643 AND `gold_max` =444;

-- Vilebranch Berserker
 UPDATE `creature_template` SET `gold_min` =310 WHERE `entry` =2644 AND `gold_min` =351;

-- Vilebranch Hideskinner
 UPDATE `creature_template` SET `gold_max` =1593 WHERE `entry` =2644 AND `gold_max` =463;

-- Vilebranch Hideskinner
 UPDATE `creature_template` SET `gold_max` =1687 WHERE `entry` =2645 AND `gold_max` =391;

-- Vilebranch Shadow Hunter
 UPDATE `creature_template` SET `gold_min` =295 WHERE `entry` =2646 AND `gold_min` =361;

-- Vilebranch Blood Drinker
 UPDATE `creature_template` SET `gold_max` =1728 WHERE `entry` =2646 AND `gold_max` =476;

-- Vilebranch Blood Drinker
 UPDATE `creature_template` SET `gold_min` =337 WHERE `entry` =2647 AND `gold_min` =361;

-- Vilebranch Soul Eater
 UPDATE `creature_template` SET `gold_max` =1710 WHERE `entry` =2647 AND `gold_max` =476;

-- Vilebranch Soul Eater
 UPDATE `creature_template` SET `gold_min` =348 WHERE `entry` =2648 AND `gold_min` =375;

-- Vilebranch Aman'zasi Guard
 UPDATE `creature_template` SET `gold_max` =1888 WHERE `entry` =2648 AND `gold_max` =495;

-- Vilebranch Aman'zasi Guard
 UPDATE `creature_template` SET `gold_min` =66 WHERE `entry` =2649 AND `gold_min` =71;

-- Witherbark Scalper
 UPDATE `creature_template` SET `gold_max` =357 WHERE `entry` =2649 AND `gold_max` =98;

-- Witherbark Scalper
 UPDATE `creature_template` SET `gold_max` =359 WHERE `entry` =2650 AND `gold_max` =102;

-- Witherbark Zealot
 UPDATE `creature_template` SET `gold_min` =79 WHERE `entry` =2651 AND `gold_min` =75;

-- Witherbark Hideskinner (sample size 23)
 UPDATE `creature_template` SET `gold_max` =392 WHERE `entry` =2651 AND `gold_max` =103;

-- Witherbark Hideskinner (sample size 23)
 UPDATE `creature_template` SET `gold_max` =415 WHERE `entry` =2652 AND `gold_max` =104;

-- Witherbark Venomblood
 UPDATE `creature_template` SET `gold_max` =445 WHERE `entry` =2653 AND `gold_max` =111;

-- Witherbark Sadist
 UPDATE `creature_template` SET `gold_min` =68 WHERE `entry` =2655 AND `gold_min` =89;

-- Green Sludge (sample size 36)
 UPDATE `creature_template` SET `gold_max` =294 WHERE `entry` =2655 AND `gold_max` =121;

-- Green Sludge (sample size 36)
 UPDATE `creature_template` SET `gold_min` =52 WHERE `entry` =2656 AND `gold_min` =94;

-- Jade Ooze (sample size 26)
 UPDATE `creature_template` SET `gold_max` =240 WHERE `entry` =2656 AND `gold_max` =128;

-- Jade Ooze (sample size 26)
 UPDATE `creature_template` SET `gold_min` =69 WHERE `entry` =2691 AND `gold_min` =78;

-- Highvale Outrunner (sample size 34)
 UPDATE `creature_template` SET `gold_max` =426 WHERE `entry` =2691 AND `gold_max` =107;

-- Highvale Outrunner (sample size 34)
 UPDATE `creature_template` SET `gold_max` =440 WHERE `entry` =2692 AND `gold_max` =111;

-- Highvale Scout (sample size 34)
 UPDATE `creature_template` SET `gold_min` =84 WHERE `entry` =2693 AND `gold_min` =88;

-- Highvale Marksman (sample size 37)
 UPDATE `creature_template` SET `gold_max` =456 WHERE `entry` =2693 AND `gold_max` =120;

-- Highvale Marksman (sample size 37)
 UPDATE `creature_template` SET `gold_min` =80 WHERE `entry` =2694 AND `gold_min` =79;

-- Highvale Ranger (sample size 32)
 UPDATE `creature_template` SET `gold_max` =501 WHERE `entry` =2694 AND `gold_max` =108;

-- Highvale Ranger (sample size 32)
 UPDATE `creature_template` SET `gold_max` =168 WHERE `entry` =2714 AND `gold_max` =79;

-- Forsaken Courier
 UPDATE `creature_template` SET `gold_max` =147 WHERE `entry` =2716 AND `gold_max` =98;

-- Dustbelcher Wyrmhunter
 UPDATE `creature_template` SET `gold_max` =165 WHERE `entry` =2718 AND `gold_max` =92;

-- Dustbelcher Shaman
 UPDATE `creature_template` SET `gold_max` =243 WHERE `entry` =2721 AND `gold_max` =76;

-- Forsaken Bodyguard
 UPDATE `creature_template` SET `gold_min` =56 WHERE `entry` =2723 AND `gold_min` =65;

-- Stone Golem
 UPDATE `creature_template` SET `gold_max` =285 WHERE `entry` =2723 AND `gold_max` =90;

-- Stone Golem
 UPDATE `creature_template` SET `gold_max` =357 WHERE `entry` =2725 AND `gold_max` =91;

-- Scalding Whelp
 UPDATE `creature_template` SET `gold_min` =183 WHERE `entry` =2726 AND `gold_min` =308;

-- Scorched Guardian
 UPDATE `creature_template` SET `gold_max` =1285 WHERE `entry` =2726 AND `gold_max` =407;

-- Scorched Guardian
 UPDATE `creature_template` SET `gold_max` =227 WHERE `entry` =2733 AND `gold_max` =85;

-- Apothecary Jorell
 UPDATE `creature_template` SET `gold_max` =794 WHERE `entry` =2738 AND `gold_max` =308;

-- Stromgarde Cavalryman
 UPDATE `creature_template` SET `gold_max` =251 WHERE `entry` =2739 AND `gold_max` =81;

-- Shadowforge Tunneler
 UPDATE `creature_template` SET `gold_max` =264 WHERE `entry` =2740 AND `gold_max` =71;

-- Shadowforge Darkweaver
 UPDATE `creature_template` SET `gold_max` =315 WHERE `entry` =2742 AND `gold_max` =82;

-- Shadowforge Chanter
 UPDATE `creature_template` SET `gold_min` =47 WHERE `entry` =2743 AND `gold_min` =65;

-- Shadowforge Warrior
 UPDATE `creature_template` SET `gold_max` =312 WHERE `entry` =2743 AND `gold_max` =90;

-- Shadowforge Warrior
 UPDATE `creature_template` SET `gold_max` =1890 WHERE `entry` =2757 AND `gold_max` =643;

-- Blacklash
 UPDATE `creature_template` SET `gold_max` =1698 WHERE `entry` =2759 AND `gold_max` =576;

-- Hematus
 UPDATE `creature_template` SET `gold_max` =814 WHERE `entry` =2763 AND `gold_max` =534;

-- Thenan
 UPDATE `creature_template` SET `gold_min` =55 WHERE `entry` =2765 AND `gold_min` =68;

-- Znort
 UPDATE `creature_template` SET `gold_max` =126 WHERE `entry` =2765 AND `gold_max` =94;

-- Znort
 UPDATE `creature_template` SET `gold_max` =306 WHERE `entry` =2779 AND `gold_max` =99;

-- Prince Nazjak
 UPDATE `creature_template` SET `gold_max` =699 WHERE `entry` =2780 AND `gold_max` =364;

-- Caretaker Nevlin
 UPDATE `creature_template` SET `gold_min` =208 WHERE `entry` =2781 AND `gold_min` =275;

-- Caretaker Weston
 UPDATE `creature_template` SET `gold_max` =609 WHERE `entry` =2782 AND `gold_max` =353;

-- Caretaker Alaric
 UPDATE `creature_template` SET `gold_max` =237 WHERE `entry` =2793 AND `gold_max` =83;

-- Kor'gresh Coldrage
 UPDATE `creature_template` SET `gold_min` =56 WHERE `entry` =2893 AND `gold_min` =66;

-- Stonevault Bonesnapper (sample size 38)
 UPDATE `creature_template` SET `gold_max` =334 WHERE `entry` =2893 AND `gold_max` =91;

-- Stonevault Bonesnapper (sample size 38)
 UPDATE `creature_template` SET `gold_max` =353 WHERE `entry` =2894 AND `gold_max` =87;

-- Stonevault Shaman
 UPDATE `creature_template` SET `gold_max` =197 WHERE `entry` =2906 AND `gold_max` =81;

-- Dustbelcher Warrior
 UPDATE `creature_template` SET `gold_min` =45 WHERE `entry` =2907 AND `gold_min` =55;

-- Dustbelcher Mystic
 UPDATE `creature_template` SET `gold_max` =210 WHERE `entry` =2907 AND `gold_max` =77;

-- Dustbelcher Mystic
 UPDATE `creature_template` SET `gold_max` =346 WHERE `entry` =2927 AND `gold_max` =106;

-- Vicious Owlbeast
 UPDATE `creature_template` SET `gold_max` =449 WHERE `entry` =2928 AND `gold_max` =111;

-- Primitive Owlbeast
 UPDATE `creature_template` SET `gold_min` =76 WHERE `entry` =2929 AND `gold_min` =89;

-- Savage Owlbeast (sample size 48)
 UPDATE `creature_template` SET `gold_max` =495 WHERE `entry` =2929 AND `gold_max` =121;

-- Savage Owlbeast (sample size 48)
 UPDATE `creature_template` SET `gold_min` =80 WHERE `entry` =2932 AND `gold_min` =249;

-- Magregan Deepshadow
 UPDATE `creature_template` SET `gold_max` =270 WHERE `entry` =2944 AND `gold_max` =105;

-- Boss Tho'grun
 UPDATE `creature_template` SET `gold_max` =186 WHERE `entry` =2945 AND `gold_max` =103;

-- Murdaloc
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =2949 AND `gold_min` =4;

-- Palemane Tanner
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =2950 AND `gold_min` =5;

-- Palemane Skinner
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =2951 AND `gold_min` =6;

-- Palemane Poacher
 UPDATE `creature_template` SET `gold_max` =5 WHERE `entry` =2952 AND `gold_max` =7;

-- Bristleback Quilboar (sample size 50)
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =2962 AND `gold_min` =6;

-- Windfury Harpy (sample size 28)
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =2963 AND `gold_min` =8;

-- Windfury Wind Witch
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =2964 AND `gold_min` =8;

-- Windfury Sorceress (sample size 24)
 UPDATE `creature_template` SET `gold_max` =15 WHERE `entry` =2964 AND `gold_max` =16;

-- Windfury Sorceress (sample size 24)
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =2965 AND `gold_min` =10;

-- Windfury Matriarch (sample size 26)
 UPDATE `creature_template` SET `gold_max` =16 WHERE `entry` =2965 AND `gold_max` =18;

-- Windfury Matriarch (sample size 26)
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =2967 AND `gold_min` =10;

-- Galak Centaur
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =2975 AND `gold_min` =4;

-- Venture Co. Hireling (sample size 24)
 UPDATE `creature_template` SET `gold_max` =7 WHERE `entry` =2975 AND `gold_max` =10;

-- Venture Co. Hireling (sample size 24)
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =2976 AND `gold_min` =6;

-- Venture Co. Laborer
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =2977 AND `gold_min` =7;

-- Venture Co. Taskmaster
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =2978 AND `gold_min` =8;

-- Venture Co. Worker (sample size 36)
 UPDATE `creature_template` SET `gold_max` =14 WHERE `entry` =2978 AND `gold_max` =15;

-- Venture Co. Worker (sample size 36)
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =2979 AND `gold_min` =9;

-- Venture Co. Supervisor (sample size 22)
 UPDATE `creature_template` SET `gold_max` =15 WHERE `entry` =2979 AND `gold_max` =17;

-- Venture Co. Supervisor (sample size 22)
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =2989 AND `gold_min` =6;

-- Bael'dun Digger
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =2990 AND `gold_min` =7;

-- Bael'dun Appraiser
 UPDATE `creature_template` SET `gold_min` =5 WHERE `entry` =3051 AND `gold_min` =13;

-- Supervisor Fizsprocket
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =3101 AND `gold_min` =2;

-- Vile Familiar
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =3103 AND `gold_min` =5;

-- Makrura Clacker
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =3104 AND `gold_min` =5;

-- Makrura Shellhide
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =3111 AND `gold_min` =5;

-- Razormane Quilboar
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =3112 AND `gold_min` =7;

-- Razormane Scout
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =3113 AND `gold_min` =7;

-- Razormane Dustrunner
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =3114 AND `gold_min` =9;

-- Razormane Battleguard
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =3115 AND `gold_min` =8;

-- Dustwind Harpy
 UPDATE `creature_template` SET `gold_min` =5 WHERE `entry` =3116 AND `gold_min` =7;

-- Dustwind Pillager
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =3117 AND `gold_min` =9;

-- Dustwind Savage
 UPDATE `creature_template` SET `gold_min` =10 WHERE `entry` =3118 AND `gold_min` =11;

-- Dustwind Storm Witch
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =3119 AND `gold_min` =5;

-- Kolkar Drudge
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =3128 AND `gold_min` =4;

-- Kul Tiras Sailor
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =3129 AND `gold_min` =5;

-- Kul Tiras Marine
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =3195 AND `gold_min` =8;

-- Burning Blade Thug
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =3196 AND `gold_min` =9;

-- Burning Blade Neophyte
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =3197 AND `gold_min` =9;

-- Burning Blade Fanatic (sample size 46)
 UPDATE `creature_template` SET `gold_max` =15 WHERE `entry` =3197 AND `gold_max` =17;

-- Burning Blade Fanatic (sample size 46)
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =3198 AND `gold_min` =10;

-- Burning Blade Apprentice (sample size 22)
 UPDATE `creature_template` SET `gold_max` =17 WHERE `entry` =3198 AND `gold_max` =18;

-- Burning Blade Apprentice (sample size 22)
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =3199 AND `gold_min` =10;

-- Burning Blade Cultist
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =3203 AND `gold_min` =12;

-- Fizzle Darkstorm
 UPDATE `creature_template` SET `gold_min` =12 WHERE `entry` =3204 AND `gold_min` =16;

-- Gazz'uz
 UPDATE `creature_template` SET `gold_min` =5 WHERE `entry` =3206 AND `gold_min` =7;

-- Voodoo Troll
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =3207 AND `gold_min` =7;

-- Hexed Troll
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =3232 AND `gold_min` =9;

-- Bristleback Interloper
 UPDATE `creature_template` SET `gold_min` =13 WHERE `entry` =3258 AND `gold_min` =20;

-- Bristleback Hunter
 UPDATE `creature_template` SET `gold_max` =49 WHERE `entry` =3258 AND `gold_max` =31;

-- Bristleback Hunter
 UPDATE `creature_template` SET `gold_min` =7 WHERE `entry` =3260 AND `gold_min` =18;

-- Bristleback Water Seeker
 UPDATE `creature_template` SET `gold_max` =47 WHERE `entry` =3260 AND `gold_max` =29;

-- Bristleback Water Seeker
 UPDATE `creature_template` SET `gold_min` =9 WHERE `entry` =3261 AND `gold_min` =21;

-- Bristleback Thornweaver (sample size 35)
 UPDATE `creature_template` SET `gold_max` =59 WHERE `entry` =3261 AND `gold_max` =33;

-- Bristleback Thornweaver (sample size 35)
 UPDATE `creature_template` SET `gold_min` =12 WHERE `entry` =3263 AND `gold_min` =22;

-- Bristleback Geomancer (sample size 31)
 UPDATE `creature_template` SET `gold_max` =62 WHERE `entry` =3263 AND `gold_max` =33;

-- Bristleback Geomancer (sample size 31)
 UPDATE `creature_template` SET `gold_min` =6 WHERE `entry` =3265 AND `gold_min` =12;

-- Razormane Hunter
 UPDATE `creature_template` SET `gold_max` =23 WHERE `entry` =3265 AND `gold_max` =20;

-- Razormane Hunter
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =3266 AND `gold_min` =13;

-- Razormane Defender
 UPDATE `creature_template` SET `gold_max` =27 WHERE `entry` =3266 AND `gold_max` =22;

-- Razormane Defender
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =3267 AND `gold_min` =11;

-- Razormane Water Seeker (sample size 27)
 UPDATE `creature_template` SET `gold_max` =17 WHERE `entry` =3267 AND `gold_max` =19;

-- Razormane Water Seeker (sample size 27)
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =3268 AND `gold_min` =10;

-- Razormane Thornweaver (sample size 24)
 UPDATE `creature_template` SET `gold_max` =17 WHERE `entry` =3268 AND `gold_max` =18;

-- Razormane Thornweaver (sample size 24)
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =3269 AND `gold_min` =12;

-- Razormane Geomancer
 UPDATE `creature_template` SET `gold_max` =27 WHERE `entry` =3269 AND `gold_max` =21;

-- Razormane Geomancer
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =3271 AND `gold_min` =14;

-- Razormane Mystic
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =3272 AND `gold_min` =13;

-- Kolkar Wrangler (sample size 29)
 UPDATE `creature_template` SET `gold_max` =27 WHERE `entry` =3272 AND `gold_max` =22;

-- Kolkar Wrangler (sample size 29)
 UPDATE `creature_template` SET `gold_min` =6 WHERE `entry` =3273 AND `gold_min` =15;

-- Kolkar Stormer (sample size 29)
 UPDATE `creature_template` SET `gold_max` =29 WHERE `entry` =3273 AND `gold_max` =24;

-- Kolkar Stormer (sample size 29)
 UPDATE `creature_template` SET `gold_min` =13 WHERE `entry` =3275 AND `gold_min` =17;

-- Kolkar Marauder
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =3276 AND `gold_min` =16;

-- Witchwing Harpy (sample size 31)
 UPDATE `creature_template` SET `gold_max` =34 WHERE `entry` =3276 AND `gold_max` =26;

-- Witchwing Harpy (sample size 31)
 UPDATE `creature_template` SET `gold_min` =5 WHERE `entry` =3277 AND `gold_min` =17;

-- Witchwing Roguefeather (sample size 22)
 UPDATE `creature_template` SET `gold_max` =38 WHERE `entry` =3277 AND `gold_max` =27;

-- Witchwing Roguefeather (sample size 22)
 UPDATE `creature_template` SET `gold_min` =9 WHERE `entry` =3278 AND `gold_min` =19;

-- Witchwing Slayer (sample size 24)
 UPDATE `creature_template` SET `gold_max` =46 WHERE `entry` =3278 AND `gold_max` =30;

-- Witchwing Slayer (sample size 24)
 UPDATE `creature_template` SET `gold_max` =47 WHERE `entry` =3280 AND `gold_max` =30;

-- Witchwing Windcaller
 UPDATE `creature_template` SET `gold_min` =6 WHERE `entry` =3282 AND `gold_min` =17;

-- Venture Co. Mercenary
 UPDATE `creature_template` SET `gold_max` =36 WHERE `entry` =3282 AND `gold_max` =28;

-- Venture Co. Mercenary
 UPDATE `creature_template` SET `gold_min` =10 WHERE `entry` =3283 AND `gold_min` =19;

-- Venture Co. Enforcer
 UPDATE `creature_template` SET `gold_max` =36 WHERE `entry` =3283 AND `gold_max` =30;

-- Venture Co. Enforcer
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =3284 AND `gold_min` =16;

-- Venture Co. Drudger (sample size 48)
 UPDATE `creature_template` SET `gold_max` =34 WHERE `entry` =3284 AND `gold_max` =26;

-- Venture Co. Drudger (sample size 48)
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =3285 AND `gold_min` =15;

-- Venture Co. Peon (sample size 25)
 UPDATE `creature_template` SET `gold_max` =29 WHERE `entry` =3285 AND `gold_max` =24;

-- Venture Co. Peon (sample size 25)
 UPDATE `creature_template` SET `gold_min` =8 WHERE `entry` =3286 AND `gold_min` =20;

-- Venture Co. Overseer
 UPDATE `creature_template` SET `gold_max` =43 WHERE `entry` =3286 AND `gold_max` =32;

-- Venture Co. Overseer
 UPDATE `creature_template` SET `gold_min` =12 WHERE `entry` =3374 AND `gold_min` =27;

-- Bael'dun Excavator (sample size 103)
 UPDATE `creature_template` SET `gold_max` =86 WHERE `entry` =3374 AND `gold_max` =40;

-- Bael'dun Excavator (sample size 103)
 UPDATE `creature_template` SET `gold_min` =13 WHERE `entry` =3375 AND `gold_min` =29;

-- Bael'dun Foreman (sample size 21)
 UPDATE `creature_template` SET `gold_max` =95 WHERE `entry` =3375 AND `gold_max` =43;

-- Bael'dun Foreman (sample size 21)
 UPDATE `creature_template` SET `gold_min` =17 WHERE `entry` =3376 AND `gold_min` =31;

-- Bael'dun Soldier
 UPDATE `creature_template` SET `gold_max` =102 WHERE `entry` =3376 AND `gold_max` =45;

-- Bael'dun Soldier
 UPDATE `creature_template` SET `gold_min` =34 WHERE `entry` =3377 AND `gold_min` =33;

-- Bael'dun Rifleman (sample size 22)
 UPDATE `creature_template` SET `gold_max` =111 WHERE `entry` =3377 AND `gold_max` =48;

-- Bael'dun Rifleman (sample size 22)
 UPDATE `creature_template` SET `gold_max` =111 WHERE `entry` =3378 AND `gold_max` =52;

-- Bael'dun Officer
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =3379 AND `gold_min` =10;

-- Burning Blade Bruiser
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =3380 AND `gold_min` =11;

-- Burning Blade Acolyte
 UPDATE `creature_template` SET `gold_max` =23 WHERE `entry` =3380 AND `gold_max` =19;

-- Burning Blade Acolyte
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =3381 AND `gold_min` =13;

-- Southsea Brigand (sample size 38)
 UPDATE `creature_template` SET `gold_max` =27 WHERE `entry` =3381 AND `gold_max` =22;

-- Southsea Brigand (sample size 38)
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =3382 AND `gold_min` =14;

-- Southsea Cannoneer (sample size 25)
 UPDATE `creature_template` SET `gold_max` =29 WHERE `entry` =3382 AND `gold_max` =24;

-- Southsea Cannoneer (sample size 25)
 UPDATE `creature_template` SET `gold_min` =5 WHERE `entry` =3385 AND `gold_min` =17;

-- Theramore Marine (sample size 58)
 UPDATE `creature_template` SET `gold_max` =38 WHERE `entry` =3385 AND `gold_max` =28;

-- Theramore Marine (sample size 58)
 UPDATE `creature_template` SET `gold_min` =7 WHERE `entry` =3386 AND `gold_min` =18;

-- Theramore Preserver
 UPDATE `creature_template` SET `gold_max` =48 WHERE `entry` =3386 AND `gold_max` =29;

-- Theramore Preserver
 UPDATE `creature_template` SET `gold_min` =24 WHERE `entry` =3392 AND `gold_min` =31;

-- Prospector Khazgorm
 UPDATE `creature_template` SET `gold_max` =134 WHERE `entry` =3392 AND `gold_max` =45;

-- Prospector Khazgorm
 UPDATE `creature_template` SET `gold_max` =55 WHERE `entry` =3393 AND `gold_max` =37;

-- Captain Fairmount
 UPDATE `creature_template` SET `gold_min` =12 WHERE `entry` =3394 AND `gold_min` =18;

-- Barak Kodobane
 UPDATE `creature_template` SET `gold_max` =40 WHERE `entry` =3394 AND `gold_max` =29;

-- Barak Kodobane
 UPDATE `creature_template` SET `gold_max` =29 WHERE `entry` =3397 AND `gold_max` =24;

-- Kolkar Bloodcharger
 UPDATE `creature_template` SET `gold_max` =54 WHERE `entry` =3434 AND `gold_max` =42;

-- Nak
 UPDATE `creature_template` SET `gold_min` =28 WHERE `entry` =3435 AND `gold_min` =34;

-- Lok Orcbane
 UPDATE `creature_template` SET `gold_max` =122 WHERE `entry` =3435 AND `gold_max` =49;

-- Lok Orcbane
 UPDATE `creature_template` SET `gold_min` =32 WHERE `entry` =3436 AND `gold_min` =34;

-- Kuz
 UPDATE `creature_template` SET `gold_max` =84 WHERE `entry` =3436 AND `gold_max` =49;

-- Kuz
 UPDATE `creature_template` SET `gold_max` =30 WHERE `entry` =3438 AND `gold_max` =27;

-- Kreenig Snarlsnout
 UPDATE `creature_template` SET `gold_min` =19 WHERE `entry` =3445 AND `gold_min` =21;

-- Supervisor Lugwizzle
 UPDATE `creature_template` SET `gold_max` =53 WHERE `entry` =3445 AND `gold_max` =33;

-- Supervisor Lugwizzle
 UPDATE `creature_template` SET `gold_max` =46 WHERE `entry` =3454 AND `gold_max` =35;

-- Cannoneer Smythe
 UPDATE `creature_template` SET `gold_max` =45 WHERE `entry` =3455 AND `gold_max` =35;

-- Cannoneer Whessan
 UPDATE `creature_template` SET `gold_min` =14 WHERE `entry` =3456 AND `gold_min` =25;

-- Razormane Pathfinder (sample size 22)
 UPDATE `creature_template` SET `gold_max` =68 WHERE `entry` =3456 AND `gold_max` =38;

-- Razormane Pathfinder (sample size 22)
 UPDATE `creature_template` SET `gold_min` =13 WHERE `entry` =3457 AND `gold_min` =27;

-- Razormane Stalker
 UPDATE `creature_template` SET `gold_max` =95 WHERE `entry` =3457 AND `gold_max` =41;

-- Razormane Stalker
 UPDATE `creature_template` SET `gold_min` =17 WHERE `entry` =3458 AND `gold_min` =29;

-- Razormane Seer
 UPDATE `creature_template` SET `gold_max` =104 WHERE `entry` =3458 AND `gold_max` =43;

-- Razormane Seer
 UPDATE `creature_template` SET `gold_min` =22 WHERE `entry` =3459 AND `gold_min` =33;

-- Razormane Warfrenzy
 UPDATE `creature_template` SET `gold_max` =113 WHERE `entry` =3459 AND `gold_max` =48;

-- Razormane Warfrenzy
 UPDATE `creature_template` SET `gold_min` =8 WHERE `entry` =3467 AND `gold_min` =18;

-- Baron Longshore
 UPDATE `creature_template` SET `gold_max` =42 WHERE `entry` =3467 AND `gold_max` =29;

-- Baron Longshore
 UPDATE `creature_template` SET `gold_min` =7 WHERE `entry` =3470 AND `gold_min` =17;

-- Rathorian
 UPDATE `creature_template` SET `gold_max` =32 WHERE `entry` =3471 AND `gold_max` =28;

-- Tinkerer Sniggles
 UPDATE `creature_template` SET `gold_min` =20 WHERE `entry` =3530 AND `gold_min` =64;

-- Pyrewood Tailor
 UPDATE `creature_template` SET `gold_max` =79 WHERE `entry` =3533 AND `gold_max` =47;

-- Moonrage Leatherworker
 UPDATE `creature_template` SET `gold_min` =9 WHERE `entry` =3535 AND `gold_min` =14;

-- Blackmoss the Fetid
 UPDATE `creature_template` SET `gold_max` =270 WHERE `entry` =3654 AND `gold_max` =248;

-- Mutanus the Devourer
 UPDATE `creature_template` SET `gold_max` =179 WHERE `entry` =3655 AND `gold_max` =108;

-- Mad Magglish
 UPDATE `creature_template` SET `gold_max` =172 WHERE `entry` =3660 AND `gold_max` =66;

-- Athrikus Narassin
 UPDATE `creature_template` SET `gold_max` =69 WHERE `entry` =3664 AND `gold_max` =50;

-- Ilkrud Magthrull
 UPDATE `creature_template` SET `gold_max` =35 WHERE `entry` =3667 AND `gold_max` =28;

-- Anaya Dawnrunner
 UPDATE `creature_template` SET `gold_min` =87 WHERE `entry` =3669 AND `gold_min` =125;

-- Lord Cobrahn
 UPDATE `creature_template` SET `gold_min` =56 WHERE `entry` =3671 AND `gold_min` =125;

-- Lady Anacondra
 UPDATE `creature_template` SET `gold_max` =180 WHERE `entry` =3671 AND `gold_max` =168;

-- Lady Anacondra
 UPDATE `creature_template` SET `gold_max` =255 WHERE `entry` =3673 AND `gold_max` =209;

-- Lord Serpentis
 UPDATE `creature_template` SET `gold_max` =151 WHERE `entry` =3696 AND `gold_max` =62;

-- Ran Bloodtooth
 UPDATE `creature_template` SET `gold_min` =23 WHERE `entry` =3711 AND `gold_min` =25;

-- Wrathtail Myrmidon
 UPDATE `creature_template` SET `gold_max` =57 WHERE `entry` =3711 AND `gold_max` =38;

-- Wrathtail Myrmidon
 UPDATE `creature_template` SET `gold_min` =13 WHERE `entry` =3712 AND `gold_min` =23;

-- Wrathtail Razortail
 UPDATE `creature_template` SET `gold_max` =43 WHERE `entry` =3712 AND `gold_max` =34;

-- Wrathtail Razortail
 UPDATE `creature_template` SET `gold_min` =8 WHERE `entry` =3713 AND `gold_min` =22;

-- Wrathtail Wave Rider (sample size 41)
 UPDATE `creature_template` SET `gold_max` =59 WHERE `entry` =3713 AND `gold_max` =33;

-- Wrathtail Wave Rider (sample size 41)
 UPDATE `creature_template` SET `gold_min` =10 WHERE `entry` =3715 AND `gold_min` =23;

-- Wrathtail Sea Witch
 UPDATE `creature_template` SET `gold_max` =64 WHERE `entry` =3715 AND `gold_max` =34;

-- Wrathtail Sea Witch
 UPDATE `creature_template` SET `gold_min` =8 WHERE `entry` =3717 AND `gold_min` =20;

-- Wrathtail Sorceress (sample size 48)
 UPDATE `creature_template` SET `gold_max` =60 WHERE `entry` =3717 AND `gold_max` =32;

-- Wrathtail Sorceress (sample size 48)
 UPDATE `creature_template` SET `gold_min` =13 WHERE `entry` =3725 AND `gold_min` =20;

-- Dark Strand Cultist
 UPDATE `creature_template` SET `gold_max` =55 WHERE `entry` =3725 AND `gold_max` =32;

-- Dark Strand Cultist
 UPDATE `creature_template` SET `gold_min` =13 WHERE `entry` =3727 AND `gold_min` =26;

-- Dark Strand Enforcer
 UPDATE `creature_template` SET `gold_max` =70 WHERE `entry` =3727 AND `gold_max` =38;

-- Dark Strand Enforcer
 UPDATE `creature_template` SET `gold_min` =15 WHERE `entry` =3728 AND `gold_min` =21;

-- Dark Strand Adept
 UPDATE `creature_template` SET `gold_max` =59 WHERE `entry` =3728 AND `gold_max` =32;

-- Dark Strand Adept
 UPDATE `creature_template` SET `gold_min` =10 WHERE `entry` =3730 AND `gold_min` =24;

-- Dark Strand Excavator
 UPDATE `creature_template` SET `gold_max` =51 WHERE `entry` =3730 AND `gold_max` =36;

-- Dark Strand Excavator
 UPDATE `creature_template` SET `gold_min` =20 WHERE `entry` =3732 AND `gold_min` =21;

-- Forsaken Seeker
 UPDATE `creature_template` SET `gold_max` =52 WHERE `entry` =3732 AND `gold_max` =32;

-- Forsaken Seeker
 UPDATE `creature_template` SET `gold_min` =14 WHERE `entry` =3733 AND `gold_min` =22;

-- Forsaken Herbalist
 UPDATE `creature_template` SET `gold_max` =53 WHERE `entry` =3733 AND `gold_max` =33;

-- Forsaken Herbalist
 UPDATE `creature_template` SET `gold_min` =20 WHERE `entry` =3734 AND `gold_min` =25;

-- Forsaken Thug
 UPDATE `creature_template` SET `gold_max` =66 WHERE `entry` =3734 AND `gold_max` =37;

-- Forsaken Thug
 UPDATE `creature_template` SET `gold_min` =13 WHERE `entry` =3737 AND `gold_min` =24;

-- Saltspittle Puddlejumper
 UPDATE `creature_template` SET `gold_max` =52 WHERE `entry` =3737 AND `gold_max` =36;

-- Saltspittle Puddlejumper
 UPDATE `creature_template` SET `gold_min` =14 WHERE `entry` =3739 AND `gold_min` =23;

-- Saltspittle Warrior
 UPDATE `creature_template` SET `gold_max` =61 WHERE `entry` =3739 AND `gold_max` =35;

-- Saltspittle Warrior
 UPDATE `creature_template` SET `gold_min` =24 WHERE `entry` =3740 AND `gold_min` =26;

-- Saltspittle Muckdweller
 UPDATE `creature_template` SET `gold_max` =70 WHERE `entry` =3740 AND `gold_max` =38;

-- Saltspittle Muckdweller
 UPDATE `creature_template` SET `gold_max` =49 WHERE `entry` =3742 AND `gold_max` =36;

-- Saltspittle Oracle
 UPDATE `creature_template` SET `gold_min` =14 WHERE `entry` =3743 AND `gold_min` =35;

-- Foulweald Warrior (sample size 28)
 UPDATE `creature_template` SET `gold_max` =104 WHERE `entry` =3743 AND `gold_max` =51;

-- Foulweald Warrior (sample size 28)
 UPDATE `creature_template` SET `gold_min` =17 WHERE `entry` =3745 AND `gold_min` =31;

-- Foulweald Pathfinder
 UPDATE `creature_template` SET `gold_max` =69 WHERE `entry` =3745 AND `gold_max` =45;

-- Foulweald Pathfinder
 UPDATE `creature_template` SET `gold_max` =68 WHERE `entry` =3746 AND `gold_max` =48;

-- Foulweald Den Watcher
 UPDATE `creature_template` SET `gold_max` =107 WHERE `entry` =3748 AND `gold_max` =45;

-- Foulweald Shaman
 UPDATE `creature_template` SET `gold_max` =99 WHERE `entry` =3749 AND `gold_max` =48;

-- Foulweald Ursa
 UPDATE `creature_template` SET `gold_min` =15 WHERE `entry` =3750 AND `gold_min` =29;

-- Foulweald Totemic (sample size 22)
 UPDATE `creature_template` SET `gold_max` =102 WHERE `entry` =3750 AND `gold_max` =43;

-- Foulweald Totemic (sample size 22)
 UPDATE `creature_template` SET `gold_max` =156 WHERE `entry` =3752 AND `gold_max` =61;

-- Xavian Rogue
 UPDATE `creature_template` SET `gold_min` =40 WHERE `entry` =3754 AND `gold_min` =42;

-- Xavian Betrayer
 UPDATE `creature_template` SET `gold_max` =128 WHERE `entry` =3754 AND `gold_max` =59;

-- Xavian Betrayer
 UPDATE `creature_template` SET `gold_min` =31 WHERE `entry` =3755 AND `gold_min` =38;

-- Xavian Felsworn
 UPDATE `creature_template` SET `gold_max` =75 WHERE `entry` =3755 AND `gold_max` =55;

-- Xavian Felsworn
 UPDATE `creature_template` SET `gold_min` =32 WHERE `entry` =3757 AND `gold_min` =40;

-- Xavian Hellcaller
 UPDATE `creature_template` SET `gold_max` =152 WHERE `entry` =3757 AND `gold_max` =57;

-- Xavian Hellcaller
 UPDATE `creature_template` SET `gold_min` =25 WHERE `entry` =3758 AND `gold_min` =35;

-- Felmusk Satyr
 UPDATE `creature_template` SET `gold_max` =113 WHERE `entry` =3758 AND `gold_max` =50;

-- Felmusk Satyr
 UPDATE `creature_template` SET `gold_min` =21 WHERE `entry` =3759 AND `gold_min` =37;

-- Felmusk Rogue
 UPDATE `creature_template` SET `gold_max` =124 WHERE `entry` =3759 AND `gold_max` =53;

-- Felmusk Rogue
 UPDATE `creature_template` SET `gold_min` =19 WHERE `entry` =3762 AND `gold_min` =33;

-- Felmusk Felsworn (sample size 29)
 UPDATE `creature_template` SET `gold_max` =121 WHERE `entry` =3762 AND `gold_max` =48;

-- Felmusk Felsworn (sample size 29)
 UPDATE `creature_template` SET `gold_min` =21 WHERE `entry` =3763 AND `gold_min` =37;

-- Felmusk Shadowstalker (sample size 21)
 UPDATE `creature_template` SET `gold_max` =128 WHERE `entry` =3763 AND `gold_max` =54;

-- Felmusk Shadowstalker (sample size 21)
 UPDATE `creature_template` SET `gold_min` =21 WHERE `entry` =3765 AND `gold_min` =37;

-- Bleakheart Satyr (sample size 27)
 UPDATE `creature_template` SET `gold_max` =129 WHERE `entry` =3765 AND `gold_max` =53;

-- Bleakheart Satyr (sample size 27)
 UPDATE `creature_template` SET `gold_min` =24 WHERE `entry` =3767 AND `gold_min` =40;

-- Bleakheart Trickster (sample size 27)
 UPDATE `creature_template` SET `gold_max` =140 WHERE `entry` =3767 AND `gold_max` =57;

-- Bleakheart Trickster (sample size 27)
 UPDATE `creature_template` SET `gold_min` =22 WHERE `entry` =3770 AND `gold_min` =39;

-- Bleakheart Shadowstalker (sample size 28)
 UPDATE `creature_template` SET `gold_max` =133 WHERE `entry` =3770 AND `gold_max` =56;

-- Bleakheart Shadowstalker (sample size 28)
 UPDATE `creature_template` SET `gold_min` =24 WHERE `entry` =3771 AND `gold_min` =35;

-- Bleakheart Hellcaller
 UPDATE `creature_template` SET `gold_max` =128 WHERE `entry` =3771 AND `gold_max` =51;

-- Bleakheart Hellcaller
 UPDATE `creature_template` SET `gold_max` =97 WHERE `entry` =3772 AND `gold_max` =43;

-- Lesser Felguard
 UPDATE `creature_template` SET `gold_min` =15 WHERE `entry` =3780 AND `gold_min` =28;

-- Shadethicket Moss Eater
 UPDATE `creature_template` SET `gold_max` =80 WHERE `entry` =3780 AND `gold_max` =41;

-- Shadethicket Moss Eater
 UPDATE `creature_template` SET `gold_max` =93 WHERE `entry` =3781 AND `gold_max` =43;

-- Shadethicket Wood Shaper
 UPDATE `creature_template` SET `gold_min` =21 WHERE `entry` =3782 AND `gold_min` =35;

-- Shadethicket Stone Mover
 UPDATE `creature_template` SET `gold_max` =94 WHERE `entry` =3782 AND `gold_max` =51;

-- Shadethicket Stone Mover
 UPDATE `creature_template` SET `gold_max` =48 WHERE `entry` =3783 AND `gold_max` =41;

-- Shadethicket Raincaller
 UPDATE `creature_template` SET `gold_min` =23 WHERE `entry` =3784 AND `gold_min` =38;

-- Shadethicket Bark Ripper
 UPDATE `creature_template` SET `gold_max` =122 WHERE `entry` =3784 AND `gold_max` =54;

-- Shadethicket Bark Ripper
 UPDATE `creature_template` SET `gold_min` =35 WHERE `entry` =3789 AND `gold_min` =41;

-- Terrowulf Fleshripper
 UPDATE `creature_template` SET `gold_max` =124 WHERE `entry` =3789 AND `gold_max` =59;

-- Terrowulf Fleshripper
 UPDATE `creature_template` SET `gold_min` =28 WHERE `entry` =3791 AND `gold_min` =43;

-- Terrowulf Shadow Weaver
 UPDATE `creature_template` SET `gold_max` =123 WHERE `entry` =3791 AND `gold_max` =61;

-- Terrowulf Shadow Weaver
 UPDATE `creature_template` SET `gold_max` =104 WHERE `entry` =3797 AND `gold_max` =48;

-- Cenarion Protector
 UPDATE `creature_template` SET `gold_max` =133 WHERE `entry` =3799 AND `gold_max` =63;

-- Severed Druid
 UPDATE `creature_template` SET `gold_min` =24 WHERE `entry` =3801 AND `gold_min` =38;

-- Severed Sleeper
 UPDATE `creature_template` SET `gold_max` =130 WHERE `entry` =3801 AND `gold_max` =55;

-- Severed Sleeper
 UPDATE `creature_template` SET `gold_min` =30 WHERE `entry` =3802 AND `gold_min` =39;

-- Severed Dreamer
 UPDATE `creature_template` SET `gold_max` =166 WHERE `entry` =3802 AND `gold_max` =55;

-- Severed Dreamer
 UPDATE `creature_template` SET `gold_min` =37 WHERE `entry` =3803 AND `gold_min` =41;

-- Severed Keeper
 UPDATE `creature_template` SET `gold_max` =166 WHERE `entry` =3803 AND `gold_max` =58;

-- Severed Keeper
 UPDATE `creature_template` SET `gold_min` =23 WHERE `entry` =3804 AND `gold_min` =42;

-- Forsaken Intruder
 UPDATE `creature_template` SET `gold_max` =131 WHERE `entry` =3804 AND `gold_max` =59;

-- Forsaken Intruder
 UPDATE `creature_template` SET `gold_min` =31 WHERE `entry` =3806 AND `gold_min` =43;

-- Forsaken Infiltrator
 UPDATE `creature_template` SET `gold_max` =156 WHERE `entry` =3806 AND `gold_max` =61;

-- Forsaken Infiltrator
 UPDATE `creature_template` SET `gold_max` =151 WHERE `entry` =3807 AND `gold_max` =61;

-- Forsaken Assassin
 UPDATE `creature_template` SET `gold_max` =150 WHERE `entry` =3808 AND `gold_max` =59;

-- Forsaken Dark Stalker
 UPDATE `creature_template` SET `gold_max` =133 WHERE `entry` =3815 AND `gold_max` =53;

-- Blink Dragon
 UPDATE `creature_template` SET `gold_min` =34 WHERE `entry` =3833 AND `gold_min` =36;

-- Cenarion Vindicator
 UPDATE `creature_template` SET `gold_max` =57 WHERE `entry` =3833 AND `gold_max` =52;

-- Cenarion Vindicator
 UPDATE `creature_template` SET `gold_min` =30 WHERE `entry` =3834 AND `gold_min` =40;

-- Crazed Ancient
 UPDATE `creature_template` SET `gold_max` =143 WHERE `entry` =3834 AND `gold_max` =57;

-- Crazed Ancient
 UPDATE `creature_template` SET `gold_min` =36 WHERE `entry` =3840 AND `gold_min` =89;

-- Druid of the Fang
 UPDATE `creature_template` SET `gold_max` =136 WHERE `entry` =3840 AND `gold_max` =121;

-- Druid of the Fang
 UPDATE `creature_template` SET `gold_max` =154 WHERE `entry` =3849 AND `gold_max` =119;

-- Deathstalker Adamant
 UPDATE `creature_template` SET `gold_min` =59 WHERE `entry` =3850 AND `gold_min` =79;

-- Sorcerer Ashcrombe
 UPDATE `creature_template` SET `gold_min` =30 WHERE `entry` =3851 AND `gold_min` =79;

-- Shadowfang Whitescalp (sample size 26)
 UPDATE `creature_template` SET `gold_max` =178 WHERE `entry` =3851 AND `gold_max` =108;

-- Shadowfang Whitescalp (sample size 26)
 UPDATE `creature_template` SET `gold_min` =32 WHERE `entry` =3853 AND `gold_min` =85;

-- Shadowfang Moonwalker (sample size 26)
 UPDATE `creature_template` SET `gold_max` =192 WHERE `entry` =3853 AND `gold_max` =115;

-- Shadowfang Moonwalker (sample size 26)
 UPDATE `creature_template` SET `gold_max` =164 WHERE `entry` =3854 AND `gold_max` =81;

-- Shadowfang Wolfguard
 UPDATE `creature_template` SET `gold_min` =40 WHERE `entry` =3855 AND `gold_min` =95;

-- Shadowfang Darksoul (sample size 24)
 UPDATE `creature_template` SET `gold_max` =209 WHERE `entry` =3855 AND `gold_max` =129;

-- Shadowfang Darksoul (sample size 24)
 UPDATE `creature_template` SET `gold_min` =41 WHERE `entry` =3857 AND `gold_min` =109;

-- Shadowfang Glutton
 UPDATE `creature_template` SET `gold_max` =202 WHERE `entry` =3857 AND `gold_max` =148;

-- Shadowfang Glutton
 UPDATE `creature_template` SET `gold_min` =74 WHERE `entry` =3859 AND `gold_min` =117;

-- Shadowfang Ragetooth
 UPDATE `creature_template` SET `gold_max` =303 WHERE `entry` =3859 AND `gold_max` =158;

-- Shadowfang Ragetooth
 UPDATE `creature_template` SET `gold_min` =48 WHERE `entry` =3873 AND `gold_min` =121;

-- Tormented Officer
 UPDATE `creature_template` SET `gold_max` =290 WHERE `entry` =3873 AND `gold_max` =163;

-- Tormented Officer
 UPDATE `creature_template` SET `gold_min` =33 WHERE `entry` =3875 AND `gold_min` =99;

-- Haunted Servitor (sample size 32)
 UPDATE `creature_template` SET `gold_max` =215 WHERE `entry` =3875 AND `gold_max` =134;

-- Haunted Servitor (sample size 32)
 UPDATE `creature_template` SET `gold_min` =68 WHERE `entry` =3877 AND `gold_min` =113;

-- Wailing Guardsman (sample size 21)
 UPDATE `creature_template` SET `gold_max` =259 WHERE `entry` =3877 AND `gold_max` =153;

-- Wailing Guardsman (sample size 21)
 UPDATE `creature_template` SET `gold_max` =60 WHERE `entry` =3879 AND `gold_max` =34;

-- Dark Strand Assassin
 UPDATE `creature_template` SET `gold_min` =120 WHERE `entry` =3886 AND `gold_min` =128;

-- Razorclaw the Butcher
 UPDATE `creature_template` SET `gold_max` =233 WHERE `entry` =3886 AND `gold_max` =172;

-- Razorclaw the Butcher
 UPDATE `creature_template` SET `gold_min` =94 WHERE `entry` =3887 AND `gold_min` =178;

-- Baron Silverlaine
 UPDATE `creature_template` SET `gold_max` =279 WHERE `entry` =3887 AND `gold_max` =237;

-- Baron Silverlaine
 UPDATE `creature_template` SET `gold_max` =88 WHERE `entry` =3893 AND `gold_max` =49;

-- Forsaken Scout
 UPDATE `creature_template` SET `gold_max` =90 WHERE `entry` =3898 AND `gold_max` =52;

-- Aligar the Tormentor
 UPDATE `creature_template` SET `gold_min` =51 WHERE `entry` =3914 AND `gold_min` =116;

-- Rethilgore
 UPDATE `creature_template` SET `gold_max` =206 WHERE `entry` =3914 AND `gold_max` =156;

-- Rethilgore
 UPDATE `creature_template` SET `gold_min` =27 WHERE `entry` =3919 AND `gold_min` =37;

-- Withered Ancient
 UPDATE `creature_template` SET `gold_max` =117 WHERE `entry` =3919 AND `gold_max` =53;

-- Withered Ancient
 UPDATE `creature_template` SET `gold_min` =14 WHERE `entry` =3921 AND `gold_min` =31;

-- Thistlefur Ursa (sample size 33)
 UPDATE `creature_template` SET `gold_max` =103 WHERE `entry` =3921 AND `gold_max` =45;

-- Thistlefur Ursa (sample size 33)
 UPDATE `creature_template` SET `gold_min` =16 WHERE `entry` =3922 AND `gold_min` =29;

-- Thistlefur Totemic (sample size 28)
 UPDATE `creature_template` SET `gold_max` =102 WHERE `entry` =3922 AND `gold_max` =43;

-- Thistlefur Totemic (sample size 28)
 UPDATE `creature_template` SET `gold_min` =14 WHERE `entry` =3923 AND `gold_min` =31;

-- Thistlefur Den Watcher
 UPDATE `creature_template` SET `gold_max` =99 WHERE `entry` =3923 AND `gold_max` =45;

-- Thistlefur Den Watcher
 UPDATE `creature_template` SET `gold_min` =21 WHERE `entry` =3924 AND `gold_min` =29;

-- Thistlefur Shaman (sample size 26)
 UPDATE `creature_template` SET `gold_max` =103 WHERE `entry` =3924 AND `gold_max` =43;

-- Thistlefur Shaman (sample size 26)
 UPDATE `creature_template` SET `gold_min` =15 WHERE `entry` =3925 AND `gold_min` =31;

-- Thistlefur Avenger (sample size 27)
 UPDATE `creature_template` SET `gold_max` =103 WHERE `entry` =3925 AND `gold_max` =45;

-- Thistlefur Avenger (sample size 27)
 UPDATE `creature_template` SET `gold_min` =17 WHERE `entry` =3926 AND `gold_min` =31;

-- Thistlefur Pathfinder
 UPDATE `creature_template` SET `gold_max` =104 WHERE `entry` =3926 AND `gold_max` =45;

-- Thistlefur Pathfinder
 UPDATE `creature_template` SET `gold_min` =114 WHERE `entry` =3927 AND `gold_min` =222;

-- Wolf Master Nandos
 UPDATE `creature_template` SET `gold_max` =338 WHERE `entry` =3927 AND `gold_max` =295;

-- Wolf Master Nandos
 UPDATE `creature_template` SET `gold_max` =123 WHERE `entry` =3931 AND `gold_max` =62;

-- Shadethicket Oracle
 UPDATE `creature_template` SET `gold_min` =29 WHERE `entry` =3932 AND `gold_min` =39;

-- Bloodtooth Guard
 UPDATE `creature_template` SET `gold_max` =145 WHERE `entry` =3932 AND `gold_max` =56;

-- Bloodtooth Guard
 UPDATE `creature_template` SET `gold_max` =112 WHERE `entry` =3941 AND `gold_max` =64;

-- Uthil Mooncall
 UPDATE `creature_template` SET `gold_max` =126 WHERE `entry` =3942 AND `gold_max` =64;

-- Mavoris Cloudsbreak
 UPDATE `creature_template` SET `gold_max` =95 WHERE `entry` =3943 AND `gold_max` =46;

-- Ruuzel
 UPDATE `creature_template` SET `gold_min` =15 WHERE `entry` =3944 AND `gold_min` =24;

-- Wrathtail Priestess
 UPDATE `creature_template` SET `gold_max` =69 WHERE `entry` =3944 AND `gold_max` =37;

-- Wrathtail Priestess
 UPDATE `creature_template` SET `gold_max` =194 WHERE `entry` =3947 AND `gold_max` =121;

-- Goblin Shipbuilder
 UPDATE `creature_template` SET `gold_min` =232 WHERE `entry` =3974 AND `gold_min` =386;

-- Houndmaster Loksey
 UPDATE `creature_template` SET `gold_max` =729 WHERE `entry` =3974 AND `gold_max` =509;

-- Houndmaster Loksey
 UPDATE `creature_template` SET `gold_min` =336 WHERE `entry` =3975 AND `gold_min` =575;

-- Herod
 UPDATE `creature_template` SET `gold_min` =285 WHERE `entry` =3976 AND `gold_min` =319;

-- Scarlet Commander Mograine
 UPDATE `creature_template` SET `gold_max` =773 WHERE `entry` =3977 AND `gold_max` =394;

-- High Inquisitor Whitemane
 UPDATE `creature_template` SET `gold_max` =588 WHERE `entry` =3983 AND `gold_max` =372;

-- Interrogator Vishas
 UPDATE `creature_template` SET `gold_max` =670 WHERE `entry` =3984 AND `gold_max` =263;

-- Nancy Vishas
 UPDATE `creature_template` SET `gold_max` =630 WHERE `entry` =3985 AND `gold_max` =271;

-- Grandpa Vishas
 UPDATE `creature_template` SET `gold_max` =116 WHERE `entry` =3986 AND `gold_max` =45;

-- Sarilus Foulborne
 UPDATE `creature_template` SET `gold_min` =19 WHERE `entry` =3987 AND `gold_min` =34;

-- Dal Bloodclaw
 UPDATE `creature_template` SET `gold_max` =77 WHERE `entry` =3987 AND `gold_max` =50;

-- Dal Bloodclaw
 UPDATE `creature_template` SET `gold_min` =14 WHERE `entry` =3988 AND `gold_min` =23;

-- Venture Co. Operator (sample size 22)
 UPDATE `creature_template` SET `gold_max` =59 WHERE `entry` =3988 AND `gold_max` =35;

-- Venture Co. Operator (sample size 22)
 UPDATE `creature_template` SET `gold_min` =8 WHERE `entry` =3989 AND `gold_min` =22;

-- Venture Co. Logger (sample size 62)
 UPDATE `creature_template` SET `gold_max` =59 WHERE `entry` =3989 AND `gold_max` =33;

-- Venture Co. Logger (sample size 62)
 UPDATE `creature_template` SET `gold_min` =9 WHERE `entry` =3991 AND `gold_min` =22;

-- Venture Co. Deforester (sample size 35)
 UPDATE `creature_template` SET `gold_max` =65 WHERE `entry` =3991 AND `gold_max` =33;

-- Venture Co. Deforester (sample size 35)
 UPDATE `creature_template` SET `gold_min` =17 WHERE `entry` =3992 AND `gold_min` =25;

-- Venture Co. Engineer
 UPDATE `creature_template` SET `gold_max` =65 WHERE `entry` =3992 AND `gold_max` =38;

-- Venture Co. Engineer
 UPDATE `creature_template` SET `gold_max` =82 WHERE `entry` =3993 AND `gold_max` =40;

-- Venture Co. Machine Smith
 UPDATE `creature_template` SET `gold_max` =66 WHERE `entry` =4003 AND `gold_max` =37;

-- Windshear Geomancer
 UPDATE `creature_template` SET `gold_min` =19 WHERE `entry` =4016 AND `gold_min` =31;

-- Fey Dragon
 UPDATE `creature_template` SET `gold_max` =65 WHERE `entry` =4016 AND `gold_max` =45;

-- Fey Dragon
 UPDATE `creature_template` SET `gold_max` =93 WHERE `entry` =4017 AND `gold_max` =50;

-- Wily Fey Dragon
 UPDATE `creature_template` SET `gold_min` =6 WHERE `entry` =4020 AND `gold_min` =29;

-- Sap Beast
 UPDATE `creature_template` SET `gold_min` =23 WHERE `entry` =4021 AND `gold_min` =33;

-- Corrosive Sap Beast
 UPDATE `creature_template` SET `gold_min` =15 WHERE `entry` =4022 AND `gold_min` =31;

-- Bloodfury Harpy (sample size 41)
 UPDATE `creature_template` SET `gold_max` =104 WHERE `entry` =4022 AND `gold_max` =45;

-- Bloodfury Harpy (sample size 41)
 UPDATE `creature_template` SET `gold_max` =121 WHERE `entry` =4023 AND `gold_max` =51;

-- Bloodfury Roguefeather
 UPDATE `creature_template` SET `gold_max` =117 WHERE `entry` =4024 AND `gold_max` =48;

-- Bloodfury Slayer
 UPDATE `creature_template` SET `gold_min` =20 WHERE `entry` =4025 AND `gold_min` =26;

-- Bloodfury Ambusher (sample size 36)
 UPDATE `creature_template` SET `gold_max` =103 WHERE `entry` =4025 AND `gold_max` =39;

-- Bloodfury Ambusher (sample size 36)
 UPDATE `creature_template` SET `gold_min` =16 WHERE `entry` =4026 AND `gold_min` =31;

-- Bloodfury Windcaller (sample size 39)
 UPDATE `creature_template` SET `gold_max` =113 WHERE `entry` =4026 AND `gold_max` =45;

-- Bloodfury Windcaller (sample size 39)
 UPDATE `creature_template` SET `gold_min` =21 WHERE `entry` =4027 AND `gold_min` =35;

-- Bloodfury Storm Witch
 UPDATE `creature_template` SET `gold_max` =115 WHERE `entry` =4027 AND `gold_max` =51;

-- Bloodfury Storm Witch
 UPDATE `creature_template` SET `gold_min` =20 WHERE `entry` =4028 AND `gold_min` =34;

-- Charred Ancient
 UPDATE `creature_template` SET `gold_max` =86 WHERE `entry` =4028 AND `gold_max` =49;

-- Charred Ancient
 UPDATE `creature_template` SET `gold_max` =116 WHERE `entry` =4029 AND `gold_max` =56;

-- Blackened Ancient
 UPDATE `creature_template` SET `gold_min` =91 WHERE `entry` =4050 AND `gold_min` =125;

-- Cenarion Caretaker
 UPDATE `creature_template` SET `gold_max` =346 WHERE `entry` =4050 AND `gold_max` =168;

-- Cenarion Caretaker
 UPDATE `creature_template` SET `gold_max` =94 WHERE `entry` =4051 AND `gold_max` =43;

-- Cenarion Botanist
 UPDATE `creature_template` SET `gold_max` =298 WHERE `entry` =4052 AND `gold_max` =175;

-- Cenarion Druid
 UPDATE `creature_template` SET `gold_min` =20 WHERE `entry` =4053 AND `gold_min` =30;

-- Daughter of Cenarius
 UPDATE `creature_template` SET `gold_max` =83 WHERE `entry` =4053 AND `gold_max` =44;

-- Daughter of Cenarius
 UPDATE `creature_template` SET `gold_min` =16 WHERE `entry` =4054 AND `gold_min` =33;

-- Laughing Sister
 UPDATE `creature_template` SET `gold_max` =106 WHERE `entry` =4054 AND `gold_max` =48;

-- Laughing Sister
 UPDATE `creature_template` SET `gold_min` =66 WHERE `entry` =4056 AND `gold_min` =143;

-- Mirkfallon Keeper
 UPDATE `creature_template` SET `gold_min` =26 WHERE `entry` =4057 AND `gold_min` =33;

-- Son of Cenarius
 UPDATE `creature_template` SET `gold_max` =72 WHERE `entry` =4057 AND `gold_max` =48;

-- Son of Cenarius
 UPDATE `creature_template` SET `gold_max` =349 WHERE `entry` =4061 AND `gold_max` =179;

-- Mirkfallon Dryad
 UPDATE `creature_template` SET `gold_min` =30 WHERE `entry` =4062 AND `gold_min` =46;

-- Dark Iron Bombardier
 UPDATE `creature_template` SET `gold_max` =180 WHERE `entry` =4062 AND `gold_max` =65;

-- Dark Iron Bombardier
 UPDATE `creature_template` SET `gold_max` =225 WHERE `entry` =4063 AND `gold_max` =80;

-- Feeboz
 UPDATE `creature_template` SET `gold_max` =66 WHERE `entry` =4070 AND `gold_max` =38;

-- Venture Co. Builder
 UPDATE `creature_template` SET `gold_max` =68 WHERE `entry` =4073 AND `gold_max` =43;

-- XT:4
 UPDATE `creature_template` SET `gold_max` =79 WHERE `entry` =4074 AND `gold_max` =43;

-- XT:9
 UPDATE `creature_template` SET `gold_min` =17 WHERE `entry` =4093 AND `gold_min` =35;

-- Galak Wrangler (sample size 34)
 UPDATE `creature_template` SET `gold_max` =117 WHERE `entry` =4093 AND `gold_max` =50;

-- Galak Wrangler (sample size 34)
 UPDATE `creature_template` SET `gold_min` =16 WHERE `entry` =4094 AND `gold_min` =32;

-- Galak Scout (sample size 31)
 UPDATE `creature_template` SET `gold_max` =99 WHERE `entry` =4094 AND `gold_max` =47;

-- Galak Scout (sample size 31)
 UPDATE `creature_template` SET `gold_min` =24 WHERE `entry` =4095 AND `gold_min` =39;

-- Galak Mauler
 UPDATE `creature_template` SET `gold_max` =143 WHERE `entry` =4095 AND `gold_max` =56;

-- Galak Mauler
 UPDATE `creature_template` SET `gold_min` =21 WHERE `entry` =4096 AND `gold_min` =31;

-- Galak Windchaser (sample size 36)
 UPDATE `creature_template` SET `gold_max` =111 WHERE `entry` =4096 AND `gold_max` =45;

-- Galak Windchaser (sample size 36)
 UPDATE `creature_template` SET `gold_min` =20 WHERE `entry` =4097 AND `gold_min` =32;

-- Galak Stormer
 UPDATE `creature_template` SET `gold_max` =134 WHERE `entry` =4097 AND `gold_max` =47;

-- Galak Stormer
 UPDATE `creature_template` SET `gold_min` =22 WHERE `entry` =4099 AND `gold_min` =37;

-- Galak Marauder
 UPDATE `creature_template` SET `gold_max` =134 WHERE `entry` =4099 AND `gold_max` =53;

-- Galak Marauder
 UPDATE `creature_template` SET `gold_min` =27 WHERE `entry` =4111 AND `gold_min` =37;

-- Gravelsnout Kobold
 UPDATE `creature_template` SET `gold_max` =125 WHERE `entry` =4111 AND `gold_max` =53;

-- Gravelsnout Kobold
 UPDATE `creature_template` SET `gold_min` =18 WHERE `entry` =4112 AND `gold_min` =35;

-- Gravelsnout Vermin
 UPDATE `creature_template` SET `gold_max` =71 WHERE `entry` =4112 AND `gold_max` =51;

-- Gravelsnout Vermin
 UPDATE `creature_template` SET `gold_max` =135 WHERE `entry` =4114 AND `gold_max` =57;

-- Gravelsnout Forager
 UPDATE `creature_template` SET `gold_max` =154 WHERE `entry` =4116 AND `gold_max` =55;

-- Gravelsnout Surveyor
 UPDATE `creature_template` SET `gold_max` =109 WHERE `entry` =4202 AND `gold_max` =55;

-- Gerenzo Wrenchwhistle
 UPDATE `creature_template` SET `gold_max` =259 WHERE `entry` =4260 AND `gold_max` =84;

-- Venture Co. Shredder
 UPDATE `creature_template` SET `gold_min` =118 WHERE `entry` =4275 AND `gold_min` =322;

-- Archmage Arugal
 UPDATE `creature_template` SET `gold_min` =115 WHERE `entry` =4278 AND `gold_min` =161;

-- Commander Springvale
 UPDATE `creature_template` SET `gold_max` =271 WHERE `entry` =4278 AND `gold_max` =215;

-- Commander Springvale
 UPDATE `creature_template` SET `gold_min` =65 WHERE `entry` =4279 AND `gold_min` =178;

-- Odo the Blindwatcher
 UPDATE `creature_template` SET `gold_max` =316 WHERE `entry` =4279 AND `gold_max` =237;

-- Odo the Blindwatcher
 UPDATE `creature_template` SET `gold_max` =499 WHERE `entry` =4280 AND `gold_max` =204;

-- Scarlet Preserver
 UPDATE `creature_template` SET `gold_min` =167 WHERE `entry` =4281 AND `gold_min` =174;

-- Scarlet Scout
 UPDATE `creature_template` SET `gold_max` =339 WHERE `entry` =4281 AND `gold_max` =232;

-- Scarlet Scout
 UPDATE `creature_template` SET `gold_max` =347 WHERE `entry` =4282 AND `gold_max` =199;

-- Scarlet Magician
 UPDATE `creature_template` SET `gold_min` =92 WHERE `entry` =4283 AND `gold_min` =188;

-- Scarlet Sentry
 UPDATE `creature_template` SET `gold_max` =528 WHERE `entry` =4283 AND `gold_max` =251;

-- Scarlet Sentry
 UPDATE `creature_template` SET `gold_min` =159 WHERE `entry` =4284 AND `gold_min` =167;

-- Scarlet Augur
 UPDATE `creature_template` SET `gold_max` =531 WHERE `entry` =4284 AND `gold_max` =223;

-- Scarlet Augur
 UPDATE `creature_template` SET `gold_min` =92 WHERE `entry` =4285 AND `gold_min` =167;

-- Scarlet Disciple
 UPDATE `creature_template` SET `gold_max` =509 WHERE `entry` =4285 AND `gold_max` =223;

-- Scarlet Disciple
 UPDATE `creature_template` SET `gold_min` =223 WHERE `entry` =4286 AND `gold_min` =227;

-- Scarlet Soldier
 UPDATE `creature_template` SET `gold_max` =787 WHERE `entry` =4286 AND `gold_max` =301;

-- Scarlet Soldier
 UPDATE `creature_template` SET `gold_min` =104 WHERE `entry` =4287 AND `gold_min` =185;

-- Scarlet Gallant (sample size 49)
 UPDATE `creature_template` SET `gold_max` =676 WHERE `entry` =4287 AND `gold_max` =246;

-- Scarlet Gallant (sample size 49)
 UPDATE `creature_template` SET `gold_max` =604 WHERE `entry` =4288 AND `gold_max` =287;

-- Scarlet Beastmaster
 UPDATE `creature_template` SET `gold_max` =786 WHERE `entry` =4289 AND `gold_max` =242;

-- Scarlet Evoker
 UPDATE `creature_template` SET `gold_min` =143 WHERE `entry` =4290 AND `gold_min` =232;

-- Scarlet Guardsman
 UPDATE `creature_template` SET `gold_max` =826 WHERE `entry` =4290 AND `gold_max` =308;

-- Scarlet Guardsman
 UPDATE `creature_template` SET `gold_min` =106 WHERE `entry` =4291 AND `gold_min` =191;

-- Scarlet Diviner (sample size 35)
 UPDATE `creature_template` SET `gold_max` =689 WHERE `entry` =4291 AND `gold_max` =254;

-- Scarlet Diviner (sample size 35)
 UPDATE `creature_template` SET `gold_min` =133 WHERE `entry` =4292 AND `gold_min` =203;

-- Scarlet Protector
 UPDATE `creature_template` SET `gold_max` =815 WHERE `entry` =4292 AND `gold_max` =270;

-- Scarlet Protector
 UPDATE `creature_template` SET `gold_max` =504 WHERE `entry` =4293 AND `gold_max` =349;

-- Scarlet Scryer
 UPDATE `creature_template` SET `gold_min` =150 WHERE `entry` =4294 AND `gold_min` =187;

-- Scarlet Sorcerer
 UPDATE `creature_template` SET `gold_max` =905 WHERE `entry` =4294 AND `gold_max` =250;

-- Scarlet Sorcerer
 UPDATE `creature_template` SET `gold_min` =142 WHERE `entry` =4295 AND `gold_min` =243;

-- Scarlet Myrmidon (sample size 48)
 UPDATE `creature_template` SET `gold_max` =882 WHERE `entry` =4295 AND `gold_max` =322;

-- Scarlet Myrmidon (sample size 48)
 UPDATE `creature_template` SET `gold_min` =139 WHERE `entry` =4296 AND `gold_min` =185;

-- Scarlet Adept
 UPDATE `creature_template` SET `gold_max` =668 WHERE `entry` =4296 AND `gold_max` =246;

-- Scarlet Adept
 UPDATE `creature_template` SET `gold_max` =488 WHERE `entry` =4297 AND `gold_max` =245;

-- Scarlet Conjuror
 UPDATE `creature_template` SET `gold_min` =206 WHERE `entry` =4298 AND `gold_min` =243;

-- Scarlet Defender (sample size 45)
 UPDATE `creature_template` SET `gold_max` =864 WHERE `entry` =4298 AND `gold_max` =322;

-- Scarlet Defender (sample size 45)
 UPDATE `creature_template` SET `gold_min` =117 WHERE `entry` =4299 AND `gold_min` =195;

-- Scarlet Chaplain (sample size 43)
 UPDATE `creature_template` SET `gold_max` =674 WHERE `entry` =4299 AND `gold_max` =260;

-- Scarlet Chaplain (sample size 43)
 UPDATE `creature_template` SET `gold_min` =137 WHERE `entry` =4300 AND `gold_min` =223;

-- Scarlet Wizard (sample size 41)
 UPDATE `creature_template` SET `gold_max` =923 WHERE `entry` =4300 AND `gold_max` =296;

-- Scarlet Wizard (sample size 41)
 UPDATE `creature_template` SET `gold_min` =151 WHERE `entry` =4301 AND `gold_min` =253;

-- Scarlet Centurion (sample size 63)
 UPDATE `creature_template` SET `gold_max` =899 WHERE `entry` =4301 AND `gold_max` =335;

-- Scarlet Centurion (sample size 63)
 UPDATE `creature_template` SET `gold_min` =209 WHERE `entry` =4302 AND `gold_min` =216;

-- Scarlet Champion (sample size 27)
 UPDATE `creature_template` SET `gold_max` =1008 WHERE `entry` =4302 AND `gold_max` =287;

-- Scarlet Champion (sample size 27)
 UPDATE `creature_template` SET `gold_max` =984 WHERE `entry` =4303 AND `gold_max` =298;

-- Scarlet Abbot
 UPDATE `creature_template` SET `gold_min` =81 WHERE `entry` =4306 AND `gold_min` =184;

-- Scarlet Torturer
 UPDATE `creature_template` SET `gold_max` =542 WHERE `entry` =4306 AND `gold_max` =245;

-- Scarlet Torturer
 UPDATE `creature_template` SET `gold_min` =29 WHERE `entry` =4308 AND `gold_min` =49;

-- Unfettered Spirit (sample size 87)
 UPDATE `creature_template` SET `gold_max` =196 WHERE `entry` =4308 AND `gold_max` =69;

-- Unfettered Spirit (sample size 87)
 UPDATE `creature_template` SET `gold_min` =54 WHERE `entry` =4323 AND `gold_min` =74;

-- Searing Hatchling (sample size 71)
 UPDATE `creature_template` SET `gold_max` =372 WHERE `entry` =4323 AND `gold_max` =102;

-- Searing Hatchling (sample size 71)
 UPDATE `creature_template` SET `gold_min` =63 WHERE `entry` =4324 AND `gold_min` =67;

-- Searing Whelp (sample size 63)
 UPDATE `creature_template` SET `gold_max` =399 WHERE `entry` =4324 AND `gold_max` =92;

-- Searing Whelp (sample size 63)
 UPDATE `creature_template` SET `gold_min` =69 WHERE `entry` =4328 AND `gold_min` =72;

-- Firemane Scalebane (sample size 26)
 UPDATE `creature_template` SET `gold_max` =426 WHERE `entry` =4328 AND `gold_max` =99;

-- Firemane Scalebane (sample size 26)
 UPDATE `creature_template` SET `gold_min` =59 WHERE `entry` =4329 AND `gold_min` =74;

-- Firemane Scout (sample size 42)
 UPDATE `creature_template` SET `gold_max` =381 WHERE `entry` =4329 AND `gold_max` =102;

-- Firemane Scout (sample size 42)
 UPDATE `creature_template` SET `gold_min` =69 WHERE `entry` =4331 AND `gold_min` =67;

-- Firemane Ash Tail (sample size 34)
 UPDATE `creature_template` SET `gold_max` =392 WHERE `entry` =4331 AND `gold_max` =92;

-- Firemane Ash Tail (sample size 34)
 UPDATE `creature_template` SET `gold_min` =69 WHERE `entry` =4334 AND `gold_min` =72;

-- Firemane Flamecaller
 UPDATE `creature_template` SET `gold_max` =409 WHERE `entry` =4334 AND `gold_max` =99;

-- Firemane Flamecaller
 UPDATE `creature_template` SET `gold_max` =249 WHERE `entry` =4358 AND `gold_max` =145;

-- Mirefin Puddlejumper
 UPDATE `creature_template` SET `gold_max` =259 WHERE `entry` =4359 AND `gold_max` =81;

-- Mirefin Murloc
 UPDATE `creature_template` SET `gold_min` =55 WHERE `entry` =4360 AND `gold_min` =29;

-- Mirefin Warrior (sample size 21)
 UPDATE `creature_template` SET `gold_max` =300 WHERE `entry` =4360 AND `gold_max` =145;

-- Mirefin Warrior (sample size 21)
 UPDATE `creature_template` SET `gold_min` =44 WHERE `entry` =4361 AND `gold_min` =62;

-- Mirefin Muckdweller (sample size 48)
 UPDATE `creature_template` SET `gold_max` =276 WHERE `entry` =4361 AND `gold_max` =85;

-- Mirefin Muckdweller (sample size 48)
 UPDATE `creature_template` SET `gold_min` =46 WHERE `entry` =4362 AND `gold_min` =59;

-- Mirefin Coastrunner (sample size 37)
 UPDATE `creature_template` SET `gold_max` =266 WHERE `entry` =4362 AND `gold_max` =83;

-- Mirefin Coastrunner (sample size 37)
 UPDATE `creature_template` SET `gold_min` =48 WHERE `entry` =4363 AND `gold_min` =56;

-- Mirefin Oracle (sample size 28)
 UPDATE `creature_template` SET `gold_max` =302 WHERE `entry` =4363 AND `gold_max` =78;

-- Mirefin Oracle (sample size 28)
 UPDATE `creature_template` SET `gold_max` =2054 WHERE `entry` =4364 AND `gold_max` =654;

-- Strashaz Warrior
 UPDATE `creature_template` SET `gold_max` =2915 WHERE `entry` =4366 AND `gold_max` =659;

-- Strashaz Serpent Guard
 UPDATE `creature_template` SET `gold_max` =2807 WHERE `entry` =4368 AND `gold_max` =661;

-- Strashaz Myrmidon
 UPDATE `creature_template` SET `gold_max` =3087 WHERE `entry` =4370 AND `gold_max` =562;

-- Strashaz Sorceress
 UPDATE `creature_template` SET `gold_max` =2727 WHERE `entry` =4371 AND `gold_max` =543;

-- Strashaz Siren
 UPDATE `creature_template` SET `gold_max` =164 WHERE `entry` =4382 AND `gold_max` =81;

-- Withervine Creeper
 UPDATE `creature_template` SET `gold_max` =190 WHERE `entry` =4386 AND `gold_max` =145;

-- Withervine Bark Ripper
 UPDATE `creature_template` SET `gold_max` =186 WHERE `entry` =4391 AND `gold_max` =138;

-- Swamp Ooze
 UPDATE `creature_template` SET `gold_max` =289 WHERE `entry` =4392 AND `gold_max` =217;

-- Corrosive Swamp Ooze
 UPDATE `creature_template` SET `gold_min` =64 WHERE `entry` =4393 AND `gold_min` =71;

-- Acidic Swamp Ooze
 UPDATE `creature_template` SET `gold_max` =318 WHERE `entry` =4401 AND `gold_max` =91;

-- Muckshell Clacker
 UPDATE `creature_template` SET `gold_max` =215 WHERE `entry` =4402 AND `gold_max` =205;

-- Muckshell Snapclaw
 UPDATE `creature_template` SET `gold_max` =357 WHERE `entry` =4404 AND `gold_max` =95;

-- Muckshell Scrabbler
 UPDATE `creature_template` SET `gold_max` =375 WHERE `entry` =4405 AND `gold_max` =222;

-- Muckshell Razorclaw
 UPDATE `creature_template` SET `gold_max` =339 WHERE `entry` =4409 AND `gold_max` =173;

-- Gatekeeper Kordurus
 UPDATE `creature_template` SET `gold_min` =8 WHERE `entry` =4416 AND `gold_min` =24;

-- Defias Strip Miner (sample size 28)
 UPDATE `creature_template` SET `gold_max` =56 WHERE `entry` =4416 AND `gold_max` =36;

-- Defias Strip Miner (sample size 28)
 UPDATE `creature_template` SET `gold_min` =66 WHERE `entry` =4417 AND `gold_min` =83;

-- Defias Taskmaster
 UPDATE `creature_template` SET `gold_max` =171 WHERE `entry` =4417 AND `gold_max` =113;

-- Defias Taskmaster
 UPDATE `creature_template` SET `gold_max` =169 WHERE `entry` =4418 AND `gold_max` =108;

-- Defias Wizard
 UPDATE `creature_template` SET `gold_min` =156 WHERE `entry` =4420 AND `gold_min` =366;

-- Overlord Ramtusk
 UPDATE `creature_template` SET `gold_min` =328 WHERE `entry` =4421 AND `gold_min` =457;

-- Charlga Razorflank
 UPDATE `creature_template` SET `gold_min` =116 WHERE `entry` =4424 AND `gold_min` =227;

-- Aggem Thorncurse
 UPDATE `creature_template` SET `gold_max` =356 WHERE `entry` =4424 AND `gold_max` =301;

-- Aggem Thorncurse
 UPDATE `creature_template` SET `gold_min` =226 WHERE `entry` =4428 AND `gold_min` =227;

-- Death Speaker Jargba
 UPDATE `creature_template` SET `gold_min` =94 WHERE `entry` =4435 AND `gold_min` =136;

-- Razorfen Warrior
 UPDATE `creature_template` SET `gold_max` =350 WHERE `entry` =4436 AND `gold_max` =184;

-- Razorfen Quilguard
 UPDATE `creature_template` SET `gold_min` =112 WHERE `entry` =4437 AND `gold_min` =133;

-- Razorfen Warden
 UPDATE `creature_template` SET `gold_max` =459 WHERE `entry` =4438 AND `gold_max` =222;

-- Razorfen Spearhide
 UPDATE `creature_template` SET `gold_max` =284 WHERE `entry` =4440 AND `gold_max` =199;

-- Razorfen Totemic
 UPDATE `creature_template` SET `gold_max` =400 WHERE `entry` =4442 AND `gold_max` =205;

-- Razorfen Defender
 UPDATE `creature_template` SET `gold_max` =159 WHERE `entry` =4458 AND `gold_max` =87;

-- Murkgill Hunter
 UPDATE `creature_template` SET `gold_max` =200 WHERE `entry` =4461 AND `gold_max` =87;

-- Murkgill Warrior
 UPDATE `creature_template` SET `gold_min` =15 WHERE `entry` =4463 AND `gold_min` =27;

-- Blackrock Summoner
 UPDATE `creature_template` SET `gold_max` =82 WHERE `entry` =4463 AND `gold_max` =40;

-- Blackrock Summoner
 UPDATE `creature_template` SET `gold_min` =285 WHERE `entry` =4465 AND `gold_min` =316;

-- Vilebranch Warrior
 UPDATE `creature_template` SET `gold_max` =1292 WHERE `entry` =4465 AND `gold_max` =417;

-- Vilebranch Warrior
 UPDATE `creature_template` SET `gold_min` =76 WHERE `entry` =4466 AND `gold_min` =89;

-- Vilebranch Scalper (sample size 72)
 UPDATE `creature_template` SET `gold_max` =507 WHERE `entry` =4466 AND `gold_max` =121;

-- Vilebranch Scalper (sample size 72)
 UPDATE `creature_template` SET `gold_min` =80 WHERE `entry` =4467 AND `gold_min` =74;

-- Vilebranch Soothsayer (sample size 32)
 UPDATE `creature_template` SET `gold_max` =505 WHERE `entry` =4467 AND `gold_max` =102;

-- Vilebranch Soothsayer (sample size 32)
 UPDATE `creature_template` SET `gold_min` =165 WHERE `entry` =4468 AND `gold_min` =340;

-- Jade Sludge
 UPDATE `creature_template` SET `gold_max` =890 WHERE `entry` =4468 AND `gold_max` =449;

-- Jade Sludge
 UPDATE `creature_template` SET `gold_min` =221 WHERE `entry` =4469 AND `gold_min` =283;

-- Emerald Ooze
 UPDATE `creature_template` SET `gold_max` =762 WHERE `entry` =4469 AND `gold_max` =375;

-- Emerald Ooze
 UPDATE `creature_template` SET `gold_max` =907 WHERE `entry` =4472 AND `gold_max` =153;

-- Haunting Vision
 UPDATE `creature_template` SET `gold_min` =113 WHERE `entry` =4474 AND `gold_min` =114;

-- Rotting Cadaver
 UPDATE `creature_template` SET `gold_max` =736 WHERE `entry` =4474 AND `gold_max` =154;

-- Rotting Cadaver
 UPDATE `creature_template` SET `gold_max` =686 WHERE `entry` =4475 AND `gold_max` =145;

-- Blighted Zombie
 UPDATE `creature_template` SET `gold_max` =155 WHERE `entry` =4479 AND `gold_max` =71;

-- Fardel Dabyrie
 UPDATE `creature_template` SET `gold_max` =280 WHERE `entry` =4480 AND `gold_max` =160;

-- Kenata Dabyrie
 UPDATE `creature_template` SET `gold_max` =244 WHERE `entry` =4481 AND `gold_max` =75;

-- Marcel Dabyrie
 UPDATE `creature_template` SET `gold_max` =823 WHERE `entry` =4493 AND `gold_max` =169;

-- Scarlet Avenger
 UPDATE `creature_template` SET `gold_max` =845 WHERE `entry` =4494 AND `gold_max` =142;

-- Scarlet Spellbinder
 UPDATE `creature_template` SET `gold_max` =270 WHERE `entry` =4504 AND `gold_max` =145;

-- Frostmaw
 UPDATE `creature_template` SET `gold_min` =67 WHERE `entry` =4505 AND `gold_min` =78;

-- Bloodsail Deckhand (sample size 26)
 UPDATE `creature_template` SET `gold_max` =422 WHERE `entry` =4505 AND `gold_max` =107;

-- Bloodsail Deckhand (sample size 26)
 UPDATE `creature_template` SET `gold_min` =71 WHERE `entry` =4506 AND `gold_min` =80;

-- Bloodsail Swabby (sample size 22)
 UPDATE `creature_template` SET `gold_max` =424 WHERE `entry` =4506 AND `gold_max` =110;

-- Bloodsail Swabby (sample size 22)
 UPDATE `creature_template` SET `gold_max` =378 WHERE `entry` =4515 AND `gold_max` =196;

-- Death's Head Acolyte
 UPDATE `creature_template` SET `gold_min` =128 WHERE `entry` =4516 AND `gold_min` =162;

-- Death's Head Adept
 UPDATE `creature_template` SET `gold_min` =74 WHERE `entry` =4517 AND `gold_min` =130;

-- Death's Head Priest
 UPDATE `creature_template` SET `gold_max` =298 WHERE `entry` =4517 AND `gold_max` =175;

-- Death's Head Priest
 UPDATE `creature_template` SET `gold_min` =150 WHERE `entry` =4518 AND `gold_min` =157;

-- Death's Head Sage
 UPDATE `creature_template` SET `gold_max` =424 WHERE `entry` =4518 AND `gold_max` =210;

-- Death's Head Sage
 UPDATE `creature_template` SET `gold_max` =373 WHERE `entry` =4519 AND `gold_max` =196;

-- Death's Head Seer
 UPDATE `creature_template` SET `gold_min` =86 WHERE `entry` =4520 AND `gold_min` =120;

-- Razorfen Geomancer
 UPDATE `creature_template` SET `gold_max` =228 WHERE `entry` =4520 AND `gold_max` =162;

-- Razorfen Geomancer
 UPDATE `creature_template` SET `gold_min` =103 WHERE `entry` =4523 AND `gold_min` =141;

-- Razorfen Groundshaker
 UPDATE `creature_template` SET `gold_max` =351 WHERE `entry` =4523 AND `gold_max` =189;

-- Razorfen Groundshaker
 UPDATE `creature_template` SET `gold_max` =355 WHERE `entry` =4525 AND `gold_max` =217;

-- Razorfen Earthbreaker
 UPDATE `creature_template` SET `gold_min` =117 WHERE `entry` =4530 AND `gold_min` =137;

-- Razorfen Handler
 UPDATE `creature_template` SET `gold_max` =314 WHERE `entry` =4531 AND `gold_max` =219;

-- Razorfen Beast Trainer
 UPDATE `creature_template` SET `gold_min` =135 WHERE `entry` =4532 AND `gold_min` =184;

-- Razorfen Beastmaster
 UPDATE `creature_template` SET `gold_max` =410 WHERE `entry` =4532 AND `gold_max` =245;

-- Razorfen Beastmaster
 UPDATE `creature_template` SET `gold_min` =125 WHERE `entry` =4540 AND `gold_min` =227;

-- Scarlet Monk (sample size 63)
 UPDATE `creature_template` SET `gold_max` =791 WHERE `entry` =4540 AND `gold_max` =301;

-- Scarlet Monk (sample size 63)
 UPDATE `creature_template` SET `gold_min` =96 WHERE `entry` =4541 AND `gold_min` =153;

-- Blood of Agamaggan
 UPDATE `creature_template` SET `gold_min` =172 WHERE `entry` =4542 AND `gold_min` =329;

-- High Inquisitor Fairbanks
 UPDATE `creature_template` SET `gold_max` =497 WHERE `entry` =4542 AND `gold_max` =434;

-- High Inquisitor Fairbanks
 UPDATE `creature_template` SET `gold_min` =121 WHERE `entry` =4543 AND `gold_min` =386;

-- Bloodmage Thalnos
 UPDATE `creature_template` SET `gold_max` =720 WHERE `entry` =4543 AND `gold_max` =509;

-- Bloodmage Thalnos
 UPDATE `creature_template` SET `gold_min` =48 WHERE `entry` =4619 AND `gold_min` =50;

-- Geltharis
 UPDATE `creature_template` SET `gold_max` =134 WHERE `entry` =4619 AND `gold_max` =70;

-- Geltharis
 UPDATE `creature_template` SET `gold_min` =137 WHERE `entry` =4623 AND `gold_min` =180;

-- Quilguard Champion
 UPDATE `creature_template` SET `gold_max` =506 WHERE `entry` =4623 AND `gold_max` =240;

-- Quilguard Champion
 UPDATE `creature_template` SET `gold_min` =28 WHERE `entry` =4632 AND `gold_min` =46;

-- Kolkar Centaur (sample size 23)
 UPDATE `creature_template` SET `gold_max` =163 WHERE `entry` =4632 AND `gold_max` =65;

-- Kolkar Centaur (sample size 23)
 UPDATE `creature_template` SET `gold_min` =34 WHERE `entry` =4633 AND `gold_min` =46;

-- Kolkar Scout (sample size 21)
 UPDATE `creature_template` SET `gold_max` =182 WHERE `entry` =4633 AND `gold_max` =65;

-- Kolkar Scout (sample size 21)
 UPDATE `creature_template` SET `gold_min` =45 WHERE `entry` =4634 AND `gold_min` =49;

-- Kolkar Mauler (sample size 21)
 UPDATE `creature_template` SET `gold_max` =186 WHERE `entry` =4634 AND `gold_max` =69;

-- Kolkar Mauler (sample size 21)
 UPDATE `creature_template` SET `gold_min` =32 WHERE `entry` =4635 AND `gold_min` =44;

-- Kolkar Windchaser
 UPDATE `creature_template` SET `gold_max` =189 WHERE `entry` =4635 AND `gold_max` =63;

-- Kolkar Windchaser
 UPDATE `creature_template` SET `gold_max` =202 WHERE `entry` =4636 AND `gold_max` =70;

-- Kolkar Battle Lord
 UPDATE `creature_template` SET `gold_max` =174 WHERE `entry` =4637 AND `gold_max` =65;

-- Kolkar Destroyer
 UPDATE `creature_template` SET `gold_min` =35 WHERE `entry` =4638 AND `gold_min` =50;

-- Magram Scout
 UPDATE `creature_template` SET `gold_max` =185 WHERE `entry` =4638 AND `gold_max` =70;

-- Magram Scout
 UPDATE `creature_template` SET `gold_min` =31 WHERE `entry` =4639 AND `gold_min` =50;

-- Magram Outrunner
 UPDATE `creature_template` SET `gold_max` =213 WHERE `entry` =4639 AND `gold_max` =70;

-- Magram Outrunner
 UPDATE `creature_template` SET `gold_min` =34 WHERE `entry` =4640 AND `gold_min` =53;

-- Magram Wrangler (sample size 23)
 UPDATE `creature_template` SET `gold_max` =227 WHERE `entry` =4640 AND `gold_max` =74;

-- Magram Wrangler (sample size 23)
 UPDATE `creature_template` SET `gold_min` =36 WHERE `entry` =4641 AND `gold_min` =48;

-- Magram Windchaser
 UPDATE `creature_template` SET `gold_max` =167 WHERE `entry` =4641 AND `gold_max` =68;

-- Magram Windchaser
 UPDATE `creature_template` SET `gold_min` =43 WHERE `entry` =4642 AND `gold_min` =49;

-- Magram Stormer
 UPDATE `creature_template` SET `gold_max` =246 WHERE `entry` =4642 AND `gold_max` =69;

-- Magram Stormer
 UPDATE `creature_template` SET `gold_min` =41 WHERE `entry` =4644 AND `gold_min` =59;

-- Magram Marauder
 UPDATE `creature_template` SET `gold_max` =244 WHERE `entry` =4644 AND `gold_max` =81;

-- Magram Marauder
 UPDATE `creature_template` SET `gold_max` =260 WHERE `entry` =4645 AND `gold_max` =81;

-- Magram Mauler
 UPDATE `creature_template` SET `gold_max` =210 WHERE `entry` =4646 AND `gold_max` =70;

-- Gelkis Outrunner
 UPDATE `creature_template` SET `gold_min` =31 WHERE `entry` =4647 AND `gold_min` =50;

-- Gelkis Scout
 UPDATE `creature_template` SET `gold_max` =213 WHERE `entry` =4647 AND `gold_max` =70;

-- Gelkis Scout
 UPDATE `creature_template` SET `gold_min` =41 WHERE `entry` =4648 AND `gold_min` =53;

-- Gelkis Stamper
 UPDATE `creature_template` SET `gold_max` =213 WHERE `entry` =4648 AND `gold_max` =74;

-- Gelkis Stamper
 UPDATE `creature_template` SET `gold_min` =42 WHERE `entry` =4649 AND `gold_min` =48;

-- Gelkis Windchaser (sample size 21)
 UPDATE `creature_template` SET `gold_max` =230 WHERE `entry` =4649 AND `gold_max` =68;

-- Gelkis Windchaser (sample size 21)
 UPDATE `creature_template` SET `gold_min` =58 WHERE `entry` =4651 AND `gold_min` =47;

-- Gelkis Earthcaller (sample size 21)
 UPDATE `creature_template` SET `gold_max` =238 WHERE `entry` =4651 AND `gold_max` =66;

-- Gelkis Earthcaller (sample size 21)
 UPDATE `creature_template` SET `gold_max` =245 WHERE `entry` =4652 AND `gold_max` =81;

-- Gelkis Mauler
 UPDATE `creature_template` SET `gold_max` =264 WHERE `entry` =4653 AND `gold_max` =81;

-- Gelkis Marauder
 UPDATE `creature_template` SET `gold_min` =44 WHERE `entry` =4654 AND `gold_min` =62;

-- Maraudine Scout
 UPDATE `creature_template` SET `gold_max` =296 WHERE `entry` =4654 AND `gold_max` =86;

-- Maraudine Scout
 UPDATE `creature_template` SET `gold_max` =157 WHERE `entry` =4655 AND `gold_max` =86;

-- Maraudine Wrangler
 UPDATE `creature_template` SET `gold_min` =56 WHERE `entry` =4656 AND `gold_min` =61;

-- Maraudine Mauler
 UPDATE `creature_template` SET `gold_max` =207 WHERE `entry` =4656 AND `gold_max` =85;

-- Maraudine Mauler
 UPDATE `creature_template` SET `gold_max` =268 WHERE `entry` =4657 AND `gold_max` =82;

-- Maraudine Windchaser
 UPDATE `creature_template` SET `gold_max` =238 WHERE `entry` =4658 AND `gold_max` =80;

-- Maraudine Stormer
 UPDATE `creature_template` SET `gold_max` =291 WHERE `entry` =4659 AND `gold_max` =94;

-- Maraudine Marauder
 UPDATE `creature_template` SET `gold_min` =33 WHERE `entry` =4663 AND `gold_min` =40;

-- Burning Blade Augur (sample size 21)
 UPDATE `creature_template` SET `gold_max` =176 WHERE `entry` =4663 AND `gold_max` =58;

-- Burning Blade Augur (sample size 21)
 UPDATE `creature_template` SET `gold_min` =34 WHERE `entry` =4664 AND `gold_min` =46;

-- Burning Blade Reaver (sample size 25)
 UPDATE `creature_template` SET `gold_max` =176 WHERE `entry` =4664 AND `gold_max` =65;

-- Burning Blade Reaver (sample size 25)
 UPDATE `creature_template` SET `gold_min` =40 WHERE `entry` =4665 AND `gold_min` =42;

-- Burning Blade Adept (sample size 23)
 UPDATE `creature_template` SET `gold_max` =193 WHERE `entry` =4665 AND `gold_max` =60;

-- Burning Blade Adept (sample size 23)
 UPDATE `creature_template` SET `gold_min` =30 WHERE `entry` =4666 AND `gold_min` =49;

-- Burning Blade Felsworn (sample size 47)
 UPDATE `creature_template` SET `gold_max` =197 WHERE `entry` =4666 AND `gold_max` =69;

-- Burning Blade Felsworn (sample size 47)
 UPDATE `creature_template` SET `gold_max` =193 WHERE `entry` =4667 AND `gold_max` =62;

-- Burning Blade Shadowmage
 UPDATE `creature_template` SET `gold_max` =201 WHERE `entry` =4668 AND `gold_max` =76;

-- Burning Blade Summoner
 UPDATE `creature_template` SET `gold_min` =28 WHERE `entry` =4670 AND `gold_min` =49;

-- Hatefury Rogue (sample size 23)
 UPDATE `creature_template` SET `gold_max` =196 WHERE `entry` =4670 AND `gold_max` =69;

-- Hatefury Rogue (sample size 23)
 UPDATE `creature_template` SET `gold_min` =28 WHERE `entry` =4671 AND `gold_min` =49;

-- Hatefury Trickster (sample size 35)
 UPDATE `creature_template` SET `gold_max` =194 WHERE `entry` =4671 AND `gold_max` =69;

-- Hatefury Trickster (sample size 35)
 UPDATE `creature_template` SET `gold_min` =29 WHERE `entry` =4672 AND `gold_min` =49;

-- Hatefury Felsworn (sample size 27)
 UPDATE `creature_template` SET `gold_max` =193 WHERE `entry` =4672 AND `gold_max` =69;

-- Hatefury Felsworn (sample size 27)
 UPDATE `creature_template` SET `gold_min` =30 WHERE `entry` =4673 AND `gold_min` =50;

-- Hatefury Betrayer (sample size 32)
 UPDATE `creature_template` SET `gold_max` =213 WHERE `entry` =4673 AND `gold_max` =70;

-- Hatefury Betrayer (sample size 32)
 UPDATE `creature_template` SET `gold_min` =42 WHERE `entry` =4674 AND `gold_min` =50;

-- Hatefury Shadowstalker (sample size 22)
 UPDATE `creature_template` SET `gold_max` =201 WHERE `entry` =4674 AND `gold_max` =70;

-- Hatefury Shadowstalker (sample size 22)
 UPDATE `creature_template` SET `gold_max` =211 WHERE `entry` =4675 AND `gold_max` =65;

-- Hatefury Hellcaller
 UPDATE `creature_template` SET `gold_min` =47 WHERE `entry` =4677 AND `gold_min` =62;

-- Doomwarder
 UPDATE `creature_template` SET `gold_max` =299 WHERE `entry` =4677 AND `gold_max` =86;

-- Doomwarder
 UPDATE `creature_template` SET `gold_max` =267 WHERE `entry` =4679 AND `gold_max` =78;

-- Nether Maiden
 UPDATE `creature_template` SET `gold_min` =54 WHERE `entry` =4680 AND `gold_min` =65;

-- Doomwarder Captain
 UPDATE `creature_template` SET `gold_max` =292 WHERE `entry` =4680 AND `gold_max` =90;

-- Doomwarder Captain
 UPDATE `creature_template` SET `gold_min` =59 WHERE `entry` =4682 AND `gold_min` =61;

-- Nether Sister
 UPDATE `creature_template` SET `gold_max` =230 WHERE `entry` =4682 AND `gold_max` =85;

-- Nether Sister
 UPDATE `creature_template` SET `gold_max` =335 WHERE `entry` =4684 AND `gold_max` =78;

-- Nether Sorceress
 UPDATE `creature_template` SET `gold_min` =93 WHERE `entry` =4686 AND `gold_min` =249;

-- Deepstrider Giant
 UPDATE `creature_template` SET `gold_max` =208 WHERE `entry` =4711 AND `gold_max` =70;

-- Slitherblade Naga (sample size 22)
 UPDATE `creature_template` SET `gold_max` =196 WHERE `entry` =4712 AND `gold_max` =62;

-- Slitherblade Sorceress
 UPDATE `creature_template` SET `gold_min` =43 WHERE `entry` =4713 AND `gold_min` =53;

-- Slitherblade Warrior
 UPDATE `creature_template` SET `gold_max` =229 WHERE `entry` =4713 AND `gold_max` =74;

-- Slitherblade Warrior
 UPDATE `creature_template` SET `gold_min` =36 WHERE `entry` =4714 AND `gold_min` =54;

-- Slitherblade Myrmidon (sample size 27)
 UPDATE `creature_template` SET `gold_max` =239 WHERE `entry` =4714 AND `gold_max` =75;

-- Slitherblade Myrmidon (sample size 27)
 UPDATE `creature_template` SET `gold_min` =40 WHERE `entry` =4715 AND `gold_min` =59;

-- Slitherblade Razortail (sample size 33)
 UPDATE `creature_template` SET `gold_max` =264 WHERE `entry` =4715 AND `gold_max` =81;

-- Slitherblade Razortail (sample size 33)
 UPDATE `creature_template` SET `gold_min` =42 WHERE `entry` =4716 AND `gold_min` =53;

-- Slitherblade Tidehunter
 UPDATE `creature_template` SET `gold_max` =257 WHERE `entry` =4716 AND `gold_max` =74;

-- Slitherblade Tidehunter
 UPDATE `creature_template` SET `gold_min` =39 WHERE `entry` =4718 AND `gold_min` =49;

-- Slitherblade Oracle
 UPDATE `creature_template` SET `gold_max` =237 WHERE `entry` =4718 AND `gold_max` =69;

-- Slitherblade Oracle
 UPDATE `creature_template` SET `gold_max` =259 WHERE `entry` =4719 AND `gold_max` =73;

-- Slitherblade Sea Witch (sample size 23)
 UPDATE `creature_template` SET `gold_max` =311 WHERE `entry` =4723 AND `gold_max` =79;

-- Foreman Cozzle
 UPDATE `creature_template` SET `gold_min` =35 WHERE `entry` =4788 AND `gold_min` =95;

-- Fallenroot Satyr
 UPDATE `creature_template` SET `gold_max` =203 WHERE `entry` =4788 AND `gold_max` =129;

-- Fallenroot Satyr
 UPDATE `creature_template` SET `gold_min` =95 WHERE `entry` =4789 AND `gold_min` =109;

-- Fallenroot Rogue
 UPDATE `creature_template` SET `gold_max` =227 WHERE `entry` =4789 AND `gold_max` =148;

-- Fallenroot Rogue
 UPDATE `creature_template` SET `gold_min` =48 WHERE `entry` =4798 AND `gold_min` =108;

-- Fallenroot Shadowstalker
 UPDATE `creature_template` SET `gold_max` =271 WHERE `entry` =4798 AND `gold_max` =146;

-- Fallenroot Shadowstalker
 UPDATE `creature_template` SET `gold_min` =65 WHERE `entry` =4799 AND `gold_min` =114;

-- Fallenroot Hellcaller
 UPDATE `creature_template` SET `gold_max` =225 WHERE `entry` =4799 AND `gold_max` =154;

-- Fallenroot Hellcaller
 UPDATE `creature_template` SET `gold_min` =41 WHERE `entry` =4802 AND `gold_min` =95;

-- Blackfathom Tide Priestess
 UPDATE `creature_template` SET `gold_min` =55 WHERE `entry` =4803 AND `gold_min` =101;

-- Blackfathom Oracle
 UPDATE `creature_template` SET `gold_max` =185 WHERE `entry` =4803 AND `gold_max` =136;

-- Blackfathom Oracle
 UPDATE `creature_template` SET `gold_min` =53 WHERE `entry` =4805 AND `gold_min` =113;

-- Blackfathom Sea Witch
 UPDATE `creature_template` SET `gold_max` =285 WHERE `entry` =4805 AND `gold_max` =152;

-- Blackfathom Sea Witch
 UPDATE `creature_template` SET `gold_min` =72 WHERE `entry` =4807 AND `gold_min` =153;

-- Blackfathom Myrmidon
 UPDATE `creature_template` SET `gold_max` =259 WHERE `entry` =4807 AND `gold_max` =205;

-- Blackfathom Myrmidon
 UPDATE `creature_template` SET `gold_max` =306 WHERE `entry` =4809 AND `gold_max` =160;

-- Twilight Acolyte
 UPDATE `creature_template` SET `gold_max` =311 WHERE `entry` =4811 AND `gold_max` =162;

-- Twilight Aquamancer
 UPDATE `creature_template` SET `gold_max` =298 WHERE `entry` =4812 AND `gold_max` =154;

-- Twilight Loreseeker
 UPDATE `creature_template` SET `gold_min` =103 WHERE `entry` =4813 AND `gold_min` =126;

-- Twilight Shadowmage
 UPDATE `creature_template` SET `gold_max` =385 WHERE `entry` =4813 AND `gold_max` =170;

-- Twilight Shadowmage
 UPDATE `creature_template` SET `gold_max` =270 WHERE `entry` =4814 AND `gold_max` =175;

-- Twilight Elementalist
 UPDATE `creature_template` SET `gold_min` =72 WHERE `entry` =4815 AND `gold_min` =115;

-- Murkshallow Snapclaw
 UPDATE `creature_template` SET `gold_max` =262 WHERE `entry` =4815 AND `gold_max` =155;

-- Murkshallow Snapclaw
 UPDATE `creature_template` SET `gold_min` =54 WHERE `entry` =4818 AND `gold_min` =111;

-- Blindlight Murloc
 UPDATE `creature_template` SET `gold_max` =262 WHERE `entry` =4818 AND `gold_max` =150;

-- Blindlight Murloc
 UPDATE `creature_template` SET `gold_min` =68 WHERE `entry` =4819 AND `gold_min` =137;

-- Blindlight Muckdweller
 UPDATE `creature_template` SET `gold_min` =76 WHERE `entry` =4820 AND `gold_min` =116;

-- Blindlight Oracle
 UPDATE `creature_template` SET `gold_max` =290 WHERE `entry` =4820 AND `gold_max` =156;

-- Blindlight Oracle
 UPDATE `creature_template` SET `gold_min` =143 WHERE `entry` =4831 AND `gold_min` =248;

-- Lady Sarevess
 UPDATE `creature_template` SET `gold_min` =43 WHERE `entry` =4834 AND `gold_min` =59;

-- Theramore Infiltrator (sample size 25)
 UPDATE `creature_template` SET `gold_max` =241 WHERE `entry` =4834 AND `gold_max` =81;

-- Theramore Infiltrator (sample size 25)
 UPDATE `creature_template` SET `gold_min` =142 WHERE `entry` =4844 AND `gold_min` =180;

-- Shadowforge Surveyor (sample size 40)
 UPDATE `creature_template` SET `gold_max` =776 WHERE `entry` =4844 AND `gold_max` =240;

-- Shadowforge Surveyor (sample size 40)
 UPDATE `creature_template` SET `gold_min` =211 WHERE `entry` =4845 AND `gold_min` =228;

-- Shadowforge Ruffian
 UPDATE `creature_template` SET `gold_max` =821 WHERE `entry` =4845 AND `gold_max` =303;

-- Shadowforge Ruffian
 UPDATE `creature_template` SET `gold_min` =138 WHERE `entry` =4846 AND `gold_min` =222;

-- Shadowforge Digger (sample size 60)
 UPDATE `creature_template` SET `gold_max` =774 WHERE `entry` =4846 AND `gold_max` =295;

-- Shadowforge Digger (sample size 60)
 UPDATE `creature_template` SET `gold_max` =930 WHERE `entry` =4848 AND `gold_max` =341;

-- Shadowforge Darkcaster
 UPDATE `creature_template` SET `gold_max` =867 WHERE `entry` =4850 AND `gold_max` =330;

-- Stonevault Cave Lurker
 UPDATE `creature_template` SET `gold_min` =216 WHERE `entry` =4851 AND `gold_min` =232;

-- Stonevault Rockchewer
 UPDATE `creature_template` SET `gold_max` =831 WHERE `entry` =4851 AND `gold_max` =308;

-- Stonevault Rockchewer
 UPDATE `creature_template` SET `gold_min` =133 WHERE `entry` =4852 AND `gold_min` =200;

-- Stonevault Oracle (sample size 30)
 UPDATE `creature_template` SET `gold_max` =887 WHERE `entry` =4852 AND `gold_max` =266;

-- Stonevault Oracle (sample size 30)
 UPDATE `creature_template` SET `gold_max` =911 WHERE `entry` =4853 AND `gold_max` =341;

-- Stonevault Geomancer
 UPDATE `creature_template` SET `gold_max` =1378 WHERE `entry` =4854 AND `gold_max` =512;

-- Grimlok
 UPDATE `creature_template` SET `gold_min` =247 WHERE `entry` =4855 AND `gold_min` =300;

-- Stonevault Brawler
 UPDATE `creature_template` SET `gold_max` =1195 WHERE `entry` =4855 AND `gold_max` =396;

-- Stonevault Brawler
 UPDATE `creature_template` SET `gold_max` =397 WHERE `entry` =4856 AND `gold_max` =303;

-- Stonevault Cave Hunter
 UPDATE `creature_template` SET `gold_max` =1455 WHERE `entry` =4857 AND `gold_max` =425;

-- Stone Keeper
 UPDATE `creature_template` SET `gold_min` =211 WHERE `entry` =4860 AND `gold_min` =297;

-- Stone Steward
 UPDATE `creature_template` SET `gold_max` =993 WHERE `entry` =4860 AND `gold_max` =393;

-- Stone Steward
 UPDATE `creature_template` SET `gold_min` =155 WHERE `entry` =4872 AND `gold_min` =249;

-- Obsidian Golem
 UPDATE `creature_template` SET `gold_max` =92 WHERE `entry` =4969 AND `gold_max` =63;

-- Old Town Thug
 UPDATE `creature_template` SET `gold_max` =278 WHERE `entry` =5044 AND `gold_max` =132;

-- Theramore Skirmisher
 UPDATE `creature_template` SET `gold_max` =255 WHERE `entry` =5046 AND `gold_max` =138;

-- Lieutenant Caldwell
 UPDATE `creature_template` SET `gold_min` =50 WHERE `entry` =5184 AND `gold_min` =61;

-- Theramore Sentry
 UPDATE `creature_template` SET `gold_max` =237 WHERE `entry` =5184 AND `gold_max` =84;

-- Theramore Sentry
 UPDATE `creature_template` SET `gold_min` =337 WHERE `entry` =5228 AND `gold_min` =349;

-- Saturated Ooze
 UPDATE `creature_template` SET `gold_max` =1182 WHERE `entry` =5228 AND `gold_max` =461;

-- Saturated Ooze
 UPDATE `creature_template` SET `gold_min` =72 WHERE `entry` =5229 AND `gold_min` =71;

-- Gordunni Ogre (sample size 23)
 UPDATE `creature_template` SET `gold_max` =322 WHERE `entry` =5229 AND `gold_max` =98;

-- Gordunni Ogre (sample size 23)
 UPDATE `creature_template` SET `gold_min` =62 WHERE `entry` =5232 AND `gold_min` =75;

-- Gordunni Brute (sample size 33)
 UPDATE `creature_template` SET `gold_max` =401 WHERE `entry` =5232 AND `gold_max` =103;

-- Gordunni Brute (sample size 33)
 UPDATE `creature_template` SET `gold_min` =71 WHERE `entry` =5234 AND `gold_min` =78;

-- Gordunni Mauler (sample size 31)
 UPDATE `creature_template` SET `gold_max` =422 WHERE `entry` =5234 AND `gold_max` =107;

-- Gordunni Mauler (sample size 31)
 UPDATE `creature_template` SET `gold_max` =785 WHERE `entry` =5235 AND `gold_max` =423;

-- Fungal Ooze
 UPDATE `creature_template` SET `gold_min` =88 WHERE `entry` =5236 AND `gold_min` =73;

-- Gordunni Shaman (sample size 23)
 UPDATE `creature_template` SET `gold_max` =414 WHERE `entry` =5236 AND `gold_max` =100;

-- Gordunni Shaman (sample size 23)
 UPDATE `creature_template` SET `gold_min` =60 WHERE `entry` =5237 AND `gold_min` =66;

-- Gordunni Ogre Mage (sample size 27)
 UPDATE `creature_template` SET `gold_max` =374 WHERE `entry` =5237 AND `gold_max` =91;

-- Gordunni Ogre Mage (sample size 27)
 UPDATE `creature_template` SET `gold_min` =76 WHERE `entry` =5238 AND `gold_min` =86;

-- Gordunni Battlemaster (sample size 28)
 UPDATE `creature_template` SET `gold_max` =475 WHERE `entry` =5238 AND `gold_max` =118;

-- Gordunni Battlemaster (sample size 28)
 UPDATE `creature_template` SET `gold_max` =467 WHERE `entry` =5239 AND `gold_max` =98;

-- Gordunni Mage-Lord
 UPDATE `creature_template` SET `gold_min` =62 WHERE `entry` =5240 AND `gold_min` =65;

-- Gordunni Warlock (sample size 38)
 UPDATE `creature_template` SET `gold_max` =421 WHERE `entry` =5240 AND `gold_max` =90;

-- Gordunni Warlock (sample size 38)
 UPDATE `creature_template` SET `gold_max` =455 WHERE `entry` =5241 AND `gold_max` =121;

-- Gordunni Warlord
 UPDATE `creature_template` SET `gold_max` =767 WHERE `entry` =5243 AND `gold_max` =423;

-- Cursed Atal'ai
 UPDATE `creature_template` SET `gold_min` =60 WHERE `entry` =5249 AND `gold_min` =71;

-- Woodpaw Mongrel (sample size 22)
 UPDATE `creature_template` SET `gold_max` =335 WHERE `entry` =5249 AND `gold_max` =98;

-- Woodpaw Mongrel (sample size 22)
 UPDATE `creature_template` SET `gold_min` =56 WHERE `entry` =5251 AND `gold_min` =74;

-- Woodpaw Trapper (sample size 29)
 UPDATE `creature_template` SET `gold_max` =361 WHERE `entry` =5251 AND `gold_max` =102;

-- Woodpaw Trapper (sample size 29)
 UPDATE `creature_template` SET `gold_min` =57 WHERE `entry` =5253 AND `gold_min` =74;

-- Woodpaw Brute (sample size 36)
 UPDATE `creature_template` SET `gold_max` =381 WHERE `entry` =5253 AND `gold_max` =102;

-- Woodpaw Brute (sample size 36)
 UPDATE `creature_template` SET `gold_min` =66 WHERE `entry` =5254 AND `gold_min` =67;

-- Woodpaw Mystic
 UPDATE `creature_template` SET `gold_max` =386 WHERE `entry` =5254 AND `gold_max` =92;

-- Woodpaw Mystic
 UPDATE `creature_template` SET `gold_min` =63 WHERE `entry` =5255 AND `gold_min` =75;

-- Woodpaw Reaver
 UPDATE `creature_template` SET `gold_max` =398 WHERE `entry` =5255 AND `gold_max` =103;

-- Woodpaw Reaver
 UPDATE `creature_template` SET `gold_max` =1621 WHERE `entry` =5256 AND `gold_max` =468;

-- Atal'ai Warrior
 UPDATE `creature_template` SET `gold_min` =74 WHERE `entry` =5258 AND `gold_min` =78;

-- Woodpaw Alpha
 UPDATE `creature_template` SET `gold_max` =376 WHERE `entry` =5258 AND `gold_max` =107;

-- Woodpaw Alpha
 UPDATE `creature_template` SET `gold_min` =262 WHERE `entry` =5259 AND `gold_min` =306;

-- Atal'ai Witch Doctor (sample size 28)
 UPDATE `creature_template` SET `gold_max` =1779 WHERE `entry` =5259 AND `gold_max` =404;

-- Atal'ai Witch Doctor (sample size 28)
 UPDATE `creature_template` SET `gold_max` =1033 WHERE `entry` =5261 AND `gold_max` =423;

-- Enthralled Atal'ai
 UPDATE `creature_template` SET `gold_max` =1450 WHERE `entry` =5263 AND `gold_max` =431;

-- Mummified Atal'ai
 UPDATE `creature_template` SET `gold_min` =259 WHERE `entry` =5267 AND `gold_min` =351;

-- Unliving Atal'ai
 UPDATE `creature_template` SET `gold_max` =1466 WHERE `entry` =5267 AND `gold_max` =463;

-- Unliving Atal'ai
 UPDATE `creature_template` SET `gold_max` =1240 WHERE `entry` =5269 AND `gold_max` =370;

-- Atal'ai Priest
 UPDATE `creature_template` SET `gold_max` =1672 WHERE `entry` =5270 AND `gold_max` =476;

-- Atal'ai Corpse Eater
 UPDATE `creature_template` SET `gold_max` =1760 WHERE `entry` =5271 AND `gold_max` =423;

-- Atal'ai Deathwalker
 UPDATE `creature_template` SET `gold_max` =1843 WHERE `entry` =5273 AND `gold_max` =423;

-- Atal'ai High Priest
 UPDATE `creature_template` SET `gold_max` =1824 WHERE `entry` =5277 AND `gold_max` =495;

-- Nightmare Scalebane
 UPDATE `creature_template` SET `gold_min` =57 WHERE `entry` =5278 AND `gold_min` =72;

-- Sprite Darter (sample size 92)
 UPDATE `creature_template` SET `gold_max` =405 WHERE `entry` =5278 AND `gold_max` =99;

-- Sprite Darter (sample size 92)
 UPDATE `creature_template` SET `gold_max` =1809 WHERE `entry` =5280 AND `gold_max` =423;

-- Nightmare Wyrmkin
 UPDATE `creature_template` SET `gold_min` =289 WHERE `entry` =5283 AND `gold_min` =361;

-- Nightmare Wanderer
 UPDATE `creature_template` SET `gold_max` =1196 WHERE `entry` =5283 AND `gold_max` =476;

-- Nightmare Wanderer
 UPDATE `creature_template` SET `gold_min` =65 WHERE `entry` =5292 AND `gold_min` =74;

-- Feral Scar Yeti (sample size 50)
 UPDATE `creature_template` SET `gold_max` =427 WHERE `entry` =5292 AND `gold_max` =101;

-- Feral Scar Yeti (sample size 50)
 UPDATE `creature_template` SET `gold_max` =455 WHERE `entry` =5293 AND `gold_max` =123;

-- Hulking Feral Scar
 UPDATE `creature_template` SET `gold_min` =72 WHERE `entry` =5295 AND `gold_min` =81;

-- Enraged Feral Scar (sample size 29)
 UPDATE `creature_template` SET `gold_max` =455 WHERE `entry` =5295 AND `gold_max` =111;

-- Enraged Feral Scar (sample size 29)
 UPDATE `creature_template` SET `gold_min` =84 WHERE `entry` =5296 AND `gold_min` =89;

-- Rage Scar Yeti
 UPDATE `creature_template` SET `gold_max` =419 WHERE `entry` =5296 AND `gold_max` =121;

-- Rage Scar Yeti
 UPDATE `creature_template` SET `gold_min` =107 WHERE `entry` =5297 AND `gold_min` =95;

-- Elder Rage Scar (sample size 22)
 UPDATE `creature_template` SET `gold_max` =557 WHERE `entry` =5297 AND `gold_max` =129;

-- Elder Rage Scar (sample size 22)
 UPDATE `creature_template` SET `gold_min` =77 WHERE `entry` =5299 AND `gold_min` =92;

-- Ferocious Rage Scar (sample size 32)
 UPDATE `creature_template` SET `gold_max` =533 WHERE `entry` =5299 AND `gold_max` =125;

-- Ferocious Rage Scar (sample size 32)
 UPDATE `creature_template` SET `gold_max` =1619 WHERE `entry` =5357 AND `gold_max` =324;

-- Land Walker
 UPDATE `creature_template` SET `gold_max` =1394 WHERE `entry` =5358 AND `gold_max` =326;

-- Cliff Giant
 UPDATE `creature_template` SET `gold_max` =571 WHERE `entry` =5362 AND `gold_max` =113;

-- Northspring Harpy
 UPDATE `creature_template` SET `gold_max` =394 WHERE `entry` =5363 AND `gold_max` =113;

-- Northspring Roguefeather
 UPDATE `creature_template` SET `gold_max` =573 WHERE `entry` =5364 AND `gold_max` =116;

-- Northspring Slayer
 UPDATE `creature_template` SET `gold_min` =121 WHERE `entry` =5366 AND `gold_min` =85;

-- Northspring Windcaller (sample size 21)
 UPDATE `creature_template` SET `gold_max` =603 WHERE `entry` =5366 AND `gold_max` =116;

-- Northspring Windcaller (sample size 21)
 UPDATE `creature_template` SET `gold_max` =671 WHERE `entry` =5402 AND `gold_max` =534;

-- Khan Hratha
 UPDATE `creature_template` SET `gold_min` =155 WHERE `entry` =5469 AND `gold_min` =351;

-- Dune Smasher
 UPDATE `creature_template` SET `gold_min` =72 WHERE `entry` =5471 AND `gold_min` =88;

-- Dunemaul Ogre
 UPDATE `creature_template` SET `gold_max` =442 WHERE `entry` =5471 AND `gold_max` =120;

-- Dunemaul Ogre
 UPDATE `creature_template` SET `gold_min` =78 WHERE `entry` =5472 AND `gold_min` =89;

-- Dunemaul Enforcer (sample size 64)
 UPDATE `creature_template` SET `gold_max` =502 WHERE `entry` =5472 AND `gold_max` =121;

-- Dunemaul Enforcer (sample size 64)
 UPDATE `creature_template` SET `gold_max` =434 WHERE `entry` =5473 AND `gold_max` =108;

-- Dunemaul Ogre Mage
 UPDATE `creature_template` SET `gold_min` =79 WHERE `entry` =5474 AND `gold_min` =90;

-- Dunemaul Brute (sample size 69)
 UPDATE `creature_template` SET `gold_max` =540 WHERE `entry` =5474 AND `gold_max` =123;

-- Dunemaul Brute (sample size 69)
 UPDATE `creature_template` SET `gold_max` =515 WHERE `entry` =5475 AND `gold_max` =111;

-- Dunemaul Warlock
 UPDATE `creature_template` SET `gold_max` =202 WHERE `entry` =5477 AND `gold_max` =91;

-- Noboru the Cudgel
 UPDATE `creature_template` SET `gold_min` =86 WHERE `entry` =5481 AND `gold_min` =92;

-- Thistleshrub Dew Collector (sample size 67)
 UPDATE `creature_template` SET `gold_max` =528 WHERE `entry` =5481 AND `gold_max` =125;

-- Thistleshrub Dew Collector (sample size 67)
 UPDATE `creature_template` SET `gold_min` =87 WHERE `entry` =5485 AND `gold_min` =85;

-- Thistleshrub Rootshaper (sample size 54)
 UPDATE `creature_template` SET `gold_max` =603 WHERE `entry` =5485 AND `gold_max` =116;

-- Thistleshrub Rootshaper (sample size 54)
 UPDATE `creature_template` SET `gold_min` =85 WHERE `entry` =5490 AND `gold_min` =95;

-- Gnarled Thistleshrub (sample size 44)
 UPDATE `creature_template` SET `gold_max` =566 WHERE `entry` =5490 AND `gold_max` =129;

-- Gnarled Thistleshrub (sample size 44)
 UPDATE `creature_template` SET `gold_max` =221 WHERE `entry` =5600 AND `gold_max` =170;

-- Khan Dez'hepah
 UPDATE `creature_template` SET `gold_max` =238 WHERE `entry` =5601 AND `gold_max` =184;

-- Khan Jehn
 UPDATE `creature_template` SET `gold_min` =94 WHERE `entry` =5602 AND `gold_min` =150;

-- Khan Shaka
 UPDATE `creature_template` SET `gold_min` =61 WHERE `entry` =5615 AND `gold_min` =78;

-- Wastewander Rogue (sample size 42)
 UPDATE `creature_template` SET `gold_max` =412 WHERE `entry` =5615 AND `gold_max` =107;

-- Wastewander Rogue (sample size 42)
 UPDATE `creature_template` SET `gold_min` =54 WHERE `entry` =5616 AND `gold_min` =71;

-- Wastewander Thief (sample size 49)
 UPDATE `creature_template` SET `gold_max` =351 WHERE `entry` =5616 AND `gold_max` =98;

-- Wastewander Thief (sample size 49)
 UPDATE `creature_template` SET `gold_min` =69 WHERE `entry` =5617 AND `gold_min` =62;

-- Wastewander Shadow Mage (sample size 56)
 UPDATE `creature_template` SET `gold_max` =405 WHERE `entry` =5617 AND `gold_max` =86;

-- Wastewander Shadow Mage (sample size 56)
 UPDATE `creature_template` SET `gold_min` =60 WHERE `entry` =5618 AND `gold_min` =74;

-- Wastewander Bandit (sample size 59)
 UPDATE `creature_template` SET `gold_max` =380 WHERE `entry` =5618 AND `gold_max` =102;

-- Wastewander Bandit (sample size 59)
 UPDATE `creature_template` SET `gold_max` =237 WHERE `entry` =5622 AND `gold_max` =84;

-- Ongeku
 UPDATE `creature_template` SET `gold_min` =80 WHERE `entry` =5623 AND `gold_min` =83;

-- Wastewander Assassin (sample size 30)
 UPDATE `creature_template` SET `gold_max` =453 WHERE `entry` =5623 AND `gold_max` =114;

-- Wastewander Assassin (sample size 30)
 UPDATE `creature_template` SET `gold_max` =255 WHERE `entry` =5643 AND `gold_max` =74;

-- Tyranis Malem
 UPDATE `creature_template` SET `gold_max` =970 WHERE `entry` =5645 AND `gold_max` =383;

-- Sandfury Hideskinner
 UPDATE `creature_template` SET `gold_max` =878 WHERE `entry` =5647 AND `gold_max` =307;

-- Sandfury Firecaller
 UPDATE `creature_template` SET `gold_min` =221 WHERE `entry` =5648 AND `gold_min` =236;

-- Sandfury Shadowcaster (sample size 50)
 UPDATE `creature_template` SET `gold_max` =1282 WHERE `entry` =5648 AND `gold_max` =313;

-- Sandfury Shadowcaster (sample size 50)
 UPDATE `creature_template` SET `gold_min` =209 WHERE `entry` =5649 AND `gold_min` =306;

-- Sandfury Blood Drinker (sample size 110)
 UPDATE `creature_template` SET `gold_max` =1360 WHERE `entry` =5649 AND `gold_max` =404;

-- Sandfury Blood Drinker (sample size 110)
 UPDATE `creature_template` SET `gold_min` =258 WHERE `entry` =5650 AND `gold_min` =263;

-- Sandfury Witch Doctor (sample size 74)
 UPDATE `creature_template` SET `gold_max` =1346 WHERE `entry` =5650 AND `gold_max` =348;

-- Sandfury Witch Doctor (sample size 74)
 UPDATE `creature_template` SET `gold_min` =672 WHERE `entry` =5709 AND `gold_min` =1536;

-- Shade of Eranikus
 UPDATE `creature_template` SET `gold_max` =2430 WHERE `entry` =5709 AND `gold_max` =2010;

-- Shade of Eranikus
 UPDATE `creature_template` SET `gold_max` =1598 WHERE `entry` =5710 AND `gold_max` =676;

-- Jammal'an the Prophet
 UPDATE `creature_template` SET `gold_max` =1745 WHERE `entry` =5711 AND `gold_max` =452;

-- Ogom the Wretched
 UPDATE `creature_template` SET `gold_max` =1994 WHERE `entry` =5712 AND `gold_max` =625;

-- Zolo
 UPDATE `creature_template` SET `gold_max` =1058 WHERE `entry` =5713 AND `gold_max` =497;

-- Gasher
 UPDATE `creature_template` SET `gold_max` =1771 WHERE `entry` =5714 AND `gold_max` =497;

-- Loro
 UPDATE `creature_template` SET `gold_max` =1959 WHERE `entry` =5715 AND `gold_max` =583;

-- Hukku
 UPDATE `creature_template` SET `gold_min` =387 WHERE `entry` =5716 AND `gold_min` =850;

-- Zul'Lor
 UPDATE `creature_template` SET `gold_max` =1262 WHERE `entry` =5716 AND `gold_max` =1115;

-- Zul'Lor
 UPDATE `creature_template` SET `gold_max` =1628 WHERE `entry` =5717 AND `gold_max` =727;

-- Mijan
 UPDATE `creature_template` SET `gold_max` =1968 WHERE `entry` =5720 AND `gold_max` =620;

-- Weaver
 UPDATE `creature_template` SET `gold_max` =1765 WHERE `entry` =5721 AND `gold_max` =643;

-- Dreamscythe
 UPDATE `creature_template` SET `gold_max` =1475 WHERE `entry` =5722 AND `gold_max` =785;

-- Hazzas
 UPDATE `creature_template` SET `gold_min` =255 WHERE `entry` =5760 AND `gold_min` =261;

-- Lord Azrethoc
 UPDATE `creature_template` SET `gold_min` =29 WHERE `entry` =5761 AND `gold_min` =89;

-- Deviate Shambler
 UPDATE `creature_template` SET `gold_max` =178 WHERE `entry` =5761 AND `gold_max` =121;

-- Deviate Shambler
 UPDATE `creature_template` SET `gold_max` =354 WHERE `entry` =5771 AND `gold_max` =84;

-- Jugkar Grim'rod
 UPDATE `creature_template` SET `gold_min` =27 WHERE `entry` =5785 AND `gold_min` =53;

-- Sister Hatelash
 UPDATE `creature_template` SET `gold_min` =10 WHERE `entry` =5787 AND `gold_min` =13;

-- Enforcer Emilgund
 UPDATE `creature_template` SET `gold_max` =329 WHERE `entry` =5799 AND `gold_max` =247;

-- Hannah Bladeleaf
 UPDATE `creature_template` SET `gold_min` =93 WHERE `entry` =5800 AND `gold_min` =192;

-- Marcus Bel
 UPDATE `creature_template` SET `gold_min` =49 WHERE `entry` =5824 AND `gold_min` =72;

-- Captain Flat Tusk
 UPDATE `creature_template` SET `gold_max` =42 WHERE `entry` =5838 AND `gold_max` =31;

-- Brokespear
 UPDATE `creature_template` SET `gold_min` =85 WHERE `entry` =5839 AND `gold_min` =65;

-- Dark Iron Geologist (sample size 32)
 UPDATE `creature_template` SET `gold_max` =423 WHERE `entry` =5839 AND `gold_max` =90;

-- Dark Iron Geologist (sample size 32)
 UPDATE `creature_template` SET `gold_min` =83 WHERE `entry` =5840 AND `gold_min` =89;

-- Dark Iron Steamsmith (sample size 25)
 UPDATE `creature_template` SET `gold_max` =479 WHERE `entry` =5840 AND `gold_max` =121;

-- Dark Iron Steamsmith (sample size 25)
 UPDATE `creature_template` SET `gold_min` =101 WHERE `entry` =5843 AND `gold_min` =91;

-- Slave Worker (sample size 24)
 UPDATE `creature_template` SET `gold_max` =477 WHERE `entry` =5843 AND `gold_max` =123;

-- Slave Worker (sample size 24)
 UPDATE `creature_template` SET `gold_min` =70 WHERE `entry` =5844 AND `gold_min` =88;

-- Dark Iron Slaver (sample size 142)
 UPDATE `creature_template` SET `gold_max` =480 WHERE `entry` =5844 AND `gold_max` =120;

-- Dark Iron Slaver (sample size 142)
 UPDATE `creature_template` SET `gold_min` =78 WHERE `entry` =5846 AND `gold_min` =94;

-- Dark Iron Taskmaster (sample size 135)
 UPDATE `creature_template` SET `gold_max` =536 WHERE `entry` =5846 AND `gold_max` =128;

-- Dark Iron Taskmaster (sample size 135)
 UPDATE `creature_template` SET `gold_max` =67 WHERE `entry` =5847 AND `gold_max` =45;

-- Heggin Stonewhisker
 UPDATE `creature_template` SET `gold_min` =21 WHERE `entry` =5849 AND `gold_min` =32;

-- Digger Flameforge
 UPDATE `creature_template` SET `gold_min` =77 WHERE `entry` =5853 AND `gold_min` =88;

-- Tempered War Golem
 UPDATE `creature_template` SET `gold_max` =454 WHERE `entry` =5853 AND `gold_max` =120;

-- Tempered War Golem
 UPDATE `creature_template` SET `gold_max` =540 WHERE `entry` =5854 AND `gold_max` =125;

-- Heavy War Golem (sample size 90)
 UPDATE `creature_template` SET `gold_min` =235 WHERE `entry` =5860 AND `gold_min` =289;

-- Twilight Dark Shaman (sample size 21)
 UPDATE `creature_template` SET `gold_max` =1459 WHERE `entry` =5860 AND `gold_max` =383;

-- Twilight Dark Shaman (sample size 21)
 UPDATE `creature_template` SET `gold_min` =271 WHERE `entry` =5861 AND `gold_min` =351;

-- Twilight Fire Guard (sample size 24)
 UPDATE `creature_template` SET `gold_max` =1697 WHERE `entry` =5861 AND `gold_max` =463;

-- Twilight Fire Guard (sample size 24)
 UPDATE `creature_template` SET `gold_max` =1738 WHERE `entry` =5862 AND `gold_max` =376;

-- Twilight Geomancer
 UPDATE `creature_template` SET `gold_max` =456 WHERE `entry` =5881 AND `gold_max` =112;

-- Cursed Sycamore
 UPDATE `creature_template` SET `gold_max` =285 WHERE `entry` =5916 AND `gold_max` =266;

-- Sentinel Amarassan
 UPDATE `creature_template` SET `gold_min` =46 WHERE `entry` =5932 AND `gold_min` =161;

-- Taskmaster Whipfang
 UPDATE `creature_template` SET `gold_max` =433 WHERE `entry` =5974 AND `gold_max` =123;

-- Dreadmaul Ogre
 UPDATE `creature_template` SET `gold_max` =391 WHERE `entry` =5975 AND `gold_max` =99;

-- Dreadmaul Ogre Mage
 UPDATE `creature_template` SET `gold_max` =455 WHERE `entry` =5976 AND `gold_max` =124;

-- Dreadmaul Brute
 UPDATE `creature_template` SET `gold_max` =666 WHERE `entry` =5977 AND `gold_max` =154;

-- Dreadmaul Mauler
 UPDATE `creature_template` SET `gold_max` =420 WHERE `entry` =5979 AND `gold_max` =109;

-- Wretched Lost One
 UPDATE `creature_template` SET `gold_max` =644 WHERE `entry` =5981 AND `gold_max` =305;

-- Portal Seeker
 UPDATE `creature_template` SET `gold_min` =113 WHERE `entry` =6004 AND `gold_min` =108;

-- Shadowsworn Cultist (sample size 32)
 UPDATE `creature_template` SET `gold_max` =674 WHERE `entry` =6004 AND `gold_max` =146;

-- Shadowsworn Cultist (sample size 32)
 UPDATE `creature_template` SET `gold_max` =704 WHERE `entry` =6005 AND `gold_max` =150;

-- Shadowsworn Thug
 UPDATE `creature_template` SET `gold_min` =117 WHERE `entry` =6006 AND `gold_min` =96;

-- Shadowsworn Adept (sample size 36)
 UPDATE `creature_template` SET `gold_max` =706 WHERE `entry` =6006 AND `gold_max` =131;

-- Shadowsworn Adept (sample size 36)
 UPDATE `creature_template` SET `gold_max` =672 WHERE `entry` =6007 AND `gold_max` =151;

-- Shadowsworn Enforcer
 UPDATE `creature_template` SET `gold_max` =614 WHERE `entry` =6008 AND `gold_max` =124;

-- Shadowsworn Warlock
 UPDATE `creature_template` SET `gold_max` =503 WHERE `entry` =6009 AND `gold_max` =136;

-- Shadowsworn Dreadweaver
 UPDATE `creature_template` SET `gold_max` =166 WHERE `entry` =6011 AND `gold_max` =158;

-- Felguard Sentry
 UPDATE `creature_template` SET `gold_max` =43 WHERE `entry` =6020 AND `gold_max` =36;

-- Slimeshell Makrura
 UPDATE `creature_template` SET `gold_max` =354 WHERE `entry` =6035 AND `gold_max` =224;

-- Razorfen Stalker
 UPDATE `creature_template` SET `gold_max` =329 WHERE `entry` =6069 AND `gold_max` =145;

-- Maraudine Khan Guard
 UPDATE `creature_template` SET `gold_min` =85 WHERE `entry` =6116 AND `gold_min` =86;

-- Highborne Apparition
 UPDATE `creature_template` SET `gold_max` =482 WHERE `entry` =6116 AND `gold_max` =118;

-- Highborne Apparition
 UPDATE `creature_template` SET `gold_max` =477 WHERE `entry` =6117 AND `gold_max` =99;

-- Highborne Lichling
 UPDATE `creature_template` SET `gold_min` =5 WHERE `entry` =6123 AND `gold_min` =9;

-- Dark Iron Spy
 UPDATE `creature_template` SET `gold_min` =78 WHERE `entry` =6125 AND `gold_min` =86;

-- Haldarr Satyr
 UPDATE `creature_template` SET `gold_max` =411 WHERE `entry` =6125 AND `gold_max` =118;

-- Haldarr Satyr
 UPDATE `creature_template` SET `gold_max` =457 WHERE `entry` =6126 AND `gold_max` =120;

-- Haldarr Trickster
 UPDATE `creature_template` SET `gold_max` =396 WHERE `entry` =6127 AND `gold_max` =99;

-- Haldarr Felsworn
 UPDATE `creature_template` SET `gold_min` =101 WHERE `entry` =6132 AND `gold_min` =121;

-- Razorfen Servitor
 UPDATE `creature_template` SET `gold_min` =9 WHERE `entry` =6133 AND `gold_min` =11;

-- Shade of Elura
 UPDATE `creature_template` SET `gold_max` =733 WHERE `entry` =6135 AND `gold_max` =154;

-- Arkkoran Clacker
 UPDATE `creature_template` SET `gold_min` =110 WHERE `entry` =6136 AND `gold_min` =114;

-- Arkkoran Muckdweller
 UPDATE `creature_template` SET `gold_max` =408 WHERE `entry` =6136 AND `gold_max` =154;

-- Arkkoran Muckdweller
 UPDATE `creature_template` SET `gold_max` =729 WHERE `entry` =6137 AND `gold_max` =158;

-- Arkkoran Pincer
 UPDATE `creature_template` SET `gold_max` =765 WHERE `entry` =6138 AND `gold_max` =136;

-- Arkkoran Oracle
 UPDATE `creature_template` SET `gold_max` =1948 WHERE `entry` =6146 AND `gold_max` =559;

-- Cliff Breaker
 UPDATE `creature_template` SET `gold_max` =1922 WHERE `entry` =6147 AND `gold_max` =540;

-- Cliff Thunderer
 UPDATE `creature_template` SET `gold_max` =1788 WHERE `entry` =6148 AND `gold_max` =527;

-- Cliff Walker
 UPDATE `creature_template` SET `gold_min` =128 WHERE `entry` =6168 AND `gold_min` =142;

-- Roogug
 UPDATE `creature_template` SET `gold_min` =32 WHERE `entry` =6170 AND `gold_min` =45;

-- Gutspill
 UPDATE `creature_template` SET `gold_max` =130 WHERE `entry` =6170 AND `gold_max` =64;

-- Gutspill
 UPDATE `creature_template` SET `gold_max` =43 WHERE `entry` =6180 AND `gold_max` =24;

-- Defias Raider
 UPDATE `creature_template` SET `gold_min` =73 WHERE `entry` =6188 AND `gold_min` =86;

-- Timbermaw Shaman
 UPDATE `creature_template` SET `gold_max` =273 WHERE `entry` =6188 AND `gold_max` =118;

-- Timbermaw Shaman
 UPDATE `creature_template` SET `gold_max` =464 WHERE `entry` =6190 AND `gold_max` =124;

-- Spitelash Warrior
 UPDATE `creature_template` SET `gold_max` =539 WHERE `entry` =6194 AND `gold_max` =113;

-- Spitelash Serpent Guard
 UPDATE `creature_template` SET `gold_min` =122 WHERE `entry` =6195 AND `gold_min` =91;

-- Spitelash Siren (sample size 34)
 UPDATE `creature_template` SET `gold_max` =678 WHERE `entry` =6195 AND `gold_max` =124;

-- Spitelash Siren (sample size 34)
 UPDATE `creature_template` SET `gold_min` =102 WHERE `entry` =6196 AND `gold_min` =105;

-- Spitelash Myrmidon
 UPDATE `creature_template` SET `gold_max` =612 WHERE `entry` =6196 AND `gold_max` =143;

-- Spitelash Myrmidon
 UPDATE `creature_template` SET `gold_max` =682 WHERE `entry` =6198 AND `gold_max` =124;

-- Blood Elf Surveyor
 UPDATE `creature_template` SET `gold_max` =690 WHERE `entry` =6199 AND `gold_max` =120;

-- Blood Elf Reclaimer
 UPDATE `creature_template` SET `gold_min` =112 WHERE `entry` =6206 AND `gold_min` =154;

-- Caverndeep Burrower
 UPDATE `creature_template` SET `gold_max` =360 WHERE `entry` =6206 AND `gold_max` =206;

-- Caverndeep Burrower
 UPDATE `creature_template` SET `gold_max` =120 WHERE `entry` =6207 AND `gold_max` =50;

-- Caverndeep Ambusher
 UPDATE `creature_template` SET `gold_min` =120 WHERE `entry` =6208 AND `gold_min` =133;

-- Caverndeep Invader
 UPDATE `creature_template` SET `gold_max` =266 WHERE `entry` =6208 AND `gold_max` =179;

-- Caverndeep Invader
 UPDATE `creature_template` SET `gold_max` =326 WHERE `entry` =6210 AND `gold_max` =176;

-- Caverndeep Pillager
 UPDATE `creature_template` SET `gold_min` =68 WHERE `entry` =6211 AND `gold_min` =153;

-- Caverndeep Reaver
 UPDATE `creature_template` SET `gold_max` =431 WHERE `entry` =6211 AND `gold_max` =205;

-- Caverndeep Reaver
 UPDATE `creature_template` SET `gold_min` =125 WHERE `entry` =6212 AND `gold_min` =200;

-- Dark Iron Agent
 UPDATE `creature_template` SET `gold_max` =538 WHERE `entry` =6212 AND `gold_max` =266;

-- Dark Iron Agent
 UPDATE `creature_template` SET `gold_min` =105 WHERE `entry` =6213 AND `gold_min` =114;

-- Irradiated Invader
 UPDATE `creature_template` SET `gold_max` =340 WHERE `entry` =6213 AND `gold_max` =154;

-- Irradiated Invader
 UPDATE `creature_template` SET `gold_min` =29 WHERE `entry` =6218 AND `gold_min` =153;

-- Irradiated Slime
 UPDATE `creature_template` SET `gold_min` =16 WHERE `entry` =6221 AND `gold_min` =32;

-- Addled Leper (sample size 22)
 UPDATE `creature_template` SET `gold_max` =113 WHERE `entry` =6221 AND `gold_max` =47;

-- Addled Leper (sample size 22)
 UPDATE `creature_template` SET `gold_min` =32 WHERE `entry` =6222 AND `gold_min` =45;

-- Leprous Technician
 UPDATE `creature_template` SET `gold_max` =166 WHERE `entry` =6222 AND `gold_max` =64;

-- Leprous Technician
 UPDATE `creature_template` SET `gold_min` =119 WHERE `entry` =6223 AND `gold_min` =164;

-- Leprous Defender
 UPDATE `creature_template` SET `gold_max` =465 WHERE `entry` =6223 AND `gold_max` =219;

-- Leprous Defender
 UPDATE `creature_template` SET `gold_max` =169 WHERE `entry` =6224 AND `gold_max` =61;

-- Leprous Machinesmith
 UPDATE `creature_template` SET `gold_min` =125 WHERE `entry` =6225 AND `gold_min` =174;

-- Mechano-Tank
 UPDATE `creature_template` SET `gold_max` =365 WHERE `entry` =6225 AND `gold_max` =232;

-- Mechano-Tank
 UPDATE `creature_template` SET `gold_max` =511 WHERE `entry` =6226 AND `gold_max` =217;

-- Mechano-Flamewalker
 UPDATE `creature_template` SET `gold_min` =164 WHERE `entry` =6227 AND `gold_min` =169;

-- Mechano-Frostwalker
 UPDATE `creature_template` SET `gold_max` =480 WHERE `entry` =6227 AND `gold_max` =225;

-- Mechano-Frostwalker
 UPDATE `creature_template` SET `gold_min` =72 WHERE `entry` =6233 AND `gold_min` =202;

-- Mechanized Sentry
 UPDATE `creature_template` SET `gold_max` =444 WHERE `entry` =6233 AND `gold_max` =269;

-- Mechanized Sentry
 UPDATE `creature_template` SET `gold_min` =113 WHERE `entry` =6234 AND `gold_min` =190;

-- Mechanized Guardian
 UPDATE `creature_template` SET `gold_max` =515 WHERE `entry` =6234 AND `gold_max` =253;

-- Mechanized Guardian
 UPDATE `creature_template` SET `gold_min` =231 WHERE `entry` =6243 AND `gold_min` =232;

-- Gelihast
 UPDATE `creature_template` SET `gold_min` =69 WHERE `entry` =6329 AND `gold_min` =125;

-- Irradiated Pillager
 UPDATE `creature_template` SET `gold_max` =369 WHERE `entry` =6329 AND `gold_max` =168;

-- Irradiated Pillager
 UPDATE `creature_template` SET `gold_min` =116 WHERE `entry` =6350 AND `gold_min` =117;

-- Makrinni Razorclaw
 UPDATE `creature_template` SET `gold_min` =90 WHERE `entry` =6426 AND `gold_min` =198;

-- Anguished Dead (sample size 29)
 UPDATE `creature_template` SET `gold_max` =541 WHERE `entry` =6426 AND `gold_max` =264;

-- Anguished Dead (sample size 29)
 UPDATE `creature_template` SET `gold_min` =95 WHERE `entry` =6427 AND `gold_min` =196;

-- Haunting Phantasm (sample size 25)
 UPDATE `creature_template` SET `gold_max` =591 WHERE `entry` =6427 AND `gold_max` =261;

-- Haunting Phantasm (sample size 25)
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =6466 AND `gold_min` =25;

-- Gamon
 UPDATE `creature_template` SET `gold_min` =230 WHERE `entry` =6487 AND `gold_min` =434;

-- Arcanist Doan
 UPDATE `creature_template` SET `gold_max` =770 WHERE `entry` =6487 AND `gold_max` =572;

-- Arcanist Doan
 UPDATE `creature_template` SET `gold_max` =453 WHERE `entry` =6488 AND `gold_max` =394;

-- Fallen Champion
 UPDATE `creature_template` SET `gold_min` =182 WHERE `entry` =6489 AND `gold_min` =298;

-- Ironspine
 UPDATE `creature_template` SET `gold_max` =624 WHERE `entry` =6490 AND `gold_max` =314;

-- Azshir the Sleepless
 UPDATE `creature_template` SET `gold_max` =27 WHERE `entry` =6494 AND `gold_max` =23;

-- Tazan
 UPDATE `creature_template` SET `gold_min` =6 WHERE `entry` =6497 AND `gold_min` =19;

-- Astor Hadren
 UPDATE `creature_template` SET `gold_max` =643 WHERE `entry` =6517 AND `gold_max` =140;

-- Tar Beast (sample size 50)
 UPDATE `creature_template` SET `gold_min` =103 WHERE `entry` =6518 AND `gold_min` =111;

-- Tar Lurker (sample size 56)
 UPDATE `creature_template` SET `gold_max` =714 WHERE `entry` =6518 AND `gold_max` =150;

-- Tar Lurker (sample size 56)
 UPDATE `creature_template` SET `gold_min` =127 WHERE `entry` =6519 AND `gold_min` =99;

-- Tar Lord (sample size 54)
 UPDATE `creature_template` SET `gold_max` =742 WHERE `entry` =6519 AND `gold_max` =135;

-- Tar Lord (sample size 54)
 UPDATE `creature_template` SET `gold_min` =94 WHERE `entry` =6523 AND `gold_min` =153;

-- Dark Iron Rifleman
 UPDATE `creature_template` SET `gold_max` =357 WHERE `entry` =6523 AND `gold_max` =205;

-- Dark Iron Rifleman
 UPDATE `creature_template` SET `gold_min` =97 WHERE `entry` =6527 AND `gold_min` =91;

-- Tar Creeper (sample size 55)
 UPDATE `creature_template` SET `gold_max` =667 WHERE `entry` =6527 AND `gold_max` =124;

-- Tar Creeper (sample size 55)
 UPDATE `creature_template` SET `gold_min` =81 WHERE `entry` =6556 AND `gold_min` =95;

-- Muculent Ooze
 UPDATE `creature_template` SET `gold_max` =322 WHERE `entry` =6556 AND `gold_max` =129;

-- Muculent Ooze
 UPDATE `creature_template` SET `gold_min` =63 WHERE `entry` =6557 AND `gold_min` =103;

-- Primal Ooze (sample size 37)
 UPDATE `creature_template` SET `gold_max` =399 WHERE `entry` =6557 AND `gold_max` =140;

-- Primal Ooze (sample size 37)
 UPDATE `creature_template` SET `gold_min` =56 WHERE `entry` =6559 AND `gold_min` =113;

-- Glutinous Ooze (sample size 31)
 UPDATE `creature_template` SET `gold_max` =247 WHERE `entry` =6559 AND `gold_max` =153;

-- Glutinous Ooze (sample size 31)
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =6575 AND `gold_min` =13;

-- Scarlet Trainee (sample size 37)
 UPDATE `creature_template` SET `gold_max` =30 WHERE `entry` =6575 AND `gold_max` =63;

-- Scarlet Trainee (sample size 37)
 UPDATE `creature_template` SET `gold_min` =14 WHERE `entry` =6606 AND `gold_min` =18;

-- Overseer Glibby
 UPDATE `creature_template` SET `gold_max` =31 WHERE `entry` =6606 AND `gold_max` =29;

-- Overseer Glibby
 UPDATE `creature_template` SET `gold_max` =62 WHERE `entry` =6668 AND `gold_max` =43;

-- Lord Cyrik Blackforge
 UPDATE `creature_template` SET `gold_max` =947 WHERE `entry` =6733 AND `gold_max` =348;

-- Stonevault Basher
 UPDATE `creature_template` SET `gold_min` =2 WHERE `entry` =6846 AND `gold_min` =10;

-- Defias Dockmaster
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =6866 AND `gold_min` =5;

-- Defias Bodyguard
 UPDATE `creature_template` SET `gold_max` =1116 WHERE `entry` =6906 AND `gold_max` =358;

-- Baelog
 UPDATE `creature_template` SET `gold_min` =231 WHERE `entry` =6907 AND `gold_min` =265;

-- Eric "The Swift"
 UPDATE `creature_template` SET `gold_max` =676 WHERE `entry` =6907 AND `gold_max` =351;

-- Eric "The Swift"
 UPDATE `creature_template` SET `gold_max` =919 WHERE `entry` =6908 AND `gold_max` =390;

-- Olaf
 UPDATE `creature_template` SET `gold_max` =696 WHERE `entry` =6910 AND `gold_max` =301;

-- Revelosh
 UPDATE `creature_template` SET `gold_max` =304 WHERE `entry` =7011 AND `gold_max` =103;

-- Earthen Rocksmasher
 UPDATE `creature_template` SET `gold_max` =1087 WHERE `entry` =7012 AND `gold_max` =89;

-- Earthen Sculptor
 UPDATE `creature_template` SET `gold_max` =34 WHERE `entry` =7015 AND `gold_max` =29;

-- Flagglemurk the Cruel
 UPDATE `creature_template` SET `gold_max` =1162 WHERE `entry` =7023 AND `gold_max` =549;

-- Obsidian Sentinel
 UPDATE `creature_template` SET `gold_max` =822 WHERE `entry` =7025 AND `gold_max` =165;

-- Blackrock Soldier
 UPDATE `creature_template` SET `gold_max` =626 WHERE `entry` =7026 AND `gold_max` =134;

-- Blackrock Sorcerer
 UPDATE `creature_template` SET `gold_min` =124 WHERE `entry` =7027 AND `gold_min` =125;

-- Blackrock Slayer
 UPDATE `creature_template` SET `gold_max` =859 WHERE `entry` =7027 AND `gold_max` =169;

-- Blackrock Slayer
 UPDATE `creature_template` SET `gold_max` =792 WHERE `entry` =7028 AND `gold_max` =135;

-- Blackrock Warlock
 UPDATE `creature_template` SET `gold_max` =996 WHERE `entry` =7030 AND `gold_max` =306;

-- Shadowforge Geologist
 UPDATE `creature_template` SET `gold_min` =110 WHERE `entry` =7033 AND `gold_min` =114;

-- Firegut Ogre (sample size 53)
 UPDATE `creature_template` SET `gold_max` =636 WHERE `entry` =7033 AND `gold_max` =154;

-- Firegut Ogre (sample size 53)
 UPDATE `creature_template` SET `gold_min` =119 WHERE `entry` =7034 AND `gold_min` =100;

-- Firegut Ogre Mage (sample size 33)
 UPDATE `creature_template` SET `gold_max` =680 WHERE `entry` =7034 AND `gold_max` =135;

-- Firegut Ogre Mage (sample size 33)
 UPDATE `creature_template` SET `gold_min` =115 WHERE `entry` =7035 AND `gold_min` =118;

-- Firegut Brute (sample size 47)
 UPDATE `creature_template` SET `gold_max` =705 WHERE `entry` =7035 AND `gold_max` =159;

-- Firegut Brute (sample size 47)
 UPDATE `creature_template` SET `gold_max` =187 WHERE `entry` =7036 AND `gold_max` =154;

-- Thaurissan Spy
 UPDATE `creature_template` SET `gold_max` =209 WHERE `entry` =7037 AND `gold_max` =154;

-- Thaurissan Firewalker
 UPDATE `creature_template` SET `gold_max` =714 WHERE `entry` =7038 AND `gold_max` =158;

-- Thaurissan Agent
 UPDATE `creature_template` SET `gold_max` =2149 WHERE `entry` =7040 AND `gold_max` =527;

-- Black Dragonspawn
 UPDATE `creature_template` SET `gold_max` =2071 WHERE `entry` =7041 AND `gold_max` =413;

-- Black Wyrmkin
 UPDATE `creature_template` SET `gold_max` =1992 WHERE `entry` =7042 AND `gold_max` =592;

-- Flamescale Dragonspawn
 UPDATE `creature_template` SET `gold_max` =1210 WHERE `entry` =7044 AND `gold_max` =512;

-- Black Drake
 UPDATE `creature_template` SET `gold_min` =103 WHERE `entry` =7047 AND `gold_min` =93;

-- Black Broodling (sample size 26)
 UPDATE `creature_template` SET `gold_max` =651 WHERE `entry` =7047 AND `gold_max` =127;

-- Black Broodling (sample size 26)
 UPDATE `creature_template` SET `gold_max` =753 WHERE `entry` =7048 AND `gold_max` =126;

-- Scalding Broodling
 UPDATE `creature_template` SET `gold_min` =34 WHERE `entry` =7076 AND `gold_min` =52;

-- Earthen Guardian
 UPDATE `creature_template` SET `gold_max` =101 WHERE `entry` =7076 AND `gold_max` =72;

-- Earthen Guardian
 UPDATE `creature_template` SET `gold_min` =33 WHERE `entry` =7077 AND `gold_min` =51;

-- Earthen Hallshaper
 UPDATE `creature_template` SET `gold_min` =53 WHERE `entry` =7086 AND `gold_min` =96;

-- Cursed Ooze (sample size 21)
 UPDATE `creature_template` SET `gold_max` =295 WHERE `entry` =7086 AND `gold_max` =130;

-- Cursed Ooze (sample size 21)
 UPDATE `creature_template` SET `gold_max` =309 WHERE `entry` =7091 AND `gold_max` =110;

-- Shadowforge Ambusher
 UPDATE `creature_template` SET `gold_min` =52 WHERE `entry` =7092 AND `gold_min` =108;

-- Tainted Ooze
 UPDATE `creature_template` SET `gold_max` =296 WHERE `entry` =7092 AND `gold_max` =146;

-- Tainted Ooze
 UPDATE `creature_template` SET `gold_max` =423 WHERE `entry` =7093 AND `gold_max` =146;

-- Vile Ooze
 UPDATE `creature_template` SET `gold_min` =111 WHERE `entry` =7100 AND `gold_min` =109;

-- Warpwood Moss Flayer (sample size 40)
 UPDATE `creature_template` SET `gold_max` =712 WHERE `entry` =7100 AND `gold_max` =148;

-- Warpwood Moss Flayer (sample size 40)
 UPDATE `creature_template` SET `gold_min` =127 WHERE `entry` =7101 AND `gold_min` =112;

-- Warpwood Shredder (sample size 23)
 UPDATE `creature_template` SET `gold_max` =725 WHERE `entry` =7101 AND `gold_max` =151;

-- Warpwood Shredder (sample size 23)
 UPDATE `creature_template` SET `gold_min` =94 WHERE `entry` =7106 AND `gold_min` =103;

-- Jadefire Rogue
 UPDATE `creature_template` SET `gold_max` =495 WHERE `entry` =7106 AND `gold_max` =140;

-- Jadefire Rogue
 UPDATE `creature_template` SET `gold_min` =112 WHERE `entry` =7107 AND `gold_min` =94;

-- Jadefire Trickster (sample size 25)
 UPDATE `creature_template` SET `gold_max` =684 WHERE `entry` =7107 AND `gold_max` =128;

-- Jadefire Trickster (sample size 25)
 UPDATE `creature_template` SET `gold_max` =706 WHERE `entry` =7108 AND `gold_max` =148;

-- Jadefire Betrayer
 UPDATE `creature_template` SET `gold_min` =105 WHERE `entry` =7109 AND `gold_min` =91;

-- Jadefire Felsworn (sample size 21)
 UPDATE `creature_template` SET `gold_max` =630 WHERE `entry` =7109 AND `gold_max` =123;

-- Jadefire Felsworn (sample size 21)
 UPDATE `creature_template` SET `gold_max` =609 WHERE `entry` =7110 AND `gold_max` =144;

-- Jadefire Shadowstalker
 UPDATE `creature_template` SET `gold_max` =718 WHERE `entry` =7111 AND `gold_max` =126;

-- Jadefire Hellcaller
 UPDATE `creature_template` SET `gold_min` =102 WHERE `entry` =7112 AND `gold_min` =85;

-- Jaedenar Cultist (sample size 51)
 UPDATE `creature_template` SET `gold_max` =664 WHERE `entry` =7112 AND `gold_max` =116;

-- Jaedenar Cultist (sample size 51)
 UPDATE `creature_template` SET `gold_min` =130 WHERE `entry` =7113 AND `gold_min` =101;

-- Jaedenar Guardian (sample size 49)
 UPDATE `creature_template` SET `gold_max` =639 WHERE `entry` =7113 AND `gold_max` =137;

-- Jaedenar Guardian (sample size 49)
 UPDATE `creature_template` SET `gold_min` =104 WHERE `entry` =7114 AND `gold_min` =107;

-- Jaedenar Enforcer (sample size 32)
 UPDATE `creature_template` SET `gold_max` =706 WHERE `entry` =7114 AND `gold_max` =145;

-- Jaedenar Enforcer (sample size 32)
 UPDATE `creature_template` SET `gold_min` =127 WHERE `entry` =7115 AND `gold_min` =85;

-- Jaedenar Adept (sample size 37)
 UPDATE `creature_template` SET `gold_max` =675 WHERE `entry` =7115 AND `gold_max` =116;

-- Jaedenar Adept (sample size 37)
 UPDATE `creature_template` SET `gold_min` =168 WHERE `entry` =7118 AND `gold_min` =99;

-- Jaedenar Darkweaver (sample size 40)
 UPDATE `creature_template` SET `gold_max` =752 WHERE `entry` =7118 AND `gold_max` =135;

-- Jaedenar Darkweaver (sample size 40)
 UPDATE `creature_template` SET `gold_min` =156 WHERE `entry` =7120 AND `gold_min` =99;

-- Jaedenar Warlock (sample size 40)
 UPDATE `creature_template` SET `gold_max` =726 WHERE `entry` =7120 AND `gold_max` =135;

-- Jaedenar Warlock (sample size 40)
 UPDATE `creature_template` SET `gold_max` =555 WHERE `entry` =7138 AND `gold_max` =150;

-- Irontree Wanderer
 UPDATE `creature_template` SET `gold_max` =717 WHERE `entry` =7139 AND `gold_max` =148;

-- Irontree Stomper
 UPDATE `creature_template` SET `gold_min` =94 WHERE `entry` =7153 AND `gold_min` =93;

-- Deadwood Warrior (sample size 38)
 UPDATE `creature_template` SET `gold_max` =573 WHERE `entry` =7153 AND `gold_max` =126;

-- Deadwood Warrior (sample size 38)
 UPDATE `creature_template` SET `gold_min` =86 WHERE `entry` =7154 AND `gold_min` =82;

-- Deadwood Gardener (sample size 32)
 UPDATE `creature_template` SET `gold_max` =570 WHERE `entry` =7154 AND `gold_max` =113;

-- Deadwood Gardener (sample size 32)
 UPDATE `creature_template` SET `gold_min` =90 WHERE `entry` =7155 AND `gold_min` =96;

-- Deadwood Pathfinder (sample size 24)
 UPDATE `creature_template` SET `gold_max` =557 WHERE `entry` =7155 AND `gold_max` =130;

-- Deadwood Pathfinder (sample size 24)
 UPDATE `creature_template` SET `gold_min` =109 WHERE `entry` =7156 AND `gold_min` =112;

-- Deadwood Den Watcher (sample size 26)
 UPDATE `creature_template` SET `gold_max` =679 WHERE `entry` =7156 AND `gold_max` =151;

-- Deadwood Den Watcher (sample size 26)
 UPDATE `creature_template` SET `gold_min` =114 WHERE `entry` =7157 AND `gold_min` =117;

-- Deadwood Avenger (sample size 24)
 UPDATE `creature_template` SET `gold_max` =754 WHERE `entry` =7157 AND `gold_max` =158;

-- Deadwood Avenger (sample size 24)
 UPDATE `creature_template` SET `gold_min` =108 WHERE `entry` =7158 AND `gold_min` =99;

-- Deadwood Shaman (sample size 25)
 UPDATE `creature_template` SET `gold_max` =754 WHERE `entry` =7158 AND `gold_max` =135;

-- Deadwood Shaman (sample size 25)
 UPDATE `creature_template` SET `gold_min` =50 WHERE `entry` =7175 AND `gold_min` =54;

-- Stonevault Ambusher
 UPDATE `creature_template` SET `gold_max` =228 WHERE `entry` =7175 AND `gold_max` =76;

-- Stonevault Ambusher
 UPDATE `creature_template` SET `gold_min` =249 WHERE `entry` =7206 AND `gold_min` =454;

-- Ancient Stone Keeper
 UPDATE `creature_template` SET `gold_max` =904 WHERE `entry` =7206 AND `gold_max` =598;

-- Ancient Stone Keeper
 UPDATE `creature_template` SET `gold_min` =379 WHERE `entry` =7228 AND `gold_min` =703;

-- Ironaya
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =7234 AND `gold_min` =7;

-- Ferocitas the Dream Eater
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =7235 AND `gold_min` =5;

-- Gnarlpine Mystic
 UPDATE `creature_template` SET `gold_max` =12 WHERE `entry` =7235 AND `gold_max` =11;

-- Gnarlpine Mystic
 UPDATE `creature_template` SET `gold_min` =208 WHERE `entry` =7246 AND `gold_min` =273;

-- Sandfury Shadowhunter (sample size 31)
 UPDATE `creature_template` SET `gold_max` =1318 WHERE `entry` =7246 AND `gold_max` =362;

-- Sandfury Shadowhunter (sample size 31)
 UPDATE `creature_template` SET `gold_min` =225 WHERE `entry` =7247 AND `gold_min` =273;

-- Sandfury Soul Eater
 UPDATE `creature_template` SET `gold_max` =1403 WHERE `entry` =7247 AND `gold_max` =362;

-- Sandfury Soul Eater
 UPDATE `creature_template` SET `gold_min` =399 WHERE `entry` =7267 AND `gold_min` =674;

-- Chief Ukorz Sandscalp
 UPDATE `creature_template` SET `gold_max` =1620 WHERE `entry` =7267 AND `gold_max` =885;

-- Chief Ukorz Sandscalp
 UPDATE `creature_template` SET `gold_min` =367 WHERE `entry` =7271 AND `gold_min` =543;

-- Witch Doctor Zum'rah
 UPDATE `creature_template` SET `gold_max` =1510 WHERE `entry` =7271 AND `gold_max` =714;

-- Witch Doctor Zum'rah
 UPDATE `creature_template` SET `gold_max` =1191 WHERE `entry` =7274 AND `gold_max` =636;

-- Sandfury Executioner
 UPDATE `creature_template` SET `gold_max` =1320 WHERE `entry` =7275 AND `gold_max` =514;

-- Shadowpriest Sezz'ziz
 UPDATE `creature_template` SET `gold_min` =244 WHERE `entry` =7276 AND `gold_min` =78;

-- Zul'Farrak Dead Hero (sample size 23)
 UPDATE `creature_template` SET `gold_max` =1362 WHERE `entry` =7276 AND `gold_max` =106;

-- Zul'Farrak Dead Hero (sample size 23)
 UPDATE `creature_template` SET `gold_min` =183 WHERE `entry` =7286 AND `gold_min` =107;

-- Zul'Farrak Zombie (sample size 343)
 UPDATE `creature_template` SET `gold_max` =1286 WHERE `entry` =7286 AND `gold_max` =534;

-- Zul'Farrak Zombie (sample size 343)
 UPDATE `creature_template` SET `gold_max` =1233 WHERE `entry` =7290 AND `gold_max` =396;

-- Shadowforge Sharpshooter
 UPDATE `creature_template` SET `gold_min` =415 WHERE `entry` =7291 AND `gold_min` =626;

-- Galgann Firehammer
 UPDATE `creature_template` SET `gold_max` =1324 WHERE `entry` =7291 AND `gold_max` =822;

-- Galgann Firehammer
 UPDATE `creature_template` SET `gold_min` =28 WHERE `entry` =7309 AND `gold_min` =47;

-- Earthen Custodian
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =7318 AND `gold_min` =10;

-- Rageclaw
 UPDATE `creature_template` SET `gold_min` =220 WHERE `entry` =7320 AND `gold_min` =310;

-- Stonevault Mauler
 UPDATE `creature_template` SET `gold_max` =1056 WHERE `entry` =7320 AND `gold_max` =409;

-- Stonevault Mauler
 UPDATE `creature_template` SET `gold_max` =687 WHERE `entry` =7321 AND `gold_max` =326;

-- Stonevault Flameweaver
 UPDATE `creature_template` SET `gold_min` =105 WHERE `entry` =7327 AND `gold_min` =216;

-- Withered Warrior
 UPDATE `creature_template` SET `gold_max` =733 WHERE `entry` =7327 AND `gold_max` =287;

-- Withered Warrior
 UPDATE `creature_template` SET `gold_min` =196 WHERE `entry` =7328 AND `gold_min` =222;

-- Withered Reaver
 UPDATE `creature_template` SET `gold_max` =761 WHERE `entry` =7328 AND `gold_max` =295;

-- Withered Reaver
 UPDATE `creature_template` SET `gold_min` =129 WHERE `entry` =7329 AND `gold_min` =222;

-- Withered Quilguard (sample size 21)
 UPDATE `creature_template` SET `gold_max` =779 WHERE `entry` =7329 AND `gold_max` =295;

-- Withered Quilguard (sample size 21)
 UPDATE `creature_template` SET `gold_min` =148 WHERE `entry` =7332 AND `gold_min` =212;

-- Withered Spearhide
 UPDATE `creature_template` SET `gold_max` =711 WHERE `entry` =7332 AND `gold_max` =282;

-- Withered Spearhide
 UPDATE `creature_template` SET `gold_min` =128 WHERE `entry` =7335 AND `gold_min` =180;

-- Death's Head Geomancer
 UPDATE `creature_template` SET `gold_max` =762 WHERE `entry` =7335 AND `gold_max` =240;

-- Death's Head Geomancer
 UPDATE `creature_template` SET `gold_min` =184 WHERE `entry` =7337 AND `gold_min` =203;

-- Death's Head Necromancer
 UPDATE `creature_template` SET `gold_max` =820 WHERE `entry` =7337 AND `gold_max` =270;

-- Death's Head Necromancer
 UPDATE `creature_template` SET `gold_max` =823 WHERE `entry` =7341 AND `gold_max` =261;

-- Skeletal Frostweaver
 UPDATE `creature_template` SET `gold_min` =153 WHERE `entry` =7342 AND `gold_min` =208;

-- Skeletal Summoner
 UPDATE `creature_template` SET `gold_max` =892 WHERE `entry` =7342 AND `gold_max` =276;

-- Skeletal Summoner
 UPDATE `creature_template` SET `gold_min` =63 WHERE `entry` =7344 AND `gold_min` =65;

-- Splinterbone Warrior
 UPDATE `creature_template` SET `gold_max` =257 WHERE `entry` =7344 AND `gold_max` =89;

-- Splinterbone Warrior
 UPDATE `creature_template` SET `gold_max` =872 WHERE `entry` =7345 AND `gold_max` =348;

-- Splinterbone Captain
 UPDATE `creature_template` SET `gold_min` =58 WHERE `entry` =7346 AND `gold_min` =65;

-- Splinterbone Centurion
 UPDATE `creature_template` SET `gold_max` =316 WHERE `entry` =7346 AND `gold_max` =90;

-- Splinterbone Centurion
 UPDATE `creature_template` SET `gold_min` =220 WHERE `entry` =7347 AND `gold_min` =249;

-- Boneflayer Ghoul
 UPDATE `creature_template` SET `gold_max` =812 WHERE `entry` =7347 AND `gold_max` =330;

-- Boneflayer Ghoul
 UPDATE `creature_template` SET `gold_max` =835 WHERE `entry` =7348 AND `gold_max` =316;

-- Thorn Eater Ghoul
 UPDATE `creature_template` SET `gold_max` =995 WHERE `entry` =7353 AND `gold_max` =298;

-- Freezing Spirit
 UPDATE `creature_template` SET `gold_max` =941 WHERE `entry` =7356 AND `gold_max` =401;

-- Plaguemaw the Rotting
 UPDATE `creature_template` SET `gold_min` =328 WHERE `entry` =7357 AND `gold_min` =416;

-- Mordresh Fire Eye
 UPDATE `creature_template` SET `gold_max` =832 WHERE `entry` =7357 AND `gold_max` =548;

-- Mordresh Fire Eye
 UPDATE `creature_template` SET `gold_min` =340 WHERE `entry` =7358 AND `gold_min` =496;

-- Amnennar the Coldbringer
 UPDATE `creature_template` SET `gold_max` =679 WHERE `entry` =7358 AND `gold_max` =653;

-- Amnennar the Coldbringer
 UPDATE `creature_template` SET `gold_max` =626 WHERE `entry` =7361 AND `gold_max` =349;

-- Grubbis
 UPDATE `creature_template` SET `gold_min` =187 WHERE `entry` =7370 AND `gold_min` =195;

-- Restless Shade
 UPDATE `creature_template` SET `gold_max` =947 WHERE `entry` =7370 AND `gold_max` =260;

-- Restless Shade
 UPDATE `creature_template` SET `gold_max` =422 WHERE `entry` =7396 AND `gold_max` =114;

-- Earthen Stonebreaker
 UPDATE `creature_template` SET `gold_max` =197 WHERE `entry` =7397 AND `gold_max` =97;

-- Earthen Stonecarver
 UPDATE `creature_template` SET `gold_max` =172 WHERE `entry` =7404 AND `gold_max` =62;

-- Galak Flame Guard
 UPDATE `creature_template` SET `gold_max` =2337 WHERE `entry` =7437 AND `gold_max` =472;

-- Cobalt Mageweaver
 UPDATE `creature_template` SET `gold_min` =141 WHERE `entry` =7438 AND `gold_min` =126;

-- Winterfall Ursa (sample size 91)
 UPDATE `creature_template` SET `gold_max` =907 WHERE `entry` =7438 AND `gold_max` =170;

-- Winterfall Ursa (sample size 91)
 UPDATE `creature_template` SET `gold_min` =127 WHERE `entry` =7439 AND `gold_min` =110;

-- Winterfall Shaman (sample size 51)
 UPDATE `creature_template` SET `gold_max` =871 WHERE `entry` =7439 AND `gold_max` =149;

-- Winterfall Shaman (sample size 51)
 UPDATE `creature_template` SET `gold_min` =121 WHERE `entry` =7440 AND `gold_min` =122;

-- Winterfall Den Watcher (sample size 144)
 UPDATE `creature_template` SET `gold_max` =830 WHERE `entry` =7440 AND `gold_max` =165;

-- Winterfall Den Watcher (sample size 144)
 UPDATE `creature_template` SET `gold_min` =115 WHERE `entry` =7441 AND `gold_min` =100;

-- Winterfall Totemic (sample size 100)
 UPDATE `creature_template` SET `gold_max` =790 WHERE `entry` =7441 AND `gold_max` =136;

-- Winterfall Totemic (sample size 100)
 UPDATE `creature_template` SET `gold_min` =118 WHERE `entry` =7442 AND `gold_min` =112;

-- Winterfall Pathfinder (sample size 168)
 UPDATE `creature_template` SET `gold_max` =750 WHERE `entry` =7442 AND `gold_max` =151;

-- Winterfall Pathfinder (sample size 168)
 UPDATE `creature_template` SET `gold_min` =154 WHERE `entry` =7450 AND `gold_min` =114;

-- Ragged Owlbeast (sample size 29)
 UPDATE `creature_template` SET `gold_max` =724 WHERE `entry` =7450 AND `gold_max` =154;

-- Ragged Owlbeast (sample size 29)
 UPDATE `creature_template` SET `gold_min` =121 WHERE `entry` =7451 AND `gold_min` =122;

-- Raging Owlbeast (sample size 31)
 UPDATE `creature_template` SET `gold_max` =789 WHERE `entry` =7451 AND `gold_max` =164;

-- Raging Owlbeast (sample size 31)
 UPDATE `creature_template` SET `gold_min` =145 WHERE `entry` =7452 AND `gold_min` =125;

-- Crazed Owlbeast (sample size 23)
 UPDATE `creature_template` SET `gold_max` =829 WHERE `entry` =7452 AND `gold_max` =169;

-- Crazed Owlbeast (sample size 23)
 UPDATE `creature_template` SET `gold_min` =135 WHERE `entry` =7453 AND `gold_min` =111;

-- Moontouched Owlbeast (sample size 31)
 UPDATE `creature_template` SET `gold_max` =907 WHERE `entry` =7453 AND `gold_max` =150;

-- Moontouched Owlbeast (sample size 31)
 UPDATE `creature_template` SET `gold_max` =823 WHERE `entry` =7454 AND `gold_max` =176;

-- Berserk Owlbeast
 UPDATE `creature_template` SET `gold_min` =118 WHERE `entry` =7457 AND `gold_min` =114;

-- Rogue Ice Thistle (sample size 49)
 UPDATE `creature_template` SET `gold_max` =729 WHERE `entry` =7457 AND `gold_max` =154;

-- Rogue Ice Thistle (sample size 49)
 UPDATE `creature_template` SET `gold_min` =119 WHERE `entry` =7458 AND `gold_min` =120;

-- Ice Thistle Yeti (sample size 118)
 UPDATE `creature_template` SET `gold_max` =828 WHERE `entry` =7458 AND `gold_max` =162;

-- Ice Thistle Yeti (sample size 118)
 UPDATE `creature_template` SET `gold_min` =132 WHERE `entry` =7459 AND `gold_min` =125;

-- Ice Thistle Matriarch (sample size 46)
 UPDATE `creature_template` SET `gold_max` =871 WHERE `entry` =7459 AND `gold_max` =169;

-- Ice Thistle Matriarch (sample size 46)
 UPDATE `creature_template` SET `gold_min` =135 WHERE `entry` =7460 AND `gold_min` =128;

-- Ice Thistle Patriarch (sample size 40)
 UPDATE `creature_template` SET `gold_max` =827 WHERE `entry` =7460 AND `gold_max` =172;

-- Ice Thistle Patriarch (sample size 40)
 UPDATE `creature_template` SET `gold_max` =2858 WHERE `entry` =7461 AND `gold_max` =853;

-- Hederine Initiate
 UPDATE `creature_template` SET `gold_max` =665 WHERE `entry` =7523 AND `gold_max` =139;

-- Suffering Highborne
 UPDATE `creature_template` SET `gold_max` =821 WHERE `entry` =7524 AND `gold_max` =145;

-- Anguished Highborne
 UPDATE `creature_template` SET `gold_min` =88 WHERE `entry` =7584 AND `gold_min` =92;

-- Wandering Forest Walker
 UPDATE `creature_template` SET `gold_max` =434 WHERE `entry` =7584 AND `gold_max` =125;

-- Wandering Forest Walker
 UPDATE `creature_template` SET `gold_min` =22 WHERE `entry` =7603 AND `gold_min` =44;

-- Leprous Assistant (sample size 41)
 UPDATE `creature_template` SET `gold_max` =155 WHERE `entry` =7603 AND `gold_max` =63;

-- Leprous Assistant (sample size 41)
 UPDATE `creature_template` SET `gold_min` =400 WHERE `entry` =7604 AND `gold_min` =839;

-- Sergeant Bly
 UPDATE `creature_template` SET `gold_max` =1265 WHERE `entry` =7606 AND `gold_max` =348;

-- Oro Eyegouge
 UPDATE `creature_template` SET `gold_min` =243 WHERE `entry` =7608 AND `gold_min` =289;

-- Murta Grimgut
 UPDATE `creature_template` SET `gold_max` =1084 WHERE `entry` =7608 AND `gold_max` =382;

-- Murta Grimgut
 UPDATE `creature_template` SET `gold_max` =2713 WHERE `entry` =7664 AND `gold_max` =838;

-- Razelikh the Defiler
 UPDATE `creature_template` SET `gold_max` =2777 WHERE `entry` =7665 AND `gold_max` =1406;

-- Grol the Destroyer
 UPDATE `creature_template` SET `gold_min` =164 WHERE `entry` =7666 AND `gold_min` =691;

-- Archmage Allistarj
 UPDATE `creature_template` SET `gold_min` =499 WHERE `entry` =7667 AND `gold_min` =990;

-- Lady Sevine
 UPDATE `creature_template` SET `gold_max` =755 WHERE `entry` =7668 AND `gold_max` =175;

-- Servant of Razelikh
 UPDATE `creature_template` SET `gold_max` =693 WHERE `entry` =7669 AND `gold_max` =154;

-- Servant of Grol
 UPDATE `creature_template` SET `gold_max` =719 WHERE `entry` =7670 AND `gold_max` =161;

-- Servant of Allistarj
 UPDATE `creature_template` SET `gold_max` =799 WHERE `entry` =7671 AND `gold_max` =168;

-- Servant of Sevine
 UPDATE `creature_template` SET `gold_min` =99 WHERE `entry` =7725 AND `gold_min` =75;

-- Grimtotem Raider (sample size 23)
 UPDATE `creature_template` SET `gold_max` =403 WHERE `entry` =7725 AND `gold_max` =103;

-- Grimtotem Raider (sample size 23)
 UPDATE `creature_template` SET `gold_min` =73 WHERE `entry` =7726 AND `gold_min` =66;

-- Grimtotem Naturalist (sample size 23)
 UPDATE `creature_template` SET `gold_max` =373 WHERE `entry` =7726 AND `gold_max` =91;

-- Grimtotem Naturalist (sample size 23)
 UPDATE `creature_template` SET `gold_min` =61 WHERE `entry` =7727 AND `gold_min` =70;

-- Grimtotem Shaman (sample size 21)
 UPDATE `creature_template` SET `gold_max` =420 WHERE `entry` =7727 AND `gold_max` =96;

-- Grimtotem Shaman (sample size 21)
 UPDATE `creature_template` SET `gold_max` =1414 WHERE `entry` =7728 AND `gold_max` =684;

-- Kirith the Damned
 UPDATE `creature_template` SET `gold_max` =982 WHERE `entry` =7795 AND `gold_max` =336;

-- Hydromancer Velratha
 UPDATE `creature_template` SET `gold_max` =723 WHERE `entry` =7800 AND `gold_max` =587;

-- Mekgineer Thermaplugg
 UPDATE `creature_template` SET `gold_max` =360 WHERE `entry` =7805 AND `gold_max` =222;

-- Wastewander Scofflaw
 UPDATE `creature_template` SET `gold_max` =374 WHERE `entry` =7808 AND `gold_max` =222;

-- Marauding Owlbeast
 UPDATE `creature_template` SET `gold_max` =498 WHERE `entry` =7809 AND `gold_max` =213;

-- Vilebranch Ambusher
 UPDATE `creature_template` SET `gold_max` =402 WHERE `entry` =7847 AND `gold_max` =119;

-- Caliph Scorpidsting
 UPDATE `creature_template` SET `gold_max` =440 WHERE `entry` =7848 AND `gold_max` =213;

-- Lurking Feral Scar
 UPDATE `creature_template` SET `gold_min` =85 WHERE `entry` =7855 AND `gold_min` =81;

-- Southsea Pirate (sample size 26)
 UPDATE `creature_template` SET `gold_max` =446 WHERE `entry` =7855 AND `gold_max` =111;

-- Southsea Pirate (sample size 26)
 UPDATE `creature_template` SET `gold_min` =65 WHERE `entry` =7856 AND `gold_min` =81;

-- Southsea Freebooter (sample size 30)
 UPDATE `creature_template` SET `gold_max` =443 WHERE `entry` =7856 AND `gold_max` =111;

-- Southsea Freebooter (sample size 30)
 UPDATE `creature_template` SET `gold_min` =68 WHERE `entry` =7857 AND `gold_min` =81;

-- Southsea Dock Worker (sample size 37)
 UPDATE `creature_template` SET `gold_max` =422 WHERE `entry` =7857 AND `gold_max` =111;

-- Southsea Dock Worker (sample size 37)
 UPDATE `creature_template` SET `gold_min` =73 WHERE `entry` =7858 AND `gold_min` =81;

-- Southsea Swashbuckler (sample size 44)
 UPDATE `creature_template` SET `gold_max` =451 WHERE `entry` =7858 AND `gold_max` =111;

-- Southsea Swashbuckler (sample size 44)
 UPDATE `creature_template` SET `gold_max` =424 WHERE `entry` =7864 AND `gold_max` =129;

-- Lingering Highborne
 UPDATE `creature_template` SET `gold_max` =475 WHERE `entry` =7872 AND `gold_max` =224;

-- Death's Head Cultist
 UPDATE `creature_template` SET `gold_min` =205 WHERE `entry` =7873 AND `gold_min` =206;

-- Razorfen Battleguard
 UPDATE `creature_template` SET `gold_max` =553 WHERE `entry` =7873 AND `gold_max` =274;

-- Razorfen Battleguard
 UPDATE `creature_template` SET `gold_min` =144 WHERE `entry` =7874 AND `gold_min` =172;

-- Razorfen Thornweaver
 UPDATE `creature_template` SET `gold_max` =689 WHERE `entry` =7874 AND `gold_max` =230;

-- Razorfen Thornweaver
 UPDATE `creature_template` SET `gold_max` =120 WHERE `entry` =7883 AND `gold_max` =115;

-- Andre Firebeard
 UPDATE `creature_template` SET `gold_max` =386 WHERE `entry` =7885 AND `gold_max` =157;

-- Spitelash Battlemaster
 UPDATE `creature_template` SET `gold_max` =486 WHERE `entry` =7886 AND `gold_max` =136;

-- Spitelash Enchantress
 UPDATE `creature_template` SET `gold_max` =370 WHERE `entry` =7899 AND `gold_max` =213;

-- Treasure Hunting Pirate
 UPDATE `creature_template` SET `gold_max` =312 WHERE `entry` =7901 AND `gold_max` =213;

-- Treasure Hunting Swashbuckler
 UPDATE `creature_template` SET `gold_max` =296 WHERE `entry` =7902 AND `gold_max` =222;

-- Treasure Hunting Buccaneer
 UPDATE `creature_template` SET `gold_max` =2194 WHERE `entry` =7995 AND `gold_max` =569;

-- Vile Priestess Hexx
 UPDATE `creature_template` SET `gold_max` =1948 WHERE `entry` =7996 AND `gold_max` =539;

-- Qiaga the Keeper
 UPDATE `creature_template` SET `gold_max` =318 WHERE `entry` =7997 AND `gold_max` =100;

-- Captured Sprite Darter
 UPDATE `creature_template` SET `gold_max` =1908 WHERE `entry` =8075 AND `gold_max` =576;

-- Edana Hatetalon
 UPDATE `creature_template` SET `gold_max` =1111 WHERE `entry` =8127 AND `gold_max` =829;

-- Antu'sul
 UPDATE `creature_template` SET `gold_max` =385 WHERE `entry` =8203 AND `gold_max` =125;

-- Kregg Keelhaul
 UPDATE `creature_template` SET `gold_max` =237 WHERE `entry` =8212 AND `gold_max` =130;

-- The Reak
 UPDATE `creature_template` SET `gold_max` =495 WHERE `entry` =8214 AND `gold_max` =130;

-- Jalinde Summerdrake
 UPDATE `creature_template` SET `gold_max` =1040 WHERE `entry` =8217 AND `gold_max` =649;

-- Mith'rethis the Enchanter
 UPDATE `creature_template` SET `gold_max` =212 WHERE `entry` =8283 AND `gold_max` =134;

-- Slave Master Blackheart
 UPDATE `creature_template` SET `gold_max` =428 WHERE `entry` =8296 AND `gold_max` =107;

-- Mojo the Twisted
 UPDATE `creature_template` SET `gold_max` =336 WHERE `entry` =8304 AND `gold_max` =170;

-- Dreadscorn
 UPDATE `creature_template` SET `gold_min` =82 WHERE `entry` =8318 AND `gold_min` =91;

-- Atal'ai Slave (sample size 32)
 UPDATE `creature_template` SET `gold_max` =494 WHERE `entry` =8318 AND `gold_max` =124;

-- Atal'ai Slave (sample size 32)
 UPDATE `creature_template` SET `gold_min` =113 WHERE `entry` =8319 AND `gold_min` =98;

-- Nightmare Whelp (sample size 37)
 UPDATE `creature_template` SET `gold_max` =604 WHERE `entry` =8319 AND `gold_max` =133;

-- Nightmare Whelp (sample size 37)
 UPDATE `creature_template` SET `gold_max` =384 WHERE `entry` =8337 AND `gold_max` =222;

-- Dark Iron Steelshifter
 UPDATE `creature_template` SET `gold_min` =311 WHERE `entry` =8384 AND `gold_min` =362;

-- Deep Lurker
 UPDATE `creature_template` SET `gold_max` =1615 WHERE `entry` =8384 AND `gold_max` =477;

-- Deep Lurker
 UPDATE `creature_template` SET `gold_min` =407 WHERE `entry` =8400 AND `gold_min` =765;

-- Obsidion
 UPDATE `creature_template` SET `gold_max` =632 WHERE `entry` =8408 AND `gold_max` =156;

-- Warlord Krellian
 UPDATE `creature_template` SET `gold_min` =261 WHERE `entry` =8419 AND `gold_min` =318;

-- Twilight Idolater
 UPDATE `creature_template` SET `gold_max` =1754 WHERE `entry` =8419 AND `gold_max` =421;

-- Twilight Idolater
 UPDATE `creature_template` SET `gold_max` =1219 WHERE `entry` =8497 AND `gold_max` =504;

-- Nightmare Suppressor
 UPDATE `creature_template` SET `gold_min` =103 WHERE `entry` =8504 AND `gold_min` =107;

-- Dark Iron Sentry
 UPDATE `creature_template` SET `gold_max` =640 WHERE `entry` =8504 AND `gold_max` =534;

-- Dark Iron Sentry
 UPDATE `creature_template` SET `gold_max` =268 WHERE `entry` =8518 AND `gold_max` =215;

-- Rynthariel the Keymaster
 UPDATE `creature_template` SET `gold_max` =708 WHERE `entry` =8523 AND `gold_max` =154;

-- Scourge Soldier
 UPDATE `creature_template` SET `gold_max` =751 WHERE `entry` =8524 AND `gold_max` =130;

-- Cursed Mage
 UPDATE `creature_template` SET `gold_max` =821 WHERE `entry` =8525 AND `gold_max` =143;

-- Scourge Warder
 UPDATE `creature_template` SET `gold_max` =441 WHERE `entry` =8527 AND `gold_max` =172;

-- Scourge Guard
 UPDATE `creature_template` SET `gold_max` =558 WHERE `entry` =8528 AND `gold_max` =143;

-- Dread Weaver
 UPDATE `creature_template` SET `gold_max` =998 WHERE `entry` =8529 AND `gold_max` =180;

-- Scourge Champion
 UPDATE `creature_template` SET `gold_min` =116 WHERE `entry` =8530 AND `gold_min` =115;

-- Cannibal Ghoul (sample size 29)
 UPDATE `creature_template` SET `gold_max` =789 WHERE `entry` =8530 AND `gold_max` =155;

-- Cannibal Ghoul (sample size 29)
 UPDATE `creature_template` SET `gold_max` =754 WHERE `entry` =8531 AND `gold_max` =169;

-- Gibbering Ghoul
 UPDATE `creature_template` SET `gold_max` =835 WHERE `entry` =8532 AND `gold_max` =179;

-- Diseased Flayer
 UPDATE `creature_template` SET `gold_min` =131 WHERE `entry` =8539 AND `gold_min` =128;

-- Eyeless Watcher (sample size 24)
 UPDATE `creature_template` SET `gold_max` =865 WHERE `entry` =8539 AND `gold_max` =172;

-- Eyeless Watcher (sample size 24)
 UPDATE `creature_template` SET `gold_max` =673 WHERE `entry` =8540 AND `gold_max` =157;

-- Torn Screamer
 UPDATE `creature_template` SET `gold_max` =812 WHERE `entry` =8541 AND `gold_max` =165;

-- Hate Shrieker
 UPDATE `creature_template` SET `gold_min` =131 WHERE `entry` =8542 AND `gold_min` =133;

-- Death Singer
 UPDATE `creature_template` SET `gold_max` =840 WHERE `entry` =8542 AND `gold_max` =178;

-- Death Singer
 UPDATE `creature_template` SET `gold_min` =149 WHERE `entry` =8543 AND `gold_min` =130;

-- Stitched Horror (sample size 25)
 UPDATE `creature_template` SET `gold_max` =908 WHERE `entry` =8543 AND `gold_max` =175;

-- Stitched Horror (sample size 25)
 UPDATE `creature_template` SET `gold_max` =567 WHERE `entry` =8545 AND `gold_max` =183;

-- Abomination
 UPDATE `creature_template` SET `gold_min` =142 WHERE `entry` =8546 AND `gold_min` =133;

-- Dark Adept (sample size 109)
 UPDATE `creature_template` SET `gold_max` =959 WHERE `entry` =8546 AND `gold_max` =179;

-- Dark Adept (sample size 109)
 UPDATE `creature_template` SET `gold_min` =145 WHERE `entry` =8550 AND `gold_min` =111;

-- Shadowmage (sample size 118)
 UPDATE `creature_template` SET `gold_max` =1005 WHERE `entry` =8550 AND `gold_max` =150;

-- Shadowmage (sample size 118)
 UPDATE `creature_template` SET `gold_max` =927 WHERE `entry` =8561 AND `gold_max` =179;

-- Mossflayer Shadowhunter
 UPDATE `creature_template` SET `gold_max` =890 WHERE `entry` =8562 AND `gold_max` =156;

-- Mossflayer Cannibal
 UPDATE `creature_template` SET `gold_max` =959 WHERE `entry` =8563 AND `gold_max` =174;

-- Woodsman
 UPDATE `creature_template` SET `gold_max` =751 WHERE `entry` =8564 AND `gold_max` =180;

-- Ranger
 UPDATE `creature_template` SET `gold_max` =905 WHERE `entry` =8565 AND `gold_max` =170;

-- Pathstrider
 UPDATE `creature_template` SET `gold_min` =80 WHERE `entry` =8566 AND `gold_min` =94;

-- Dark Iron Lookout (sample size 64)
 UPDATE `creature_template` SET `gold_max` =537 WHERE `entry` =8566 AND `gold_max` =128;

-- Dark Iron Lookout (sample size 64)
 UPDATE `creature_template` SET `gold_min` =259 WHERE `entry` =8567 AND `gold_min` =575;

-- Glutton
 UPDATE `creature_template` SET `gold_max` =873 WHERE `entry` =8567 AND `gold_max` =756;

-- Glutton
 UPDATE `creature_template` SET `gold_min` =497 WHERE `entry` =8580 AND `gold_min` =762;

-- Atal'alarion
 UPDATE `creature_template` SET `gold_max` =1757 WHERE `entry` =8580 AND `gold_max` =1000;

-- Atal'alarion
 UPDATE `creature_template` SET `gold_min` =118 WHERE `entry` =8606 AND `gold_min` =122;

-- Living Decay
 UPDATE `creature_template` SET `gold_max` =471 WHERE `entry` =8607 AND `gold_max` =161;

-- Rotting Sludge
 UPDATE `creature_template` SET `gold_max` =437 WHERE `entry` =8637 AND `gold_max` =111;

-- Dark Iron Watchman
 UPDATE `creature_template` SET `gold_min` =103 WHERE `entry` =8766 AND `gold_min` =109;

-- Forest Ooze
 UPDATE `creature_template` SET `gold_max` =646 WHERE `entry` =8766 AND `gold_max` =148;

-- Forest Ooze
 UPDATE `creature_template` SET `gold_min` =275 WHERE `entry` =8889 AND `gold_min` =355;

-- Anvilrage Overseer (sample size 40)
 UPDATE `creature_template` SET `gold_max` =1667 WHERE `entry` =8889 AND `gold_max` =468;

-- Anvilrage Overseer (sample size 40)
 UPDATE `creature_template` SET `gold_min` =274 WHERE `entry` =8890 AND `gold_min` =365;

-- Anvilrage Warden (sample size 68)
 UPDATE `creature_template` SET `gold_max` =1820 WHERE `entry` =8890 AND `gold_max` =482;

-- Anvilrage Warden (sample size 68)
 UPDATE `creature_template` SET `gold_min` =286 WHERE `entry` =8891 AND `gold_min` =375;

-- Anvilrage Guardsman (sample size 107)
 UPDATE `creature_template` SET `gold_max` =1930 WHERE `entry` =8891 AND `gold_max` =495;

-- Anvilrage Guardsman (sample size 107)
 UPDATE `creature_template` SET `gold_min` =355 WHERE `entry` =8892 AND `gold_min` =394;

-- Anvilrage Footman (sample size 65)
 UPDATE `creature_template` SET `gold_max` =1944 WHERE `entry` =8892 AND `gold_max` =519;

-- Anvilrage Footman (sample size 65)
 UPDATE `creature_template` SET `gold_max` =2049 WHERE `entry` =8893 AND `gold_max` =594;

-- Anvilrage Soldier
 UPDATE `creature_template` SET `gold_min` =311 WHERE `entry` =8894 AND `gold_min` =366;

-- Anvilrage Medic
 UPDATE `creature_template` SET `gold_max` =2119 WHERE `entry` =8894 AND `gold_max` =483;

-- Anvilrage Medic
 UPDATE `creature_template` SET `gold_max` =1987 WHERE `entry` =8895 AND `gold_max` =502;

-- Anvilrage Officer
 UPDATE `creature_template` SET `gold_min` =135 WHERE `entry` =8896 AND `gold_min` =103;

-- Shadowforge Peasant (sample size 40)
 UPDATE `creature_template` SET `gold_max` =698 WHERE `entry` =8896 AND `gold_max` =139;

-- Shadowforge Peasant (sample size 40)
 UPDATE `creature_template` SET `gold_min` =108 WHERE `entry` =8897 AND `gold_min` =113;

-- Doomforge Craftsman (sample size 40)
 UPDATE `creature_template` SET `gold_max` =694 WHERE `entry` =8897 AND `gold_max` =153;

-- Doomforge Craftsman (sample size 40)
 UPDATE `creature_template` SET `gold_max` =2149 WHERE `entry` =8898 AND `gold_max` =476;

-- Anvilrage Marshal
 UPDATE `creature_template` SET `gold_min` =339 WHERE `entry` =8899 AND `gold_min` =420;

-- Doomforge Dragoon (sample size 26)
 UPDATE `creature_template` SET `gold_max` =2373 WHERE `entry` =8899 AND `gold_max` =554;

-- Doomforge Dragoon (sample size 26)
 UPDATE `creature_template` SET `gold_max` =611 WHERE `entry` =8900 AND `gold_max` =130;

-- Doomforge Arcanasmith
 UPDATE `creature_template` SET `gold_max` =702 WHERE `entry` =8902 AND `gold_max` =125;

-- Shadowforge Citizen
 UPDATE `creature_template` SET `gold_max` =1606 WHERE `entry` =8903 AND `gold_max` =578;

-- Anvilrage Captain
 UPDATE `creature_template` SET `gold_max` =797 WHERE `entry` =8904 AND `gold_max` =140;

-- Shadowforge Senator
 UPDATE `creature_template` SET `gold_max` =2225 WHERE `entry` =8905 AND `gold_max` =540;

-- Warbringer Construct
 UPDATE `creature_template` SET `gold_max` =2104 WHERE `entry` =8906 AND `gold_max` =565;

-- Ragereaver Golem
 UPDATE `creature_template` SET `gold_max` =2436 WHERE `entry` =8907 AND `gold_max` =573;

-- Wrath Hammer Construct
 UPDATE `creature_template` SET `gold_max` =1223 WHERE `entry` =8908 AND `gold_max` =789;

-- Molten War Golem
 UPDATE `creature_template` SET `gold_min` =299 WHERE `entry` =8912 AND `gold_min` =324;

-- Twilight's Hammer Torturer (sample size 25)
 UPDATE `creature_template` SET `gold_max` =1890 WHERE `entry` =8912 AND `gold_max` =428;

-- Twilight's Hammer Torturer (sample size 25)
 UPDATE `creature_template` SET `gold_min` =109 WHERE `entry` =8913 AND `gold_min` =311;

-- Twilight Emissary
 UPDATE `creature_template` SET `gold_max` =666 WHERE `entry` =8913 AND `gold_max` =411;

-- Twilight Emissary
 UPDATE `creature_template` SET `gold_max` =1894 WHERE `entry` =8914 AND `gold_max` =463;

-- Twilight Bodyguard
 UPDATE `creature_template` SET `gold_max` =324 WHERE `entry` =8915 AND `gold_max` =139;

-- Twilight's Hammer Ambassador
 UPDATE `creature_template` SET `gold_max` =642 WHERE `entry` =8916 AND `gold_max` =150;

-- Arena Spectator
 UPDATE `creature_template` SET `gold_min` =115 WHERE `entry` =8920 AND `gold_min` =102;

-- Weapon Technician (sample size 41)
 UPDATE `creature_template` SET `gold_max` =754 WHERE `entry` =8920 AND `gold_max` =139;

-- Weapon Technician (sample size 41)
 UPDATE `creature_template` SET `gold_max` =2561 WHERE `entry` =8923 AND `gold_max` =737;

-- Panzor the Invincible
 UPDATE `creature_template` SET `gold_max` =935 WHERE `entry` =8983 AND `gold_max` =849;

-- Golem Lord Argelmach
 UPDATE `creature_template` SET `gold_max` =1649 WHERE `entry` =9018 AND `gold_max` =644;

-- High Interrogator Gerstahn
 UPDATE `creature_template` SET `gold_max` =945 WHERE `entry` =9024 AND `gold_max` =865;

-- Pyromancer Loregrain
 UPDATE `creature_template` SET `gold_max` =1784 WHERE `entry` =9027 AND `gold_max` =737;

-- Gorosh the Dervish
 UPDATE `creature_template` SET `gold_max` =2301 WHERE `entry` =9028 AND `gold_max` =713;

-- Grizzle
 UPDATE `creature_template` SET `gold_max` =1352 WHERE `entry` =9033 AND `gold_max` =1349;

-- General Angerforge
 UPDATE `creature_template` SET `gold_max` =2306 WHERE `entry` =9045 AND `gold_max` =487;

-- Scarshield Acolyte
 UPDATE `creature_template` SET `gold_min` =376 WHERE `entry` =9056 AND `gold_min` =1279;

-- Fineous Darkvire
 UPDATE `creature_template` SET `gold_min` =497 WHERE `entry` =9096 AND `gold_min` =691;

-- Rage Talon Dragonspawn (sample size 21)
 UPDATE `creature_template` SET `gold_max` =2819 WHERE `entry` =9096 AND `gold_max` =907;

-- Rage Talon Dragonspawn (sample size 21)
 UPDATE `creature_template` SET `gold_min` =402 WHERE `entry` =9097 AND `gold_min` =425;

-- Scarshield Legionnaire (sample size 65)
 UPDATE `creature_template` SET `gold_max` =2367 WHERE `entry` =9097 AND `gold_max` =559;

-- Scarshield Legionnaire (sample size 65)
 UPDATE `creature_template` SET `gold_max` =2313 WHERE `entry` =9098 AND `gold_max` =476;

-- Scarshield Spellbinder
 UPDATE `creature_template` SET `gold_max` =2000 WHERE `entry` =9156 AND `gold_max` =1682;

-- Ambassador Flamelash
 UPDATE `creature_template` SET `gold_max` =620 WHERE `entry` =9176 AND `gold_max` =153;

-- Gor'tesh
 UPDATE `creature_template` SET `gold_min` =920 WHERE `entry` =9196 AND `gold_min` =1117;

-- Highlord Omokk
 UPDATE `creature_template` SET `gold_max` =2139 WHERE `entry` =9196 AND `gold_max` =1464;

-- Highlord Omokk
 UPDATE `creature_template` SET `gold_max` =2274 WHERE `entry` =9197 AND `gold_max` =738;

-- Spirestone Battle Mage
 UPDATE `creature_template` SET `gold_min` =394 WHERE `entry` =9199 AND `gold_min` =641;

-- Spirestone Enforcer
 UPDATE `creature_template` SET `gold_max` =2303 WHERE `entry` =9199 AND `gold_max` =842;

-- Spirestone Enforcer
 UPDATE `creature_template` SET `gold_min` =388 WHERE `entry` =9200 AND `gold_min` =643;

-- Spirestone Reaver
 UPDATE `creature_template` SET `gold_max` =2391 WHERE `entry` =9200 AND `gold_max` =845;

-- Spirestone Reaver
 UPDATE `creature_template` SET `gold_max` =1687 WHERE `entry` =9201 AND `gold_max` =681;

-- Spirestone Ogre Magus
 UPDATE `creature_template` SET `gold_max` =2376 WHERE `entry` =9216 AND `gold_max` =905;

-- Spirestone Warlord
 UPDATE `creature_template` SET `gold_max` =2245 WHERE `entry` =9219 AND `gold_max` =1606;

-- Spirestone Butcher
 UPDATE `creature_template` SET `gold_max` =2172 WHERE `entry` =9236 AND `gold_max` =1295;

-- Shadow Hunter Vosh'gajin
 UPDATE `creature_template` SET `gold_max` =2212 WHERE `entry` =9237 AND `gold_max` =1741;

-- War Master Voone
 UPDATE `creature_template` SET `gold_max` =2470 WHERE `entry` =9239 AND `gold_max` =495;

-- Smolderthorn Mystic
 UPDATE `creature_template` SET `gold_min` =427 WHERE `entry` =9240 AND `gold_min` =371;

-- Smolderthorn Shadow Priest (sample size 26)
 UPDATE `creature_template` SET `gold_max` =2460 WHERE `entry` =9240 AND `gold_max` =489;

-- Smolderthorn Shadow Priest (sample size 26)
 UPDATE `creature_template` SET `gold_max` =2378 WHERE `entry` =9241 AND `gold_max` =603;

-- Smolderthorn Headhunter
 UPDATE `creature_template` SET `gold_max` =2228 WHERE `entry` =9257 AND `gold_max` =487;

-- Scarshield Warlock
 UPDATE `creature_template` SET `gold_min` =419 WHERE `entry` =9258 AND `gold_min` =439;

-- Scarshield Raider
 UPDATE `creature_template` SET `gold_max` =2288 WHERE `entry` =9258 AND `gold_max` =578;

-- Scarshield Raider
 UPDATE `creature_template` SET `gold_min` =474 WHERE `entry` =9259 AND `gold_min` =449;

-- Firebrand Grunt (sample size 24)
 UPDATE `creature_template` SET `gold_max` =2573 WHERE `entry` =9259 AND `gold_max` =592;

-- Firebrand Grunt (sample size 24)
 UPDATE `creature_template` SET `gold_max` =2655 WHERE `entry` =9260 AND `gold_max` =610;

-- Firebrand Legionnaire
 UPDATE `creature_template` SET `gold_max` =2553 WHERE `entry` =9261 AND `gold_max` =514;

-- Firebrand Darkweaver
 UPDATE `creature_template` SET `gold_max` =2574 WHERE `entry` =9262 AND `gold_max` =508;

-- Firebrand Invoker
 UPDATE `creature_template` SET `gold_max` =2694 WHERE `entry` =9263 AND `gold_max` =527;

-- Firebrand Dreadweaver
 UPDATE `creature_template` SET `gold_max` =2532 WHERE `entry` =9264 AND `gold_max` =522;

-- Firebrand Pyromancer
 UPDATE `creature_template` SET `gold_max` =2242 WHERE `entry` =9265 AND `gold_max` =508;

-- Smolderthorn Shadow Hunter
 UPDATE `creature_template` SET `gold_max` =2611 WHERE `entry` =9266 AND `gold_max` =503;

-- Smolderthorn Witch Doctor
 UPDATE `creature_template` SET `gold_min` =379 WHERE `entry` =9267 AND `gold_min` =443;

-- Smolderthorn Axe Thrower (sample size 29)
 UPDATE `creature_template` SET `gold_max` =2466 WHERE `entry` =9267 AND `gold_max` =584;

-- Smolderthorn Axe Thrower (sample size 29)
 UPDATE `creature_template` SET `gold_max` =2533 WHERE `entry` =9268 AND `gold_max` =610;

-- Smolderthorn Berserker
 UPDATE `creature_template` SET `gold_max` =2594 WHERE `entry` =9269 AND `gold_max` =508;

-- Smolderthorn Seer
 UPDATE `creature_template` SET `gold_max` =1388 WHERE `entry` =9445 AND `gold_max` =737;

-- Dark Guard
 UPDATE `creature_template` SET `gold_max` =593 WHERE `entry` =9454 AND `gold_max` =155;

-- Xavathras
 UPDATE `creature_template` SET `gold_min` =16 WHERE `entry` =9456 AND `gold_min` =17;

-- Warlord Krom'zar
 UPDATE `creature_template` SET `gold_max` =309 WHERE `entry` =9462 AND `gold_max` =166;

-- Chieftain Bloodmaw
 UPDATE `creature_template` SET `gold_max` =364 WHERE `entry` =9464 AND `gold_max` =138;

-- Overlord Ror
 UPDATE `creature_template` SET `gold_max` =1133 WHERE `entry` =9476 AND `gold_max` =761;

-- Watchman Doomgrip
 UPDATE `creature_template` SET `gold_max` =1955 WHERE `entry` =9502 AND `gold_max` =1122;

-- Phalanx
 UPDATE `creature_template` SET `gold_max` =905 WHERE `entry` =9517 AND `gold_max` =136;

-- Shadow Lord Fel'dan
 UPDATE `creature_template` SET `gold_max` =852 WHERE `entry` =9518 AND `gold_max` =155;

-- Rakaiah
 UPDATE `creature_template` SET `gold_max` =27 WHERE `entry` =9523 AND `gold_max` =23;

-- Kolkar Stormseer
 UPDATE `creature_template` SET `gold_min` =14 WHERE `entry` =9524 AND `gold_min` =5;

-- Kolkar Invader (sample size 29)
 UPDATE `creature_template` SET `gold_max` =49 WHERE `entry` =9524 AND `gold_max` =23;

-- Kolkar Invader (sample size 29)
 UPDATE `creature_template` SET `gold_max` =2123 WHERE `entry` =9537 AND `gold_max` =737;

-- Hurley Blackbreath
 UPDATE `creature_template` SET `gold_max` =1575 WHERE `entry` =9541 AND `gold_max` =737;

-- Blackbreath Crony
 UPDATE `creature_template` SET `gold_min` =240 WHERE `entry` =9543 AND `gold_min` =406;

-- Ribbly Screwspigot
 UPDATE `creature_template` SET `gold_max` =572 WHERE `entry` =9545 AND `gold_max` =126;

-- Grim Patron
 UPDATE `creature_template` SET `gold_max` =530 WHERE `entry` =9547 AND `gold_max` =107;

-- Guzzling Patron
 UPDATE `creature_template` SET `gold_min` =377 WHERE `entry` =9554 AND `gold_min` =389;

-- Hammered Patron
 UPDATE `creature_template` SET `gold_max` =924 WHERE `entry` =9554 AND `gold_max` =513;

-- Hammered Patron
 UPDATE `creature_template` SET `gold_min` =1124 WHERE `entry` =9568 AND `gold_min` =1502;

-- Overlord Wyrmthalak
 UPDATE `creature_template` SET `gold_min` =620 WHERE `entry` =9583 AND `gold_min` =474;

-- Bloodaxe Veteran (sample size 24)
 UPDATE `creature_template` SET `gold_max` =2692 WHERE `entry` =9583 AND `gold_max` =624;

-- Bloodaxe Veteran (sample size 24)
 UPDATE `creature_template` SET `gold_max` =2688 WHERE `entry` =9596 AND `gold_max` =955;

-- Bannok Grimaxe
 UPDATE `creature_template` SET `gold_max` =1460 WHERE `entry` =9680 AND `gold_max` =681;

-- Crest Killer
 UPDATE `creature_template` SET `gold_max` =1732 WHERE `entry` =9692 AND `gold_max` =610;

-- Bloodaxe Raider
 UPDATE `creature_template` SET `gold_max` =2640 WHERE `entry` =9693 AND `gold_max` =546;

-- Bloodaxe Evoker
 UPDATE `creature_template` SET `gold_max` =2526 WHERE `entry` =9716 AND `gold_max` =616;

-- Bloodaxe Warmonger
 UPDATE `creature_template` SET `gold_max` =2104 WHERE `entry` =9717 AND `gold_max` =510;

-- Bloodaxe Summoner
 UPDATE `creature_template` SET `gold_min` =737 WHERE `entry` =9736 AND `gold_min` =1677;

-- Quartermaster Zigris
 UPDATE `creature_template` SET `gold_max` =376 WHERE `entry` =9776 AND `gold_max` =102;

-- Flamekin Spitter
 UPDATE `creature_template` SET `gold_max` =567 WHERE `entry` =9777 AND `gold_max` =88;

-- Flamekin Sprite
 UPDATE `creature_template` SET `gold_max` =2911 WHERE `entry` =9817 AND `gold_max` =548;

-- Blackhand Dreadweaver
 UPDATE `creature_template` SET `gold_max` =2937 WHERE `entry` =9818 AND `gold_max` =543;

-- Blackhand Summoner
 UPDATE `creature_template` SET `gold_max` =3001 WHERE `entry` =9819 AND `gold_max` =631;

-- Blackhand Veteran
 UPDATE `creature_template` SET `gold_max` =565 WHERE `entry` =9860 AND `gold_max` =155;

-- Salia
 UPDATE `creature_template` SET `gold_max` =301 WHERE `entry` =9861 AND `gold_max` =125;

-- Moora
 UPDATE `creature_template` SET `gold_max` =760 WHERE `entry` =9862 AND `gold_max` =154;

-- Jaedenar Legionnaire
 UPDATE `creature_template` SET `gold_max` =797 WHERE `entry` =9877 AND `gold_max` =140;

-- Prince Xavalis
 UPDATE `creature_template` SET `gold_max` =467 WHERE `entry` =9916 AND `gold_max` =119;

-- Jarquia
 UPDATE `creature_template` SET `gold_max` =752 WHERE `entry` =9956 AND `gold_max` =94;

-- Shadowforge Flame Keeper
 UPDATE `creature_template` SET `gold_max` =868 WHERE `entry` =10043 AND `gold_max` =513;

-- Ribbly's Crony
 UPDATE `creature_template` SET `gold_max` =1307 WHERE `entry` =10082 AND `gold_max` =463;

-- Zerillis
 UPDATE `creature_template` SET `gold_max` =2600 WHERE `entry` =10083 AND `gold_max` =535;

-- Rage Talon Flamescale
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =10158 AND `gold_min` =13;

-- Moonkin
 UPDATE `creature_template` SET `gold_max` =26 WHERE `entry` =10158 AND `gold_max` =22;

-- Moonkin
 UPDATE `creature_template` SET `gold_min` =9 WHERE `entry` =10160 AND `gold_min` =14;

-- Raging Moonkin
 UPDATE `creature_template` SET `gold_max` =24 WHERE `entry` =10160 AND `gold_max` =23;

-- Raging Moonkin
 UPDATE `creature_template` SET `gold_min` =193 WHERE `entry` =10316 AND `gold_min` =215;

-- Blackhand Incarcerator
 UPDATE `creature_template` SET `gold_max` =985 WHERE `entry` =10316 AND `gold_max` =286;

-- Blackhand Incarcerator
 UPDATE `creature_template` SET `gold_min` =528 WHERE `entry` =10317 AND `gold_min` =779;

-- Blackhand Elite
 UPDATE `creature_template` SET `gold_max` =3166 WHERE `entry` =10317 AND `gold_max` =1022;

-- Blackhand Elite
 UPDATE `creature_template` SET `gold_min` =766 WHERE `entry` =10318 AND `gold_min` =779;

-- Blackhand Assassin
 UPDATE `creature_template` SET `gold_max` =2658 WHERE `entry` =10318 AND `gold_max` =1022;

-- Blackhand Assassin
 UPDATE `creature_template` SET `gold_max` =2771 WHERE `entry` =10319 AND `gold_max` =800;

-- Blackhand Iron Guard
 UPDATE `creature_template` SET `gold_max` =55 WHERE `entry` =10323 AND `gold_max` =27;

-- Murkdeep
 UPDATE `creature_template` SET `gold_min` =5 WHERE `entry` =10358 AND `gold_min` =13;

-- Fellicent's Shade
 UPDATE `creature_template` SET `gold_min` =1439 WHERE `entry` =10363 AND `gold_min` =1771;

-- General Drakkisath
 UPDATE `creature_template` SET `gold_max` =2717 WHERE `entry` =10366 AND `gold_max` =1189;

-- Rage Talon Dragon Guard
 UPDATE `creature_template` SET `gold_max` =2568 WHERE `entry` =10371 AND `gold_max` =1193;

-- Rage Talon Captain
 UPDATE `creature_template` SET `gold_min` =619 WHERE `entry` =10372 AND `gold_min` =907;

-- Rage Talon Fire Tongue
 UPDATE `creature_template` SET `gold_max` =2979 WHERE `entry` =10372 AND `gold_max` =1189;

-- Rage Talon Fire Tongue
 UPDATE `creature_template` SET `gold_min` =423 WHERE `entry` =10381 AND `gold_min` =449;

-- Ravaged Cadaver (sample size 34)
 UPDATE `creature_template` SET `gold_max` =2614 WHERE `entry` =10381 AND `gold_max` =592;

-- Ravaged Cadaver (sample size 34)
 UPDATE `creature_template` SET `gold_min` =411 WHERE `entry` =10382 AND `gold_min` =435;

-- Mangled Cadaver (sample size 30)
 UPDATE `creature_template` SET `gold_max` =2481 WHERE `entry` =10382 AND `gold_max` =573;

-- Mangled Cadaver (sample size 30)
 UPDATE `creature_template` SET `gold_max` =522 WHERE `entry` =10384 AND `gold_max` =456;

-- Spectral Citizen
 UPDATE `creature_template` SET `gold_min` =122 WHERE `entry` =10390 AND `gold_min` =112;

-- Skeletal Guardian (sample size 85)
 UPDATE `creature_template` SET `gold_max` =821 WHERE `entry` =10390 AND `gold_max` =151;

-- Skeletal Guardian (sample size 85)
 UPDATE `creature_template` SET `gold_min` =136 WHERE `entry` =10391 AND `gold_min` =108;

-- Skeletal Berserker (sample size 74)
 UPDATE `creature_template` SET `gold_max` =843 WHERE `entry` =10391 AND `gold_max` =146;

-- Skeletal Berserker (sample size 74)
 UPDATE `creature_template` SET `gold_max` =2854 WHERE `entry` =10393 AND `gold_max` =1728;

-- Skul
 UPDATE `creature_template` SET `gold_max` =2416 WHERE `entry` =10394 AND `gold_max` =761;

-- Black Guard Sentry
 UPDATE `creature_template` SET `gold_max` =2613 WHERE `entry` =10398 AND `gold_max` =502;

-- Thuzadin Shadowcaster
 UPDATE `creature_template` SET `gold_min` =472 WHERE `entry` =10399 AND `gold_min` =113;

-- Thuzadin Acolyte (sample size 26)
 UPDATE `creature_template` SET `gold_max` =3028 WHERE `entry` =10399 AND `gold_max` =153;

-- Thuzadin Acolyte (sample size 26)
 UPDATE `creature_template` SET `gold_max` =2812 WHERE `entry` =10400 AND `gold_max` =501;

-- Thuzadin Necromancer
 UPDATE `creature_template` SET `gold_min` =420 WHERE `entry` =10405 AND `gold_min` =468;

-- Plague Ghoul (sample size 45)
 UPDATE `creature_template` SET `gold_max` =2677 WHERE `entry` =10405 AND `gold_max` =616;

-- Plague Ghoul (sample size 45)
 UPDATE `creature_template` SET `gold_min` =547 WHERE `entry` =10406 AND `gold_min` =474;

-- Ghoul Ravener (sample size 24)
 UPDATE `creature_template` SET `gold_max` =2727 WHERE `entry` =10406 AND `gold_max` =624;

-- Ghoul Ravener (sample size 24)
 UPDATE `creature_template` SET `gold_max` =2930 WHERE `entry` =10407 AND `gold_max` =648;

-- Fleshflayer Ghoul
 UPDATE `creature_template` SET `gold_max` =808 WHERE `entry` =10411 AND `gold_max` =143;

-- Eye of Naxxramas
 UPDATE `creature_template` SET `gold_max` =2663 WHERE `entry` =10414 AND `gold_max` =1071;

-- Patchwork Horror
 UPDATE `creature_template` SET `gold_min` =503 WHERE `entry` =10416 AND `gold_min` =735;

-- Bile Spewer
 UPDATE `creature_template` SET `gold_max` =2070 WHERE `entry` =10416 AND `gold_max` =964;

-- Bile Spewer
 UPDATE `creature_template` SET `gold_min` =743 WHERE `entry` =10417 AND `gold_min` =779;

-- Venom Belcher
 UPDATE `creature_template` SET `gold_max` =2721 WHERE `entry` =10417 AND `gold_max` =1022;

-- Venom Belcher
 UPDATE `creature_template` SET `gold_min` =393 WHERE `entry` =10418 AND `gold_min` =464;

-- Crimson Guardsman (sample size 37)
 UPDATE `creature_template` SET `gold_max` =2709 WHERE `entry` =10418 AND `gold_max` =610;

-- Crimson Guardsman (sample size 37)
 UPDATE `creature_template` SET `gold_max` =2525 WHERE `entry` =10419 AND `gold_max` =477;

-- Crimson Conjuror
 UPDATE `creature_template` SET `gold_min` =460 WHERE `entry` =10420 AND `gold_min` =396;

-- Crimson Initiate (sample size 21)
 UPDATE `creature_template` SET `gold_max` =2637 WHERE `entry` =10420 AND `gold_max` =522;

-- Crimson Initiate (sample size 21)
 UPDATE `creature_template` SET `gold_max` =2844 WHERE `entry` =10421 AND `gold_max` =529;

-- Crimson Defender
 UPDATE `creature_template` SET `gold_max` =2364 WHERE `entry` =10422 AND `gold_max` =485;

-- Crimson Sorcerer
 UPDATE `creature_template` SET `gold_max` =2296 WHERE `entry` =10423 AND `gold_max` =485;

-- Crimson Priest
 UPDATE `creature_template` SET `gold_min` =438 WHERE `entry` =10424 AND `gold_min` =416;

-- Crimson Gallant (sample size 22)
 UPDATE `creature_template` SET `gold_max` =2897 WHERE `entry` =10424 AND `gold_max` =548;

-- Crimson Gallant (sample size 22)
 UPDATE `creature_template` SET `gold_max` =2054 WHERE `entry` =10425 AND `gold_max` =504;

-- Crimson Battle Mage
 UPDATE `creature_template` SET `gold_max` =2388 WHERE `entry` =10426 AND `gold_max` =532;

-- Crimson Inquisitor
 UPDATE `creature_template` SET `gold_max` =2654 WHERE `entry` =10435 AND `gold_max` =1684;

-- Magistrate Barthilas
 UPDATE `creature_template` SET `gold_max` =2697 WHERE `entry` =10438 AND `gold_max` =1357;

-- Maleki the Pallid
 UPDATE `creature_template` SET `gold_max` =1757 WHERE `entry` =10439 AND `gold_max` =900;

-- Ramstein the Gorger
 UPDATE `creature_template` SET `gold_min` =159 WHERE `entry` =10442 AND `gold_min` =120;

-- Chromatic Whelp (sample size 21)
 UPDATE `creature_template` SET `gold_max` =896 WHERE `entry` =10442 AND `gold_max` =161;

-- Chromatic Whelp (sample size 21)
 UPDATE `creature_template` SET `gold_min` =663 WHERE `entry` =10447 AND `gold_min` =735;

-- Chromatic Dragonspawn
 UPDATE `creature_template` SET `gold_max` =2288 WHERE `entry` =10447 AND `gold_max` =964;

-- Chromatic Dragonspawn
 UPDATE `creature_template` SET `gold_max` =2746 WHERE `entry` =10463 AND `gold_max` =610;

-- Shrieking Banshee
 UPDATE `creature_template` SET `gold_max` =2869 WHERE `entry` =10464 AND `gold_max` =629;

-- Wailing Banshee
 UPDATE `creature_template` SET `gold_min` =516 WHERE `entry` =10469 AND `gold_min` =402;

-- Scholomance Adept (sample size 45)
 UPDATE `creature_template` SET `gold_max` =2862 WHERE `entry` =10469 AND `gold_max` =529;

-- Scholomance Adept (sample size 45)
 UPDATE `creature_template` SET `gold_max` =2748 WHERE `entry` =10470 AND `gold_max` =472;

-- Scholomance Neophyte
 UPDATE `creature_template` SET `gold_max` =2587 WHERE `entry` =10471 AND `gold_max` =510;

-- Scholomance Acolyte
 UPDATE `creature_template` SET `gold_min` =511 WHERE `entry` =10476 AND `gold_min` =391;

-- Scholomance Necrolyte (sample size 22)
 UPDATE `creature_template` SET `gold_max` =2747 WHERE `entry` =10476 AND `gold_max` =516;

-- Scholomance Necrolyte (sample size 22)
 UPDATE `creature_template` SET `gold_min` =465 WHERE `entry` =10477 AND `gold_min` =406;

-- Scholomance Necromancer (sample size 36)
 UPDATE `creature_template` SET `gold_max` =2834 WHERE `entry` =10477 AND `gold_max` =535;

-- Scholomance Necromancer (sample size 36)
 UPDATE `creature_template` SET `gold_max` =2653 WHERE `entry` =10478 AND `gold_max` =643;

-- Splintered Skeleton
 UPDATE `creature_template` SET `gold_min` =141 WHERE `entry` =10480 AND `gold_min` =133;

-- Unstable Corpse (sample size 48)
 UPDATE `creature_template` SET `gold_max` =940 WHERE `entry` =10480 AND `gold_max` =179;

-- Unstable Corpse (sample size 48)
 UPDATE `creature_template` SET `gold_min` =145 WHERE `entry` =10481 AND `gold_min` =89;

-- Reanimated Corpse (sample size 23)
 UPDATE `creature_template` SET `gold_max` =954 WHERE `entry` =10481 AND `gold_max` =121;

-- Reanimated Corpse (sample size 23)
 UPDATE `creature_template` SET `gold_min` =132 WHERE `entry` =10485 AND `gold_min` =101;

-- Risen Aberration (sample size 34)
 UPDATE `creature_template` SET `gold_max` =916 WHERE `entry` =10485 AND `gold_max` =136;

-- Risen Aberration (sample size 34)
 UPDATE `creature_template` SET `gold_min` =477 WHERE `entry` =10486 AND `gold_min` =777;

-- Risen Warrior
 UPDATE `creature_template` SET `gold_max` =2890 WHERE `entry` =10486 AND `gold_max` =1020;

-- Risen Warrior
 UPDATE `creature_template` SET `gold_max` =2637 WHERE `entry` =10487 AND `gold_max` =540;

-- Risen Protector
 UPDATE `creature_template` SET `gold_min` =664 WHERE `entry` =10488 AND `gold_min` =776;

-- Risen Construct
 UPDATE `creature_template` SET `gold_max` =2707 WHERE `entry` =10488 AND `gold_max` =1018;

-- Risen Construct
 UPDATE `creature_template` SET `gold_min` =412 WHERE `entry` =10489 AND `gold_min` =472;

-- Risen Guard (sample size 25)
 UPDATE `creature_template` SET `gold_max` =2500 WHERE `entry` =10489 AND `gold_max` =621;

-- Risen Guard (sample size 25)
 UPDATE `creature_template` SET `gold_max` =2194 WHERE `entry` =10491 AND `gold_max` =535;

-- Risen Bonewarder
 UPDATE `creature_template` SET `gold_min` =535 WHERE `entry` =10495 AND `gold_min` =470;

-- Diseased Ghoul (sample size 25)
 UPDATE `creature_template` SET `gold_max` =2844 WHERE `entry` =10495 AND `gold_max` =618;

-- Diseased Ghoul (sample size 25)
 UPDATE `creature_template` SET `gold_max` =2832 WHERE `entry` =10498 AND `gold_max` =546;

-- Spectral Tutor
 UPDATE `creature_template` SET `gold_max` =2285 WHERE `entry` =10499 AND `gold_max` =540;

-- Spectral Researcher
 UPDATE `creature_template` SET `gold_max` =2640 WHERE `entry` =10500 AND `gold_max` =563;

-- Spectral Teacher
 UPDATE `creature_template` SET `gold_max` =3038 WHERE `entry` =10502 AND `gold_max` =1633;

-- Lady Illucia Barov
 UPDATE `creature_template` SET `gold_max` =2572 WHERE `entry` =10504 AND `gold_max` =1633;

-- Lord Alexei Barov
 UPDATE `creature_template` SET `gold_max` =2122 WHERE `entry` =10505 AND `gold_max` =2021;

-- Instructor Malicia
 UPDATE `creature_template` SET `gold_min` =470 WHERE `entry` =10507 AND `gold_min` =1629;

-- The Ravenian
 UPDATE `creature_template` SET `gold_max` =3291 WHERE `entry` =10508 AND `gold_max` =1970;

-- Ras Frostwhisper
 UPDATE `creature_template` SET `gold_max` =2708 WHERE `entry` =10558 AND `gold_max` =774;

-- Hearthsinger Forresten
 UPDATE `creature_template` SET `gold_max` =727 WHERE `entry` =10580 AND `gold_max` =164;

-- Fetid Zombie
 UPDATE `creature_template` SET `gold_max` =714 WHERE `entry` =10605 AND `gold_max` =134;

-- Scarlet Medic
 UPDATE `creature_template` SET `gold_max` =1897 WHERE `entry` =10608 AND `gold_max` =528;

-- Scarlet Priest
 UPDATE `creature_template` SET `gold_min` =35 WHERE `entry` =10617 AND `gold_min` =37;

-- Galak Messenger
 UPDATE `creature_template` SET `gold_max` =97 WHERE `entry` =10617 AND `gold_max` =53;

-- Galak Messenger
 UPDATE `creature_template` SET `gold_max` =135 WHERE `entry` =10640 AND `gold_max` =55;

-- Oakpaw
 UPDATE `creature_template` SET `gold_max` =88 WHERE `entry` =10641 AND `gold_max` =49;

-- Branch Snapper
 UPDATE `creature_template` SET `gold_max` =156 WHERE `entry` =10647 AND `gold_max` =73;

-- Prince Raze
 UPDATE `creature_template` SET `gold_max` =712 WHERE `entry` =10648 AND `gold_max` =140;

-- Xavaric
 UPDATE `creature_template` SET `gold_min` =133 WHERE `entry` =10678 AND `gold_min` =130;

-- Plagued Hatchling (sample size 43)
 UPDATE `creature_template` SET `gold_max` =916 WHERE `entry` =10678 AND `gold_max` =175;

-- Plagued Hatchling (sample size 43)
 UPDATE `creature_template` SET `gold_max` =1363 WHERE `entry` =10680 AND `gold_max` =543;

-- Summoned Blackhand Dreadweaver
 UPDATE `creature_template` SET `gold_max` =903 WHERE `entry` =10683 AND `gold_max` =761;

-- Rookery Hatcher
 UPDATE `creature_template` SET `gold_max` =2823 WHERE `entry` =10738 AND `gold_max` =526;

-- High Chief Winterfall
 UPDATE `creature_template` SET `gold_max` =1506 WHERE `entry` =10742 AND `gold_max` =648;

-- Blackhand Dragon Handler
 UPDATE `creature_template` SET `gold_min` =18 WHERE `entry` =10758 AND `gold_min` =35;

-- Grimtotem Bandit (sample size 35)
 UPDATE `creature_template` SET `gold_max` =123 WHERE `entry` =10758 AND `gold_max` =51;

-- Grimtotem Bandit (sample size 35)
 UPDATE `creature_template` SET `gold_min` =27 WHERE `entry` =10759 AND `gold_min` =34;

-- Grimtotem Stomper (sample size 31)
 UPDATE `creature_template` SET `gold_max` =132 WHERE `entry` =10759 AND `gold_max` =50;

-- Grimtotem Stomper (sample size 31)
 UPDATE `creature_template` SET `gold_min` =17 WHERE `entry` =10760 AND `gold_min` =32;

-- Grimtotem Geomancer (sample size 45)
 UPDATE `creature_template` SET `gold_max` =121 WHERE `entry` =10760 AND `gold_max` =47;

-- Grimtotem Geomancer (sample size 45)
 UPDATE `creature_template` SET `gold_max` =154 WHERE `entry` =10761 AND `gold_max` =59;

-- Grimtotem Reaver
 UPDATE `creature_template` SET `gold_max` =652 WHERE `entry` =10801 AND `gold_max` =153;

-- Jabbering Ghoul
 UPDATE `creature_template` SET `gold_max` =1414 WHERE `entry` =10802 AND `gold_max` =558;

-- Hitah'ya the Keeper
 UPDATE `creature_template` SET `gold_max` =2828 WHERE `entry` =10808 AND `gold_max` =708;

-- Timmy the Cruel
 UPDATE `creature_template` SET `gold_max` =2609 WHERE `entry` =10811 AND `gold_max` =1189;

-- Archivist Galford
 UPDATE `creature_template` SET `gold_max` =3148 WHERE `entry` =10813 AND `gold_max` =2307;

-- Balnazzar
 UPDATE `creature_template` SET `gold_max` =2830 WHERE `entry` =10814 AND `gold_max` =1464;

-- Chromatic Elite Guard
 UPDATE `creature_template` SET `gold_max` =773 WHERE `entry` =10816 AND `gold_max` =159;

-- Wandering Skeleton
 UPDATE `creature_template` SET `gold_max` =159 WHERE `entry` =10896 AND `gold_max` =61;

-- Arnak Grimtotem
 UPDATE `creature_template` SET `gold_max` =1862 WHERE `entry` =10901 AND `gold_max` =1633;

-- Lorekeeper Polkelt
 UPDATE `creature_template` SET `gold_max` =896 WHERE `entry` =10916 AND `gold_max` =170;

-- Winterfall Runner
 UPDATE `creature_template` SET `gold_min` =126 WHERE `entry` =10986 AND `gold_min` =59;

-- Snowblind Harpy (sample size 25)
 UPDATE `creature_template` SET `gold_max` =699 WHERE `entry` =10986 AND `gold_max` =296;

-- Snowblind Harpy (sample size 25)
 UPDATE `creature_template` SET `gold_max` =488 WHERE `entry` =10991 AND `gold_max` =305;

-- Wildpaw Gnoll
 UPDATE `creature_template` SET `gold_max` =2726 WHERE `entry` =11032 AND `gold_max` =1078;

-- Malor the Zealous
 UPDATE `creature_template` SET `gold_min` =486 WHERE `entry` =11043 AND `gold_min` =491;

-- Crimson Monk
 UPDATE `creature_template` SET `gold_max` =2365 WHERE `entry` =11043 AND `gold_max` =646;

-- Crimson Monk
 UPDATE `creature_template` SET `gold_max` =1981 WHERE `entry` =11058 AND `gold_max` =864;

-- Fras Siabi
 UPDATE `creature_template` SET `gold_max` =2895 WHERE `entry` =11257 AND `gold_max` =554;

-- Scholomance Handler
 UPDATE `creature_template` SET `gold_max` =2253 WHERE `entry` =11261 AND `gold_max` =1799;

-- Doctor Theolen Krastinov
 UPDATE `creature_template` SET `gold_max` =2857 WHERE `entry` =11284 AND `gold_max` =812;

-- Dark Shade
 UPDATE `creature_template` SET `gold_min` =110 WHERE `entry` =11290 AND `gold_min` =112;

-- Mossflayer Zombie
 UPDATE `creature_template` SET `gold_max` =741 WHERE `entry` =11290 AND `gold_max` =151;

-- Mossflayer Zombie
 UPDATE `creature_template` SET `gold_max` =531 WHERE `entry` =11291 AND `gold_max` =158;

-- Unliving Mossflayer
 UPDATE `creature_template` SET `gold_min` =19 WHERE `entry` =11318 AND `gold_min` =60;

-- Ragefire Trogg (sample size 24)
 UPDATE `creature_template` SET `gold_max` =85 WHERE `entry` =11318 AND `gold_max` =84;

-- Ragefire Trogg (sample size 24)
 UPDATE `creature_template` SET `gold_min` =18 WHERE `entry` =11319 AND `gold_min` =59;

-- Ragefire Shaman
 UPDATE `creature_template` SET `gold_max` =82 WHERE `entry` =11319 AND `gold_max` =81;

-- Ragefire Shaman
 UPDATE `creature_template` SET `gold_min` =12 WHERE `entry` =11322 AND `gold_min` =59;

-- Searing Blade Cultist (sample size 35)
 UPDATE `creature_template` SET `gold_max` =87 WHERE `entry` =11322 AND `gold_max` =81;

-- Searing Blade Cultist (sample size 35)
 UPDATE `creature_template` SET `gold_min` =13 WHERE `entry` =11323 AND `gold_min` =62;

-- Searing Blade Enforcer (sample size 26)
 UPDATE `creature_template` SET `gold_max` =87 WHERE `entry` =11323 AND `gold_max` =86;

-- Searing Blade Enforcer (sample size 26)
 UPDATE `creature_template` SET `gold_min` =30 WHERE `entry` =11324 AND `gold_min` =57;

-- Searing Blade Warlock
 UPDATE `creature_template` SET `gold_max` =84 WHERE `entry` =11324 AND `gold_max` =80;

-- Searing Blade Warlock
 UPDATE `creature_template` SET `gold_max` =2162 WHERE `entry` =11338 AND `gold_max` =1513;

-- Hakkari Shadowcaster
 UPDATE `creature_template` SET `gold_min` =542 WHERE `entry` =11339 AND `gold_min` =1163;

-- Hakkari Shadow Hunter
 UPDATE `creature_template` SET `gold_max` =2246 WHERE `entry` =11339 AND `gold_max` =1524;

-- Hakkari Shadow Hunter
 UPDATE `creature_template` SET `gold_max` =1290 WHERE `entry` =11340 AND `gold_max` =969;

-- Hakkari Blood Priest
 UPDATE `creature_template` SET `gold_min` =753 WHERE `entry` =11350 AND `gold_min` =992;

-- Gurubashi Axe Thrower
 UPDATE `creature_template` SET `gold_max` =3174 WHERE `entry` =11350 AND `gold_max` =1300;

-- Gurubashi Axe Thrower
 UPDATE `creature_template` SET `gold_min` =574 WHERE `entry` =11351 AND `gold_min` =992;

-- Gurubashi Headhunter
 UPDATE `creature_template` SET `gold_max` =2247 WHERE `entry` =11351 AND `gold_max` =1300;

-- Gurubashi Headhunter
 UPDATE `creature_template` SET `gold_max` =3429 WHERE `entry` =11352 AND `gold_max` =2803;

-- Gurubashi Berserker
 UPDATE `creature_template` SET `gold_min` =476 WHERE `entry` =11353 AND `gold_min` =907;

-- Gurubashi Blood Drinker
 UPDATE `creature_template` SET `gold_max` =3084 WHERE `entry` =11353 AND `gold_max` =1189;

-- Gurubashi Blood Drinker
 UPDATE `creature_template` SET `gold_min` =693 WHERE `entry` =11356 AND `gold_min` =1418;

-- Gurubashi Champion
 UPDATE `creature_template` SET `gold_max` =3265 WHERE `entry` =11356 AND `gold_max` =1857;

-- Gurubashi Champion
 UPDATE `creature_template` SET `gold_max` =410164 WHERE `entry` =11382 AND `gold_max` =405013;

-- Bloodlord Mandokir
 UPDATE `creature_template` SET `gold_max` =2155 WHERE `entry` =11387 AND `gold_max` =171;

-- Sandfury Speaker
 UPDATE `creature_template` SET `gold_max` =2571 WHERE `entry` =11391 AND `gold_max` =182;

-- Vilebranch Speaker
 UPDATE `creature_template` SET `gold_max` =2711 WHERE `entry` =11441 AND `gold_max` =905;

-- Gordok Brute
 UPDATE `creature_template` SET `gold_max` =1764 WHERE `entry` =11442 AND `gold_max` =546;

-- Gordok Mauler
 UPDATE `creature_template` SET `gold_max` =511 WHERE `entry` =11443 AND `gold_max` =449;

-- Gordok Ogre-Mage
 UPDATE `creature_template` SET `gold_max` =1812 WHERE `entry` =11444 AND `gold_max` =683;

-- Gordok Mage-Lord
 UPDATE `creature_template` SET `gold_max` =2093 WHERE `entry` =11445 AND `gold_max` =1020;

-- Gordok Captain
 UPDATE `creature_template` SET `gold_min` =481 WHERE `entry` =11448 AND `gold_min` =0;

-- Gordok Warlock (sample size 4)
 UPDATE `creature_template` SET `gold_max` =2236 WHERE `entry` =11448 AND `gold_max` =0;

-- Gordok Warlock (sample size 4)
 UPDATE `creature_template` SET `gold_max` =2232 WHERE `entry` =11450 AND `gold_max` =1018;

-- Gordok Reaver
 UPDATE `creature_template` SET `gold_max` =723 WHERE `entry` =11451 AND `gold_max` =578;

-- Wildspawn Satyr
 UPDATE `creature_template` SET `gold_max` =625 WHERE `entry` =11452 AND `gold_max` =597;

-- Wildspawn Rogue
 UPDATE `creature_template` SET `gold_max` =2439 WHERE `entry` =11453 AND `gold_max` =492;

-- Wildspawn Trickster
 UPDATE `creature_template` SET `gold_max` =1067 WHERE `entry` =11454 AND `gold_max` =584;

-- Wildspawn Betrayer
 UPDATE `creature_template` SET `gold_max` =1966 WHERE `entry` =11455 AND `gold_max` =451;

-- Wildspawn Felsworn
 UPDATE `creature_template` SET `gold_max` =1787 WHERE `entry` =11456 AND `gold_max` =580;

-- Wildspawn Shadowstalker
 UPDATE `creature_template` SET `gold_max` =1128 WHERE `entry` =11457 AND `gold_max` =503;

-- Wildspawn Hellcaller
 UPDATE `creature_template` SET `gold_max` =1918 WHERE `entry` =11459 AND `gold_max` =1127;

-- Ironbark Protector
 UPDATE `creature_template` SET `gold_max` =1306 WHERE `entry` =11461 AND `gold_max` =522;

-- Warpwood Guardian
 UPDATE `creature_template` SET `gold_max` =2236 WHERE `entry` =11462 AND `gold_max` =559;

-- Warpwood Treant
 UPDATE `creature_template` SET `gold_max` =1996 WHERE `entry` =11464 AND `gold_max` =484;

-- Warpwood Tangler
 UPDATE `creature_template` SET `gold_max` =1842 WHERE `entry` =11470 AND `gold_max` =529;

-- Eldreth Sorcerer
 UPDATE `creature_template` SET `gold_max` =2405 WHERE `entry` =11471 AND `gold_max` =522;

-- Eldreth Apparition
 UPDATE `creature_template` SET `gold_max` =2684 WHERE `entry` =11472 AND `gold_max` =610;

-- Eldreth Spirit
 UPDATE `creature_template` SET `gold_max` =2858 WHERE `entry` =11473 AND `gold_max` =529;

-- Eldreth Spectre
 UPDATE `creature_template` SET `gold_max` =2299 WHERE `entry` =11475 AND `gold_max` =629;

-- Eldreth Phantasm
 UPDATE `creature_template` SET `gold_max` =780 WHERE `entry` =11476 AND `gold_max` =172;

-- Skeletal Highborne
 UPDATE `creature_template` SET `gold_max` =416 WHERE `entry` =11477 AND `gold_max` =176;

-- Rotting Highborne
 UPDATE `creature_template` SET `gold_max` =3263 WHERE `entry` =11486 AND `gold_max` =1857;

-- Prince Tortheldrin
 UPDATE `creature_template` SET `gold_max` =2111 WHERE `entry` =11487 AND `gold_max` =1300;

-- Magister Kalendris
 UPDATE `creature_template` SET `gold_max` =1222 WHERE `entry` =11488 AND `gold_max` =1078;

-- Illyanna Ravenoak
 UPDATE `creature_template` SET `gold_min` =731 WHERE `entry` =11489 AND `gold_min` =1502;

-- Tendris Warpwood
 UPDATE `creature_template` SET `gold_min` =21 WHERE `entry` =11518 AND `gold_min` =88;

-- Jergosh the Invoker
 UPDATE `creature_template` SET `gold_min` =33 WHERE `entry` =11519 AND `gold_min` =92;

-- Bazzalan
 UPDATE `creature_template` SET `gold_min` =50 WHERE `entry` =11520 AND `gold_min` =109;

-- Taragaman the Hungerer
 UPDATE `creature_template` SET `gold_max` =294 WHERE `entry` =11559 AND `gold_max` =78;

-- Outcast Necromancer
 UPDATE `creature_template` SET `gold_min` =69 WHERE `entry` =11561 AND `gold_min` =62;

-- Undead Ravager (sample size 41)
 UPDATE `creature_template` SET `gold_max` =292 WHERE `entry` =11561 AND `gold_max` =86;

-- Undead Ravager (sample size 41)
 UPDATE `creature_template` SET `gold_min` =41 WHERE `entry` =11562 AND `gold_min` =46;

-- Drysnap Crawler (sample size 23)
 UPDATE `creature_template` SET `gold_max` =205 WHERE `entry` =11562 AND `gold_max` =65;

-- Drysnap Crawler (sample size 23)
 UPDATE `creature_template` SET `gold_min` =40 WHERE `entry` =11563 AND `gold_min` =51;

-- Drysnap Pincer
 UPDATE `creature_template` SET `gold_max` =239 WHERE `entry` =11563 AND `gold_max` =72;

-- Drysnap Pincer
 UPDATE `creature_template` SET `gold_max` =2710 WHERE `entry` =11582 AND `gold_max` =535;

-- Scholomance Dark Summoner
 UPDATE `creature_template` SET `gold_max` =866 WHERE `entry` =11598 AND `gold_max` =326;

-- Risen Guardian
 UPDATE `creature_template` SET `gold_max` =615 WHERE `entry` =11611 AND `gold_max` =150;

-- Cavalier Durgen
 UPDATE `creature_template` SET `gold_max` =788 WHERE `entry` =11613 AND `gold_max` =166;

-- Huntsman Radley
 UPDATE `creature_template` SET `gold_min` =1861 WHERE `entry` =11661 AND `gold_min` =3417;

-- Flamewaker
 UPDATE `creature_template` SET `gold_min` =1989 WHERE `entry` =11662 AND `gold_min` =0;

-- Flamewaker Priest (sample size 1)
 UPDATE `creature_template` SET `gold_max` =3242 WHERE `entry` =11662 AND `gold_max` =0;

-- Flamewaker Priest (sample size 1)
 UPDATE `creature_template` SET `gold_min` =122 WHERE `entry` =11675 AND `gold_min` =61;

-- Snowblind Windcaller (sample size 21)
 UPDATE `creature_template` SET `gold_max` =638 WHERE `entry` =11675 AND `gold_max` =305;

-- Snowblind Windcaller (sample size 21)
 UPDATE `creature_template` SET `gold_max` =675 WHERE `entry` =11678 AND `gold_max` =305;

-- Snowblind Ambusher
 UPDATE `creature_template` SET `gold_max` =128 WHERE `entry` =11680 AND `gold_max` =52;

-- Horde Scout
 UPDATE `creature_template` SET `gold_max` =141 WHERE `entry` =11685 AND `gold_max` =79;

-- Maraudine Priest
 UPDATE `creature_template` SET `gold_max` =354 WHERE `entry` =11686 AND `gold_max` =98;

-- Ghostly Raider
 UPDATE `creature_template` SET `gold_max` =256 WHERE `entry` =11687 AND `gold_max` =96;

-- Ghostly Marauder
 UPDATE `creature_template` SET `gold_min` =24 WHERE `entry` =11697 AND `gold_min` =40;

-- Mannoroc Lasher
 UPDATE `creature_template` SET `gold_max` =117 WHERE `entry` =11697 AND `gold_max` =58;

-- Mannoroc Lasher
 UPDATE `creature_template` SET `gold_max` =58 WHERE `entry` =11714 AND `gold_max` =23;

-- Marosh the Devious
 UPDATE `creature_template` SET `gold_max` =856 WHERE `entry` =11790 AND `gold_max` =396;

-- Putridus Satyr
 UPDATE `creature_template` SET `gold_min` =199 WHERE `entry` =11791 AND `gold_min` =310;

-- Putridus Trickster
 UPDATE `creature_template` SET `gold_max` =1008 WHERE `entry` =11791 AND `gold_max` =409;

-- Putridus Trickster
 UPDATE `creature_template` SET `gold_max` =856 WHERE `entry` =11792 AND `gold_max` =394;

-- Putridus Shadowstalker
 UPDATE `creature_template` SET `gold_max` =1090 WHERE `entry` =11793 AND `gold_max` =362;

-- Celebrian Dryad
 UPDATE `creature_template` SET `gold_max` =1476 WHERE `entry` =11794 AND `gold_max` =436;

-- Sister of Celebrian
 UPDATE `creature_template` SET `gold_min` =681 WHERE `entry` =11830 AND `gold_min` =822;

-- Hakkari Priest
 UPDATE `creature_template` SET `gold_max` =2107 WHERE `entry` =11830 AND `gold_max` =1078;

-- Hakkari Priest
 UPDATE `creature_template` SET `gold_min` =877 WHERE `entry` =11831 AND `gold_min` =1119;

-- Hakkari Witch Doctor
 UPDATE `creature_template` SET `gold_max` =2480 WHERE `entry` =11831 AND `gold_max` =1466;

-- Hakkari Witch Doctor
 UPDATE `creature_template` SET `gold_max` =721 WHERE `entry` =11837 AND `gold_max` =305;

-- Wildpaw Shaman
 UPDATE `creature_template` SET `gold_max` =69 WHERE `entry` =11858 AND `gold_max` =32;

-- Grundig Darkcloud
 UPDATE `creature_template` SET `gold_max` =544 WHERE `entry` =11873 AND `gold_max` =159;

-- Spectral Attendant
 UPDATE `creature_template` SET `gold_min` =131 WHERE `entry` =11880 AND `gold_min` =133;

-- Twilight Avenger
 UPDATE `creature_template` SET `gold_max` =842 WHERE `entry` =11880 AND `gold_max` =179;

-- Twilight Avenger
 UPDATE `creature_template` SET `gold_max` =911 WHERE `entry` =11881 AND `gold_max` =183;

-- Twilight Geolord
 UPDATE `creature_template` SET `gold_max` =549 WHERE `entry` =11883 AND `gold_max` =162;

-- Twilight Master
 UPDATE `creature_template` SET `gold_max` =2033 WHERE `entry` =11887 AND `gold_max` =305;

-- Crypt Robber
 UPDATE `creature_template` SET `gold_min` =4 WHERE `entry` =11910 AND `gold_min` =16;

-- Grimtotem Ruffian
 UPDATE `creature_template` SET `gold_max` =34 WHERE `entry` =11910 AND `gold_max` =26;

-- Grimtotem Ruffian
 UPDATE `creature_template` SET `gold_min` =5 WHERE `entry` =11911 AND `gold_min` =16;

-- Grimtotem Mercenary
 UPDATE `creature_template` SET `gold_max` =30 WHERE `entry` =11911 AND `gold_max` =26;

-- Grimtotem Mercenary
 UPDATE `creature_template` SET `gold_min` =7 WHERE `entry` =11912 AND `gold_min` =17;

-- Grimtotem Brute
 UPDATE `creature_template` SET `gold_max` =37 WHERE `entry` =11912 AND `gold_max` =28;

-- Grimtotem Brute
 UPDATE `creature_template` SET `gold_min` =10 WHERE `entry` =11913 AND `gold_min` =17;

-- Grimtotem Sorcerer
 UPDATE `creature_template` SET `gold_max` =38 WHERE `entry` =11913 AND `gold_max` =27;

-- Grimtotem Sorcerer
 UPDATE `creature_template` SET `gold_max` =44 WHERE `entry` =11914 AND `gold_max` =34;

-- Gorehoof the Black
 UPDATE `creature_template` SET `gold_min` =7 WHERE `entry` =11915 AND `gold_min` =18;

-- Gogger Rock Keeper (sample size 23)
 UPDATE `creature_template` SET `gold_max` =49 WHERE `entry` =11915 AND `gold_max` =28;

-- Gogger Rock Keeper (sample size 23)
 UPDATE `creature_template` SET `gold_min` =8 WHERE `entry` =11917 AND `gold_min` =18;

-- Gogger Geomancer (sample size 22)
 UPDATE `creature_template` SET `gold_max` =49 WHERE `entry` =11917 AND `gold_max` =28;

-- Gogger Geomancer (sample size 22)
 UPDATE `creature_template` SET `gold_min` =13 WHERE `entry` =11918 AND `gold_min` =20;

-- Gogger Stonepounder
 UPDATE `creature_template` SET `gold_max` =52 WHERE `entry` =11918 AND `gold_max` =32;

-- Gogger Stonepounder
 UPDATE `creature_template` SET `gold_max` =458 WHERE `entry` =12046 AND `gold_max` =133;

-- Gor'marok the Ravager
 UPDATE `creature_template` SET `gold_max` =604 WHERE `entry` =12050 AND `gold_max` =352;

-- Stormpike Defender
 UPDATE `creature_template` SET `gold_max` =772 WHERE `entry` =12127 AND `gold_max` =416;

-- Stormpike Guardsman
 UPDATE `creature_template` SET `gold_max` =3033 WHERE `entry` =12128 AND `gold_max` =880;

-- Crimson Elite
 UPDATE `creature_template` SET `gold_min` =1185 WHERE `entry` =12129 AND `gold_min` =3627;

-- Onyxian Warder
 UPDATE `creature_template` SET `gold_min` =12 WHERE `entry` =12138 AND `gold_min` =0;

-- Lunaclaw (sample size 1)
 UPDATE `creature_template` SET `gold_max` =16 WHERE `entry` =12138 AND `gold_max` =0;

-- Lunaclaw (sample size 1)
 UPDATE `creature_template` SET `gold_max` =513 WHERE `entry` =12178 AND `gold_max` =145;

-- Tortured Druid
 UPDATE `creature_template` SET `gold_max` =753 WHERE `entry` =12179 AND `gold_max` =166;

-- Tortured Sentinel
 UPDATE `creature_template` SET `gold_max` =2040 WHERE `entry` =12201 AND `gold_max` =1391;

-- Princess Theradras
 UPDATE `creature_template` SET `gold_min` =582 WHERE `entry` =12203 AND `gold_min` =720;

-- Landslide
 UPDATE `creature_template` SET `gold_max` =1360 WHERE `entry` =12203 AND `gold_max` =945;

-- Landslide
 UPDATE `creature_template` SET `gold_min` =410 WHERE `entry` =12206 AND `gold_min` =631;

-- Primordial Behemoth
 UPDATE `creature_template` SET `gold_max` =1610 WHERE `entry` =12206 AND `gold_max` =829;

-- Primordial Behemoth
 UPDATE `creature_template` SET `gold_max` =281 WHERE `entry` =12216 AND `gold_max` =89;

-- Poison Sprite
 UPDATE `creature_template` SET `gold_max` =267 WHERE `entry` =12217 AND `gold_max` =86;

-- Corruptor
 UPDATE `creature_template` SET `gold_min` =179 WHERE `entry` =12221 AND `gold_min` =292;

-- Noxious Slime
 UPDATE `creature_template` SET `gold_max` =866 WHERE `entry` =12221 AND `gold_max` =386;

-- Noxious Slime
 UPDATE `creature_template` SET `gold_min` =221 WHERE `entry` =12222 AND `gold_min` =320;

-- Creeping Sludge
 UPDATE `creature_template` SET `gold_max` =498 WHERE `entry` =12222 AND `gold_max` =423;

-- Creeping Sludge
 UPDATE `creature_template` SET `gold_max` =1162 WHERE `entry` =12223 AND `gold_max` =423;

-- Cavern Lurker
 UPDATE `creature_template` SET `gold_max` =845 WHERE `entry` =12248 AND `gold_max` =184;

-- Infiltrator Hameya
 UPDATE `creature_template` SET `gold_max` =695 WHERE `entry` =12250 AND `gold_max` =159;

-- Zaeldarr the Outcast
 UPDATE `creature_template` SET `gold_max` =902 WHERE `entry` =12261 AND `gold_max` =305;

-- Infected Mossflayer
 UPDATE `creature_template` SET `gold_max` =455 WHERE `entry` =12322 AND `gold_max` =204;

-- Quel'Lithien Protector
 UPDATE `creature_template` SET `gold_min` =137 WHERE `entry` =12377 AND `gold_min` =234;

-- Wailing Spectre (sample size 21)
 UPDATE `creature_template` SET `gold_max` =897 WHERE `entry` =12377 AND `gold_max` =311;

-- Wailing Spectre (sample size 21)
 UPDATE `creature_template` SET `gold_min` =162 WHERE `entry` =12378 AND `gold_min` =193;

-- Damned Soul
 UPDATE `creature_template` SET `gold_max` =848 WHERE `entry` =12378 AND `gold_max` =257;

-- Damned Soul
 UPDATE `creature_template` SET `gold_min` =167 WHERE `entry` =12379 AND `gold_min` =204;

-- Unliving Caretaker
 UPDATE `creature_template` SET `gold_max` =946 WHERE `entry` =12379 AND `gold_max` =272;

-- Unliving Caretaker
 UPDATE `creature_template` SET `gold_min` =146 WHERE `entry` =12380 AND `gold_min` =211;

-- Unliving Resident (sample size 25)
 UPDATE `creature_template` SET `gold_max` =946 WHERE `entry` =12380 AND `gold_max` =280;

-- Unliving Resident (sample size 25)
 UPDATE `creature_template` SET `gold_min` =6610 WHERE `entry` =12468 AND `gold_min` =0;

-- Death Talon Hatcher (sample size 1)
 UPDATE `creature_template` SET `gold_max` =6720 WHERE `entry` =12468 AND `gold_max` =0;

-- Death Talon Hatcher (sample size 1)
 UPDATE `creature_template` SET `gold_min` =579 WHERE `entry` =12475 AND `gold_min` =3372;

-- Emeraldon Tree Warder
 UPDATE `creature_template` SET `gold_min` =2104 WHERE `entry` =12477 AND `gold_min` =3502;

-- Verdantine Boughguard
 UPDATE `creature_template` SET `gold_min` =2655 WHERE `entry` =12478 AND `gold_min` =2863;

-- Verdantine Oracle
 UPDATE `creature_template` SET `gold_min` =3163 WHERE `entry` =12498 AND `gold_min` =3502;

-- Dreamstalker
 UPDATE `creature_template` SET `gold_max` =260 WHERE `entry` =12579 AND `gold_max` =181;

-- Bloodfury Ripper
 UPDATE `creature_template` SET `gold_min` =15 WHERE `entry` =12856 AND `gold_min` =29;

-- Ashenvale Outrunner
 UPDATE `creature_template` SET `gold_max` =100 WHERE `entry` =12856 AND `gold_max` =43;

-- Ashenvale Outrunner
 UPDATE `creature_template` SET `gold_max` =809 WHERE `entry` =12865 AND `gold_max` =270;

-- Ambassador Malcin
 UPDATE `creature_template` SET `gold_max` =68 WHERE `entry` =12896 AND `gold_max` =43;

-- Silverwing Sentinel
 UPDATE `creature_template` SET `gold_max` =80 WHERE `entry` =12897 AND `gold_max` =40;

-- Silverwing Warrior
 UPDATE `creature_template` SET `gold_max` =167 WHERE `entry` =12976 AND `gold_max` =138;

-- Kolkar Waylayer
 UPDATE `creature_template` SET `gold_max` =207 WHERE `entry` =12977 AND `gold_max` =145;

-- Kolkar Ambusher
 UPDATE `creature_template` SET `gold_min` =37 WHERE `entry` =13019 AND `gold_min` =45;

-- Burning Blade Seer
 UPDATE `creature_template` SET `gold_max` =224 WHERE `entry` =13019 AND `gold_max` =63;

-- Burning Blade Seer
 UPDATE `creature_template` SET `gold_max` =2207 WHERE `entry` =13021 AND `gold_max` =1013;

-- Warpwood Crusher
 UPDATE `creature_template` SET `gold_min` =282 WHERE `entry` =13141 AND `gold_min` =300;

-- Deeprot Stomper
 UPDATE `creature_template` SET `gold_max` =880 WHERE `entry` =13141 AND `gold_max` =396;

-- Deeprot Stomper
 UPDATE `creature_template` SET `gold_max` =1354 WHERE `entry` =13142 AND `gold_max` =409;

-- Deeprot Tangler
 UPDATE `creature_template` SET `gold_max` =1230 WHERE `entry` =13153 AND `gold_max` =886;

-- Commander Mulfort
 UPDATE `creature_template` SET `gold_max` =3035 WHERE `entry` =13181 AND `gold_max` =761;

-- Wing Commander Mulverick
 UPDATE `creature_template` SET `gold_max` =557 WHERE `entry` =13276 AND `gold_max` =163;

-- Wildspawn Imp
 UPDATE `creature_template` SET `gold_max` =2742 WHERE `entry` =13296 AND `gold_max` =786;

-- Lieutenant Largent
 UPDATE `creature_template` SET `gold_max` =1175 WHERE `entry` =13297 AND `gold_max` =786;

-- Lieutenant Stouthandle
 UPDATE `creature_template` SET `gold_max` =648 WHERE `entry` =13358 AND `gold_max` =326;

-- Stormpike Bowman
 UPDATE `creature_template` SET `gold_max` =765 WHERE `entry` =13359 AND `gold_max` =326;

-- Frostwolf Bowman
 UPDATE `creature_template` SET `gold_min` =195 WHERE `entry` =13718 AND `gold_min` =232;

-- The Nameless Prophet
 UPDATE `creature_template` SET `gold_max` =994 WHERE `entry` =13718 AND `gold_max` =308;

-- The Nameless Prophet
 UPDATE `creature_template` SET `gold_max` =1122 WHERE `entry` =13738 AND `gold_max` =534;

-- Veng
 UPDATE `creature_template` SET `gold_max` =540 WHERE `entry` =13739 AND `gold_max` =514;

-- Maraudos
 UPDATE `creature_template` SET `gold_max` =622 WHERE `entry` =13740 AND `gold_max` =534;

-- Magra
 UPDATE `creature_template` SET `gold_max` =1060 WHERE `entry` =13741 AND `gold_max` =534;

-- Gelk
 UPDATE `creature_template` SET `gold_max` =1073 WHERE `entry` =13742 AND `gold_max` =534;

-- Kolk
 UPDATE `creature_template` SET `gold_max` =1387 WHERE `entry` =13797 AND `gold_max` =900;

-- Mountaineer Boombellow
 UPDATE `creature_template` SET `gold_max` =3066 WHERE `entry` =13959 AND `gold_max` =786;

-- Alterac Yeti
 UPDATE `creature_template` SET `gold_min` =1010 WHERE `entry` =13996 AND `gold_min` =1500;

-- Blackwing Technician
 UPDATE `creature_template` SET `gold_max` =99 WHERE `entry` =14221 AND `gold_max` =83;

-- Gravis Slipknot
 UPDATE `creature_template` SET `gold_max` =195 WHERE `entry` =14267 AND `gold_max` =172;

-- Emogg the Crusher
 UPDATE `creature_template` SET `gold_max` =50 WHERE `entry` =14270 AND `gold_max` =35;

-- Squiddic
 UPDATE `creature_template` SET `gold_max` =38 WHERE `entry` =14271 AND `gold_max` =31;

-- Ribchaser
 UPDATE `creature_template` SET `gold_max` =102 WHERE `entry` =14273 AND `gold_max` =48;

-- Boulderheart
 UPDATE `creature_template` SET `gold_max` =65 WHERE `entry` =14276 AND `gold_max` =62;

-- Scargil
 UPDATE `creature_template` SET `gold_max` =179 WHERE `entry` =14277 AND `gold_max` =64;

-- Lady Zephris
 UPDATE `creature_template` SET `gold_max` =100 WHERE `entry` =14278 AND `gold_max` =58;

-- Ro'Bark
 UPDATE `creature_template` SET `gold_min` =17 WHERE `entry` =14281 AND `gold_min` =30;

-- Jimmy the Bleeder
 UPDATE `creature_template` SET `gold_max` =2051 WHERE `entry` =14303 AND `gold_max` =544;

-- Petrified Guardian
 UPDATE `creature_template` SET `gold_max` =1201 WHERE `entry` =14327 AND `gold_max` =1127;

-- Lethtendris
 UPDATE `creature_template` SET `gold_max` =147 WHERE `entry` =14425 AND `gold_max` =47;

-- Gnawbone
 UPDATE `creature_template` SET `gold_max` =86 WHERE `entry` =14426 AND `gold_max` =57;

-- Harb Foulmountain
 UPDATE `creature_template` SET `gold_min` =1 WHERE `entry` =14428 AND `gold_min` =6;

-- Uruson
 UPDATE `creature_template` SET `gold_min` =3 WHERE `entry` =14431 AND `gold_min` =7;

-- Fury Shelda
 UPDATE `creature_template` SET `gold_max` =285 WHERE `entry` =14447 AND `gold_max` =107;

-- Gilmorian
 UPDATE `creature_template` SET `gold_max` =310 WHERE `entry` =14492 AND `gold_max` =103;

-- Verifonix
 UPDATE `creature_template` SET `gold_min` =1167 WHERE `entry` =14506 AND `gold_min` =2131;

-- Lord Hel'nurath
 UPDATE `creature_template` SET `gold_max` =401620 WHERE `entry` =14510 AND `gold_max` =400563;

-- High Priestess Mar'li
 UPDATE `creature_template` SET `gold_max` =369259 WHERE `entry` =14515 AND `gold_max` =36687;

-- High Priestess Arlokk
 UPDATE `creature_template` SET `gold_max` =411647 WHERE `entry` =14517 AND `gold_max` =407457;

-- High Priestess Jeklik
 UPDATE `creature_template` SET `gold_max` =1806 WHERE `entry` =14521 AND `gold_max` =838;

-- Aspect of Shadow
 UPDATE `creature_template` SET `gold_max` =467 WHERE `entry` =14523 AND `gold_max` =145;

-- Ulathek
 UPDATE `creature_template` SET `gold_max` =560 WHERE `entry` =14603 AND `gold_max` =205;

-- Zapped Shore Strider
 UPDATE `creature_template` SET `gold_max` =556 WHERE `entry` =14604 AND `gold_max` =222;

-- Zapped Land Walker
 UPDATE `creature_template` SET `gold_max` =540 WHERE `entry` =14638 AND `gold_max` =213;

-- Zapped Wave Strider
 UPDATE `creature_template` SET `gold_max` =603 WHERE `entry` =14639 AND `gold_max` =213;

-- Zapped Deep Strider
 UPDATE `creature_template` SET `gold_max` =603 WHERE `entry` =14640 AND `gold_max` =222;

-- Zapped Cliff Giant
 UPDATE `creature_template` SET `gold_min` =64 WHERE `entry` =14682 AND `gold_min` =0;

-- Sever (sample size 5)
 UPDATE `creature_template` SET `gold_max` =151 WHERE `entry` =14682 AND `gold_max` =0;

-- Sever (sample size 5)
 UPDATE `creature_template` SET `gold_min` =2275 WHERE `entry` =14684 AND `gold_min` =0;

-- Balzaphon (sample size 2)
 UPDATE `creature_template` SET `gold_max` =2984 WHERE `entry` =14684 AND `gold_max` =0;

-- Balzaphon (sample size 2)
 UPDATE `creature_template` SET `gold_min` =208 WHERE `entry` =14686 AND `gold_min` =0;

-- Lady Falther'ess (sample size 3)
 UPDATE `creature_template` SET `gold_max` =378 WHERE `entry` =14686 AND `gold_max` =0;

-- Lady Falther'ess (sample size 3)
 UPDATE `creature_template` SET `gold_min` =238 WHERE `entry` =14693 AND `gold_min` =0;

-- Scorn (sample size 4)
 UPDATE `creature_template` SET `gold_max` =657 WHERE `entry` =14693 AND `gold_max` =0;

-- Scorn (sample size 4)
 UPDATE `creature_template` SET `gold_min` =1041 WHERE `entry` =14695 AND `gold_min` =0;

-- Lord Blackwood (sample size 1)
 UPDATE `creature_template` SET `gold_max` =1871 WHERE `entry` =14695 AND `gold_max` =0;

-- Lord Blackwood (sample size 1)
 UPDATE `creature_template` SET `gold_min` =271 WHERE `entry` =14697 AND `gold_min` =0;

-- Lumbering Horror (sample size 3)
 UPDATE `creature_template` SET `gold_max` =503 WHERE `entry` =14697 AND `gold_max` =0;

-- Lumbering Horror (sample size 3)
 UPDATE `creature_template` SET `gold_min` =618 WHERE `entry` =14750 AND `gold_min` =1418;

-- Gurubashi Bat Rider
 UPDATE `creature_template` SET `gold_max` =3102 WHERE `entry` =14750 AND `gold_max` =1857;

-- Gurubashi Bat Rider
 UPDATE `creature_template` SET `gold_min` =794 WHERE `entry` =14825 AND `gold_min` =0;

-- Withered Mistress (sample size 13)
 UPDATE `creature_template` SET `gold_max` =3086 WHERE `entry` =14825 AND `gold_max` =0;

-- Withered Mistress (sample size 13)
 UPDATE `creature_template` SET `gold_min` =433457 WHERE `entry` =14834 AND `gold_min` =450277;

-- Hakkar
 UPDATE `creature_template` SET `gold_max` =590558 WHERE `entry` =14834 AND `gold_max` =550338;

-- Hakkar
 UPDATE `creature_template` SET `gold_min` =598 WHERE `entry` =14882 AND `gold_min` =779;

-- Atal'ai Mistress
 UPDATE `creature_template` SET `gold_max` =3056 WHERE `entry` =14882 AND `gold_max` =1022;

-- Atal'ai Mistress
 UPDATE `creature_template` SET `gold_min` =2174 WHERE `entry` =14883 AND `gold_min` =0;

-- Voodoo Slave (sample size 2)
 UPDATE `creature_template` SET `gold_max` =2476 WHERE `entry` =14883 AND `gold_max` =0;

-- Voodoo Slave (sample size 2)
 UPDATE `creature_template` SET `gold_max` =3270 WHERE `entry` =15111 AND `gold_max` =802;

-- Mad Servant
 UPDATE `creature_template` SET `gold_max` =1059 WHERE `entry` =15201 AND `gold_max` =181;

-- Twilight Flamereaver
 UPDATE `creature_template` SET `gold_max` =218688 WHERE `entry` =15204 AND `gold_max` =21638;

-- High Marshal Whirlaxis
 UPDATE `creature_template` SET `gold_max` =830 WHERE `entry` =15213 AND `gold_max` =165;

-- Twilight Overlord
 UPDATE `creature_template` SET `gold_max` =537463 WHERE `entry` =15543 AND `gold_max` =36968;

-- Princess Yauj
 UPDATE `creature_template` SET `gold_min` =490638 WHERE `entry` =15544 AND `gold_min` =0;

-- Vem (sample size 2)
 UPDATE `creature_template` SET `gold_max` =516334 WHERE `entry` =15544 AND `gold_max` =0;

-- Vem (sample size 2)
 UPDATE `creature_template` SET `gold_min` =1528 WHERE `entry` =15591 AND `gold_min` =0;

-- Minion of Weavil (sample size 1)
 UPDATE `creature_template` SET `gold_max` =1794 WHERE `entry` =15591 AND `gold_max` =0;

-- Minion of Weavil (sample size 1)
 UPDATE `creature_template` SET `gold_min` =258 WHERE `entry` =15685 AND `gold_min` =0;

-- Southsea Kidnapper (sample size 1)
 UPDATE `creature_template` SET `gold_max` =417 WHERE `entry` =15685 AND `gold_max` =0;

-- Southsea Kidnapper (sample size 1)
 UPDATE `creature_template` SET `gold_min` =92 WHERE `entry` =15692 AND `gold_min` =0;

-- Dark Iron Kidnapper (sample size 14)
 UPDATE `creature_template` SET `gold_max` =471 WHERE `entry` =15692 AND `gold_max` =0;

-- Dark Iron Kidnapper (sample size 14)
 UPDATE `creature_template` SET `gold_max` =1485537 WHERE `entry` =15727 AND `gold_max` =1258227;

-- C'Thun
 UPDATE `creature_template` SET `gold_min` =47736 WHERE `entry` =15743 AND `gold_min` =205024;

-- Colossal Anubisath Warbringer
 UPDATE `creature_template` SET `gold_max` =13960 WHERE `entry` =15748 AND `gold_max` =5524;

-- Lesser Anubisath Warbringer
 UPDATE `creature_template` SET `gold_min` =6082 WHERE `entry` =15806 AND `gold_min` =7502;

-- Qiraji Lieutenant
 UPDATE `creature_template` SET `gold_min` =6189 WHERE `entry` =15807 AND `gold_min` =2024;

-- Minor Anubisath Warbringer (sample size 36)
 UPDATE `creature_template` SET `gold_max` =8033 WHERE `entry` =15807 AND `gold_max` =2524;

-- Minor Anubisath Warbringer (sample size 36)
 UPDATE `creature_template` SET `gold_min` =5011 WHERE `entry` =15810 AND `gold_min` =1024;

-- Eroded Anubisath Warbringer (sample size 104)
 UPDATE `creature_template` SET `gold_max` =7488 WHERE `entry` =15810 AND `gold_max` =1524;

-- Eroded Anubisath Warbringer (sample size 104)
 UPDATE `creature_template` SET `gold_min` =5241 WHERE `entry` =15812 AND `gold_min` =5124;

-- Qiraji Officer (sample size 44)
 UPDATE `creature_template` SET `gold_max` =7158 WHERE `entry` =15812 AND `gold_max` =7552;

-- Qiraji Officer (sample size 44)
 UPDATE `creature_template` SET `gold_max` =10862 WHERE `entry` =15813 AND `gold_max` =9552;

-- Qiraji Officer Zod
 UPDATE `creature_template` SET `gold_min` =8494 WHERE `entry` =15814 AND `gold_min` =9502;

-- Qiraji Lieutenant Jo-rel
 UPDATE `creature_template` SET `gold_max` =9596 WHERE `entry` =15980 AND `gold_max` =1579;

-- Naxxramas Cultist
 UPDATE `creature_template` SET `gold_max` =9661 WHERE `entry` =15981 AND `gold_max` =1635;

-- Naxxramas Acolyte
 UPDATE `creature_template` SET `gold_max` =9694 WHERE `entry` =16020 AND `gold_max` =2911;

-- Mad Scientist
 UPDATE `creature_template` SET `gold_max` =9706 WHERE `entry` =16021 AND `gold_max` =2636;

-- Living Monstrosity
 UPDATE `creature_template` SET `gold_max` =9796 WHERE `entry` =16022 AND `gold_max` =2410;

-- Surgical Assistant
 UPDATE `creature_template` SET `gold_min` =6445 WHERE `entry` =16025 AND `gold_min` =6479;

-- Stitched Spewer
 UPDATE `creature_template` SET `gold_min` =144 WHERE `entry` =16141 AND `gold_min` =0;

-- Ghoul Berserker (sample size 360)
 UPDATE `creature_template` SET `gold_max` =659 WHERE `entry` =16141 AND `gold_max` =0;

-- Ghoul Berserker (sample size 360)
 UPDATE `creature_template` SET `gold_min` =186 WHERE `entry` =16143 AND `gold_min` =0;

-- Shadow of Doom (sample size 15)
 UPDATE `creature_template` SET `gold_max` =615 WHERE `entry` =16143 AND `gold_max` =0;

-- Shadow of Doom (sample size 15)
 UPDATE `creature_template` SET `gold_max` =9644 WHERE `entry` =16145 AND `gold_max` =5409;

-- Deathknight Captain
 UPDATE `creature_template` SET `gold_max` =8599 WHERE `entry` =16154 AND `gold_max` =1524;

-- Risen Deathknight
 UPDATE `creature_template` SET `gold_max` =8752 WHERE `entry` =16163 AND `gold_max` =6241;

-- Deathknight Cavalier
 UPDATE `creature_template` SET `gold_max` =5231 WHERE `entry` =16165 AND `gold_max` =2356;

-- Necro Knight
 UPDATE `creature_template` SET `gold_max` =7378 WHERE `entry` =16193 AND `gold_max` =2911;

-- Skeletal Smith
 UPDATE `creature_template` SET `gold_min` =6690 WHERE `entry` =16194 AND `gold_min` =0;

-- Unholy Axe (sample size 1)
 UPDATE `creature_template` SET `gold_max` =9414 WHERE `entry` =16194 AND `gold_max` =0;

-- Unholy Axe (sample size 1)
 UPDATE `creature_template` SET `gold_min` =4506 WHERE `entry` =16244 AND `gold_min` =0;

-- Infectious Ghoul (sample size 10)
 UPDATE `creature_template` SET `gold_max` =9113 WHERE `entry` =16244 AND `gold_max` =0;

-- Infectious Ghoul (sample size 10)
 UPDATE `creature_template` SET `gold_min` =148 WHERE `entry` =16298 AND `gold_min` =0;

-- Spectral Soldier (sample size 280)
 UPDATE `creature_template` SET `gold_max` =660 WHERE `entry` =16298 AND `gold_max` =0;

-- Spectral Soldier (sample size 280)
 UPDATE `creature_template` SET `gold_min` =144 WHERE `entry` =16299 AND `gold_min` =0;

-- Skeletal Shocktrooper (sample size 296)
 UPDATE `creature_template` SET `gold_max` =658 WHERE `entry` =16299 AND `gold_max` =0;

-- Skeletal Shocktrooper (sample size 296)
 UPDATE `creature_template` SET `gold_max` =9398 WHERE `entry` =16368 AND `gold_max` =3746;

-- Necropolis Acolyte
 UPDATE `creature_template` SET `gold_min` =482 WHERE `entry` =16380 AND `gold_min` =0;

-- Bone Witch (sample size 2)
 UPDATE `creature_template` SET `gold_max` =558 WHERE `entry` =16380 AND `gold_max` =0;

-- Bone Witch (sample size 2)
 UPDATE `creature_template` SET `gold_min` =202 WHERE `entry` =16383 AND `gold_min` =0;

-- Flameshocker (sample size 2)
 UPDATE `creature_template` SET `gold_max` =560 WHERE `entry` =16383 AND `gold_max` =0;

-- Flameshocker (sample size 2)
 UPDATE `creature_template` SET `gold_min` =3878 WHERE `entry` =16447 AND `gold_min` =4011;

-- Plagued Ghoul
 UPDATE `creature_template` SET `gold_max` =8925 WHERE `entry` =16447 AND `gold_max` =5242;

-- Plagued Ghoul
 UPDATE `creature_template` SET `gold_min` =4220 WHERE `entry` =16451 AND `gold_min` =5627;

-- Deathknight Vindicator
 -- These mobs should not drop any gold.
 UPDATE `creature_template` SET `gold_min` =0, `gold_max` =0 WHERE `entry` =8932;

-- Borer Beetle
 UPDATE `creature_template` SET `gold_min` =0, `gold_max` =0 WHERE `entry` =9683;

-- Lar'korwi Mate
 UPDATE `creature_template` SET `gold_min` =0, `gold_max` =0 WHERE `entry` =10177;

-- Spire Scarab
 UPDATE `creature_template` SET `gold_min` =0, `gold_max` =0 WHERE `entry` =10717;

-- Temporal Parasite
 UPDATE `creature_template` SET `gold_min` =0, `gold_max` =0 WHERE `entry` =15749;

-- Lesser Silithid Flayer
 UPDATE `creature_template` SET `gold_min` =0, `gold_max` =0 WHERE `entry` =15808;

-- Minor Silithid Flayer
 UPDATE `creature_template` SET `gold_min` =0, `gold_max` =0 WHERE `entry` =15811;

-- Faltering Silithid Flayer
 -- End of migration.
;
