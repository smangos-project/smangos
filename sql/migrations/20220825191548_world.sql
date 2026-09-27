INSERT INTO `migrations` VALUES ('20220825191548');

-- Add your query below.
 -- Damage Variance
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =430;

-- Redridge Mystic (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =475;

-- Kobold Tunneler (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =507;

-- Fenros (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.1 WHERE `entry` =706;

-- Frostmane Troll Whelp (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =707;

-- Rockjaw Trogg (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.07 WHERE `entry` =724;

-- Burly Rockjaw Trogg (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =762;

-- Lost One Riftseeker (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =910;

-- Defias Enchanter (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =1009;

-- Mosshide Mistweaver (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =1134;

-- Young Wendigo (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.1 WHERE `entry` =1718;

-- Rockjaw Raider (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =2006;

-- Gnarlpine Ursa (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.12 WHERE `entry` =2007;

-- Gnarlpine Gardener (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.12 WHERE `entry` =2114;

-- Faruza (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.12 WHERE `entry` =2310;

-- Jamie Nore (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.12 WHERE `entry` =2311;

-- Doreen Beltis (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.12 WHERE `entry` =2314;

-- Sahvan Bloodshadow (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =2337;

-- Dark Strand Voidcaller (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =2654;

-- Witherbark Caller (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =2740;

-- Shadowforge Darkweaver (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.13 WHERE `entry` =3191;

-- Cook Torka (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =4116;

-- Gravelsnout Surveyor (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =4467;

-- Vilebranch Soothsayer (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =5978;

-- Dreadmaul Warlock (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =6188;

-- Timbermaw Shaman (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.09 WHERE `entry` =6513;

-- Un'Goro Stomper (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =6868;

-- Jarkal Mossmeld (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =7026;

-- Blackrock Sorcerer (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =7028;

-- Blackrock Warlock (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =7372;

-- Deadwind Warlock (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =7379;

-- Deadwind Ogre Mage (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =8551;

-- Dark Summoner (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =8553;

-- Necromancer (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =9451;

-- Scarlet Archmage (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =9452;

-- Scarlet Enchanter (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.1 WHERE `entry` =9990;

-- Lanti'gah (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =10307;

-- Witch Doctor Mau'ari (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.09 WHERE `entry` =10880;

-- Warcaller Gorlach (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =11057;

-- Apothecary Dithers (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =11615;

-- Mickey Levine (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.17 WHERE `entry` =12736;

-- Je'neu Sancrea (was 0.14)
 UPDATE `creature_template` SET `damage_variance` =0.16 WHERE `entry` =14487;

-- Gluggle (was 0.14)
 -- Damage Multipliers
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =40 AND `patch` IN (0);

-- Kobold Miner (was 0.85)
 UPDATE `creature_template` SET `damage_multiplier` =0.85 WHERE `entry` =61 AND `patch` IN (0);

-- Thuros Lightfingers (was 1.4)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =79 AND `patch` IN (0);

-- Narg the Taskmaster (was 1.65)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =97 AND `patch` IN (0);

-- Riverpaw Runt (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =99 AND `patch` IN (0);

-- Morgaine the Sly (was 1.65)
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =100 AND `patch` IN (0);

-- Gruff Swiftbite (was 1)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =123 AND `patch` IN (0);

-- Riverpaw Mongrel (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1.35 WHERE `entry` =124 AND `patch` IN (0);

-- Riverpaw Brute (was 1.3)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =285 AND `patch` IN (0);

-- Murloc (was 0.85)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =422 AND `patch` IN (0);

-- Murloc Flesheater (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =423 AND `patch` IN (0);

-- Redridge Mongrel (was 0.9)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =424 AND `patch` IN (0);

-- Redridge Poacher (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1.6875 WHERE `entry` =426 AND `patch` IN (0);

-- Redridge Brute (was 1.65)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =430 AND `patch` IN (0);

-- Redridge Mystic (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =445 AND `patch` IN (0);

-- Redridge Alpha (was 0.9)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =446 AND `patch` IN (0);

-- Redridge Basher (was 0.9)
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =448 AND `patch` IN (0);

-- Hogger (was 2.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =449 AND `patch` IN (0);

-- Defias Knuckleduster (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1.11111 WHERE `entry` =472 AND `patch` IN (0);

-- Fedfennel (was 1.25)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =475 AND `patch` IN (0);

-- Kobold Tunneler (was 1.1)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =500 AND `patch` IN (0);

-- Riverpaw Scout (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =501 AND `patch` IN (0);

-- Riverpaw Herbalist (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =504 AND `patch` IN (0);

-- Defias Trapper (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =507 AND `patch` IN (0);

-- Fenros (was 5.55)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =518 AND `patch` IN (0);

-- Yowler (was 0.8)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =519 AND `patch` IN (0);

-- Slark (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1.4 WHERE `entry` =520 AND `patch` IN (0);

-- Brack (was 1.35)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =534 AND `patch` IN (0);

-- Nefaru (was 3.75)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =578 AND `patch` IN (0);

-- Murloc Scout (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =2.3324 WHERE `entry` =584 AND `patch` IN (0);

-- Kazon (was 3.2)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =706 AND `patch` IN (0);

-- Frostmane Troll Whelp (was 0.9)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =707 AND `patch` IN (0);

-- Rockjaw Trogg (was 0.65)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =712 AND `patch` IN (0);

-- Redridge Thrasher (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =724 AND `patch` IN (0);

-- Burly Rockjaw Trogg (was 0.9)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =735 AND `patch` IN (0);

-- Murloc Streamrunner (was 0.85)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =762 AND `patch` IN (0);

-- Lost One Riftseeker (was 1.6)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =763 AND `patch` IN (0);

-- Lost One Chieftain (was 3.6)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =910 AND `patch` IN (0);

-- Defias Enchanter (was 4.55)
 UPDATE `creature_template` SET `damage_multiplier` =0.975 WHERE `entry` =947 AND `patch` IN (0);

-- Rohh the Silent (was 3.3)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1008 AND `patch` IN (0);

-- Mosshide Mongrel (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1010 AND `patch` IN (0);

-- Mosshide Fenrunner (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1.6875 WHERE `entry` =1012 AND `patch` IN (0);

-- Mosshide Brute (was 1.65)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1025 AND `patch` IN (0);

-- Bluegill Puddlejumper (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1026 AND `patch` IN (0);

-- Bluegill Forager (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1068 AND `patch` IN (0);

-- Gorn (was 1.1)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1083 AND `patch` IN (0);

-- Murloc Shorestriker (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1106 AND `patch` IN (0);

-- Lost One Cook (was 3.8)
 UPDATE `creature_template` SET `damage_multiplier` =1.3 WHERE `entry` =1119 AND `patch` IN (0);

-- Hammerspine (was 1.25)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1120 AND `patch` IN (0);

-- Frostmane Troll (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1123 AND `patch` IN (0);

-- Frostmane Headhunter (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =0.7 WHERE `entry` =1134 AND `patch` IN (0);

-- Young Wendigo (was 0.65)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1135 AND `patch` IN (0);

-- Wendigo (was 0.85)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1137 AND `patch` IN (0);

-- Edan the Howler (was 1.2)
 UPDATE `creature_template` SET `damage_multiplier` =1.82 WHERE `entry` =1142 AND `patch` IN (0);

-- Mosh'Ogg Brute (was 1.4)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1163 AND `patch` IN (0);

-- Stonesplinter Skullthumper (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1175 AND `patch` IN (0);

-- Tunnel Rat Digger (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1177 AND `patch` IN (0);

-- Tunnel Rat Surveyor (was 0.7)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1206 AND `patch` IN (0);

-- Gnasher (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1207 AND `patch` IN (0);

-- Brawler (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1211 AND `patch` IN (0);

-- Leper Gnome (was 0.85)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1236 AND `patch` IN (0);

-- Kobold Digger (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =1253 AND `patch` IN (0);

-- Father Gavin (was 0.8)
 UPDATE `creature_template` SET `damage_multiplier` =2.125 WHERE `entry` =1259 AND `patch` IN (0);

-- Gobbler (was 2.1)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1380 AND `patch` IN (0);

-- Saean (was 2)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1399 AND `patch` IN (0);

-- Magosh (was 3.35)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1424 AND `patch` IN (0);

-- Master Digger (was 2.8)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1425 AND `patch` IN (0);

-- Grizlak (was 3)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1495 AND `patch` IN (0);

-- Deathguard Linnea (was 0.85)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1496 AND `patch` IN (0);

-- Deathguard Dillinger (was 0.85)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1497 AND `patch` IN (0);

-- Gunther Arcanus (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =0.5 WHERE `entry` =1499 AND `patch` IN (0);

-- Magistrate Sevren (was 0.75)
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =1500 AND `patch` IN (0);

-- Coleman Farthing (was 0.8)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1515 AND `patch` IN (0);

-- Executor Zygand (was 0.75)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1518 AND `patch` IN (0);

-- Apothecary Johaan (was 0.85)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1519 AND `patch` IN (0);

-- Deathguard Simmer (was 0.85)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1521 AND `patch` IN (0);

-- Gretchen Dedmar (was 0.75)
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =1560 AND `patch` IN (0);

-- Yvette Farthing (was 0.8)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1718 AND `patch` IN (0);

-- Rockjaw Raider (was 0.9)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1727 AND `patch` IN (0);

-- Defias Worker (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1735 AND `patch` IN (0);

-- Deathguard Abraham (was 0.9)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1736 AND `patch` IN (0);

-- Deathguard Randolph (was 5.45)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1737 AND `patch` IN (0);

-- Deathguard Oliver (was 7.1)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1739 AND `patch` IN (0);

-- Deathguard Phillip (was 7.1)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1740 AND `patch` IN (0);

-- Deathguard Saltain (was 18.4)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1741 AND `patch` IN (0);

-- Deathguard Bartrand (was 7.9)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1742 AND `patch` IN (0);

-- Deathguard Bartholomew (was 4.25)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1743 AND `patch` IN (0);

-- Deathguard Lawrence (was 0.85)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1745 AND `patch` IN (0);

-- Deathguard Morris (was 0.85)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1746 AND `patch` IN (0);

-- Deathguard Cyrus (was 0.8)
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1837 AND `patch` IN (0);

-- Scarlet Judge (was 5.6)
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1841 AND `patch` IN (0);

-- Scarlet Executioner (was 5.45)
 UPDATE `creature_template` SET `damage_multiplier` =1.2 WHERE `entry` =1844 AND `patch` IN (0);

-- Foreman Marcrid (was 3.65)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1848 AND `patch` IN (0);

-- Lord Maldazzar (was 3.65)
 UPDATE `creature_template` SET `damage_multiplier` =2 WHERE `entry` =1892 AND `patch` IN (0);

-- Moonrage Watcher (was 2.8)
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1893 AND `patch` IN (0);

-- Moonrage Sentry (was 2.75)
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =1896 AND `patch` IN (0);

-- Moonrage Elder (was 2.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1907 AND `patch` IN (0);

-- Naga Explorer (was 1.2)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1910 AND `patch` IN (0);

-- Muad (was 1.75)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1911 AND `patch` IN (0);

-- Deeb (was 3.15)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =1981 AND `patch` IN (0);

-- Dark Iron Ambusher (was 1.85)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2006 AND `patch` IN (0);

-- Gnarlpine Ursa (was 1.1)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2007 AND `patch` IN (0);

-- Gnarlpine Gardener (was 1.1)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2008 AND `patch` IN (0);

-- Gnarlpine Warrior (was 0.85)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2010 AND `patch` IN (0);

-- Gnarlpine Defender (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2012 AND `patch` IN (0);

-- Gnarlpine Pathfinder (was 0.9)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2014 AND `patch` IN (0);

-- Gnarlpine Totemic (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =0.6 WHERE `entry` =2017 AND `patch` IN (0);

-- Bloodfeather Rogue (was 0.55)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2058 AND `patch` IN (0);

-- Deathstalker Faerleia (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2090 AND `patch` IN (0);

-- Ma'ruk Wyrmscale (was 3.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2103 AND `patch` IN (0);

-- Dragonmaw Scout (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2114 AND `patch` IN (0);

-- Faruza (was 1.2)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2127 AND `patch` IN (0);

-- Rupert Boch (was 0.8)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2128 AND `patch` IN (0);

-- Cain Firesong (was 0.75)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2131 AND `patch` IN (0);

-- Austil de Mon (was 0.75)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2132 AND `patch` IN (0);

-- Carolai Anise (was 0.9)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2134 AND `patch` IN (0);

-- Mrs. Winters (was 0.75)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2135 AND `patch` IN (0);

-- Abe Winters (was 0.75)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2136 AND `patch` IN (0);

-- Oliver Dwor (was 0.75)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2137 AND `patch` IN (0);

-- Eliza Callen (was 0.7)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2149 AND `patch` IN (0);

-- Dark Iron Raider (was 1.85)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2168 AND `patch` IN (0);

-- Blackwood Warrior (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2169 AND `patch` IN (0);

-- Blackwood Totemic (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2181 AND `patch` IN (0);

-- Stormscale Myrmidon (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2186 AND `patch` IN (0);

-- Carnivous the Breaker (was 4.35)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2202 AND `patch` IN (0);

-- Greymist Coastrunner (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2206 AND `patch` IN (0);

-- Greymist Hunter (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2209 AND `patch` IN (0);

-- Deathguard Gavin (was 0.9)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2210 AND `patch` IN (0);

-- Deathguard Royann (was 0.9)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2310 AND `patch` IN (0);

-- Jamie Nore (was 1.2)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2311 AND `patch` IN (0);

-- Doreen Beltis (was 1.2)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2314 AND `patch` IN (0);

-- Sahvan Bloodshadow (was 1.2)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2324 AND `patch` IN (0);

-- Blackwood Windtalker (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2336 AND `patch` IN (0);

-- Dark Strand Fanatic (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2487 AND `patch` IN (0);

-- Fleet Master Seahorn (was 1.6)
 UPDATE `creature_template` SET `damage_multiplier` =1.2 WHERE `entry` =2541 AND `patch` IN (0);

-- Lord Sakrasis (was 3.35)
 UPDATE `creature_template` SET `damage_multiplier` =0.825 WHERE `entry` =2653 AND `patch` IN (0, 7);

-- Witherbark Sadist (was 0.8)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2654 AND `patch` IN (0);

-- Witherbark Caller (was 1.15)
 UPDATE `creature_template` SET `damage_multiplier` =1.625 WHERE `entry` =2715 AND `patch` IN (0);

-- Dustbelcher Brute (was 1.6)
 UPDATE `creature_template` SET `damage_multiplier` =0.85 WHERE `entry` =2717 AND `patch` IN (0);

-- Dustbelcher Mauler (was 0.8)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2817 AND `patch` IN (0);

-- Rigglefuzz (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2894 AND `patch` IN (0);

-- Stonevault Shaman (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2908 AND `patch` IN (0);

-- Grawl (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2928 AND `patch` IN (0);

-- Primitive Owlbeast (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =2929 AND `patch` IN (0);

-- Savage Owlbeast (was 1.1)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3191 AND `patch` IN (0);

-- Cook Torka (was 0.9)
 UPDATE `creature_template` SET `damage_multiplier` =1.05 WHERE `entry` =3266 AND `patch` IN (0, 7);

-- Razormane Defender (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =3270 AND `patch` IN (0);

-- Elder Mystic Razorsnout (was 3.1)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3272 AND `patch` IN (0);

-- Kolkar Wrangler (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3274 AND `patch` IN (0);

-- Kolkar Pack Runner (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3278 AND `patch` IN (0);

-- Witchwing Slayer (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3280 AND `patch` IN (0);

-- Witchwing Windcaller (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3381 AND `patch` IN (0);

-- Southsea Brigand (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3389 AND `patch` IN (0);

-- Regthar Deathgate (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3438 AND `patch` IN (0);

-- Kreenig Snarlsnout (was 0.8)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3452 AND `patch` IN (0);

-- Serena Bloodfeather (was 0.75)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3467 AND `patch` IN (0);

-- Baron Longshore (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3492 AND `patch` IN (0);

-- Vexspindle (was 0.9)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3493 AND `patch` IN (0);

-- Grazlix (was 0.9)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3496 AND `patch` IN (0);

-- Fuzruckle (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3501 AND `patch` IN (0);

-- Horde Guard (was 5.75)
 UPDATE `creature_template` SET `damage_multiplier` =2 WHERE `entry` =3502 AND `patch` IN (0, 2, 10);

-- Ratchet Bruiser (was 5.8)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3522 AND `patch` IN (0);

-- Constance Brisboise (was 0.5)
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =3529 AND `patch` IN (0);

-- Moonrage Armorer (was 2.9)
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =3531 AND `patch` IN (0);

-- Moonrage Tailor (was 3.35)
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =3533 AND `patch` IN (0);

-- Moonrage Leatherworker (was 2.75)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3547 AND `patch` IN (0);

-- Hamlin Atkins (was 0.75)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3548 AND `patch` IN (0);

-- Selina Weston (was 0.7)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3550 AND `patch` IN (0);

-- Martine Tramblay (was 0.8)
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =3672 AND `patch` IN (0);

-- Boahn (was 3.85)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3682 AND `patch` IN (0);

-- Vrang Wildgore (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3713 AND `patch` IN (0);

-- Wrathtail Wave Rider (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3737 AND `patch` IN (0);

-- Saltspittle Puddlejumper (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3739 AND `patch` IN (0);

-- Saltspittle Warrior (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3748 AND `patch` IN (0);

-- Foulweald Shaman (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =3881 AND `patch` IN (0);

-- Grimtak (was 0.75)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4116 AND `patch` IN (0);

-- Gravelsnout Surveyor (was 1.1)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4358 AND `patch` IN (0);

-- Mirefin Puddlejumper (was 1.75)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4359 AND `patch` IN (0);

-- Mirefin Murloc (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =4.1 WHERE `entry` =4364 AND `patch` IN (0);

-- Strashaz Warrior (was 5.45)
 UPDATE `creature_template` SET `damage_multiplier` =4.2 WHERE `entry` =4366 AND `patch` IN (0);

-- Strashaz Serpent Guard (was 5.35)
 UPDATE `creature_template` SET `damage_multiplier` =4.2 WHERE `entry` =4368 AND `patch` IN (0);

-- Strashaz Myrmidon (was 5.45)
 UPDATE `creature_template` SET `damage_multiplier` =4.2 WHERE `entry` =4370 AND `patch` IN (0);

-- Strashaz Sorceress (was 5.85)
 UPDATE `creature_template` SET `damage_multiplier` =4.1 WHERE `entry` =4371 AND `patch` IN (0);

-- Strashaz Siren (was 5.7)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4467 AND `patch` IN (0);

-- Vilebranch Soothsayer (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4501 AND `patch` IN (0);

-- Draz'Zilb (was 1.1)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4502 AND `patch` IN (0);

-- Tharg (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4791 AND `patch` IN (0);

-- Nazeer Bloodpike (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4875 AND `patch` IN (0);

-- Turhaw (was 1.1)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4876 AND `patch` IN (0);

-- Jawn Highmesa (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4877 AND `patch` IN (0);

-- Jandia (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4878 AND `patch` IN (0);

-- Montarr (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4879 AND `patch` IN (0);

-- Ogg'marr (was 1.1)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4926 AND `patch` IN (0);

-- Krog (was 1.1)
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =4961 AND `patch` IN (0);

-- Dashel Stonefist (was 0.9)
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =4962 AND `patch` IN (0);

-- Tapoke "Slim" Jahn (was 1)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4966 AND `patch` IN (0);

-- Private Hendel (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4967 AND `patch` IN (0);

-- Archmage Tervosh (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4969 AND `patch` IN (0);

-- Old Town Thug (was 1.65)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =4983 AND `patch` IN (0);

-- Ogron (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5085 AND `patch` IN (0);

-- Sentry Point Guard (was 1.3)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5086 AND `patch` IN (0);

-- Sentry Point Captain (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5184 AND `patch` IN (0);

-- Theramore Sentry (was 1.25)
 UPDATE `creature_template` SET `damage_multiplier` =2.8 WHERE `entry` =5399 AND `patch` IN (0);

-- Veyzhak the Cannibal (was 3.4)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5414 AND `patch` IN (0);

-- Apothecary Faustin (was 1.1)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5690 AND `patch` IN (0);

-- Clyde Kellen (was 0.75)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5695 AND `patch` IN (0);

-- Vance Undergloom (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5724 AND `patch` IN (0);

-- Ageron Kargal (was 0.8)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5725 AND `patch` IN (0);

-- Deathguard Lundmark (was 0.8)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5759 AND `patch` IN (0);

-- Nurse Neela (was 0.75)
 UPDATE `creature_template` SET `damage_multiplier` =2.6 WHERE `entry` =5824 AND `patch` IN (0, 7);

-- Captain Flat Tusk (was 4.55)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5826 AND `patch` IN (0);

-- Geolord Mottle (was 1.3)
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =5830 AND `patch` IN (0);

-- Sister Rathtalon (was 4.3)
 UPDATE `creature_template` SET `damage_multiplier` =1.36 WHERE `entry` =5835 AND `patch` IN (0);

-- Foreman Grills (was 3.8)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5836 AND `patch` IN (0);

-- Engineer Whirleygig (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =2.625 WHERE `entry` =5837 AND `patch` IN (0);

-- Stonearm (was 2.8)
 UPDATE `creature_template` SET `damage_multiplier` =1.6 WHERE `entry` =5838 AND `patch` IN (0);

-- Brokespear (was 3.8)
 UPDATE `creature_template` SET `damage_multiplier` =1.7 WHERE `entry` =5841 AND `patch` IN (0);

-- Rocklance (was 2.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5863 AND `patch` IN (0);

-- Geopriest Gukk'rok (was 4.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =5953 AND `patch` IN (0);

-- Razor Hill Grunt (was 3.8)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6124 AND `patch` IN (0);

-- Captain Beld (was 1.15)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6184 AND `patch` IN (0);

-- Timbermaw Pathfinder (was 1.1)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6186 AND `patch` IN (0);

-- Timbermaw Totemic (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6188 AND `patch` IN (0);

-- Timbermaw Shaman (was 0.8)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6221 AND `patch` IN (0);

-- Addled Leper (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6289 AND `patch` IN (0);

-- Rand Rhobart (was 0.75)
 UPDATE `creature_template` SET `damage_multiplier` =1.5 WHERE `entry` =6513 AND `patch` IN (0);

-- Un'Goro Stomper (was 1)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6649 AND `patch` IN (0);

-- Lady Sesspira (was 4.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6650 AND `patch` IN (0);

-- General Fangferror (was 3.3)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6651 AND `patch` IN (0);

-- Gatekeeper Rageroar (was 3.3)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6846 AND `patch` IN (0);

-- Defias Dockmaster (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6866 AND `patch` IN (0);

-- Defias Bodyguard (was 1.85)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =6868 AND `patch` IN (0);

-- Jarkal Mossmeld (was 1.45)
 UPDATE `creature_template` SET `damage_multiplier` =0.65 WHERE `entry` =6913 AND `patch` IN (0);

-- Lost One Rift Traveler (was 1.4)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7015 AND `patch` IN (0);

-- Flagglemurk the Cruel (was 3.7)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7016 AND `patch` IN (0);

-- Lady Vespira (was 3.2)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7017 AND `patch` IN (0);

-- Lord Sinslayer (was 3.7)
 UPDATE `creature_template` SET `damage_multiplier` =1.6875 WHERE `entry` =7027 AND `patch` IN (0);

-- Blackrock Slayer (was 1.65)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7028 AND `patch` IN (0);

-- Blackrock Warlock (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =0.84 WHERE `entry` =7036 AND `patch` IN (0);

-- Thaurissan Spy (was 1.15)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7037 AND `patch` IN (0);

-- Thaurissan Firewalker (was 1.15)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7038 AND `patch` IN (0);

-- Thaurissan Agent (was 1.15)
 UPDATE `creature_template` SET `damage_multiplier` =3 WHERE `entry` =7057 AND `patch` IN (0);

-- Digmaster Shovelphlange (was 3.75)
 UPDATE `creature_template` SET `damage_multiplier` =1.5 WHERE `entry` =7371 AND `patch` IN (0);

-- Deadwind Mauler (was 1.45)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7372 AND `patch` IN (0);

-- Deadwind Warlock (was 1.2)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =7379 AND `patch` IN (0);

-- Deadwind Ogre Mage (was 1.15)
 UPDATE `creature_template` SET `damage_multiplier` =1.68 WHERE `entry` =7438 AND `patch` IN (0);

-- Winterfall Ursa (was 1.65)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8119 AND `patch` IN (0);

-- Zikkel (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1.25 WHERE `entry` =8218 AND `patch` IN (0);

-- Witherheart the Stalker (was 3.15)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8297 AND `patch` IN (0);

-- Magronos the Unyielding (was 3.4)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8298 AND `patch` IN (0);

-- Akubar the Seer (was 4.1)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8304 AND `patch` IN (0);

-- Dreadscorn (was 3.6)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8305 AND `patch` IN (0);

-- Kixxle (was 0.9)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8337 AND `patch` IN (0);

-- Dark Iron Steelshifter (was 1.6)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8442 AND `patch` IN (0);

-- Shadowsilk Poacher (was 1.1)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8546 AND `patch` IN (0);

-- Dark Adept (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8547 AND `patch` IN (0);

-- Death Cultist (was 1.25)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8548 AND `patch` IN (0);

-- Vile Tutor (was 1.15)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8551 AND `patch` IN (0);

-- Dark Summoner (was 1.5)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8553 AND `patch` IN (0);

-- Necromancer (was 1.5)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =8978 AND `patch` IN (0);

-- Thauris Balgarr (was 3.4)
 UPDATE `creature_template` SET `damage_multiplier` =1.755 WHERE `entry` =8979 AND `patch` IN (0);

-- Gruklash (was 3.45)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =9084 AND `patch` IN (0);

-- Thunderheart (was 4.65)
 UPDATE `creature_template` SET `damage_multiplier` =3 WHERE `entry` =9176 AND `patch` IN (0);

-- Gor'tesh (was 2.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =9179 AND `patch` IN (0);

-- Jazzrik (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =9316 AND `patch` IN (0);

-- Wenikee Boltbucket (was 0.75)
 UPDATE `creature_template` SET `damage_multiplier` =3.4 WHERE `entry` =9447 AND `patch` IN (0);

-- Scarlet Warder (was 4)
 UPDATE `creature_template` SET `damage_multiplier` =3.7 WHERE `entry` =9448 AND `patch` IN (0);

-- Scarlet Praetorian (was 3.75)
 UPDATE `creature_template` SET `damage_multiplier` =3.4 WHERE `entry` =9449 AND `patch` IN (0);

-- Scarlet Cleric (was 3.9)
 UPDATE `creature_template` SET `damage_multiplier` =3.5 WHERE `entry` =9450 AND `patch` IN (0, 7);

-- Scarlet Curate (was 3.8)
 UPDATE `creature_template` SET `damage_multiplier` =3.7 WHERE `entry` =9451 AND `patch` IN (0);

-- Scarlet Archmage (was 4.8)
 UPDATE `creature_template` SET `damage_multiplier` =3.5 WHERE `entry` =9452 AND `patch` IN (0, 7);

-- Scarlet Enchanter (was 5.1)
 UPDATE `creature_template` SET `damage_multiplier` =6 WHERE `entry` =9520 AND `patch` IN (0);

-- Grark Lorkrub (was 3.4)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =9525 AND `patch` IN (0);

-- Freewind Brave (was 4.1)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =9551 AND `patch` IN (0);

-- Starn (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =0.5 WHERE `entry` =9566 AND `patch` IN (0);

-- Zapetta (was 4.9)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =9602 AND `patch` IN (0);

-- Hahk'Zor (was 3.55)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =9990 AND `patch` IN (0);

-- Lanti'gah (was 0.9)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =10158 AND `patch` IN (0);

-- Moonkin (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =10197 AND `patch` IN (0);

-- Mezzir the Howler (was 3.45)
 UPDATE `creature_template` SET `damage_multiplier` =1.68 WHERE `entry` =10199 AND `patch` IN (0);

-- Grizzle Snowpaw (was 3.7)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =10303 AND `patch` IN (0);

-- Storm Shadowhoof (was 1.8)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =10307 AND `patch` IN (0, 8);

-- Witch Doctor Mau'ari (was 1.4)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =10377 AND `patch` IN (0);

-- Elu (was 7.75)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =10537 AND `patch` IN (0);

-- Cliffwatcher Longhorn (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =10559 AND `patch` IN (0);

-- Lady Vespia (was 3.55)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =10612 AND `patch` IN (0);

-- Guard Wachabe (was 1.3)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =10617 AND `patch` IN (0);

-- Galak Messenger (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =0.75 WHERE `entry` =10639 AND `patch` IN (0);

-- Rorgish Jowl (was 3.1)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =10640 AND `patch` IN (0);

-- Oakpaw (was 3.4)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =10643 AND `patch` IN (0);

-- Mugglefin (was 3.2)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =10665 AND `patch` IN (0);

-- Junior Apothecary Holland (was 0.85)
 UPDATE `creature_template` SET `damage_multiplier` =1.25 WHERE `entry` =10822 AND `patch` IN (0);

-- Warlord Thresh'jin (was 3.45)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =10837 AND `patch` IN (0);

-- High Executor Derrington (was 1.6)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =10856 AND `patch` IN (0);

-- Argent Quartermaster Hasana (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =10857 AND `patch` IN (0);

-- Argent Quartermaster Lightspark (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =10880 AND `patch` IN (0);

-- Warcaller Gorlach (was 1.25)
 UPDATE `creature_template` SET `damage_multiplier` =1.68 WHERE `entry` =10916 AND `patch` IN (0);

-- Winterfall Runner (was 1.65)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11035 AND `patch` IN (0);

-- Betina Bigglezink (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11057 AND `patch` IN (0);

-- Apothecary Dithers (was 1.45)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11063 AND `patch` IN (0);

-- Carlin Redpath (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11066 AND `patch` IN (0);

-- Jhag (was 0.9)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11119 AND `patch` IN (0);

-- Azzleby (was 2.6)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11184 AND `patch` IN (0);

-- Wixxrak (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11536 AND `patch` IN (0);

-- Quartermaster Miranda Breechlock (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11608 AND `patch` IN (0);

-- Bardu Sharpeye (was 1.1)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11613 AND `patch` IN (0);

-- Huntsman Radley (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11615 AND `patch` IN (0);

-- Mickey Levine (was 1.4)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11680 AND `patch` IN (0);

-- Horde Scout (was 1.15)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11720 AND `patch` IN (0);

-- Loruk Foreststrider (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11820 AND `patch` IN (0);

-- Locke Okarr (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =11829 AND `patch` IN (0);

-- Fahrak (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =5 WHERE `entry` =11898 AND `patch` IN (0);

-- Crusader Lord Valdelmar (was 5.4)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =12138 AND `patch` IN (0);

-- Lunaclaw (was 1.45)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =12428 AND `patch` IN (0);

-- Deathguard Kel (was 0.85)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =12717 AND `patch` IN (0);

-- Muglash (was 0.9)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =12719 AND `patch` IN (0);

-- Marukai (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =12736 AND `patch` IN (0);

-- Je'neu Sancrea (was 1.2)
 UPDATE `creature_template` SET `damage_multiplier` =1.5 WHERE `entry` =12903 AND `patch` IN (0);

-- Splintertree Guard (was 1.7)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =12941 AND `patch` IN (0);

-- Jase Farlane (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =12943 AND `patch` IN (0);

-- Werg Thickblade (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14230 AND `patch` IN (0);

-- Burgle Eye (was 3.85)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14270 AND `patch` IN (0);

-- Squiddic (was 3.8)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14271 AND `patch` IN (0);

-- Ribchaser (was 3.8)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14304 AND `patch` IN (1);

-- Kor'kron Elite (was 5)
 UPDATE `creature_template` SET `damage_multiplier` =12 WHERE `entry` =14392 AND `patch` IN (1);

-- Overlord Runthak (was 5)
 UPDATE `creature_template` SET `damage_multiplier` =1.1 WHERE `entry` =14425 AND `patch` IN (0);

-- Gnawbone (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14428 AND `patch` IN (0);

-- Uruson (was 1.1)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14429 AND `patch` IN (0);

-- Grimmaw (was 1.4)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14487 AND `patch` IN (0);

-- Gluggle (was 4.85)
 UPDATE `creature_template` SET `damage_multiplier` =1.82 WHERE `entry` =14488 AND `patch` IN (0);

-- Roloch (was 3.6)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14731 AND `patch` IN (3);

-- Lard (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14736 AND `patch` IN (3);

-- Primal Torntusk (was 1.6)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14737 AND `patch` IN (3);

-- Smith Slagtree (was 0.9)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14739 AND `patch` IN (3);

-- Mystic Yayo'jin (was 1.35)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14740 AND `patch` IN (3);

-- Katoom the Angler (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14742 AND `patch` IN (3);

-- Zap Farflinger (was 1.05)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =14909 AND `patch` IN (4);

-- Pooka (was 0.8)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =15131 AND `patch` IN (5);

-- Qeeju (was 0.95)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =15761 AND `patch` IN (7);

-- Officer Vu'Shalay (was 3.7)
 UPDATE `creature_template` SET `damage_multiplier` =2 WHERE `entry` =16227 AND `patch` IN (9);

-- Bragok (was 3.45)
 UPDATE `creature_template` SET `damage_multiplier` =3 WHERE `entry` =16229 AND `patch` IN (9);

-- Injured Argent Dawn Infantry (was 3.65)
 UPDATE `creature_template` SET `damage_multiplier` =1 WHERE `entry` =17598 AND `patch` IN (10);

-- Renn'az (was 3.75)
 UPDATE `creature_template` SET `damage_multiplier` =2.8 WHERE `entry` =17635 AND `patch` IN (10);

-- Lordaeron Commander (was 3.9)
 UPDATE `creature_template` SET `damage_multiplier` =2.5 WHERE `entry` =17647 AND `patch` IN (10);

-- Lordaeron Soldier (was 3.9)
 UPDATE `creature_template` SET `damage_multiplier` =2.8 WHERE `entry` =17995 AND `patch` IN (10);

-- Lordaeron Veteran (was 3.9)
 UPDATE `creature_template` SET `damage_multiplier` =2.5 WHERE `entry` =17996 AND `patch` IN (10);

-- Lordaeron Fighter (was 3.9)
 -- Armor Multipliers
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =40 AND `patch` IN (0);

-- Kobold Miner (was 0.8)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =46 AND `patch` IN (0);

-- Murloc Forager (was 0.9)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =97 AND `patch` IN (0);

-- Riverpaw Runt (was 0.9)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =285 AND `patch` IN (0);

-- Murloc (was 0.85)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =445 AND `patch` IN (0);

-- Redridge Alpha (was 0.9)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =446 AND `patch` IN (0);

-- Redridge Basher (was 0.95)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =475 AND `patch` IN (0);

-- Kobold Tunneler (was 0.8)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =478 AND `patch` IN (0);

-- Riverpaw Outrunner (was 0.9)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =500 AND `patch` IN (0);

-- Riverpaw Scout (was 0.95)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =507 AND `patch` IN (0);

-- Fenros (was 1.6)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =518 AND `patch` IN (0);

-- Yowler (was 0.85)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =706 AND `patch` IN (0);

-- Frostmane Troll Whelp (was 0.65)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =732 AND `patch` IN (0);

-- Murloc Lurker (was 0.9)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =735 AND `patch` IN (0);

-- Murloc Streamrunner (was 0.85)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =910 AND `patch` IN (0);

-- Defias Enchanter (was 3.15)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1009 AND `patch` IN (0);

-- Mosshide Mistweaver (was 1.55)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1065 AND `patch` IN (0);

-- Riverpaw Shaman (was 1.55)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1116 AND `patch` IN (0);

-- Rockjaw Ambusher (was 0.9)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1117 AND `patch` IN (0);

-- Rockjaw Bonesnapper (was 0.9)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1120 AND `patch` IN (0);

-- Frostmane Troll (was 0.9)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1122 AND `patch` IN (0);

-- Frostmane Hideskinner (was 0.9)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1123 AND `patch` IN (0);

-- Frostmane Headhunter (was 0.9)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1134 AND `patch` IN (0);

-- Young Wendigo (was 0.8)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1135 AND `patch` IN (0);

-- Wendigo (was 0.8)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1177 AND `patch` IN (0);

-- Tunnel Rat Surveyor (was 0.8)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1397 AND `patch` IN (0);

-- Frostmane Seer (was 0.9)
 UPDATE `creature_template` SET `armor_multiplier` =1.2 WHERE `entry` =1495 AND `patch` IN (0);

-- Deathguard Linnea (was 1)
 UPDATE `creature_template` SET `armor_multiplier` =1.2 WHERE `entry` =1496 AND `patch` IN (0);

-- Deathguard Dillinger (was 1)
 UPDATE `creature_template` SET `armor_multiplier` =1.2 WHERE `entry` =1519 AND `patch` IN (0);

-- Deathguard Simmer (was 1)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1718 AND `patch` IN (0);

-- Rockjaw Raider (was 0.6)
 UPDATE `creature_template` SET `armor_multiplier` =1.2 WHERE `entry` =1735 AND `patch` IN (0);

-- Deathguard Abraham (was 1)
 UPDATE `creature_template` SET `armor_multiplier` =1.2 WHERE `entry` =1736 AND `patch` IN (0);

-- Deathguard Randolph (was 1.4)
 UPDATE `creature_template` SET `armor_multiplier` =1.2 WHERE `entry` =1737 AND `patch` IN (0);

-- Deathguard Oliver (was 1.4)
 UPDATE `creature_template` SET `armor_multiplier` =1.2 WHERE `entry` =1739 AND `patch` IN (0);

-- Deathguard Phillip (was 1.4)
 UPDATE `creature_template` SET `armor_multiplier` =1.2 WHERE `entry` =1740 AND `patch` IN (0);

-- Deathguard Saltain (was 1)
 UPDATE `creature_template` SET `armor_multiplier` =1.2 WHERE `entry` =1741 AND `patch` IN (0);

-- Deathguard Bartrand (was 1.4)
 UPDATE `creature_template` SET `armor_multiplier` =1.2 WHERE `entry` =1742 AND `patch` IN (0);

-- Deathguard Bartholomew (was 0.95)
 UPDATE `creature_template` SET `armor_multiplier` =1.2 WHERE `entry` =1743 AND `patch` IN (0);

-- Deathguard Lawrence (was 1)
 UPDATE `creature_template` SET `armor_multiplier` =1.2 WHERE `entry` =1745 AND `patch` IN (0);

-- Deathguard Morris (was 1)
 UPDATE `creature_template` SET `armor_multiplier` =1.2 WHERE `entry` =1746 AND `patch` IN (0);

-- Deathguard Cyrus (was 1)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1837 AND `patch` IN (0);

-- Scarlet Judge (was 1.1)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1841 AND `patch` IN (0);

-- Scarlet Executioner (was 1.1)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =1907 AND `patch` IN (0);

-- Naga Explorer (was 0.9)
 UPDATE `creature_template` SET `armor_multiplier` =2 WHERE `entry` =2090 AND `patch` IN (0);

-- Ma'ruk Wyrmscale (was 1)
 UPDATE `creature_template` SET `armor_multiplier` =1.2 WHERE `entry` =2209 AND `patch` IN (0);

-- Deathguard Gavin (was 1)
 UPDATE `creature_template` SET `armor_multiplier` =1.2 WHERE `entry` =2210 AND `patch` IN (0);

-- Deathguard Royann (was 1)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2487 AND `patch` IN (0);

-- Fleet Master Seahorn (was 2.4)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2817 AND `patch` IN (0);

-- Rigglefuzz (was 0.85)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =2929 AND `patch` IN (0);

-- Savage Owlbeast (was 0.8)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3493 AND `patch` IN (0);

-- Grazlix (was 0.95)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3501 AND `patch` IN (0);

-- Horde Guard (was 0.6)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3502 AND `patch` IN (0, 2, 10);

-- Ratchet Bruiser (was 0.35)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3672 AND `patch` IN (0);

-- Boahn (was 0.9)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =3682 AND `patch` IN (0);

-- Vrang Wildgore (was 0.25)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4358 AND `patch` IN (0);

-- Mirefin Puddlejumper (was 0.95)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4359 AND `patch` IN (0);

-- Mirefin Murloc (was 1.05)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4364 AND `patch` IN (0);

-- Strashaz Warrior (was 1.05)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4366 AND `patch` IN (0);

-- Strashaz Serpent Guard (was 1.05)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4368 AND `patch` IN (0);

-- Strashaz Myrmidon (was 1.1)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4370 AND `patch` IN (0);

-- Strashaz Sorceress (was 1.1)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4371 AND `patch` IN (0);

-- Strashaz Siren (was 1.05)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4457 AND `patch` IN (0);

-- Murkgill Forager (was 0.95)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4791 AND `patch` IN (0);

-- Nazeer Bloodpike (was 0.25)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4877 AND `patch` IN (0);

-- Jandia (was 0.85)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4883 AND `patch` IN (0);

-- Krak (was 0.85)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4961 AND `patch` IN (0);

-- Dashel Stonefist (was 1.5)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =4962 AND `patch` IN (0);

-- Tapoke "Slim" Jahn (was 0.75)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5085 AND `patch` IN (0);

-- Sentry Point Guard (was 1.35)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5086 AND `patch` IN (0);

-- Sentry Point Captain (was 0.9)
 UPDATE `creature_template` SET `armor_multiplier` =1.2 WHERE `entry` =5725 AND `patch` IN (0);

-- Deathguard Lundmark (was 1)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =5953 AND `patch` IN (0);

-- Razor Hill Grunt (was 0.95)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =6868 AND `patch` IN (0);

-- Jarkal Mossmeld (was 1.6)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =6913 AND `patch` IN (0);

-- Lost One Rift Traveler (was 1.4)
 UPDATE `creature_template` SET `armor_multiplier` =0.8 WHERE `entry` =7036 AND `patch` IN (0);

-- Thaurissan Spy (was 0.75)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =7037 AND `patch` IN (0);

-- Thaurissan Firewalker (was 1.05)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =7038 AND `patch` IN (0);

-- Thaurissan Agent (was 0.95)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =7379 AND `patch` IN (0);

-- Deadwind Ogre Mage (was 3.05)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =7450 AND `patch` IN (0);

-- Ragged Owlbeast (was 0.95)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =8337 AND `patch` IN (0);

-- Dark Iron Steelshifter (was 0.95)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =8546 AND `patch` IN (0);

-- Dark Adept (was 1.05)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =8548 AND `patch` IN (0);

-- Vile Tutor (was 0.95)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =8551 AND `patch` IN (0);

-- Dark Summoner (was 1.35)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =8553 AND `patch` IN (0);

-- Necromancer (was 0.7)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =9525 AND `patch` IN (0);

-- Freewind Brave (was 0.95)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =9551 AND `patch` IN (0);

-- Starn (was 0.85)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =10303 AND `patch` IN (0);

-- Storm Shadowhoof (was 2.4)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =10537 AND `patch` IN (0);

-- Cliffwatcher Longhorn (was 1.4)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =10837 AND `patch` IN (0);

-- High Executor Derrington (was 1.05)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =10840 AND `patch` IN (0);

-- Argent Officer Pureheart (was 1.1)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11038 AND `patch` IN (0);

-- Caretaker Alen (was 0.85)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11039 AND `patch` IN (0);

-- Duke Nicholas Zverenhoff (was 1.1)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11055 AND `patch` IN (0);

-- Shadow Priestess Vandis (was 1.1)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11066 AND `patch` IN (0);

-- Jhag (was 1.5)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11119 AND `patch` IN (0);

-- Azzleby (was 2.9)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11184 AND `patch` IN (0);

-- Wixxrak (was 0.95)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11613 AND `patch` IN (0);

-- Huntsman Radley (was 1.2)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =11898 AND `patch` IN (0);

-- Crusader Lord Valdelmar (was 1.1)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =12736 AND `patch` IN (0);

-- Je'neu Sancrea (was 0)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =12863 AND `patch` IN (0);

-- Warsong Runner (was 0.9)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =12943 AND `patch` IN (0);

-- Werg Thickblade (was 1.4)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =14230 AND `patch` IN (0);

-- Burgle Eye (was 0.95)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =14304 AND `patch` IN (1);

-- Kor'kron Elite (was 1.2)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =14392 AND `patch` IN (1);

-- Overlord Runthak (was 1.1)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =14731 AND `patch` IN (3);

-- Lard (was 0.25)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =14736 AND `patch` IN (3);

-- Primal Torntusk (was 1.25)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =14737 AND `patch` IN (3);

-- Smith Slagtree (was 1.3)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =14739 AND `patch` IN (3);

-- Mystic Yayo'jin (was 1.2)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =14740 AND `patch` IN (3);

-- Katoom the Angler (was 1.5)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =15761 AND `patch` IN (7);

-- Officer Vu'Shalay (was 1.4)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =16072 AND `patch` IN (8);

-- Tidelord Rrurgaz (was 1.2)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =16227 AND `patch` IN (9);

-- Bragok (was 1.4)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =16229 AND `patch` IN (9);

-- Injured Argent Dawn Infantry (was 2.4)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =17635 AND `patch` IN (10);

-- Lordaeron Commander (was 1.1)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =17647 AND `patch` IN (10);

-- Lordaeron Soldier (was 1.1)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =17995 AND `patch` IN (10);

-- Lordaeron Veteran (was 1.1)
 UPDATE `creature_template` SET `armor_multiplier` =1 WHERE `entry` =17996 AND `patch` IN (10);

-- Lordaeron Fighter (was 1.1)
 -- fire resistance
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =40 AND `fire_res` =21;

-- Kobold Miner
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =475 AND `fire_res` =21;

-- Kobold Tunneler
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =476 AND `fire_res` =24;

-- Kobold Geomancer
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =1236 AND `fire_res` =39;

-- Kobold Digger
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =1253 AND `fire_res` =30;

-- Father Gavin
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =1514 AND `fire_res` =30;

-- Mokk the Savage
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =1841 AND `fire_res` =5;

-- Scarlet Executioner
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =3272 AND `fire_res` =70;

-- Kolkar Wrangler
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =3630 AND `fire_res` =2;

-- Deviate Coiler
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =3631 AND `fire_res` =2;

-- Deviate Stinglash
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =3653 AND `fire_res` =60;

-- Kresh
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =4116 AND `fire_res` =90;

-- Gravelsnout Surveyor
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =4366 AND `fire_res` =15;

-- Strashaz Serpent Guard
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =4368 AND `fire_res` =15;

-- Strashaz Myrmidon
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =4371 AND `fire_res` =15;

-- Strashaz Siren
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =5056 AND `fire_res` =2;

-- Deviate Dreadfang
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =5349 AND `fire_res` =85;

-- Arash-ethis
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =5694 AND `fire_res` =10;

-- High Sorcerer Andromath
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =5756 AND `fire_res` =2;

-- Deviate Venomwing
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =5769 AND `fire_res` =10;

-- Arch Druid Hamuul Runetotem
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =5834 AND `fire_res` =55;

-- Azzere the Skyblade
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =6352 AND `fire_res` =108;

-- Coralshell Lurker
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =8211 AND `fire_res` =90;

-- Old Cliff Jumper
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =8213 AND `fire_res` =153;

-- Ironback
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =8303 AND `fire_res` =40;

-- Grunter
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =10077 AND `fire_res` =100;

-- Deathmaw
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =10837 AND `fire_res` =5;

-- High Executor Derrington
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =10882 AND `fire_res` =2;

-- Arikara
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =11035 AND `fire_res` =30;

-- Betina Bigglezink
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =11039 AND `fire_res` =30;

-- Duke Nicholas Zverenhoff
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =11063 AND `fire_res` =30;

-- Carlin Redpath
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =11871 AND `fire_res` =2;

-- Grinning Dog
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =11896 AND `fire_res` =5;

-- Borelgore
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =11898 AND `fire_res` =5;

-- Crusader Lord Valdelmar
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =12431 AND `fire_res` =70;

-- Gorefang
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =14343 AND `fire_res` =20;

-- Olm the Wise
 UPDATE `creature_template` SET `fire_res` =0 WHERE `entry` =16072 AND `fire_res` =10;

-- Tidelord Rrurgaz
 -- nature resistance
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =474 AND `nature_res` =10;

-- Defias Rogue Wizard
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =1514 AND `nature_res` =10;

-- Mokk the Savage
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =1841 AND `nature_res` =5;

-- Scarlet Executioner
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =3630 AND `nature_res` =2;

-- Deviate Coiler
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =3631 AND `nature_res` =2;

-- Deviate Stinglash
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =4366 AND `nature_res` =15;

-- Strashaz Serpent Guard
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =4368 AND `nature_res` =15;

-- Strashaz Myrmidon
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =4371 AND `nature_res` =15;

-- Strashaz Siren
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =5056 AND `nature_res` =2;

-- Deviate Dreadfang
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =5288 AND `nature_res` =1;

-- Rabid Longtooth
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =5349 AND `nature_res` =85;

-- Arash-ethis
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =5694 AND `nature_res` =10;

-- High Sorcerer Andromath
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =5756 AND `nature_res` =2;

-- Deviate Venomwing
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =5769 AND `nature_res` =10;

-- Arch Druid Hamuul Runetotem
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =5834 AND `nature_res` =55;

-- Azzere the Skyblade
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =7432 AND `nature_res` =50;

-- Frostsaber Stalker
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =7433 AND `nature_res` =50;

-- Frostsaber Huntress
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =7434 AND `nature_res` =50;

-- Frostsaber Pride Watcher
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =7455 AND `nature_res` =55;

-- Winterspring Owl
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =7456 AND `nature_res` =55;

-- Winterspring Screecher
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =8211 AND `nature_res` =90;

-- Old Cliff Jumper
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =8213 AND `nature_res` =51;

-- Ironback
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =8303 AND `nature_res` =10;

-- Grunter
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =10077 AND `nature_res` =100;

-- Deathmaw
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =10837 AND `nature_res` =5;

-- High Executor Derrington
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =10882 AND `nature_res` =2;

-- Arikara
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =11871 AND `nature_res` =2;

-- Grinning Dog
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =11896 AND `nature_res` =5;

-- Borelgore
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =11898 AND `nature_res` =5;

-- Crusader Lord Valdelmar
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =12431 AND `nature_res` =70;

-- Gorefang
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =14343 AND `nature_res` =20;

-- Olm the Wise
 UPDATE `creature_template` SET `nature_res` =0 WHERE `entry` =16072 AND `nature_res` =10;

-- Tidelord Rrurgaz
 -- frost resistance
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =1514 AND `frost_res` =10;

-- Mokk the Savage
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =1841 AND `frost_res` =5;

-- Scarlet Executioner
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =3630 AND `frost_res` =2;

-- Deviate Coiler
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =3631 AND `frost_res` =2;

-- Deviate Stinglash
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =4366 AND `frost_res` =15;

-- Strashaz Serpent Guard
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =4368 AND `frost_res` =15;

-- Strashaz Myrmidon
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =4371 AND `frost_res` =15;

-- Strashaz Siren
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =5056 AND `frost_res` =2;

-- Deviate Dreadfang
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =5349 AND `frost_res` =85;

-- Arash-ethis
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =5694 AND `frost_res` =10;

-- High Sorcerer Andromath
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =5756 AND `frost_res` =2;

-- Deviate Venomwing
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =5769 AND `frost_res` =10;

-- Arch Druid Hamuul Runetotem
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =5834 AND `frost_res` =55;

-- Azzere the Skyblade
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =8211 AND `frost_res` =90;

-- Old Cliff Jumper
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =8213 AND `frost_res` =51;

-- Ironback
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =10077 AND `frost_res` =100;

-- Deathmaw
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =10837 AND `frost_res` =5;

-- High Executor Derrington
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =10882 AND `frost_res` =2;

-- Arikara
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =11871 AND `frost_res` =2;

-- Grinning Dog
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =11896 AND `frost_res` =5;

-- Borelgore
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =11898 AND `frost_res` =5;

-- Crusader Lord Valdelmar
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =12431 AND `frost_res` =70;

-- Gorefang
 UPDATE `creature_template` SET `frost_res` =0 WHERE `entry` =16072 AND `frost_res` =10;

-- Tidelord Rrurgaz
 -- shadow resistance
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =423 AND `shadow_res` =50;

-- Redridge Mongrel
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =1514 AND `shadow_res` =160;

-- Mokk the Savage
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =1841 AND `shadow_res` =5;

-- Scarlet Executioner
 UPDATE `creature_template` SET `shadow_res` =5 WHERE `entry` =1892 AND `shadow_res` =0;

-- Moonrage Watcher
 UPDATE `creature_template` SET `shadow_res` =5 WHERE `entry` =1893 AND `shadow_res` =0;

-- Moonrage Sentry
 UPDATE `creature_template` SET `shadow_res` =5 WHERE `entry` =1896 AND `shadow_res` =0;

-- Moonrage Elder
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =3272 AND `shadow_res` =30;

-- Kolkar Wrangler
 UPDATE `creature_template` SET `shadow_res` =5 WHERE `entry` =3529 AND `shadow_res` =0;

-- Moonrage Armorer
 UPDATE `creature_template` SET `shadow_res` =5 WHERE `entry` =3531 AND `shadow_res` =0;

-- Moonrage Tailor
 UPDATE `creature_template` SET `shadow_res` =5 WHERE `entry` =3533 AND `shadow_res` =0;

-- Moonrage Leatherworker
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =3630 AND `shadow_res` =2;

-- Deviate Coiler
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =3631 AND `shadow_res` =2;

-- Deviate Stinglash
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =4366 AND `shadow_res` =15;

-- Strashaz Serpent Guard
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =4368 AND `shadow_res` =15;

-- Strashaz Myrmidon
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =4371 AND `shadow_res` =15;

-- Strashaz Siren
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =5056 AND `shadow_res` =2;

-- Deviate Dreadfang
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =5349 AND `shadow_res` =85;

-- Arash-ethis
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =5694 AND `shadow_res` =10;

-- High Sorcerer Andromath
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =5756 AND `shadow_res` =2;

-- Deviate Venomwing
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =5769 AND `shadow_res` =10;

-- Arch Druid Hamuul Runetotem
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =5834 AND `shadow_res` =55;

-- Azzere the Skyblade
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =7099 AND `shadow_res` =20;

-- Ironbeak Hunter
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =7455 AND `shadow_res` =25;

-- Winterspring Owl
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =7456 AND `shadow_res` =25;

-- Winterspring Screecher
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =8211 AND `shadow_res` =90;

-- Old Cliff Jumper
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =8213 AND `shadow_res` =51;

-- Ironback
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =8303 AND `shadow_res` =230;

-- Grunter
 UPDATE `creature_template` SET `shadow_res` =88 WHERE `entry` =8546 AND `shadow_res` =0;

-- Dark Adept
 UPDATE `creature_template` SET `shadow_res` =81 WHERE `entry` =8547 AND `shadow_res` =0;

-- Death Cultist
 UPDATE `creature_template` SET `shadow_res` =85 WHERE `entry` =8548 AND `shadow_res` =0;

-- Vile Tutor
 UPDATE `creature_template` SET `shadow_res` =84 WHERE `entry` =8551 AND `shadow_res` =0;

-- Dark Summoner
 UPDATE `creature_template` SET `shadow_res` =91 WHERE `entry` =8553 AND `shadow_res` =0;

-- Necromancer
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =10077 AND `shadow_res` =100;

-- Deathmaw
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =10837 AND `shadow_res` =5;

-- High Executor Derrington
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =10882 AND `shadow_res` =2;

-- Arikara
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =11871 AND `shadow_res` =2;

-- Grinning Dog
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =11896 AND `shadow_res` =5;

-- Borelgore
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =11898 AND `shadow_res` =5;

-- Crusader Lord Valdelmar
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =12431 AND `shadow_res` =70;

-- Gorefang
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =14343 AND `shadow_res` =87;

-- Olm the Wise
 UPDATE `creature_template` SET `shadow_res` =0 WHERE `entry` =16072 AND `shadow_res` =10;

-- Tidelord Rrurgaz
 -- arcane resistance
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1253 AND `arcane_res` =70;

-- Father Gavin
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1495 AND `arcane_res` =10;

-- Deathguard Linnea
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1496 AND `arcane_res` =10;

-- Deathguard Dillinger
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1514 AND `arcane_res` =20;

-- Mokk the Savage
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1515 AND `arcane_res` =10;

-- Executor Zygand
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1519 AND `arcane_res` =10;

-- Deathguard Simmer
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1735 AND `arcane_res` =10;

-- Deathguard Abraham
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1736 AND `arcane_res` =10;

-- Deathguard Randolph
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1737 AND `arcane_res` =10;

-- Deathguard Oliver
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1739 AND `arcane_res` =10;

-- Deathguard Phillip
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1741 AND `arcane_res` =10;

-- Deathguard Bartrand
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1742 AND `arcane_res` =10;

-- Deathguard Bartholomew
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1743 AND `arcane_res` =10;

-- Deathguard Lawrence
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1745 AND `arcane_res` =10;

-- Deathguard Morris
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1746 AND `arcane_res` =10;

-- Deathguard Cyrus
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1841 AND `arcane_res` =5;

-- Scarlet Executioner
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =1981 AND `arcane_res` =100;

-- Dark Iron Ambusher
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =2149 AND `arcane_res` =100;

-- Dark Iron Raider
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =2175 AND `arcane_res` =10;

-- Shadowclaw
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =2209 AND `arcane_res` =10;

-- Deathguard Gavin
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =2210 AND `arcane_res` =10;

-- Deathguard Royann
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =3630 AND `arcane_res` =2;

-- Deviate Coiler
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =3631 AND `arcane_res` =2;

-- Deviate Stinglash
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =4366 AND `arcane_res` =15;

-- Strashaz Serpent Guard
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =4368 AND `arcane_res` =15;

-- Strashaz Myrmidon
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =4371 AND `arcane_res` =15;

-- Strashaz Siren
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =5056 AND `arcane_res` =2;

-- Deviate Dreadfang
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =5349 AND `arcane_res` =85;

-- Arash-ethis
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =5694 AND `arcane_res` =10;

-- High Sorcerer Andromath
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =5725 AND `arcane_res` =10;

-- Deathguard Lundmark
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =5756 AND `arcane_res` =2;

-- Deviate Venomwing
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =5769 AND `arcane_res` =10;

-- Arch Druid Hamuul Runetotem
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =5834 AND `arcane_res` =55;

-- Azzere the Skyblade
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =6124 AND `arcane_res` =100;

-- Captain Beld
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =8211 AND `arcane_res` =90;

-- Old Cliff Jumper
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =8213 AND `arcane_res` =51;

-- Ironback
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =8303 AND `arcane_res` =60;

-- Grunter
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =8337 AND `arcane_res` =100;

-- Dark Iron Steelshifter
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =10077 AND `arcane_res` =100;

-- Deathmaw
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =10665 AND `arcane_res` =10;

-- Junior Apothecary Holland
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =10837 AND `arcane_res` =5;

-- High Executor Derrington
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =10882 AND `arcane_res` =2;

-- Arikara
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =11035 AND `arcane_res` =70;

-- Betina Bigglezink
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =11039 AND `arcane_res` =70;

-- Duke Nicholas Zverenhoff
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =11063 AND `arcane_res` =70;

-- Carlin Redpath
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =11871 AND `arcane_res` =2;

-- Grinning Dog
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =11896 AND `arcane_res` =5;

-- Borelgore
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =11898 AND `arcane_res` =5;

-- Crusader Lord Valdelmar
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =12428 AND `arcane_res` =10;

-- Deathguard Kel
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =12431 AND `arcane_res` =70;

-- Gorefang
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =14343 AND `arcane_res` =30;

-- Olm the Wise
 UPDATE `creature_template` SET `arcane_res` =0 WHERE `entry` =16072 AND `arcane_res` =10;

-- Tidelord Rrurgaz
 -- End of migration.
;
