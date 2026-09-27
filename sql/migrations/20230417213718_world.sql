INSERT INTO `migrations` VALUES ('20230417213718');

-- Add your query below.
 -- El Pollo Grande
 -- Old: Blackfathom
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =659 AND `faction` =128;

-- Talin Keeneye
 -- Old: Ironforge
 -- New: Ironforge
 UPDATE `creature_template` SET `faction` =55, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =714 AND `faction` =57;

-- Grelin Whitebeard
 -- Old: Ironforge
 -- New: Ironforge
 UPDATE `creature_template` SET `faction` =55, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =786 AND `faction` =57;

-- Durnan Furcutter
 -- Old: Ironforge
 -- New: Ironforge
 UPDATE `creature_template` SET `faction` =55, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =836 AND `faction` =57;

-- Branstock Khalder
 -- Old: Ironforge
 -- New: Ironforge
 UPDATE `creature_template` SET `faction` =55, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =837 AND `faction` =57;

-- Merrin Rockweaver
 -- Old: Ironforge
 -- New: Ironforge
 UPDATE `creature_template` SET `faction` =55 WHERE `entry` =1076 AND `faction` =57;

-- Prospector Whelgar
 -- Old: Ironforge
 -- New: Ironforge
 UPDATE `creature_template` SET `faction` =55 WHERE `entry` =1077 AND `faction` =57;

-- Ormer Ironbraid
 -- Old: Ironforge
 -- New: Ironforge
 UPDATE `creature_template` SET `faction` =55 WHERE `entry` =1078 AND `faction` =57;

-- Vyrin Swiftwind
 -- Old: Alliance Generic
 -- New: Silvermoon Remnant
 UPDATE `creature_template` SET `faction` =371, `unit_flags` = (`unit_flags` | 512) WHERE `entry` =1156 AND `faction` =84;

-- Chok'sul
 -- Old: Monster
 -- New: Ogre
 UPDATE `creature_template` SET `faction` =45 WHERE `entry` =1210 AND `faction` =14;

-- Apprentice Soren
 -- Old: Ironforge
 -- New: Ironforge
 UPDATE `creature_template` SET `faction` =55, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =1354 AND `faction` =57;

-- Paxton Ganter
 -- Old: Ironforge
 -- New: Ironforge
 UPDATE `creature_template` SET `faction` =55, `unit_flags` = (`unit_flags` | 512) WHERE `entry` =1700 AND `faction` =57;

-- Scarlet Interrogator
 -- Old: Beast - Wolf
 -- New: Scarlet Crusade
 UPDATE `creature_template` SET `faction` =67 WHERE `entry` =1838 AND `faction` =32;

-- Putridius
 -- Old: Monster
 -- New: Undead, Scourge
 UPDATE `creature_template` SET `faction` =21 WHERE `entry` =1850 AND `faction` =14;

-- The Husk
 -- Old: Elemental
 -- New: Monster
 UPDATE `creature_template` SET `faction` =14 WHERE `entry` =1851 AND `faction` =91;

-- Scarlet Smith
 -- Old: Monster
 -- New: Scarlet Crusade
 UPDATE `creature_template` SET `faction` =67 WHERE `entry` =1885 AND `faction` =14;

-- Pilot Hammerfoot
 -- Old: Ironforge
 -- New: Ironforge
 UPDATE `creature_template` SET `faction` =55, `unit_flags` = (`unit_flags` | 512) WHERE `entry` =1960 AND `faction` =57;

-- Haggard Refugee
 -- Old: Stormwind
 -- New: Stormwind
 UPDATE `creature_template` SET `faction` =123 WHERE `entry` =2053 AND `faction` =11;

-- Sickly Refugee
 -- Old: Stormwind
 -- New: Stormwind
 UPDATE `creature_template` SET `faction` =123 WHERE `entry` =2054 AND `faction` =11;

-- Denalan
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =2080 AND `faction` =79;

-- Gilshalan Windwalker
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =2082 AND `faction` =79;

-- Gaerolas Talvethren
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =2107 AND `faction` =79;

-- Gravelflint Bonesnapper
 -- Old: Beast - Spider
 -- New: Trogg
 UPDATE `creature_template` SET `faction` =59 WHERE `entry` =2159 AND `faction` =22;

-- Gravelflint Geomancer
 -- Old: Beast - Spider
 -- New: Trogg
 UPDATE `creature_template` SET `faction` =59 WHERE `entry` =2160 AND `faction` =22;

-- Enraged Stanley
 -- Old: Beast - Wolf
 -- New: Monster
 UPDATE `creature_template` SET `faction` =14 WHERE `entry` =2275 AND `faction` =32;

-- Valdred Moray
 -- Old: Stormwind
 -- New: Stormwind
 UPDATE `creature_template` SET `faction` =123 WHERE `entry` =2332 AND `faction` =11;

-- Jailor Borhuin
 -- Old: Syndicate
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =2431 AND `faction` =87;

-- Stromgarde Defender
 -- Old: Stormwind
 -- New: Stormwind
 UPDATE `creature_template` SET `faction` =123 WHERE `entry` =2584 AND `faction` =11;

-- Prince Galen Trollbane
 -- Old: Stormwind
 -- New: Stormwind
 UPDATE `creature_template` SET `faction` =123 WHERE `entry` =2607 AND `faction` =11;

-- Syndicate Spectre
 -- Old: Gnoll - Rothide
 -- New: Kurzen's Mercenaries
 UPDATE `creature_template` SET `faction` =46 WHERE `entry` =2638 AND `faction` =70;

-- George Candarte
 -- Old: Undercity
 -- New: Undercity
 UPDATE `creature_template` SET `faction` =68 WHERE `entry` =2698 AND `faction` =71;

-- Shadra
 -- Old: Beast - Wolf
 -- New: Beast - Spider
 UPDATE `creature_template` SET `faction` =312 WHERE `entry` =2707 AND `faction` =32;

-- Shadowforge Commander
 -- Old: Monster
 -- New: Dark Iron Dwarves
 UPDATE `creature_template` SET `faction` =54 WHERE `entry` =2744 AND `faction` =14;

-- War Golem
 -- Old: Elemental
 -- New: Dark Iron Dwarves
 UPDATE `creature_template` SET `faction` =54 WHERE `entry` =2751 AND `faction` =91;

-- Thenan
 -- Old: Monster
 -- New: Giant
 UPDATE `creature_template` SET `faction` =778 WHERE `entry` =2763 AND `faction` =14;

-- Vengeful Surge
 -- Old: Elemental
 -- New: Villian
 UPDATE `creature_template` SET `faction` =41 WHERE `entry` =2776 AND `faction` =91;

-- Caretaker Nevlin
 -- Old: Stormwind
 -- New: Stormwind
 UPDATE `creature_template` SET `faction` =123 WHERE `entry` =2780 AND `faction` =11;

-- Caretaker Weston
 -- Old: Stormwind
 -- New: Stormwind
 UPDATE `creature_template` SET `faction` =123 WHERE `entry` =2781 AND `faction` =11;

-- Caretaker Alaric
 -- Old: Stormwind
 -- New: Stormwind
 UPDATE `creature_template` SET `faction` =123 WHERE `entry` =2782 AND `faction` =11;

-- Marez Cowl
 -- Old: Syndicate
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =2783 AND `faction` =87;

-- Deneb Walker
 -- Old: Stormwind
 -- New: Stormwind
 UPDATE `creature_template` SET `faction` =12 WHERE `entry` =2805 AND `faction` =11;

-- Broken Tooth
 -- Old: Leopard
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =2850 AND `faction` =66;

-- Theldren
 -- Old: Beast - Raptor
 -- New: Monster
 UPDATE `creature_template` SET `faction` =14 WHERE `entry` =16059 AND `faction` =48;

-- Hammertoe's Spirit
 -- Old: Friendly
 -- New: Ironforge
 UPDATE `creature_template` SET `faction` =122, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =2915 AND `faction` =35;

-- Morgan Ladimore
 -- Old: Friendly
 -- New: Stormwind
 UPDATE `creature_template` SET `faction` =123, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =3301 AND `faction` =35;

-- Tenaron Stormgrip
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =3514 AND `faction` =79;

-- Sentinel Arynia Cloudsbreak
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =3519 AND `faction` =79;

-- Bogling
 -- Old: Elemental
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =3569 AND `faction` =91;

-- Lyrai
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =3587 AND `faction` =79;

-- Khardan Proudblade
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =3588 AND `faction` =79;

-- Keina
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =3589 AND `faction` =79;

