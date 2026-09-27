INSERT INTO `migrations` VALUES ('20250320032550');

-- Add your query below.
 -- Add name column for areatriggers.
 CREATE TABLE `areatrigger_template__new` (
  `id` INTEGER NOT NULL DEFAULT 0,
  `build` INTEGER NOT NULL DEFAULT 0,
  `name` TEXT DEFAULT '' COLLATE NOCASE,
  `map_id` INTEGER NOT NULL DEFAULT 0,
  `x` REAL NOT NULL DEFAULT 0,
  `y` REAL NOT NULL DEFAULT 0,
  `z` REAL NOT NULL DEFAULT 0,
  `radius` REAL NOT NULL DEFAULT 0,
  `box_x` REAL NOT NULL DEFAULT 0,
  `box_y` REAL NOT NULL DEFAULT 0,
  `box_z` REAL NOT NULL DEFAULT 0,
  `box_orientation` REAL NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`, `build`)
);

INSERT INTO `areatrigger_template__new` (`id`, `build`, `name`, `map_id`, `x`, `y`, `z`, `radius`, `box_x`, `box_y`, `box_z`, `box_orientation`)
SELECT `id`, `build`, '', `map_id`, `x`, `y`, `z`, `radius`, `box_x`, `box_y`, `box_z`, `box_orientation` FROM `areatrigger_template`;

DROP TABLE `areatrigger_template`;
ALTER TABLE `areatrigger_template__new` RENAME TO `areatrigger_template`;

-- Assign names to all areatriggers.
 UPDATE `areatrigger_template` SET `name` = 'Scarlet Monastery Graveyard - Entrance' WHERE `id` =45 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Westfall, Sentinel Hill - Inn' WHERE `id` =71 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Deadmines - Entrance' WHERE `id` =78 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Elwynn Forest, Northshire Valley - Quest The Jasperlode Mine' WHERE `id` =87 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Elwynn Forest, Fargodeep Mine - Quest The Fargodeep Mine' WHERE `id` =88 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Alterac Valley - Unused, Under Terrain' WHERE `id` =95 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Dun Morogh - Quest Frostmane Hold' WHERE `id` =97 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Stranglethorn Vale, Nesingwary''s Expedition - Quest Investigate the Camp' WHERE `id` =98 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Tirisfal Glades, Night Web''s Hollow' WHERE `id` =100 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Stormwind Stockades - Entrance' WHERE `id` =101 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Stormwind Stockades - Entrance' WHERE `id` =101 AND `build` =4375;

UPDATE `areatrigger_template` SET `name` = 'Stormwind Stockades - Entrance' WHERE `id` =101 AND `build` =4449;

UPDATE `areatrigger_template` SET `name` = 'Stormwind Vault - Entrance' WHERE `id` =107 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Stormwind Vault - Exit' WHERE `id` =109 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Deadmines - Exit' WHERE `id` =119 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Deadmines - Back Exit' WHERE `id` =121 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Shadowfang Keep - Entrance' WHERE `id` =145 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Dun Morogh, Frostmane Hold - Unused, Wide Open Cave' WHERE `id` =168 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Dun Morogh, Frostmane Hold - Unused, Tunnel to Arctikus' WHERE `id` =169 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Loch Modan, The Loch - Quest Resupplying the Excavation, Ambush' WHERE `id` =171 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Silverpine Forest, The Dead Field - Quest The Dead Fields' WHERE `id` =173 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Wetlands, Dun Algaz - Quest The Algaz Gauntlet' WHERE `id` =175 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Alterac Mountains, Strahnbrad Inn - Quest Gol''dir' WHERE `id` =178 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Shadowfang Keep - Exit' WHERE `id` =194 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Stranglethorn Vale, The Savage Coast - Quest The Stone of the Tides' WHERE `id` =196 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Elwynn Forest, Fargodeep Mine - Quest The Fargodeep Mine' WHERE `id` =197 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Mulgore, The Venture Co. Mine - Unused, Mine Entrance' WHERE `id` =205 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'The Barrens, The Forgotten Pools - Quest The Forgotten Pools' WHERE `id` =216 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Teldrassil, Shadowglen - Unused, Moonwell' WHERE `id` =217 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Teldrassil, Starbreeze Village - Unused, Moonwell' WHERE `id` =218 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Teldrassil, Pools of Arlithrien - Unused, Moonwell' WHERE `id` =219 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Teldrassil, The Oracle Glade - Unused, Moonwell' WHERE `id` =220 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Teldrassil, The Oracle Glade - Unused, Moonwell' WHERE `id` =220 AND `build` =4297;

UPDATE `areatrigger_template` SET `name` = 'Darkshore, The Master''s Glaive - Quest The Master''s Glaive' WHERE `id` =223 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Darkshore, The Master''s Glaive - Quest The Master''s Glaive' WHERE `id` =224 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Darkshore, The Master''s Glaive - Quest The Master''s Glaive' WHERE `id` =225 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Wailing Caverns - Exit' WHERE `id` =226 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Wailing Caverns - Entrance' WHERE `id` =228 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Darkshore, Bashal''Aran - Quest Bashal''Aran' WHERE `id` =230 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Darkshore - Quest How Big a Threat?' WHERE `id` =231 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Darkshore - Quest How Big a Threat?' WHERE `id` =232 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Darkshore, Twilight Vale - Unused, Blackwood Furbolg Bonfire' WHERE `id` =233 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Darkshore, Blackwood Den - Unused, Near Volcor' WHERE `id` =234 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Darkshore, Twilight Vale - Quest How Big a Threat?' WHERE `id` =235 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Darkshore, Twilight Vale - Unused, Blackwood Furbolg Campfire' WHERE `id` =236 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Darkshore, Twilight Vale - Unused, Blackwood Furbolg Camp' WHERE `id` =237 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Darkshore, Cliffspring River - Unused, Blackwood Furbolg Camp' WHERE `id` =238 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Ashenvale - Unused, Near Talen' WHERE `id` =239 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Razorfen Kraul - Exit' WHERE `id` =242 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Razorfen Kraul - Entrance' WHERE `id` =244 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Thousand Needles - Quest Test of Faith' WHERE `id` =246 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Shadowfang Keep - Unused, First Wall' WHERE `id` =254 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Shadowfang Keep - Unsued, Second Wall' WHERE `id` =255 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Shadowfang Keep - Unused, Third Wall' WHERE `id` =256 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackfathom Deeps - Entrance' WHERE `id` =257 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackfathom Deeps - Exit' WHERE `id` =259 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Razorfen Kraul - Agathelos'' Cave' WHERE `id` =262 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackfathom Deeps - Thaelrids Cave' WHERE `id` =283 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Stormwind City - Near Dashel Stonefist' WHERE `id` =284 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Uldaman - Entrance' WHERE `id` =286 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Uldaman - Exit' WHERE `id` =288 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Dustwallow Marsh, Sentry Point - Tower Entrance' WHERE `id` =302 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Dustwallow Marsh - Unused, Near Road' WHERE `id` =303 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Gnomeregan - Exit' WHERE `id` =322 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Gnomeregan - Entrance' WHERE `id` =324 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Elwynn Forest, Jasperlode Mine - Quest The Jasperlode Mine' WHERE `id` =342 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Swamp of Sorrows, Pool of Tears - Quest In Search of The Temple' WHERE `id` =362 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Stormwind City - Near Dashel Stonefirst' WHERE `id` =382 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Desolace - Unused, Near Destroyed Caravan' WHERE `id` =422 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Razorfen Downs - Entrance' WHERE `id` =442 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Razorfen Downs - Exit' WHERE `id` =444 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Sunken Temple - Entrance' WHERE `id` =446 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Sunken Temple - Exit' WHERE `id` =448 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Razorfen Kraul - Unused, Near Bridge' WHERE `id` =462 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Razorfen Kraul - Unused, Bat Cave, Near Charlga Razorflank' WHERE `id` =463 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Redridge Mountains, Redridge Canyons - Quest The Rethban Gauntlet' WHERE `id` =482 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Stormwind Stockades - Exit' WHERE `id` =503 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'The Barrens, Fray Island - Quest The Affray - at_twiggy_flathead' WHERE `id` =522 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Gnomeregan - Back Entrance' WHERE `id` =523 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Gnomeregan - Back Exit' WHERE `id` =525 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Darnassus - Exit' WHERE `id` =527 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Darnassus - Entrance' WHERE `id` =542 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Elwynn Forest, Goldshire - Inn' WHERE `id` =562 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Scarlet Monastery Graveyard - Exit' WHERE `id` =602 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Scarlet Monastery Cathedral - Exit' WHERE `id` =604 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Scarlet Monastery Armory - Exit' WHERE `id` =606 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Scarlet Monastery Library - Exit' WHERE `id` =608 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Scarlet Monastery Cathedral - Entrance' WHERE `id` =610 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Scarlet Monastery Armory - Entrance' WHERE `id` =612 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Scarlet Monastery Library - Entrance' WHERE `id` =614 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Redridge Mountains, Lakeshire - Inn' WHERE `id` =682 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Stormwind Wizard''s Sanctum - Exit' WHERE `id` =702 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Stormwind Wizard''s Sanctum - Entrance' WHERE `id` =704 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Duskwood, Darkshire - Inn' WHERE `id` =707 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Hillsbrad Foothills, Southshore - Inn' WHERE `id` =708 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Hillsbrad Foothills, Southshore - Inn' WHERE `id` =708 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Dustwallow Marsh, Theramore Isle - Inn' WHERE `id` =709 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Dun Morogh, Kharanos - Inn' WHERE `id` =710 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Loch Modan, Thelsamar - Inn' WHERE `id` =712 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Wetlands, Menethil Harbor - Inn' WHERE `id` =713 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Teldrassil, Dolanaar - Inn' WHERE `id` =715 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Darkshore, Auberdine - Inn' WHERE `id` =716 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Ashenvale, Astranaar - Inn' WHERE `id` =717 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Tirisfal Glades, Brill - Inn' WHERE `id` =719 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Silverpine Forest, The Sepulcher - Inn' WHERE `id` =720 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Hillsbrad Foothills, Tarren Mill - Inn' WHERE `id` =721 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Hillsbrad Foothills, Tarren Mill - Inn' WHERE `id` =721 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Mulgore, Bloodhoof Village - Inn' WHERE `id` =722 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'The Barrens, The Crossroads - Inn' WHERE `id` =742 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'The Barrens, Ratchet - Inn' WHERE `id` =743 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackfathom Deeps - Unused, Near Orb, After Aku''mai' WHERE `id` =762 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Wetlands, Menethil Harbor - Unused, Near Edwina Monzor' WHERE `id` =802 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Wetlands, Menethil Harbor - Unused, In Front of Keep' WHERE `id` =803 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Uldaman - Quest The Hidden Chamber' WHERE `id` =822 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Durotar, Razor Hill - Inn' WHERE `id` =843 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Swamp of Sorrows, Stonard - Inn' WHERE `id` =844 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Stranglethorn Vale, Booty Bay - Inn' WHERE `id` =862 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Uldaman - Back Exit' WHERE `id` =882 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Uldaman - Back Entrance' WHERE `id` =902 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Zul''Farrak - Exit' WHERE `id` =922 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Zul''Farrak - Entrance' WHERE `id` =924 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Thousand Needles - Test of Faith' WHERE `id` =942 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Thousand Needles - Test of Faith' WHERE `id` =943 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Thousand Needles - Test of Faith' WHERE `id` =944 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Zul''Farrak - Near Zum''rah' WHERE `id` =962 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'The Barrens, Camp Taurajo - Inn' WHERE `id` =982 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Stonetalon Mountains, Sun Rock Retreat - Inn' WHERE `id` =1022 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Tanaris, Gadgetzan - Inn' WHERE `id` =1023 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Tanaris, Gadgetzan - Inn' WHERE `id` =1023 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Feralas, Feathermoon Stronghold - Inn' WHERE `id` =1024 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Feralas, Camp Mojache - Inn' WHERE `id` =1025 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Feralas, Camp Mojache - Inn' WHERE `id` =1025 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'The Hinterlands, Wildhammer Keep - Inn' WHERE `id` =1042 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Onyxia''s Lair - Exit' WHERE `id` =1064 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Transpolyporter - Booty Bay to Gnomeregan' WHERE `id` =1103 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Transpolyporter - Gnomeregan to Booty Bay' WHERE `id` =1104 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Gnomeregan, The Clockwerk Run - Near Beginning' WHERE `id` =1105 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Stormwind City - Unused, Cathedral of Light Entrance' WHERE `id` =1125 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'The Hinterlands, The Altar of Zul - Quest The Altar of Zul' WHERE `id` =1205 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'The Temple of Atal''Hakkar, The Pit of Sacrifice - Unused, Path to Lair of the Chosen' WHERE `id` =1306 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'The Temple of Atal''Hakkar, The Pit of Refuse - Unused, Inside Hole' WHERE `id` =1326 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Azshara, The Shattered Strand - Quest Betrayed' WHERE `id` =1387 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Azshara, Thalassian Base Camp - Unused, Near Kaldorei Tome of Summoning' WHERE `id` =1388 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blasted Lands, Rise of the Defiler - Unused, Top of Hill' WHERE `id` =1426 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blasted Lands, Rise of the Defiler - Unused, Rune of the Defiler' WHERE `id` =1427 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blasted Lands, Rise of the Defiler - Unused, Rune of the Defiler' WHERE `id` =1428 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blasted Lands, Rise of the Defiler - Unused, Rune of the Defiler' WHERE `id` =1429 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blasted Lands, Rise of the Defiler - Unused, Rune of Return' WHERE `id` =1446 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Zul''Farrak - Near Antu''sul' WHERE `id` =1447 AND `build` =4222;

DELETE FROM `creature_ai_events` WHERE `creature_id` =8127;

INSERT INTO `creature_ai_events` (`id`, `creature_id`, `condition_id`, `event_type`, `event_inverse_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action1_script`, `action2_script`, `action3_script`, `comment`) VALUES (812701, 8127, 0, 4, 0, 100, 0, 0, 0, 0, 0, 812701, 0, 0, 'Antu''sul - Set Data on Aggro'), (812702, 8127, 0, 7, 0, 100, 0, 0, 0, 0, 0, 812702, 0, 0, 'Antu''sul - Set Data on Evade'), (812703, 8127, 0, 6, 0, 100, 0, 0, 0, 0, 0, 812703, 0, 0, 'Antu''sul - Set Data on Death'), (812704, 8127, 0, 0, 0, 100, 3, 11600, 11600, 13300, 21700, 812724, 812725, 0, 'Antu''sul - Random Cast Earthgrab Totem or Healing Ward'), (812705, 8127, 0, 2, 0, 100, 0, 75, 0, 0, 0, 812705, 0, 0, 'Antu''sul - Yell and Cast Antu''sul''s Minion at 75% HP'), (812706, 8127, 0, 2, 0, 100, 0, 25, 0, 0, 0, 812706, 0, 0, 'Antu''sul - Yell and Cast 2 Antu''sul''s Minions at 25% HP'), (812707, 8127, 0, 2, 0, 100, 0, 20, 0, 0, 0, 812707, 0, 0, 'Antu''sul - Cast Healing Wave of Antu''sul at 20% HP'), (812708, 8127, 0, 2, 0, 100, 1, 30, 0, 12000, 12000, 812708, 0, 0, 'Antu''sul - Cast Wave of Antu''sul at 30%'), (812709, 8127, 0, 2, 0, 100, 1, 60, 0, 17000, 20000, 812709, 0, 0, 'Antu''sul - Cast Flash Heal at 60%');

DELETE FROM `creature_ai_scripts` WHERE `id` =812701;

INSERT INTO `creature_ai_scripts` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (812701, 0, 0, 37, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Antu''sul - Set Data to In Progress');

DELETE FROM `creature_ai_scripts` WHERE `id` =812702;

INSERT INTO `creature_ai_scripts` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (812702, 0, 0, 37, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Antu''sul - Set Data to Not Started');

DELETE FROM `creature_ai_scripts` WHERE `id` =812703;

INSERT INTO `creature_ai_scripts` (`id`, `delay`, `priority`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES (812703, 0, 0, 37, 5, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Antu''sul - Set Data to Done');

INSERT INTO `scripted_areatrigger` (`entry`, `script_name`) VALUES (1447, 'at_antusul');

UPDATE `areatrigger_template` SET `name` = 'Blackrock Depths - Entrance' WHERE `id` =1466 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackrock Spire - Entrance' WHERE `id` =1468 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackrock Spire - Exit' WHERE `id` =1470 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackrock Depths - Exit' WHERE `id` =1472 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Burning Steppes, Dreadmaul Rock - Unused, Near Sha''ni Proudtusk''s Remains' WHERE `id` =1506 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackrock Depths, Ring of the Law - Arena Center' WHERE `id` =1526 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackrock Depths, Detention Block - Unused, Near Gor''Shak''s Cell' WHERE `id` =1590 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Badlands, Kargath - Inn' WHERE `id` =1606 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Old Hillsbrad Foothills - Exit' WHERE `id` =1626 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackrock Spire, Hordemar City  - Unused, Jump Down' WHERE `id` =1628 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'The Black Morass - Unused, Exit' WHERE `id` =1629 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Caverns of Time - Unused, Dungeon Entrance 1' WHERE `id` =1631 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Caverns of Time - Unused, Dungeon Entrance 2' WHERE `id` =1632 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Arathi Highlands, Hammerfall - Inn' WHERE `id` =1646 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Dustwallow Marsh, Sentry Point - Quest The Missing Diplomat 14, Tower Entrance' WHERE `id` =1667 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackrock Depths, Detention Block - Unused, Near Marshal Windsor''s Cell' WHERE `id` =1686 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Un''Goro Crater - Quest The Scent of Lar''korwi, Spawn Mate' WHERE `id` =1726 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Un''Goro Crater - Quest The Scent of Lar''korwi, Spawn Mate' WHERE `id` =1727 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Un''Goro Crater - Quest The Scent of Lar''korwi, Spawn Mate' WHERE `id` =1728 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Un''Goro Crater - Quest The Scent of Lar''korwi, Spawn Mate' WHERE `id` =1729 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Un''Goro Crater - Quest The Scent of Lar''korwi, Spawn Mate' WHERE `id` =1730 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Un''Goro Crater - Quest The Scent of Lar''korwi, Spawn Mate' WHERE `id` =1731 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Un''Goro Crater - Quest The Scent of Lar''korwi, Spawn Mate' WHERE `id` =1732 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Un''Goro Crater - Quest The Scent of Lar''korwi, Spawn Mate' WHERE `id` =1733 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Un''Goro Crater - Quest The Scent of Lar''korwi, Spawn Mate' WHERE `id` =1734 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Un''Goro Crater - Quest The Scent of Lar''korwi, Spawn Mate' WHERE `id` =1735 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Un''Goro Crater - Quest The Scent of Lar''korwi, Spawn Mate' WHERE `id` =1736 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Un''Goro Crater - Quest The Scent of Lar''korwi, Spawn Mate' WHERE `id` =1737 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Un''Goro Crater - Quest The Scent of Lar''korwi, Spawn Mate' WHERE `id` =1738 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Un''Goro Crater - Quest The Scent of Lar''korwi, Spawn Mate' WHERE `id` =1739 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Un''Goro Crater - Quest The Scent of Lar''korwi, Spawn Mate' WHERE `id` =1740 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackrock Depths, Detention Block - Near Kharan Mighthammer''s Cell' WHERE `id` =1746 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Un''Goro Crater - Quest The Scent of Lar''korwi, Spawn Mate' WHERE `id` =1766 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackrock Depths, Shadowforge City - Bridge Ambush' WHERE `id` =1786 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackrock Depths - Unused, Near Beginning' WHERE `id` =1826 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackrock Depths, The Lyceum - Unused, Near Doors' WHERE `id` =1827 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackrock Depths - Unused, Behind Locked Gate to Dark Iron Highway' WHERE `id` =1828 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Darkshore - Unused, Blackwood Furbolg Camp' WHERE `id` =1906 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackrock Depths, The Black Vault - Unused' WHERE `id` =1926 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackrock Spire, Hordemar City - Near Scarshield Infiltrator' WHERE `id` =1946 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Darkshore, Twilight Shore - Spawn Murkdeep' WHERE `id` =1966 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackrock Spire, Hordemar City - Near Scarshield Infiltrator' WHERE `id` =1986 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackrock Spire, Hordemar City - Near Scarshield Infiltrator' WHERE `id` =1987 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackrock Spire, Blackrock Stadium - Start Encounter' WHERE `id` =2026 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackrock Spire, Hall of Blackhand - Near Dragonspine Door' WHERE `id` =2046 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackrock Spire, The Furnace - Near The Beast' WHERE `id` =2066 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackrock Spire, The Furnace - Unused, Near the Beast' WHERE `id` =2067 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackrock Spire - Jump Exit' WHERE `id` =2068 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Deeprun Tram Ironforge - Exit' WHERE `id` =2166 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Deeprun Tram Stormwind - Exit' WHERE `id` =2171 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Deeprun Tram Stormwind - Entrance' WHERE `id` =2173 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Deeprun Tram Ironforge - Exit' WHERE `id` =2175 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Stratholme, Alonsus Chapel - Unused' WHERE `id` =2187 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Felwood, Shatter Scar Vale - Quest Verifying the Corruption' WHERE `id` =2206 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Felwood, Shatter Scar Vale - Quest Verifying the Corruption' WHERE `id` =2207 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Felwood, Shatter Scar Vale - Quest Verifying the Corruption' WHERE `id` =2208 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Stratholme, Crusaders'' Square - Unused, Entrance to The Scarlet Bastion' WHERE `id` =2209 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Stratholme, Crusaders'' Square - Unused, Entrance to The Scarlet Bastion' WHERE `id` =2210 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Winterspring - Rune of Mazthoril, Outside, Near Haleh' WHERE `id` =2211 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Winterspring - Rune of Mazthoril, Inside, Near Scryer' WHERE `id` =2213 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Stratholme - Back Entrance' WHERE `id` =2214 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Stratholme - Back Entrance' WHERE `id` =2214 AND `build` =4375;

UPDATE `areatrigger_template` SET `name` = 'Stratholme - Right Entrance' WHERE `id` =2216 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Stratholme - Right Entrance' WHERE `id` =2216 AND `build` =4375;

UPDATE `areatrigger_template` SET `name` = 'Stratholme - Left Entrance' WHERE `id` =2217 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Stratholme - Left Entrance' WHERE `id` =2217 AND `build` =4375;

UPDATE `areatrigger_template` SET `name` = 'Stratholme - Left Entrance' WHERE `id` =2217 AND `build` =4449;

UPDATE `areatrigger_template` SET `name` = 'Stratholme - Back Exit' WHERE `id` =2221 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Ragefire Chasm - Exit' WHERE `id` =2226 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Ragefire Chasm - Entrance' WHERE `id` =2230 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Ragefire Chasm - Entrance' WHERE `id` =2230 AND `build` =4375;

UPDATE `areatrigger_template` SET `name` = 'Western Plaguelands, Felstone Field - Unused, Near Scourge Cauldron' WHERE `id` =2246 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Western Plaguelands, Dalson''s Tears - Unused, Near Scourge Cauldron' WHERE `id` =2248 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Western Plaguelands, The Writhing Haunt - Unused, Near Scourge Cauldron' WHERE `id` =2250 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Western Plaguelands, Gahrron''s Withering - Unused, Near Scourge Cauldron' WHERE `id` =2252 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Desolace, Nijel''s Point - Inn' WHERE `id` =2266 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Desolace, Shadowprey Village - Inn' WHERE `id` =2267 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Desolace, Shadowprey Village - Inn' WHERE `id` =2267 AND `build` =4449;

UPDATE `areatrigger_template` SET `name` = 'Desolace, Shadowprey Village - Inn' WHERE `id` =2267 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Thousand Needles, Freewind Post - Inn' WHERE `id` =2286 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Thousand Needles, Freewind Post - Inn' WHERE `id` =2286 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Winterspring, Everlook - Inn' WHERE `id` =2287 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Winterspring, Darkwhisper Gorge - Quest Strange Sources' WHERE `id` =2327 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Swamp of Sorrows - Unused, Near Galen''s Cage' WHERE `id` =2387 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Shadowfang Keep - Left Jump Exit' WHERE `id` =2406 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Shadowfang Keep - Left Jump Exit' WHERE `id` =2407 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Shadowfang Keep - Front Jump Exit' WHERE `id` =2408 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Shadowfang Keep - Front Jump Exit' WHERE `id` =2409 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Shadowfang Keep - Right Jump Exit' WHERE `id` =2410 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Shadowfang Keep - Right Jump Exit' WHERE `id` =2411 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Alterac Mountains - Alterac Valley Alliance Entrance' WHERE `id` =2412 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Alterac Mountains - Alterac Valley Horde Entrance' WHERE `id` =2413 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Alterac Mountains - Alterac Valley Horde Entrance' WHERE `id` =2413 AND `build` =4297;

UPDATE `areatrigger_template` SET `name` = 'Programmer Isle - Unused' WHERE `id` =2416 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackwing Lair, Nefarians Lair - Jump Exit' WHERE `id` =2426 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackwing Lair, Nefarians Lair - Jump Exit' WHERE `id` =2427 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackwing Lair, Nefarians Lair - Jump Exit' WHERE `id` =2428 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackwing Lair, Nefarians Lair - Jump Exit' WHERE `id` =2429 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Darkshore - Quest The Red Crystal' WHERE `id` =2486 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Hall of Legends - Entrance' WHERE `id` =2527 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Hall of Legends - Exit' WHERE `id` =2530 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Champions'' Hall - Entrance' WHERE `id` =2532 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Champions'' Hall - Exit' WHERE `id` =2534 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Scholomance - Balcony Exit' WHERE `id` =2547 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Scholomance - Balcony Exit' WHERE `id` =2548 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Scholomance - Balcony Exit' WHERE `id` =2549 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Scholomance - Entrance' WHERE `id` =2567 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Scholomance - Entrance' WHERE `id` =2567 AND `build` =4375;

UPDATE `areatrigger_template` SET `name` = 'Scholomance - Exit' WHERE `id` =2568 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Alterac Valley - Horde Exit' WHERE `id` =2606 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Alterac Valley - Alliance Exit' WHERE `id` =2608 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Ashenvale, Splintertree Post - Inn' WHERE `id` =2610 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Western Plaguelands, Felstone Field - Unused, Near Scourge Cauldron' WHERE `id` =2626 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Western Plaguelands, Felstone Field - Unused, Near Scourge Cauldron' WHERE `id` =2627 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Western Plaguelands, Felstone Field - Unused, Near Scourge Cauldron' WHERE `id` =2628 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Western Plaguelands, Dalson''s Tears - Unused, Near Scourge Cauldron' WHERE `id` =2629 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Western Plaguelands, Dalson''s Tears - Unused, Near Scourge Cauldron' WHERE `id` =2630 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Western Plaguelands, Dalson''s Tears - Unused, Near Scourge Cauldron' WHERE `id` =2631 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Western Plaguelands, The Writhing Haunt - Unused, Near Scourge Cauldron' WHERE `id` =2632 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Western Plaguelands, The Writhing Haunt - Unused, Near Scourge Cauldron' WHERE `id` =2633 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Western Plaguelands, The Writhing Haunt - Unused, Near Scourge Cauldron' WHERE `id` =2634 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Western Plaguelands, Gahrron''s Withering - Unused, Near Scourge Cauldron' WHERE `id` =2635 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Western Plaguelands, Gahrron''s Withering - Unused, Near Scourge Cauldron' WHERE `id` =2636 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Western Plaguelands, Gahrron''s Withering - Unused, Near Scourge Cauldron' WHERE `id` =2637 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Western Plaguelands, Hearthglen - Unused, Top of Tower' WHERE `id` =2647 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Western Plaguelands, Hearthglen - Unused, Top of Tower' WHERE `id` =2647 AND `build` =4297;

UPDATE `areatrigger_template` SET `name` = 'Eastern Plaguelands, Terrorweb Tunnel - Unused' WHERE `id` =2706 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Eastern Plaguelands, Terrorweb Tunnel - Unused' WHERE `id` =2707 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Eastern Plaguelands, The Marris Stead - Quest The Eastern Plagues' WHERE `id` =2726 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Stormwind City, Trade District - Unused, Inside Tree' WHERE `id` =2746 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackwing Lair, Dragonmaw Garrison - Unused, Near Beginning' WHERE `id` =2766 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackwing Lair, Dragonmaw Garrison - Unused, Near Beginning' WHERE `id` =2767 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Stormwind City - Bridge, Spawn Windsor' WHERE `id` =2786 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Stormwind City - Bridge, Spawn Windsor' WHERE `id` =2786 AND `build` =5875;

UPDATE `areatrigger_template` SET `name` = 'Onyxia''s Lair - Entrance' WHERE `id` =2848 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Molten Core - Entrance' WHERE `id` =2886 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Molten Core - Exit' WHERE `id` =2890 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Ashenvale, Mystral Lake - Quest Stonetalon Standstill' WHERE `id` =2926 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Stonetalon Mountains - Quest Boulderslide Ravine' WHERE `id` =2946 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Hillsbrad Foothills - Quest The Manor, Ravenholdt' WHERE `id` =3066 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Alterac Mountains, Ravenholdt Manor - Unused, Training Area' WHERE `id` =3106 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Maraudon - Purple Exit' WHERE `id` =3126 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Maraudon - Orange Exit' WHERE `id` =3131 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Maraudon - Orange Entrance' WHERE `id` =3133 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Maraudon - Orange Entrance' WHERE `id` =3133 AND `build` =4449;

UPDATE `areatrigger_template` SET `name` = 'Maraudon - Purple Entrance' WHERE `id` =3134 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Maraudon - Purple Entrance' WHERE `id` =3134 AND `build` =4375;

UPDATE `areatrigger_template` SET `name` = 'Silithus, Southwind Village - Unused, Tower Floor' WHERE `id` =3146 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Dire Maul - Unused, Unreachable Cave Tunnel' WHERE `id` =3182 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Dire Maul - Entrance 1' WHERE `id` =3183 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Dire Maul - Entrance 2' WHERE `id` =3184 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Dire Maul - Entrance 2' WHERE `id` =3184 AND `build` =4297;

UPDATE `areatrigger_template` SET `name` = 'Dire Maul - Entrance 2' WHERE `id` =3184 AND `build` =4375;

UPDATE `areatrigger_template` SET `name` = 'Dire Maul - Entrance 3' WHERE `id` =3185 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Dire Maul - Entrance 3' WHERE `id` =3185 AND `build` =4375;

UPDATE `areatrigger_template` SET `name` = 'Dire Maul - Entrance 3' WHERE `id` =3185 AND `build` =4449;

UPDATE `areatrigger_template` SET `name` = 'Dire Maul - Entrance 4' WHERE `id` =3186 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Dire Maul - Entrance 4' WHERE `id` =3186 AND `build` =4297;

UPDATE `areatrigger_template` SET `name` = 'Dire Maul - Entrance 4' WHERE `id` =3186 AND `build` =4375;

UPDATE `areatrigger_template` SET `name` = 'Dire Maul - Entrance 5' WHERE `id` =3187 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Dire Maul - Entrance 5' WHERE `id` =3187 AND `build` =4297;

UPDATE `areatrigger_template` SET `name` = 'Dire Maul - Entrance 5' WHERE `id` =3187 AND `build` =4375;

UPDATE `areatrigger_template` SET `name` = 'Dire Maul - Unused, Copy of Exit in Open World' WHERE `id` =3188 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Dire Maul - Entrance 6' WHERE `id` =3189 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Dire Maul - Exit 4' WHERE `id` =3190 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Dire Maul - Exit 5' WHERE `id` =3191 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Dire Maul - Unused, Copy of Entrance inside Dungeon' WHERE `id` =3192 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Dire Maul - Exit 6' WHERE `id` =3193 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Dire Maul - Exit 1' WHERE `id` =3194 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Dire Maul - Exit 2' WHERE `id` =3195 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Dire Maul - Exit 3' WHERE `id` =3196 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Dire Maul - Tunnel Exit' WHERE `id` =3197 AND `build` =4222;

UPDATE `areatrigger_template` SET `name` = 'Blackwing Lair, Shadow Wing Lair - Unused, Near Vaelastrasz' WHERE `id` =3306 AND `build` =4297;

UPDATE `areatrigger_template` SET `name` = 'Alterac Valley, Tower Point - Unused, Near Slidore' WHERE `id` =3326 AND `build` =4297;

UPDATE `areatrigger_template` SET `name` = 'Alterac Valley, Tower Point - Unused, Near Slidore' WHERE `id` =3326 AND `build` =4449;

UPDATE `areatrigger_template` SET `name` = 'Alterac Valley, Tower Point - Unused, Near Slidore' WHERE `id` =3326 AND `build` =5302;

UPDATE `areatrigger_template` SET `name` = 'Alterac Valley, Frostwolf Village - Unused, Near Vipore' WHERE `id` =3327 AND `build` =4297;

UPDATE `areatrigger_template` SET `name` = 'Alterac Valley, Frostwolf Keep - Unused, Near Ichman' WHERE `id` =3328 AND `build` =4297;

UPDATE `areatrigger_template` SET `name` = 'Alterac Valley, Frostwolf Keep - Unused, Near Ichman' WHERE `id` =3328 AND `build` =5875;

UPDATE `areatrigger_template` SET `name` = 'Alterac Valley, Icewing Bunker - Unused, Near Karl Philips' WHERE `id` =3329 AND `build` =4297;

UPDATE `areatrigger_template` SET `name` = 'Alterac Valley, Icewing Bunker - Unused, Near Karl Philips' WHERE `id` =3329 AND `build` =4449;

UPDATE `areatrigger_template` SET `name` = 'Alterac Valley, Icewing Bunker - Unused, Near Karl Philips' WHERE `id` =3329 AND `build` =4544;

UPDATE `areatrigger_template` SET `name` = 'Alterac Valley - Unused, Near Jeztor' WHERE `id` =3330 AND `build` =4297;

UPDATE `areatrigger_template` SET `name` = 'Alterac Valley - Unused, Near Jeztor' WHERE `id` =3330 AND `build` =4449;

UPDATE `areatrigger_template` SET `name` = 'Alterac Valley - Unused, Near Jeztor' WHERE `id` =3330 AND `build` =4544;

UPDATE `areatrigger_template` SET `name` = 'Alterac Valley, Dun Baldar - Unused, North Bunker' WHERE `id` =3331 AND `build` =4297;

UPDATE `areatrigger_template` SET `name` = 'Alterac Valley, Dun Baldar - Unused, North Bunker' WHERE `id` =3331 AND `build` =4449;

UPDATE `areatrigger_template` SET `name` = 'Alterac Valley, Dun Baldar - Unused, North Bunker' WHERE `id` =3331 AND `build` =4544;

UPDATE `areatrigger_template` SET `name` = 'Western Plaguelands, Hearthglen - Quest Unfinished Business' WHERE `id` =3366 AND `build` =4297;

UPDATE `areatrigger_template` SET `name` = 'Western Plaguelands, Hearthglen - Quest Unfinished Business' WHERE `id` =3367 AND `build` =4297;

INSERT INTO `areatrigger_involvedrelation` (`id`, `quest`) VALUES (3367, 6025);

UPDATE `areatrigger_template` SET `name` = 'Dire Maul, Halls of Destruction - Unused, Near Captain Kromcrush' WHERE `id` =3506 AND `build` =4297;

UPDATE `areatrigger_template` SET `name` = 'Dire Maul, Halls of Destruction - Unused, Near Captain Kromcrush' WHERE `id` =3507 AND `build` =4297;

UPDATE `areatrigger_template` SET `name` = 'Dire Maul, Halls of Destruction - Unused, Near Captain Kromcrush' WHERE `id` =3508 AND `build` =4297;

UPDATE `areatrigger_template` SET `name` = 'Dire Maul, Halls of Destruction - Unused, Near Captain Kromcrush' WHERE `id` =3509 AND `build` =4297;

UPDATE `areatrigger_template` SET `name` = 'Molten Core - Window Entrance' WHERE `id` =3528 AND `build` =4297;

UPDATE `areatrigger_template` SET `name` = 'Molten Core - Window Entrance' WHERE `id` =3528 AND `build` =4375;

UPDATE `areatrigger_template` SET `name` = 'Molten Core - Window Entrance' WHERE `id` =3528 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Molten Core - Window Lava Entrance' WHERE `id` =3529 AND `build` =4297;

UPDATE `areatrigger_template` SET `name` = 'Stormwind City - Unused, Stormwind Keep Entrance' WHERE `id` =3530 AND `build` =4297;

UPDATE `areatrigger_template` SET `name` = 'Darnassus - Quest The Bough of the Eternals, Children''s Week' WHERE `id` =3546 AND `build` =4375;

UPDATE `areatrigger_template` SET `name` = 'Undercity - Quest Lordaeron Throne Room, Children''s Week' WHERE `id` =3547 AND `build` =4375;

UPDATE `areatrigger_template` SET `name` = 'Loch Modan - Quest The Stonewrought Dam, Children''s Week' WHERE `id` =3548 AND `build` =4375;

UPDATE `areatrigger_template` SET `name` = 'The Barrens - Quest Gateway to the Frontier, Children''s Week' WHERE `id` =3549 AND `build` =4375;

UPDATE `areatrigger_template` SET `name` = 'The Barrens - Quest Down at the Docks, Children''s Week' WHERE `id` =3550 AND `build` =4375;

UPDATE `areatrigger_template` SET `name` = 'The Barrens, Ratchet - Unused, Docks' WHERE `id` =3551 AND `build` =4375;

UPDATE `areatrigger_template` SET `name` = 'Westfall - Quest Spooky Lighthouse, Children''s Week' WHERE `id` =3552 AND `build` =4375;

UPDATE `areatrigger_template` SET `name` = 'Felwood, Irontree Woods - Quest The Ancient Leaf' WHERE `id` =3586 AND `build` =4375;

UPDATE `areatrigger_template` SET `name` = 'Felwood, Irontree Woods - Quest The Ancient Leaf' WHERE `id` =3587 AND `build` =4375;

UPDATE `areatrigger_template` SET `name` = 'Blackrock Mountain, The Molten Span - Unused, Lava Floor' WHERE `id` =3606 AND `build` =4375;

UPDATE `areatrigger_template` SET `name` = 'Blackwing Lair, Shadow Wing Lair - Start Vaelastrasz Encounter' WHERE `id` =3626 AND `build` =4375;

UPDATE `areatrigger_template` SET `name` = 'Warsong Gulch, Silverwing Hold - Alliance Flag Capture Point' WHERE `id` =3646 AND `build` =4449;

UPDATE `areatrigger_template` SET `name` = 'Warsong Gulch, Silverwing Hold - Alliance Flag Capture Point' WHERE `id` =3646 AND `build` =4544;

UPDATE `areatrigger_template` SET `name` = 'Warsong Gulch, Silverwing Hold - Alliance Flag Capture Point' WHERE `id` =3646 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Warsong Gulch, Warsong Lumber Mill - Horde Flag Capture Point' WHERE `id` =3647 AND `build` =4449;

UPDATE `areatrigger_template` SET `name` = 'Warsong Gulch, Warsong Lumber Mill - Horde Flag Capture Point' WHERE `id` =3647 AND `build` =4544;

UPDATE `areatrigger_template` SET `name` = 'Warsong Gulch, Warsong Lumber Mill - Horde Flag Capture Point' WHERE `id` =3647 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Warsong Gulch, Warsong Lumber Mill - Horde Flag Capture Point' WHERE `id` =3647 AND `build` =5875;

UPDATE `areatrigger_template` SET `name` = 'Warsong Gulch - Unused, Under Terrain' WHERE `id` =3649 AND `build` =4449;

UPDATE `areatrigger_template` SET `name` = 'Ashenvale - Warsong Gulch Alliance Entrance' WHERE `id` =3650 AND `build` =4449;

UPDATE `areatrigger_template` SET `name` = 'The Barrens - Warsong Gulch Horde Entrance' WHERE `id` =3654 AND `build` =4449;

UPDATE `areatrigger_template` SET `name` = 'The Barrens - Warsong Gulch Horde Entrance' WHERE `id` =3654 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'The Barrens - Warsong Gulch Horde Entrance' WHERE `id` =3654 AND `build` =4878;

UPDATE `areatrigger_template` SET `name` = 'Warsong Gulch - Horde Exit' WHERE `id` =3669 AND `build` =4449;

UPDATE `areatrigger_template` SET `name` = 'Warsong Gulch - Alliance Exit' WHERE `id` =3671 AND `build` =4449;

UPDATE `areatrigger_template` SET `name` = 'Warsong Gulch, Alliance Tunnel - Speed Buff' WHERE `id` =3686 AND `build` =4449;

UPDATE `areatrigger_template` SET `name` = 'Warsong Gulch, Horde Tunnel - Speed Buff' WHERE `id` =3687 AND `build` =4449;

UPDATE `areatrigger_template` SET `name` = 'Warsong Gulch - Unused, Outside Horde Damage Buff' WHERE `id` =3688 AND `build` =4449;

UPDATE `areatrigger_template` SET `name` = 'The Hinterlands, Revantusk Village - Inn' WHERE `id` =3690 AND `build` =4449;

UPDATE `areatrigger_template` SET `name` = 'Warsong Gulch - Alliance Heal Buff' WHERE `id` =3706 AND `build` =4449;

UPDATE `areatrigger_template` SET `name` = 'Warsong Gulch - Alliance Damage Buff' WHERE `id` =3707 AND `build` =4449;

UPDATE `areatrigger_template` SET `name` = 'Warsong Gulch - Horde Heal Buff' WHERE `id` =3708 AND `build` =4449;

UPDATE `areatrigger_template` SET `name` = 'Warsong Gulch - Horde Heal Buff' WHERE `id` =3708 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Warsong Gulch - Horde Heal Buff' WHERE `id` =3708 AND `build` =4878;

UPDATE `areatrigger_template` SET `name` = 'Warsong Gulch - Horde Damage Buff' WHERE `id` =3709 AND `build` =4449;

UPDATE `areatrigger_template` SET `name` = 'Blackwing Lair - Entrance' WHERE `id` =3726 AND `build` =4544;

UPDATE `areatrigger_template` SET `name` = 'Blackwing Lair - Exit' WHERE `id` =3728 AND `build` =4544;

UPDATE `areatrigger_template` SET `name` = 'The Deadmines, Near Beginning - Quest Fortune Awaits, Spawn Chest' WHERE `id` =3746 AND `build` =4544;

INSERT INTO `scripted_areatrigger` (`entry`, `script_name`) VALUES (3746, 'at_dmf_chest_dm');

UPDATE `areatrigger_template` SET `name` = 'Wailing Caverns, Near Beginning - Quest Fortune Awaits, Spawn Chest' WHERE `id` =3766 AND `build` =4544;

INSERT INTO `scripted_areatrigger` (`entry`, `script_name`) VALUES (3766, 'at_dmf_chest_wc');

UPDATE `areatrigger_template` SET `name` = 'Arathi Basin, Stables - Unused' WHERE `id` =3806 AND `build` =4544;

UPDATE `areatrigger_template` SET `name` = 'Arathi Basin, Stables - Unused' WHERE `id` =3807 AND `build` =4544;

UPDATE `areatrigger_template` SET `name` = 'Arathi Basin, Blacksmith - Unused' WHERE `id` =3808 AND `build` =4544;

UPDATE `areatrigger_template` SET `name` = 'Arathi Basin, Blacksmith - Unused' WHERE `id` =3809 AND `build` =4544;

UPDATE `areatrigger_template` SET `name` = 'Arathi Basin, Farm - Unused' WHERE `id` =3810 AND `build` =4544;

UPDATE `areatrigger_template` SET `name` = 'Arathi Basin, Farm - Unused' WHERE `id` =3811 AND `build` =4544;

UPDATE `areatrigger_template` SET `name` = 'Arathi Basin, Lumber Mill - Unused' WHERE `id` =3812 AND `build` =4544;

UPDATE `areatrigger_template` SET `name` = 'Arathi Basin, Lumber Mill - Unused' WHERE `id` =3813 AND `build` =4544;

UPDATE `areatrigger_template` SET `name` = 'Arathi Basin, Gold Mine - Unused' WHERE `id` =3814 AND `build` =4544;

UPDATE `areatrigger_template` SET `name` = 'Arathi Basin, Gold Mine - Unused' WHERE `id` =3815 AND `build` =4544;

UPDATE `areatrigger_template` SET `name` = 'Blackrock Mountain, Blackrock Spire - Near Orb of Command' WHERE `id` =3846 AND `build` =4544;

UPDATE `areatrigger_template` SET `name` = 'Blackrock Mountain, Blackrock Spire - Near Orb of Command' WHERE `id` =3847 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Arathi Basin, Stables - Random Buff' WHERE `id` =3866 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Arathi Basin, Farm - Random Buff' WHERE `id` =3867 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Arathi Basin, Lumber Mill - Random Buff' WHERE `id` =3868 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Arathi Basin, Gold Mine - Random Buff' WHERE `id` =3869 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Arathi Basin, Blacksmith - Random Buff' WHERE `id` =3870 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Stranglethorn Vale, Grom''gol Base Camp - Inn' WHERE `id` =3886 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Zul''Gurub, Temple of Bethekk - Unused, Outside Before Stairs' WHERE `id` =3926 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Zul''Gurub - Entrance' WHERE `id` =3928 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Zul''Gurub - Exit' WHERE `id` =3930 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Arathi Basin - Alliance Exit' WHERE `id` =3948 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Arathi Basin - Horde Exit' WHERE `id` =3949 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Arathi Highlands - Arathi Basin Alliance Entrance' WHERE `id` =3953 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Arathi Highlands - Arathi Basin Horde Entrance' WHERE `id` =3954 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Zul''Gurub - Unused, Before First Bridge' WHERE `id` =3956 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Zul''Gurub - Unused, Near Beginning, Up the Stairs' WHERE `id` =3957 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Zul''Gurub - Unused, Near Beginning, Down the Stairs' WHERE `id` =3958 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Zul''Gurub - Unused, On Second Bridge' WHERE `id` =3959 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Zul''Gurub, Altar of the Blood God - Unused, Up the Stairs' WHERE `id` =3960 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Zul''Gurub, The Bloodfire Pit - Unused, Entrance to Area' WHERE `id` =3961 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Zul''Gurub, Edge of Madness - Unused, Entrance to Area' WHERE `id` =3962 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Zul''Gurub, Hakkari Grounds - Unused, Entrance to Area' WHERE `id` =3963 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Zul''Gurub, Altar of the Blood God - Unused, Down the Stairs' WHERE `id` =3964 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Zul''Gurub, Pagle''s Pointe - Unused, Entrance to Area' WHERE `id` =3965 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Zul''Gurub, Altar of Hir''eek - Unused, Entrance to Area' WHERE `id` =3966 AND `build` =4695;

UPDATE `areatrigger_template` SET `name` = 'Silithus, Cenarion Hold - Inn' WHERE `id` =3985 AND `build` =4878;

UPDATE `areatrigger_template` SET `name` = 'Tanaris, Caverns of Time - Quest What Tomorrow Brings' WHERE `id` =3986 AND `build` =4878;

UPDATE `areatrigger_template` SET `name` = 'Tirisfal Glades, Ruins of Lordaeron - Quest Crashing the Wickerman Festival' WHERE `id` =3991 AND `build` =4878;

UPDATE `areatrigger_template` SET `name` = 'Ruins Of Ahn''Qiraj - Exit' WHERE `id` =4006 AND `build` =5086;

UPDATE `areatrigger_template` SET `name` = 'Ruins Of Ahn''Qiraj - Entrance' WHERE `id` =4008 AND `build` =5086;

UPDATE `areatrigger_template` SET `name` = 'Temple of Ahn''Qiraj - Entrance' WHERE `id` =4010 AND `build` =5086;

UPDATE `areatrigger_template` SET `name` = 'Temple of Ahn''Qiraj - Exit' WHERE `id` =4012 AND `build` =5086;

UPDATE `areatrigger_template` SET `name` = 'Winterspring, Frostwhisper Gorge - Quest Decoy!' WHERE `id` =4015 AND `build` =5086;

UPDATE `areatrigger_template` SET `name` = 'The Temple of Atal''Hakkar, Chamber of the Dreamer - Spawn Malfurion' WHERE `id` =4016 AND `build` =5086;

UPDATE `areatrigger_template` SET `name` = 'Duskwood, Twilight Grove - Quest The Nightmare''s Corruption, Spawn Twilight Corrupter' WHERE `id` =4017 AND `build` =5086;

UPDATE `areatrigger_template` SET `name` = 'Arathi Basin, Trollbane Hall - Alliance Spawn Point' WHERE `id` =4020 AND `build` =5086;

UPDATE `areatrigger_template` SET `name` = 'Arathi Basin, Defiler''s Den - Horde Spawn Point' WHERE `id` =4021 AND `build` =5086;

UPDATE `areatrigger_template` SET `name` = 'Dun Morogh, Gates of Ironforge - PX-238 Winter Wondervolt, Winter Veil' WHERE `id` =4026 AND `build` =4878;

UPDATE `areatrigger_template` SET `name` = 'Durotar, Jaggedswine Farm - PX-238 Winter Wondervolt, Winter Veil' WHERE `id` =4027 AND `build` =4878;

UPDATE `areatrigger_template` SET `name` = 'Tirisfal Glades, Zeppelin Tower - PX-238 Winter Wondervolt, Winter Veil' WHERE `id` =4028 AND `build` =4878;

UPDATE `areatrigger_template` SET `name` = 'Stranglethorn Vale, Booty Bay - PX-238 Winter Wondervolt, Winter Veil' WHERE `id` =4029 AND `build` =4878;

UPDATE `areatrigger_template` SET `name` = 'Wetlands, Menethil Harbor - PX-238 Winter Wondervolt, Winter Veil' WHERE `id` =4030 AND `build` =4878;

UPDATE `areatrigger_template` SET `name` = 'Tanaris, Gadgetzan - PX-238 Winter Wondervolt, Winter Veil' WHERE `id` =4031 AND `build` =4878;

UPDATE `areatrigger_template` SET `name` = 'Stormwind City - PX-238 Winter Wondervolt, Winter Veil' WHERE `id` =4032 AND `build` =4878;

UPDATE `areatrigger_template` SET `name` = 'Ahn''Qiraj - C''thun Stomach Ground' WHERE `id` =4033 AND `build` =5086;

UPDATE `areatrigger_template` SET `name` = 'Ahn''Qiraj - C''thun Stomach Air' WHERE `id` =4034 AND `build` =5086;

UPDATE `areatrigger_template` SET `name` = 'Ahn''Qiraj - C''thun Knockback' WHERE `id` =4036 AND `build` =5086;

UPDATE `areatrigger_template` SET `name` = 'Ahn''Qiraj - Near Twin Emperors' WHERE `id` =4047 AND `build` =5086;

UPDATE `areatrigger_template` SET `name` = 'Ahn''Qiraj - Near Sartura' WHERE `id` =4052 AND `build` =5086;

UPDATE `areatrigger_template` SET `name` = 'Naxxramas - Entrance' WHERE `id` =4055 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Naxxramas - Entrance' WHERE `id` =4055 AND `build` =5875;

UPDATE `areatrigger_template` SET `name` = 'Naxxramas - Entrance' WHERE `id` =4057 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Eastern Plaguelands, Light''s Hope Chapel - Inn' WHERE `id` =4058 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Winterspring, Frostsaber Rock - Unused, Near Road' WHERE `id` =4085 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Naxxramas, Construct Quarter - Unused, Near Gluth' WHERE `id` =4086 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Naxxramas, Construct Quarter - Unused, Near Gluth' WHERE `id` =4087 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Naxxramas, Construct Quarter - Unused, After Door to Gluth' WHERE `id` =4088 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Scarlet Monastery, Cathedral - Start Ashbringer Event' WHERE `id` =4089 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Stonetalon Mountains, Stonetalon Peak - Inn' WHERE `id` =4090 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Stonetalon Mountains, Stonetalon Peak - Inn' WHERE `id` =4090 AND `build` =5875;

UPDATE `areatrigger_template` SET `name` = 'Elwynn Forest - Quest Investigate the Scourge of Stormwind' WHERE `id` =4092 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Elwynn Forest - Quest Investigate the Scourge of Stormwind' WHERE `id` =4094 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Elwynn Forest - Quest Investigate the Scourge of Stormwind' WHERE `id` =4095 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Elwynn Forest - Quest Investigate the Scourge of Stormwind' WHERE `id` =4096 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Naxxramas, Construct Quarter - Unused, After Door to Gluth' WHERE `id` =4097 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Dun Morogh - Quest Investigate the Scourge of Ironforge' WHERE `id` =4098 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Dun Morogh - Quest Investigate the Scourge of Ironforge' WHERE `id` =4099 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Tirisfal Glades - Quest Investigate the Scourge of the Undercity' WHERE `id` =4100 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Durotar - Quest Investigate the Scourge of Orgrimmar' WHERE `id` =4101 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Durotar - Quest Investigate the Scourge of Orgrimmar' WHERE `id` =4102 AND `build` =5464;

INSERT INTO `areatrigger_involvedrelation` (`id`, `quest`) VALUES (4102, 9263);

UPDATE `areatrigger_template` SET `name` = 'Mulgore - Quest Investigate the Scourge of Thunder Bluff' WHERE `id` =4103 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Teldrassil - Quest Investigate the Scourge of Darnassus' WHERE `id` =4104 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Teldrassil - Quest Investigate the Scourge of Darnassus' WHERE `id` =4105 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Naxxramas, Kel''Thuzad''s Chamber - Start Encounter' WHERE `id` =4112 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Naxxramas, Construct Quarter - Near Thaddius' WHERE `id` =4113 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Naxxramas, Construct Quarter - Near Thaddius' WHERE `id` =4114 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Naxxramas, Arachnid Quarter - Near Faerlina' WHERE `id` =4115 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Naxxramas, Military Quarter - Near Gothik' WHERE `id` =4116 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Naxxramas, Military Quarter - Entrance to Wing' WHERE `id` =4117 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Naxxramas, Arachnid Quarter - Naer Anub''Rekhan' WHERE `id` =4119 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Naxxramas, Sapphiron''s Lair - Entrance to Wing, Air' WHERE `id` =4120 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Naxxramas - Teleport to Frostwyrm Lair' WHERE `id` =4156 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Naxxramas - Teleport to Frostwyrm Lair' WHERE `id` =4156 AND `build` =5875;

UPDATE `areatrigger_template` SET `name` = 'Naxxramas, Kel''Thuzad''s Chamber - Unused, Behind Boss' WHERE `id` =4158 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Silithus, Hive''Zora - Alliance Silithyst Capture Point' WHERE `id` =4162 AND `build` =5875;

UPDATE `areatrigger_template` SET `name` = 'Naxxramas, Construct Quarter - Unused, After Door to Gluth' WHERE `id` =4163 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Naxxramas, Construct Quarter - Unused, Between Gluth and Thaddius' WHERE `id` =4166 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Naxxramas, Sapphiron''s Lair - Entrance to Wing, Ground' WHERE `id` =4167 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Silithus, Hive''Regal - Horde Silithyst Capture Point' WHERE `id` =4168 AND `build` =5875;

UPDATE `areatrigger_template` SET `name` = 'Naxxramas, Plague Quarter - Unused, Near Noth' WHERE `id` =4177 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Naxxramas, Construct Quarter - Unused, Between Gluth and Thaddius' WHERE `id` =4180 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Naxxramas, Plague Quarter - Unused, Near Heigan' WHERE `id` =4184 AND `build` =5464;

UPDATE `areatrigger_template` SET `name` = 'Naxxramas, Plague Quarter - Unused, Near Heigan' WHERE `id` =4184 AND `build` =5875;

UPDATE `areatrigger_template` SET `name` = 'Undercity, Magic Quarter - Unused, Near Warlock Summons' WHERE `id` =4189 AND `build` =5875;

UPDATE `areatrigger_template` SET `name` = 'Scarlet Monastery, Graveyard - Unused, Near Beginning' WHERE `id` =4261 AND `build` =5875;

UPDATE `areatrigger_template` SET `name` = 'Scarlet Monastery, Armory - Unused, Near Beginning' WHERE `id` =4262 AND `build` =5875;

UPDATE `areatrigger_template` SET `name` = 'Scarlet Monastery, Libary - Unused, Near Beginning' WHERE `id` =4263 AND `build` =5875;

UPDATE `areatrigger_template` SET `name` = 'Eastern Plaguelands, Eastwall Tower - Unused' WHERE `id` =4294 AND `build` =5875;

UPDATE `areatrigger_template` SET `name` = 'Silithus, Southwind Village - Unused' WHERE `id` =4299 AND `build` =5875;

-- End of migration.
;