-- Janna Brightmoon
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =3590 AND `faction` =79;

-- Freja Nightwing
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =3591 AND `faction` =79;

-- Andiss
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =3592 AND `faction` =79;

-- Alyissia
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =3593 AND `faction` =79;

-- Frahun Shadewhisper
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =3594 AND `faction` =79;

-- Shanda
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =3595 AND `faction` =79;

-- Ayanna Everstride
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =3596 AND `faction` =79;

-- Mardant Strongoak
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =3597 AND `faction` =79;

-- Alanna Raveneye
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =3606 AND `faction` =79;

-- Sentinel Tysha Moonblade
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =3639 AND `faction` =79;

-- Asterion
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =3650 AND `faction` =79;

-- Balthule Shadowstrike
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =3661 AND `faction` =79;

-- Sentinel Selarin
 -- Old: Alliance Generic
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80, `unit_flags` = (`unit_flags` | 512) WHERE `entry` =3694 AND `faction` =84;

-- Bolyun
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =3698 AND `faction` =79;

-- Forsaken Thug
 -- Old: Undercity
 -- New: Undercity
 UPDATE `creature_template` SET `faction` =118 WHERE `entry` =3734 AND `faction` =71;

-- Apothecary Falthis
 -- Old: Undercity
 -- New: Undercity
 UPDATE `creature_template` SET `faction` =118 WHERE `entry` =3735 AND `faction` =71;

-- Darkslayer Mordenthal
 -- Old: Undercity
 -- New: Undercity
 UPDATE `creature_template` SET `faction` =118 WHERE `entry` =3736 AND `faction` =71;

-- Terrowulf Packlord
 -- Old: Monster
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =3792 AND `faction` =14;

-- Mountaineer Pebblebitty
 -- Old: Ironforge
 -- New: Ironforge
 UPDATE `creature_template` SET `faction` =55, `unit_flags` = (`unit_flags` | 512) WHERE `entry` =3836 AND `faction` =57;

-- Anaya
 -- Old: Alliance Generic
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =3843 AND `faction` =84;

-- Dark Strand Assassin
 -- Old: Monster
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =3879 AND `faction` =14;

-- Forsaken Scout
 -- Old: Undercity
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =3893 AND `faction` =71;

-- Aligar the Tormentor
 -- Old: Beast - Spider
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =3898 AND `faction` =22;

-- Balizar the Umbrage
 -- Old: Beast - Spider
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =3899 AND `faction` =22;

-- Caedakar the Vicious
 -- Old: Beast - Spider
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =3900 AND `faction` =22;

-- Shael'dryn
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =3916 AND `faction` =79;

-- Minor Water Guardian
 -- Old: Elemental
 -- New: Undercity
 UPDATE `creature_template` SET `faction` =118 WHERE `entry` =3950 AND `faction` =91;

-- Illusionary Nightmare
 -- Old: Beast - Wolf
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =4785 AND `faction` =32;

-- Deep Pool Threshfin
 -- Old: Blackfathom
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =4827 AND `faction` =128;

-- Old Serra'kis
 -- Old: Blackfathom
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =4830 AND `faction` =128;

-- Haunting Spirit
 -- Old: Spirit
 -- New: Worgen
 UPDATE `creature_template` SET `faction` =24 WHERE `entry` =4958 AND `faction` =92;

-- Bloodroar the Stalker
 -- Old: Beast - Wolf
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =5346 AND `faction` =32;

-- Antilus the Soarer
 -- Old: Beast - Wolf
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =5347 AND `faction` =32;

-- Gnarl Leafbrother
 -- Old: Elemental
 -- New: Creature
 UPDATE `creature_template` SET `faction` =7 WHERE `entry` =5354 AND `faction` =91;

-- Harvester Swarm
 -- Old: Monster
 -- New: Silithid
 UPDATE `creature_template` SET `faction` =310 WHERE `entry` =5409 AND `faction` =14;

-- Dalin Forgewright
 -- Old: Stormwind
 -- New: Stormwind
 UPDATE `creature_template` SET `faction` =123 WHERE `entry` =5682 AND `faction` =11;

-- Dalin Forgewright Projection
 -- Old: Beast - Gorilla
 -- New: Friendly
 UPDATE `creature_template` SET `faction` =35, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =5691 AND `faction` =72;

-- Comar Villard Projection
 -- Old: Beast - Gorilla
 -- New: Friendly
 UPDATE `creature_template` SET `faction` =35, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =5692 AND `faction` =72;

-- Caged Squirrel
 -- Old: Undercity
 -- New: Undercity
 UPDATE `creature_template` SET `faction` =68 WHERE `entry` =5739 AND `faction` =71;

-- Caged Rabbit
 -- Old: Undercity
 -- New: Undercity
 UPDATE `creature_template` SET `faction` =68, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =5741 AND `faction` =71;

-- Caged Toad
 -- Old: Undercity
 -- New: Undercity
 UPDATE `creature_template` SET `faction` =68, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =5742 AND `faction` =71;

-- Caged Sheep
 -- Old: Undercity
 -- New: Undercity
 UPDATE `creature_template` SET `faction` =68, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =5743 AND `faction` =71;

-- Lilly
 -- Old: Undercity
 -- New: Undercity
 UPDATE `creature_template` SET `faction` =68 WHERE `entry` =5757 AND `faction` =71;

-- Leo Sarn
 -- Old: Undercity
 -- New: Undercity
 UPDATE `creature_template` SET `faction` =68 WHERE `entry` =5758 AND `faction` =71;

-- Mesa Earth Spirit
 -- Old: Spirit
 -- New: Creature
 UPDATE `creature_template` SET `faction` =7, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =5889 AND `faction` =92;

-- Air Spirit
 -- Old: Spirit
 -- New: Creature
 UPDATE `creature_template` SET `faction` =7, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =5898 AND `faction` =92;

-- Minor Manifestation of Air
 -- Old: Elemental
 -- New: Friendly
 UPDATE `creature_template` SET `faction` =35, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =5902 AND `faction` =91;

-- Deviate Faerie Dragon
 -- Old: Beast - Wolf
 -- New: Wailing Caverns
 UPDATE `creature_template` SET `faction` =270 WHERE `entry` =5912 AND `faction` =32;

-- Deviate Nightmare
 -- Old: Monster
 -- New: Friendly
 UPDATE `creature_template` SET `faction` =35 WHERE `entry` =5914 AND `faction` =14;

-- Brother Ravenoak
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =124 WHERE `entry` =5915 AND `faction` =79;

-- Dellylah
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =6091 AND `faction` =79;

-- Chomper
 -- Old: Monster
 -- New: Trogg
 UPDATE `creature_template` SET `faction` =59 WHERE `entry` =6215 AND `faction` =14;

-- Anathera
 -- Old: Darnassus
 -- New: Friendly
 UPDATE `creature_template` SET `faction` =35, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =6245 AND `faction` =79;

-- Latherion
 -- Old: Darnassus
 -- New: Friendly
 UPDATE `creature_template` SET `faction` =35, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =6246 AND `faction` =79;

-- Demon of the Orb
 -- Old: Demon
 -- New: Monster
 UPDATE `creature_template` SET `faction` =14 WHERE `entry` =6549 AND `faction` =90;

-- Monnos the Elder
 -- Old: Monster
 -- New: Giant
 UPDATE `creature_template` SET `faction` =778 WHERE `entry` =6646 AND `faction` =14;

-- Magister Hawkhelm
 -- Old: Monster
 -- New: Naga
 UPDATE `creature_template` SET `faction` =74 WHERE `entry` =6647 AND `faction` =14;

-- Antilos
 -- Old: Beast - Wolf
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =6648 AND `faction` =32;

-- Gelkak Gyromast
 -- Old: Ironforge
 -- New: Ironforge
 UPDATE `creature_template` SET `faction` =55 WHERE `entry` =6667 AND `faction` =57;

-- Morridune
 -- Old: Friendly
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =6729 AND `faction` =35;

-- Harlown Darkweave
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =6731 AND `faction` =79;

-- Water Spirit
 -- Old: Elemental
 -- New: Creature
 UPDATE `creature_template` SET `faction` =7, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =6748 AND `faction` =91;

-- Onin MacHammar
 -- Old: Ironforge
 -- New: Ironforge
 UPDATE `creature_template` SET `faction` =55, `unit_flags` = (`unit_flags` | 512) WHERE `entry` =6886 AND `faction` =57;

-- Swamp Spirit
 -- Old: Spirit
 -- New: Murloc
 UPDATE `creature_template` SET `faction` =18 WHERE `entry` =6932 AND `faction` =92;

-- Agent Kearnen
 -- Old: Stormwind
 -- New: Stormwind
 UPDATE `creature_template` SET `faction` =12 WHERE `entry` =7024 AND `faction` =11;

-- Earthen Guardian
 -- Old: Friendly
 -- New: Titan
 UPDATE `creature_template` SET `faction` =470, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =7076 AND `faction` =35;

-- Earthen Hallshaper
 -- Old: Friendly
 -- New: Titan
 UPDATE `creature_template` SET `faction` =470, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =7077 AND `faction` =35;

-- Ember
 -- Old: Stormwind
 -- New: Gnome - Leper
 UPDATE `creature_template` SET `faction` =63 WHERE `entry` =7266 AND `faction` =11;

-- Earthen Custodian
 -- Old: Friendly
 -- New: Titan
 UPDATE `creature_template` SET `faction` =470, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =7309 AND `faction` =35;

-- Lady Sathrah
 -- Old: Beast - Spider
 -- New: Beast - Spider
 UPDATE `creature_template` SET `faction` =312 WHERE `entry` =7319 AND `faction` =22;

-- Ragglesnout
 -- Old: Monster
 -- New: Quilboar, Razorfen
 UPDATE `creature_template` SET `faction` =152 WHERE `entry` =7354 AND `faction` =14;

-- Plaguemaw the Rotting
 -- Old: Monster
 -- New: Quilboar, Razorfen
 UPDATE `creature_template` SET `faction` =152 WHERE `entry` =7356 AND `faction` =14;

-- Grubbis
 -- Old: Gnome - Leper
 -- New: Trogg
 UPDATE `creature_template` SET `faction` =59 WHERE `entry` =7361 AND `faction` =63;

-- Razelikh the Defiler
 -- Old: Beast - Spider
 -- New: Demon
 UPDATE `creature_template` SET `faction` =90 WHERE `entry` =7664 AND `faction` =22;

-- Ilifar
 -- Old: Beast - Spider
 -- New: Demon
 UPDATE `creature_template` SET `faction` =90 WHERE `entry` =7734 AND `faction` =22;

-- Felcular
 -- Old: Monster
 -- New: Demon
 UPDATE `creature_template` SET `faction` =90 WHERE `entry` =7735 AND `faction` =14;

-- Burning Servant
 -- Old: Monster
 -- New: Gnome - Leper
 UPDATE `creature_template` SET `faction` =63 WHERE `entry` =7738 AND `faction` =14;

-- Corporal Thund Splithoof
 -- Old: Friendly
 -- New: Friendly
 UPDATE `creature_template` SET `faction` =35, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =7750 AND `faction` =1080;

-- Rockbiter
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =124 WHERE `entry` =7765 AND `faction` =79;

-- Ambassador Bloodrage
 -- Old: Beast - Wolf
 -- New: Quilboar, Razorfen
 UPDATE `creature_template` SET `faction` =152 WHERE `entry` =7895 AND `faction` =32;

-- Treasure Hunting Pirate
 -- Old: Monster
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =7899 AND `faction` =14;

-- Treasure Hunting Swashbuckler
 -- Old: Monster
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =7901 AND `faction` =14;

-- Treasure Hunting Buccaneer
 -- Old: Monster
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =7902 AND `faction` =14;

-- Omgorn the Lost
 -- Old: Beast - Spider
 -- New: Ogre
 UPDATE `creature_template` SET `faction` =45 WHERE `entry` =8201 AND `faction` =22;

-- Cyclok the Mad
 -- Old: Beast - Wolf
 -- New: Ogre
 UPDATE `creature_template` SET `faction` =45 WHERE `entry` =8202 AND `faction` =32;

-- Kregg Keelhaul
 -- Old: Monster
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =8203 AND `faction` =14;

-- Haarka the Ravenous
 -- Old: Beast - Spider
 -- New: Silithid
 UPDATE `creature_template` SET `faction` =310 WHERE `entry` =8205 AND `faction` =22;

-- Murderous Blisterpaw
 -- Old: Beast - Wolf
 -- New: Beast - Wolf
 UPDATE `creature_template` SET `faction` =38 WHERE `entry` =8208 AND `faction` =32;

-- Ironback
 -- Old: Beast - Wolf
 -- New: Monster
 UPDATE `creature_template` SET `faction` =14 WHERE `entry` =8213 AND `faction` =32;

-- Mith'rethis the Enchanter
 -- Old: Monster
 -- New: Troll, Vilebranch
 UPDATE `creature_template` SET `faction` =795 WHERE `entry` =8217 AND `faction` =14;

-- Witherheart the Stalker
 -- Old: Beast - Wolf
 -- New: Troll, Vilebranch
 UPDATE `creature_template` SET `faction` =795 WHERE `entry` =8218 AND `faction` =32;

-- Shleipnarr
 -- Old: Demon
 -- New: Monster
 UPDATE `creature_template` SET `faction` =14 WHERE `entry` =8280 AND `faction` =90;

-- Alenndaar Lapidaar
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =1076 WHERE `entry` =8308 AND `faction` =79;

-- Atal'ai Skeleton
 -- Old: Monster
 -- New: Troll, Frostmane
 UPDATE `creature_template` SET `faction` =37 WHERE `entry` =8324 AND `faction` =14;

-- Dorius
 -- Old: Beast - Gorilla
 -- New: Booty Bay
 UPDATE `creature_template` SET `faction` =120 WHERE `entry` =8421 AND `faction` =72;

-- Dirania Silvershine
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =8583 AND `faction` =79;

-- Iverron
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =8584 AND `faction` =79;

-- Sandfury Acolyte
 -- Old: Monster
 -- New: Troll, Frostmane
 UPDATE `creature_template` SET `faction` =37 WHERE `entry` =8876 AND `faction` =14;

-- Panzor the Invincible
 -- Old: Monster
 -- New: Dark Iron Dwarves
 UPDATE `creature_template` SET `faction` =54 WHERE `entry` =8923 AND `faction` =14;

-- The Behemoth
 -- Old: Monster
 -- New: Dark Iron Dwarves
 UPDATE `creature_template` SET `faction` =54 WHERE `entry` =8924 AND `faction` =14;

-- Dark Screecher
 -- Old: Monster
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =8927 AND `faction` =14;

-- Malfunctioning Reaver
 -- Old: Elemental
 -- New: Dark Iron Dwarves
 UPDATE `creature_template` SET `faction` =54 WHERE `entry` =8981 AND `faction` =91;

-- Spawn of Bael'Gar
 -- Old: Monster
 -- New: Dark Iron Dwarves
 UPDATE `creature_template` SET `faction` =54 WHERE `entry` =9436 AND `faction` =14;

-- Dark Keeper Zimrel
 -- Old: Beast - Wolf
 -- New: Dark Iron Dwarves
 UPDATE `creature_template` SET `faction` =54 WHERE `entry` =9441 AND `faction` =32;

-- Dark Keeper Pelver
 -- Old: Beast - Wolf
 -- New: Dark Iron Dwarves
 UPDATE `creature_template` SET `faction` =54 WHERE `entry` =9443 AND `faction` =32;

-- Frenzied Black Drake
 -- Old: Monster
 -- New: Dragonflight, Black
 UPDATE `creature_template` SET `faction` =103 WHERE `entry` =9461 AND `faction` =14;

-- Enraged Hippogryph
 -- Old: Monster
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =9527 AND `faction` =14;

-- Shadow of Lexlort
 -- Old: Orgrimmar
 -- New: Horde Generic
 UPDATE `creature_template` SET `faction` =83, `unit_flags` = (`unit_flags` | 512) WHERE `entry` =9539 AND `faction` =85;

-- Raschal the Courier
 -- Old: Friendly
 -- New: Escortee
 UPDATE `creature_template` SET `faction` =495, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =9546 AND `faction` =1080;

-- Necropolis Acolyte
 -- Old: Undead, Scourge
 -- New: Undead, Scourge
 UPDATE `creature_template` SET `faction` =233 WHERE `entry` =16368 AND `faction` =21;

-- Tomb Horror
 -- Old: Undead, Scourge
 -- New: Beast - Spider
 UPDATE `creature_template` SET `faction` =312 WHERE `entry` =15979 AND `faction` =21;

-- Treant Spirit
 -- Old: Spirit
 -- New: Alliance Generic
 UPDATE `creature_template` SET `faction` =534 WHERE `entry` =9601 AND `faction` =92;

-- Gargantuan Ooze
 -- Old: Enemy
 -- New: Monster
 UPDATE `creature_template` SET `faction` =14 WHERE `entry` =9621 AND `faction` =168;

-- Marshal Reginald Windsor
 -- Old: Beast - Wolf
 -- New: Escortee
 UPDATE `creature_template` SET `faction` =231 WHERE `entry` =9682 AND `faction` =32;

-- Ghok Bashguud
 -- Old: Beast - Wolf
 -- New: Orc, Blackrock
 UPDATE `creature_template` SET `faction` =40 WHERE `entry` =9718 AND `faction` =32;

-- Gorishi Hive Guard
 -- Old: Enemy
 -- New: Silithid
 UPDATE `creature_template` SET `faction` =310 WHERE `entry` =10040 AND `faction` =168;

-- Gorishi Hive Queen
 -- Old: Enemy
 -- New: Silithid
 UPDATE `creature_template` SET `faction` =310 WHERE `entry` =10041 AND `faction` =168;

-- Deathmaw
 -- Old: Monster
 -- New: Beast - Wolf
 UPDATE `creature_template` SET `faction` =38 WHERE `entry` =10077 AND `faction` =14;

-- Rookery Whelp
 -- Old: Monster
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =10161 AND `faction` =14;

-- General Colbatann
 -- Old: Monster
 -- New: Dragonflight, Black
 UPDATE `creature_template` SET `faction` =103 WHERE `entry` =10196 AND `faction` =14;

-- Rookery Guardian
 -- Old: Monster
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =10258 AND `faction` =14;

-- Burning Felhound
 -- Old: Demon
 -- New: Orc, Blackrock
 UPDATE `creature_template` SET `faction` =40 WHERE `entry` =10261 AND `faction` =90;

-- Burning Felguard
 -- Old: Monster
 -- New: Orc, Blackrock
 UPDATE `creature_template` SET `faction` =40 WHERE `entry` =10263 AND `faction` =14;

-- Solakar Flamewreath
 -- Old: Monster
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =10264 AND `faction` =14;

-- Captured Felwood Ooze
 -- Old: Enemy
 -- New: Friendly
 UPDATE `creature_template` SET `faction` =35, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =10290 AND `faction` =168;

-- Vaelan
 -- Old: Friendly
 -- New: Friendly
 UPDATE `creature_template` SET `faction` =35 WHERE `entry` =10296 AND `faction` =1080;

-- Vaelastrasz
 -- Old: Beast - Wolf
 -- New: Escortee
 UPDATE `creature_template` SET `faction` =250 WHERE `entry` =10538 AND `faction` =32;

-- Branch Snapper
 -- Old: Elemental
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =10641 AND `faction` =91;

-- Corrupted Cat
 -- Old: Orgrimmar
 -- New: Friendly
 UPDATE `creature_template` SET `faction` =35 WHERE `entry` =10657 AND `faction` =85;

-- Rookery Hatcher
 -- Old: Monster
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =10683 AND `faction` =14;

-- Shy-Rotam
 -- Old: Leopard
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =10737 AND `faction` =66;

-- Sian-Rotam
 -- Old: Leopard
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =10741 AND `faction` =66;

-- Finkle Einhorn
 -- Old: Beast - Wolf
 -- New: Friendly
 UPDATE `creature_template` SET `faction` =35 WHERE `entry` =10776 AND `faction` =32;

-- Warosh the Redeemed
 -- Old: Monster
 -- New: Friendly
 UPDATE `creature_template` SET `faction` =35 WHERE `entry` =10800 AND `faction` =14;

-- Stonespine
 -- Old: Monster
 -- New: Undead, Scourge
 UPDATE `creature_template` SET `faction` =21 WHERE `entry` =10809 AND `faction` =14;

-- Balnazzar
 -- Old: Demon
 -- New: Scarlet Crusade
 UPDATE `creature_template` SET `faction` =67 WHERE `entry` =10813 AND `faction` =90;

-- Warlord Thresh'jin
 -- Old: Beast - Wolf
 -- New: Troll, Frostmane
 UPDATE `creature_template` SET `faction` =37 WHERE `entry` =10822 AND `faction` =32;

-- Zul'Brin Warpbranch
 -- Old: Beast - Wolf
 -- New: Troll, Frostmane
 UPDATE `creature_template` SET `faction` =37 WHERE `entry` =10823 AND `faction` =32;

-- Deathspeaker Selendre
 -- Old: Monster
 -- New: Undead, Scourge
 UPDATE `creature_template` SET `faction` =21 WHERE `entry` =10827 AND `faction` =14;

-- Davil Lightfire
 -- Old: Escortee
 -- New: Escortee
 UPDATE `creature_template` SET `faction` =250 WHERE `entry` =10944 AND `faction` =113;

-- Darrowshire Betrayer
 -- Old: Monster
 -- New: Undead, Scourge
 UPDATE `creature_template` SET `faction` =974 WHERE `entry` =10947 AND `faction` =14;

-- Darrowshire Defender
 -- Old: Escortee
 -- New: Escortee
 UPDATE `creature_template` SET `faction` =495, `unit_flags` = (`unit_flags` | 256) WHERE `entry` =10948 AND `faction` =113;

-- Silver Hand Disciple
 -- Old: Escortee
 -- New: Escortee
 UPDATE `creature_template` SET `faction` =250 WHERE `entry` =10949 AND `faction` =113;

-- Marauding Corpse
 -- Old: Monster
 -- New: Undead, Scourge
 UPDATE `creature_template` SET `faction` =974 WHERE `entry` =10951 AND `faction` =14;

-- Marauding Skeleton
 -- Old: Monster
 -- New: Undead, Scourge
 UPDATE `creature_template` SET `faction` =974 WHERE `entry` =10952 AND `faction` =14;

-- Servant of Horgus
 -- Old: Monster
 -- New: Undead, Scourge
 UPDATE `creature_template` SET `faction` =974 WHERE `entry` =10953 AND `faction` =14;

-- Bloodletter
 -- Old: Monster
 -- New: Undead, Scourge
 UPDATE `creature_template` SET `faction` =974 WHERE `entry` =10954 AND `faction` =14;

-- Summoned Water Elemental
 -- Old: Naga
 -- New: Scarlet Crusade
 UPDATE `creature_template` SET `faction` =67 WHERE `entry` =10955 AND `faction` =74;

-- Arko'narin
 -- Old: Friendly
 -- New: Escortee
 UPDATE `creature_template` SET `faction` =250 WHERE `entry` =11018 AND `faction` =1080;

-- Crimson Rifleman
 -- Old: Monster
 -- New: Scarlet Crusade
 UPDATE `creature_template` SET `faction` =67 WHERE `entry` =11054 AND `faction` =14;

-- Fras Siabi
 -- Old: Monster
 -- New: Undead, Scourge
 UPDATE `creature_template` SET `faction` =21 WHERE `entry` =11058 AND `faction` =14;

-- Undead Postman
 -- Old: Undead, Scourge
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =11142 AND `faction` =974;

-- Postmaster Malown
 -- Old: Monster
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =11143 AND `faction` =14;

-- Dark Shade
 -- Old: Monster
 -- New: Undead, Scourge
 UPDATE `creature_template` SET `faction` =233 WHERE `entry` =11284 AND `faction` =14;

-- Alzzin the Wildshaper
 -- Old: Elemental
 -- New: Demon
 UPDATE `creature_template` SET `faction` =90 WHERE `entry` =11492 AND `faction` =91;

-- Immol'thar
 -- Old: Demon
 -- New: Dark Iron Dwarves
 UPDATE `creature_template` SET `faction` =754 WHERE `entry` =11496 AND `faction` =90;

-- Skarr the Unbreakable
 -- Old: Monster
 -- New: Ogre
 UPDATE `creature_template` SET `faction` =45 WHERE `entry` =11498 AND `faction` =14;

-- Chal Fairwind
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =11696 AND `faction` =79;

-- Coral Moongale
 -- Old: Darnassus
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =80 WHERE `entry` =11708 AND `faction` =79;

-- Siege Golem
 -- Old: Elemental
 -- New: Dark Iron Dwarves
 UPDATE `creature_template` SET `faction` =54 WHERE `entry` =2749 AND `faction` =91;

-- Alliance Sentinel
 -- Old: Stormpike Guard
 -- New: Stormpike Guard
 UPDATE `creature_template` SET `faction` =1216 WHERE `entry` =12048 AND `faction` =1217;

-- Frostwolf Warrior
 -- Old: Frostwolf Clan
 -- New: Frostwolf Clan
 UPDATE `creature_template` SET `faction` =1214 WHERE `entry` =12052 AND `faction` =1215;

-- Plagueland Termite
 -- Old: Creature
 -- New: Friendly
 UPDATE `creature_template` SET `faction` =35, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =12120 AND `faction` =7;

-- Great Shark
 -- Old: Beast - Spider
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =12124 AND `faction` =22;

-- Crimson Elite
 -- Old: Monster
 -- New: Scarlet Crusade
 UPDATE `creature_template` SET `faction` =67 WHERE `entry` =12128 AND `faction` =14;

-- Cured Gazelle
 -- Old: Creature
 -- New: Ambient
 UPDATE `creature_template` SET `faction` =188 WHERE `entry` =12297 AND `faction` =7;

-- Cured Deer
 -- Old: Creature
 -- New: Ambient
 UPDATE `creature_template` SET `faction` =188 WHERE `entry` =12299 AND `faction` =7;

-- Stormscale Toxicologist
 -- Old: Beast - Spider
 -- New: Naga
 UPDATE `creature_template` SET `faction` =74 WHERE `entry` =12321 AND `faction` =22;

-- Mortar Team Advanced Target Dummy
 -- Old: Training Dummy
 -- New: Ironforge
 UPDATE `creature_template` SET `faction` =55 WHERE `entry` =12385 AND `faction` =914;

-- Lord Kazzak
 -- Old: Monster
 -- New: Demon
 UPDATE `creature_template` SET `faction` =90 WHERE `entry` =12397 AND `faction` =14;

-- Nori Pridedrift
 -- Old: Ironforge
 -- New: Ironforge
 UPDATE `creature_template` SET `faction` =55, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =12738 AND `faction` =57;

-- Legionnaire Teena
 -- Old: Orgrimmar
 -- New: Orgrimmar
 UPDATE `creature_template` SET `faction` =29, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =12788 AND `faction` =85;

-- Advisor Willington
 -- Old: Undercity
 -- New: Undercity
 UPDATE `creature_template` SET `faction` =68, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =12790 AND `faction` =71;

-- Chieftain Earthbind
 -- Old: Thunder Bluff
 -- New: Thunder Bluff
 UPDATE `creature_template` SET `faction` =104, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =12791 AND `faction` =105;

-- Stone Guard Zarg
 -- Old: Orgrimmar
 -- New: Orgrimmar
 UPDATE `creature_template` SET `faction` =29, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =12794 AND `faction` =85;

-- First Sergeant Hola'mahi
 -- Old: Orgrimmar
 -- New: Darkspear Trolls
 UPDATE `creature_template` SET `faction` =126, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =12795 AND `faction` =85;

-- Raider Bork
 -- Old: Orgrimmar
 -- New: Friendly
 UPDATE `creature_template` SET `faction` =35, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =12796 AND `faction` =85;

-- Chief Murgut
 -- Old: Monster
 -- New: Furbolg
 UPDATE `creature_template` SET `faction` =82 WHERE `entry` =12918 AND `faction` =14;

-- Aerie Gryphon
 -- Old: Stormwind
 -- New: Stormpike Guard
 UPDATE `creature_template` SET `faction` =1216, `unit_flags` = (`unit_flags` | 512) WHERE `entry` =13161 AND `faction` =11;

-- Wing Commander Guse
 -- Old: Friendly
 -- New: Frostwolf Clan
 UPDATE `creature_template` SET `faction` =1215, `unit_flags` = (`unit_flags` | 256) WHERE `entry` =13179 AND `faction` =35;

-- Wing Commander Jeztor
 -- Old: Friendly
 -- New: Frostwolf Clan
 UPDATE `creature_template` SET `faction` =1215, `unit_flags` = (`unit_flags` | 256) WHERE `entry` =13180 AND `faction` =35;

-- Wing Commander Mulverick
 -- Old: Friendly
 -- New: Frostwolf Clan
 UPDATE `creature_template` SET `faction` =1215, `unit_flags` = (`unit_flags` | 256) WHERE `entry` =13181 AND `faction` =35;

-- Hive'Ashi Ambusher
 -- Old: Monster
 -- New: Silithid
 UPDATE `creature_template` SET `faction` =310 WHERE `entry` =13301 AND `faction` =14;

-- Seasoned Guardsman
 -- Old: Stormpike Guard
 -- New: Stormpike Guard
 UPDATE `creature_template` SET `faction` =1216 WHERE `entry` =13324 AND `faction` =1217;

-- Seasoned Mountaineer
 -- Old: Stormpike Guard
 -- New: Stormpike Guard
 UPDATE `creature_template` SET `faction` =1216 WHERE `entry` =13325 AND `faction` =1217;

-- Seasoned Sentinel
 -- Old: Stormpike Guard
 -- New: Stormpike Guard
 UPDATE `creature_template` SET `faction` =1216 WHERE `entry` =13327 AND `faction` =1217;

-- Seasoned Legionnaire
 -- Old: Frostwolf Clan
 -- New: Frostwolf Clan
 UPDATE `creature_template` SET `faction` =1214 WHERE `entry` =13329 AND `faction` =1215;

-- Seasoned Warrior
 -- Old: Frostwolf Clan
 -- New: Frostwolf Clan
 UPDATE `creature_template` SET `faction` =1214 WHERE `entry` =13330 AND `faction` =1215;

-- Veteran Guardsman
 -- Old: Stormpike Guard
 -- New: Stormpike Guard
 UPDATE `creature_template` SET `faction` =1216 WHERE `entry` =13333 AND `faction` =1217;

-- Veteran Legionnaire
 -- Old: Frostwolf Clan
 -- New: Frostwolf Clan
 UPDATE `creature_template` SET `faction` =1214 WHERE `entry` =13334 AND `faction` =1215;

-- Veteran Mountaineer
 -- Old: Stormpike Guard
 -- New: Stormpike Guard
 UPDATE `creature_template` SET `faction` =1216 WHERE `entry` =13335 AND `faction` =1217;

-- Veteran Sentinel
 -- Old: Stormpike Guard
 -- New: Stormpike Guard
 UPDATE `creature_template` SET `faction` =1216 WHERE `entry` =13336 AND `faction` =1217;

-- Veteran Warrior
 -- Old: Frostwolf Clan
 -- New: Frostwolf Clan
 UPDATE `creature_template` SET `faction` =1214 WHERE `entry` =13337 AND `faction` =1215;

-- Ivus the Forest Lord
 -- Old: Stormwind
 -- New: Darnassus
 UPDATE `creature_template` SET `faction` =1594, `unit_flags` = (`unit_flags` | 512) WHERE `entry` =13419 AND `faction` =11;

-- Champion Defender
 -- Old: Stormpike Guard
 -- New: Stormpike Guard
 UPDATE `creature_template` SET `faction` =1216 WHERE `entry` =13422 AND `faction` =1217;

-- Champion Guardsman
 -- Old: Stormpike Guard
 -- New: Stormpike Guard
 UPDATE `creature_template` SET `faction` =1216 WHERE `entry` =13424 AND `faction` =1217;

-- Champion Legionnaire
 -- Old: Frostwolf Clan
 -- New: Frostwolf Clan
 UPDATE `creature_template` SET `faction` =1214 WHERE `entry` =13425 AND `faction` =1215;

-- Champion Mountaineer
 -- Old: Stormpike Guard
 -- New: Stormpike Guard
 UPDATE `creature_template` SET `faction` =1216 WHERE `entry` =13426 AND `faction` =1217;

-- Champion Sentinel
 -- Old: Stormpike Guard
 -- New: Stormpike Guard
 UPDATE `creature_template` SET `faction` =1216 WHERE `entry` =13427 AND `faction` =1217;

-- Champion Warrior
 -- Old: Frostwolf Clan
 -- New: Frostwolf Clan
 UPDATE `creature_template` SET `faction` =1214 WHERE `entry` =13428 AND `faction` =1215;

-- Wing Commander Ichman
 -- Old: Friendly
 -- New: Stormpike Guard
 UPDATE `creature_template` SET `faction` =1217, `unit_flags` = (`unit_flags` | 256) WHERE `entry` =13437 AND `faction` =35;

-- Wing Commander Slidore
 -- Old: Friendly
 -- New: Stormpike Guard
 UPDATE `creature_template` SET `faction` =1217, `unit_flags` = (`unit_flags` | 256) WHERE `entry` =13438 AND `faction` =35;

-- Wing Commander Vipore
 -- Old: Friendly
 -- New: Stormpike Guard
 UPDATE `creature_template` SET `faction` =1217, `unit_flags` = (`unit_flags` | 256) WHERE `entry` =13439 AND `faction` =35;

-- Frostwolf Wolf Rider
 -- Old: Orgrimmar
 -- New: Frostwolf Clan
 UPDATE `creature_template` SET `faction` =1214 WHERE `entry` =13440 AND `faction` =85;

-- 7:XT
 -- Old: Beast - Wolf
 -- New: Creature
 UPDATE `creature_template` SET `faction` =7 WHERE `entry` =14224 AND `faction` =32;

-- Giggler
 -- Old: Beast - Wolf
 -- New: Beast - Wolf
 UPDATE `creature_template` SET `faction` =38 WHERE `entry` =14228 AND `faction` =32;

-- Lord Angler
 -- Old: Beast - Wolf
 -- New: Makrura
 UPDATE `creature_template` SET `faction` =129 WHERE `entry` =14236 AND `faction` =32;

-- Oozeworm
 -- Old: Beast - Wolf
 -- New: Monster
 UPDATE `creature_template` SET `faction` =14 WHERE `entry` =14237 AND `faction` =32;

-- Lord Condar
 -- Old: Beast - Carrion Bird
 -- New: Monster
 UPDATE `creature_template` SET `faction` =14 WHERE `entry` =14268 AND `faction` =73;

-- Tamra Stormpike
 -- Old: Stormwind
 -- New: Hillsbrad Militia
 UPDATE `creature_template` SET `faction` =88 WHERE `entry` =14275 AND `faction` =11;

-- Shen'dralar Spirit
 -- Old: Shen'dralar
 -- New: Shen'dralar
 UPDATE `creature_template` SET `faction` =1355, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =14364 AND `faction` =1354;

-- Cadaverous Worm
 -- Old: Monster
 -- New: Undead, Scourge
 UPDATE `creature_template` SET `faction` =21 WHERE `entry` =14370 AND `faction` =14;

-- Netherwalker
 -- Old: Monster
 -- New: Demon
 UPDATE `creature_template` SET `faction` =90 WHERE `entry` =14389 AND `faction` =14;

-- Eye of Immol'thar
 -- Old: Monster
 -- New: Dark Iron Dwarves
 UPDATE `creature_template` SET `faction` =754 WHERE `entry` =14396 AND `faction` =14;

-- Mana Burst
 -- Old: Monster
 -- New: Elemental
 UPDATE `creature_template` SET `faction` =834 WHERE `entry` =14397 AND `faction` =14;

-- Gilmorian
 -- Old: Beast - Wolf
 -- New: Murloc
 UPDATE `creature_template` SET `faction` =18 WHERE `entry` =14447 AND `faction` =32;

-- Princess Tempestria
 -- Old: Monster
 -- New: Elemental
 UPDATE `creature_template` SET `faction` =91 WHERE `entry` =14457 AND `faction` =14;

-- Setis
 -- Old: Monster
 -- New: Elemental
 UPDATE `creature_template` SET `faction` =91 WHERE `entry` =14471 AND `faction` =14;

-- Scourge Footsoldier
 -- Old: Undead, Scourge
 -- New: Monster
 UPDATE `creature_template` SET `faction` =14 WHERE `entry` =14486 AND `faction` =21;

-- Roloch
 -- Old: Beast - Wolf
 -- New: Ogre
 UPDATE `creature_template` SET `faction` =45 WHERE `entry` =14488 AND `faction` =32;

-- Kurmokk
 -- Old: Monster
 -- New: Beast - Gorilla
 UPDATE `creature_template` SET `faction` =72 WHERE `entry` =14491 AND `faction` =14;

-- Verifonix
 -- Old: Beast - Wolf
 -- New: Venture Company
 UPDATE `creature_template` SET `faction` =47 WHERE `entry` =14492 AND `faction` =32;

-- Terrordale Spirit
 -- Old: Spirit
 -- New: Undead, Scourge
 UPDATE `creature_template` SET `faction` =21 WHERE `entry` =14564 AND `faction` =92;

-- Darkreaver's Fallen Charger
 -- Old: Friendly
 -- New: Friendly
 UPDATE `creature_template` SET `faction` =35, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =14568 AND `faction` =1080;

-- Zapped Shore Strider
 -- Old: Monster
 -- New: Giant
 UPDATE `creature_template` SET `faction` =778 WHERE `entry` =14603 AND `faction` =14;

-- Zapped Land Walker
 -- Old: Monster
 -- New: Giant
 UPDATE `creature_template` SET `faction` =778 WHERE `entry` =14604 AND `faction` =14;

-- Zapped Wave Strider
 -- Old: Monster
 -- New: Giant
 UPDATE `creature_template` SET `faction` =778 WHERE `entry` =14638 AND `faction` =14;

-- Zapped Deep Strider
 -- Old: Monster
 -- New: Giant
 UPDATE `creature_template` SET `faction` =778 WHERE `entry` =14639 AND `faction` =14;

-- Wagner Hammerstrike
 -- Old: Ironforge
 -- New: Ironforge
 UPDATE `creature_template` SET `faction` =55 WHERE `entry` =15011 AND `faction` =57;

-- Spirit of Jin'do
 -- Old: Spirit
 -- New: Gadgetzan
 UPDATE `creature_template` SET `faction` =474 WHERE `entry` =15061 AND `faction` =92;

-- Pat's Hellfire Guy
 -- Old: Friendly
 -- New: Treasure
 UPDATE `creature_template` SET `faction` =114, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =15073 AND `faction` =35;

-- Gri'lek
 -- Old: Monster
 -- New: Troll, Frostmane
 UPDATE `creature_template` SET `faction` =37 WHERE `entry` =15082 AND `faction` =14;

-- Hammerfall Elite
 -- Old: Horde Generic
 -- New: Orgrimmar
 UPDATE `creature_template` SET `faction` =85 WHERE `entry` =15136 AND `faction` =83;

-- Spirit Shade
 -- Old: Dragonflight, Green
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =15261 AND `faction` =50;

-- Ironforge Brigade Rifleman
 -- Old: Ironforge
 -- New: Ironforge
 UPDATE `creature_template` SET `faction` =1611 WHERE `entry` =15441 AND `faction` =57;

-- Ironforge Brigade Footman
 -- Old: Ironforge
 -- New: Ironforge
 UPDATE `creature_template` SET `faction` =1611 WHERE `entry` =15442 AND `faction` =57;

-- Janela Stouthammer
 -- Old: Ironforge
 -- New: Ironforge
 UPDATE `creature_template` SET `faction` =1611 WHERE `entry` =15443 AND `faction` =57;

-- Arcanist Nozzlespring
 -- Old: Ironforge
 -- New: Ironforge
 UPDATE `creature_template` SET `faction` =1611 WHERE `entry` =15444 AND `faction` =57;

-- Krug Skullsplit
 -- Old: Orgrimmar
 -- New: Orgrimmar
 UPDATE `creature_template` SET `faction` =1612 WHERE `entry` =15612 AND `faction` =85;

-- Merok Longstride
 -- Old: Orgrimmar
 -- New: Orgrimmar
 UPDATE `creature_template` SET `faction` =1612 WHERE `entry` =15613 AND `faction` =85;

-- Shadow Priestess Shai
 -- Old: Orgrimmar
 -- New: Orgrimmar
 UPDATE `creature_template` SET `faction` =1612 WHERE `entry` =15615 AND `faction` =85;

-- Orgrimmar Legion Grunt
 -- Old: Orgrimmar
 -- New: Orgrimmar
 UPDATE `creature_template` SET `faction` =1612 WHERE `entry` =15616 AND `faction` =85;

-- Orgrimmar Legion Axe Thrower
 -- Old: Orgrimmar
 -- New: Orgrimmar
 UPDATE `creature_template` SET `faction` =1612 WHERE `entry` =15617 AND `faction` =85;

-- Hive'Regal Hunter-Killer
 -- Old: Monster
 -- New: Silithid Attackers
 UPDATE `creature_template` SET `faction` =1395 WHERE `entry` =15620 AND `faction` =14;

-- Sergeant Carnes
 -- Old: Ironforge
 -- New: Ironforge
 UPDATE `creature_template` SET `faction` =1611 WHERE `entry` =15903 AND `faction` =57;

-- Dread Creeper
 -- Old: Undead, Scourge
 -- New: Beast - Spider
 UPDATE `creature_template` SET `faction` =312 WHERE `entry` =15974 AND `faction` =21;

-- Bony Construct
 -- Old: Undead, Scourge
 -- New: Undead, Scourge
 UPDATE `creature_template` SET `faction` =233 WHERE `entry` =16167 AND `faction` =21;

-- Stoneskin Gargoyle
 -- Old: Undead, Scourge
 -- New: Undead, Scourge
 UPDATE `creature_template` SET `faction` =233 WHERE `entry` =16168 AND `faction` =21;

-- Eye Stalk
 -- Old: Undead, Scourge
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =16236 AND `faction` =21;

-- Plague Slime
 -- Old: Undead, Scourge
 -- New: Monster
 UPDATE `creature_template` SET `faction` =14 WHERE `entry` =16243 AND `faction` =21;

-- Mutated Grub
 -- Old: Undead, Scourge
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =16297 AND `faction` =21;

-- Sewage Slime
 -- Old: Undead, Scourge
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =16375 AND `faction` =21;

-- Archmage Tarsis Kir-Moldir
 -- Old: Friendly
 -- New: Friendly
 UPDATE `creature_template` SET `faction` =35, `unit_flags` = (`unit_flags` | 512) WHERE `entry` =16381 AND `faction` =1080;

-- Plagued Gargoyle
 -- Old: Undead, Scourge
 -- New: Undead, Scourge
 UPDATE `creature_template` SET `faction` =233 WHERE `entry` =16446 AND `faction` =21;

-- Plagued Deathhound
 -- Old: Undead, Scourge
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =16448 AND `faction` =21;

-- Necro Stalker
 -- Old: Undead, Scourge
 -- New: Beast - Spider
 UPDATE `creature_template` SET `faction` =312 WHERE `entry` =16453 AND `faction` =21;

-- Naxxramas Follower
 -- Old: Undead, Scourge
 -- New: Beast - Spider
 UPDATE `creature_template` SET `faction` =312 WHERE `entry` =16505 AND `faction` =21;

-- Naxxramas Worshipper
 -- Old: Undead, Scourge
 -- New: Beast - Spider
 UPDATE `creature_template` SET `faction` =312 WHERE `entry` =16506 AND `faction` =21;

-- Crypt Guard
 -- Old: Undead, Scourge
 -- New: Beast - Spider
 UPDATE `creature_template` SET `faction` =312 WHERE `entry` =16573 AND `faction` =21;

-- Mr. Bigglesworth
 -- Old: Prey
 -- New: Beast - Wolf
 UPDATE `creature_template` SET `faction` =32 WHERE `entry` =16998 AND `faction` =31;

-- Carrion Spinner
 -- Old: Undead, Scourge
 -- New: Beast - Spider
 UPDATE `creature_template` SET `faction` =312 WHERE `entry` =15975 AND `faction` =21;

-- Crypt Reaver
 -- Old: Undead, Scourge
 -- New: Beast - Spider
 UPDATE `creature_template` SET `faction` =312 WHERE `entry` =15978 AND `faction` =21;

-- Frenzied Bat
 -- Old: Undead, Scourge
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =16036 AND `faction` =21;

-- Infectious Skitterer
 -- Old: Undead, Scourge
 -- New: Beast - Spider
 UPDATE `creature_template` SET `faction` =312 WHERE `entry` =15977 AND `faction` =21;

-- Naxxramas Cultist
 -- Old: Undead, Scourge
 -- New: Beast - Spider
 UPDATE `creature_template` SET `faction` =312 WHERE `entry` =15980 AND `faction` =21;

-- Naxxramas Acolyte
 -- Old: Undead, Scourge
 -- New: Beast - Spider
 UPDATE `creature_template` SET `faction` =312 WHERE `entry` =15981 AND `faction` =21;

-- Plague Beast
 -- Old: Undead, Scourge
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =16034 AND `faction` =21;

-- Plagued Bat
 -- Old: Undead, Scourge
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =16037 AND `faction` =21;

-- Venom Stalker
 -- Old: Undead, Scourge
 -- New: Beast - Spider
 UPDATE `creature_template` SET `faction` =312 WHERE `entry` =15976 AND `faction` =21;

-- Apothecary Quinard
 -- Old: Undercity
 -- New: Undercity
 UPDATE `creature_template` SET `faction` =68 WHERE `entry` =17070 AND `faction` =71;

-- Marshal Bluewall
 -- Old: Ironforge
 -- New: Ironforge
 UPDATE `creature_template` SET `faction` =55 WHERE `entry` =17080 AND `faction` =57;

-- Captain Blackanvil
 -- Old: Ironforge
 -- New: Ironforge
 UPDATE `creature_template` SET `faction` =1611 WHERE `entry` =15440 AND `faction` =57;

-- Atiesh
 -- Old: Monster
 -- New: Demon
 UPDATE `creature_template` SET `faction` =954 WHERE `entry` =16387 AND `faction` =14;

-- Archaedas
 -- Old: Friendly
 -- New: Monster
 UPDATE `creature_template` SET `faction` =14, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =2748 AND `faction` =35;

-- Polymorph Clone
 -- Old: Creature
 -- New: Prey
 UPDATE `creature_template` SET `faction` =31 WHERE `entry` =16479 AND `faction` =7;

-- Soul Weaver
 -- Old: Undead, Scourge
 -- New: Monster
 UPDATE `creature_template` SET `faction` =14 WHERE `entry` =16429 AND `faction` =21;

-- Unstoppable Abomination
 -- Old: Undead, Scourge
 -- New: Monster
 UPDATE `creature_template` SET `faction` =14 WHERE `entry` =16428 AND `faction` =21;

-- Soldier of the Frozen Wastes
 -- Old: Undead, Scourge
 -- New: Monster
 UPDATE `creature_template` SET `faction` =14 WHERE `entry` =16427 AND `faction` =21;

-- Brave Stonehide
 -- Old: Orgrimmar
 -- New: Friendly
 UPDATE `creature_template` SET `faction` =35 WHERE `entry` =12793 AND `faction` =85;

-- Chief Expeditionary Requisitioner Enkles
 -- Old: Ironforge
 -- New: Ironforge
 UPDATE `creature_template` SET `faction` =55 WHERE `entry` =17068 AND `faction` =57;

-- Wushoolay
 -- Old: Monster
 -- New: Troll, Frostmane
 UPDATE `creature_template` SET `faction` =37 WHERE `entry` =15085 AND `faction` =14;

-- Sergeant Thunderhorn
 -- Old: Orgrimmar
 -- New: Friendly
 UPDATE `creature_template` SET `faction` =35 WHERE `entry` =14581 AND `faction` =85;

-- Hive'Zara Hornet
 -- Old: Monster
 -- New: Silithid
 UPDATE `creature_template` SET `faction` =310 WHERE `entry` =15934 AND `faction` =14;

-- Rotting Maggot
 -- Old: Undead, Scourge
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =16057 AND `faction` =21;

-- Spore
 -- Old: Undead, Scourge
 -- New: Monster
 UPDATE `creature_template` SET `faction` =14 WHERE `entry` =16286 AND `faction` =21;

-- Web Wrap
 -- Old: Monster
 -- New: Monster
 UPDATE `creature_template` SET `faction` =14 WHERE `entry` =16486 AND `faction` =16;

-- Caravan Mule
 -- Old: Friendly
 -- New: Escortee
 UPDATE `creature_template` SET `faction` =290 WHERE `entry` =16232 AND `faction` =35;

-- Field Marshal Chambers
 -- Old: Friendly
 -- New: Argent Dawn
 UPDATE `creature_template` SET `faction` =794, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =16254 AND `faction` =35;

-- Blizzard
 -- Old: Undead, Scourge
 -- New: Monster
 UPDATE `creature_template` SET `faction` =14 WHERE `entry` =16474 AND `faction` =21;

-- Void Zone
 -- Old: Gnoll - Riverpaw
 -- New: Armies of C'Thun
 UPDATE `creature_template` SET `faction` =370 WHERE `entry` =16697 AND `faction` =20;

-- Garden Gas
 -- Old: Friendly
 -- New: Treasure
 UPDATE `creature_template` SET `faction` =114, `unit_flags` = (`unit_flags` | 512) WHERE `entry` =17231 AND `faction` =35;

-- Hive'Zora Abomination
 -- Old: Friendly
 -- New: Silithid Attackers
 UPDATE `creature_template` SET `faction` =1395 WHERE `entry` =15449 AND `faction` =35;

-- Nighthaven Defender
 -- Old: Friendly
 -- New: Escortee
 UPDATE `creature_template` SET `faction` =495 WHERE `entry` =15495 AND `faction` =35;

-- Nightmare Phantasm
 -- Old: Monster
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =15629 AND `faction` =14;

-- Might of Kalimdor Captain
 -- Old: Friendly
 -- New: Might of Kalimdor
 UPDATE `creature_template` SET `faction` =777 WHERE `entry` =15845 AND `faction` =35;

-- Might of Kalimdor Shaman
 -- Old: Friendly
 -- New: Might of Kalimdor
 UPDATE `creature_template` SET `faction` =777 WHERE `entry` =15847 AND `faction` =35;

-- Snokh Blackspine
 -- Old: Beast - Raptor
 -- New: Monster
 UPDATE `creature_template` SET `faction` =14 WHERE `entry` =16051 AND `faction` =48;

-- Malgen Longspear
 -- Old: Beast - Raptor
 -- New: Monster
 UPDATE `creature_template` SET `faction` =14 WHERE `entry` =16052 AND `faction` =48;

-- Korv
 -- Old: Beast - Raptor
 -- New: Monster
 UPDATE `creature_template` SET `faction` =14 WHERE `entry` =16053 AND `faction` =48;

-- Rezznik
 -- Old: Beast - Raptor
 -- New: Monster
 UPDATE `creature_template` SET `faction` =14 WHERE `entry` =16054 AND `faction` =48;

-- Diseased Maggot
 -- Old: Undead, Scourge
 -- New: Monster
 UPDATE `creature_template` SET `faction` =16 WHERE `entry` =16056 AND `faction` =21;

-- Gnashjaw
 -- Old: Beast - Raptor
 -- New: Ogre
 UPDATE `creature_template` SET `faction` =45 WHERE `entry` =16095 AND `faction` =48;

-- Zombie Chow
 -- Old: Undead, Scourge
 -- New: Undead, Scourge
 UPDATE `creature_template` SET `faction` =233 WHERE `entry` =16360 AND `faction` =21;

-- Grobbulus Cloud
 -- Old: Undead, Scourge
 -- New: Monster
 UPDATE `creature_template` SET `faction` =14 WHERE `entry` =16363 AND `faction` =21;

-- Pat's Firework Guy - YELLOW BIG
 -- Old: Friendly
 -- New: Treasure
 UPDATE `creature_template` SET `faction` =114 WHERE `entry` =15890 AND `faction` =35;

-- Pat's Firework Guy - WHITE
 -- Old: Friendly
 -- New: Treasure
 UPDATE `creature_template` SET `faction` =114 WHERE `entry` =15884 AND `faction` =35;

-- Pat's Firework Guy - GREEN
 -- Old: Friendly
 -- New: Treasure
 UPDATE `creature_template` SET `faction` =114 WHERE `entry` =15880 AND `faction` =35;

-- Pat's Firework Guy - BLUE
 -- Old: Friendly
 -- New: Treasure
 UPDATE `creature_template` SET `faction` =114 WHERE `entry` =15879 AND `faction` =35;

-- Pat's Firework Cluster Guy (GREEN)
 -- Old: Friendly
 -- New: Treasure
 UPDATE `creature_template` SET `faction` =114 WHERE `entry` =15874 AND `faction` =35;

-- Pat's Firework Cluster Guy (RED)
 -- Old: Friendly
 -- New: Treasure
 UPDATE `creature_template` SET `faction` =114 WHERE `entry` =15873 AND `faction` =35;

-- Pat's Firework Cluster Guy (BLUE)
 -- Old: Friendly
 -- New: Treasure
 UPDATE `creature_template` SET `faction` =114 WHERE `entry` =15872 AND `faction` =35;

-- Pat's Firework Cluster Guy (ELUNE)
 -- Old: Friendly
 -- New: Treasure
 UPDATE `creature_template` SET `faction` =114 WHERE `entry` =15918 AND `faction` =35;

-- Pat's Firework Guy - YELLOW
 -- Old: Friendly
 -- New: Treasure
 UPDATE `creature_template` SET `faction` =114 WHERE `entry` =15883 AND `faction` =35;

-- Giant Spotlight
 -- Old: Friendly
 -- New: Treasure
 UPDATE `creature_template` SET `faction` =114, `unit_flags` = (`unit_flags` | 256) WHERE `entry` =15902 AND `faction` =35;

-- Living Poison
 -- Old: Undead, Scourge
 -- New: Undead, Scourge
 UPDATE `creature_template` SET `faction` =974 WHERE `entry` =16027 AND `faction` =21;

-- Shadow Fissure
 -- Old: Undead, Scourge
 -- New: Monster
 UPDATE `creature_template` SET `faction` =14 WHERE `entry` =16129 AND `faction` =21;

-- Theldren Kill Credit
 -- Old: Friendly
 -- New: Treasure
 UPDATE `creature_template` SET `faction` =114, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =16166 AND `faction` =35;

-- Naxxramas Combat Dummy
 -- Old: Undead, Scourge
 -- New: Training Dummy
 UPDATE `creature_template` SET `faction` =1095 WHERE `entry` =16211 AND `faction` =21;

-- Spirit of Mograine
 -- Old: Gnoll - Riverpaw
 -- New: Undead, Scourge
 UPDATE `creature_template` SET `faction` =21 WHERE `entry` =16775 AND `faction` =20;

-- Spirit of Blaumeux
 -- Old: Gnoll - Riverpaw
 -- New: Undead, Scourge
 UPDATE `creature_template` SET `faction` =21 WHERE `entry` =16776 AND `faction` =20;

-- Spirit of Zeliek
 -- Old: Gnoll - Riverpaw
 -- New: Undead, Scourge
 UPDATE `creature_template` SET `faction` =21 WHERE `entry` =16777 AND `faction` =20;

-- Spirit of Korth'azz
 -- Old: Gnoll - Riverpaw
 -- New: Undead, Scourge
 UPDATE `creature_template` SET `faction` =21 WHERE `entry` =16778 AND `faction` =20;

-- Sapphiron's Wing buffet
 -- Old: Friendly
 -- New: Treasure
 UPDATE `creature_template` SET `faction` =114 WHERE `entry` =17025 AND `faction` =35;

-- Lady Palanseer
 -- Old: Orgrimmar
 -- New: Friendly
 UPDATE `creature_template` SET `faction` =35, `unit_flags` = (`unit_flags` | 768) WHERE `entry` =12792 AND `faction` =85;

-- Defias Raider
 -- Old: Monster
 -- New: Defias Brotherhood
 UPDATE `creature_template` SET `faction` =17 WHERE `entry` =6180 AND `faction` =14;

-- Ragepaw
 -- Old: Monster
 -- New: Furbolg
 UPDATE `creature_template` SET `faction` =82 WHERE `entry` =14342 AND `faction` =14;

-- Corrupted Fire Nova Totem V
 -- Old: Troll, Frostmane
 -- New: Dragonflight, Black
 UPDATE `creature_template` SET `faction` =103 WHERE `entry` =14662 AND `faction` =37;

-- Corrupted Stoneskin Totem VI
 -- Old: Troll, Frostmane
 -- New: Dragonflight, Black
 UPDATE `creature_template` SET `faction` =103 WHERE `entry` =14663 AND `faction` =37;

-- Corrupted Healing Stream Totem V
 -- Old: Troll, Frostmane
 -- New: Dragonflight, Black
 UPDATE `creature_template` SET `faction` =103 WHERE `entry` =14664 AND `faction` =37;

-- Corrupted Windfury Totem III
 -- Old: Troll, Frostmane
 -- New: Dragonflight, Black
 UPDATE `creature_template` SET `faction` =103 WHERE `entry` =14666 AND `faction` =37;

-- Lady Onyxia
 -- Old: Beast - Wolf
 -- New: Stormwind
 UPDATE `creature_template` SET `faction` =12 WHERE `entry` =12756 AND `faction` =32;

-- Horgus the Ravager
 -- Old: Monster
 -- New: Undead, Scourge
 UPDATE `creature_template` SET `faction` =974 WHERE `entry` =10946 AND `faction` =14;

-- Stone Keeper
 -- Old: Friendly
 -- New: Titan
 UPDATE `creature_template` SET `faction` =416, `unit_flags` =33555264 WHERE `entry` =4857 AND `faction` =35;

-- Ironaya
 -- Old: Titan
 -- New: Titan
 UPDATE `creature_template` SET `faction` =416 WHERE `entry` =7228 AND `faction` =415;

-- Vault Warder
 -- Old: Friendly
 -- New: Titan
 UPDATE `creature_template` SET `faction` =416, `unit_flags` =33555264 WHERE `entry` =10120 AND `faction` =35;

-- End of migration.
;
