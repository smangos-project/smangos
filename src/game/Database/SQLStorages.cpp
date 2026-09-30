/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2009-2011 MaNGOSZero <https://github.com/mangos/zero>
 * Copyright (C) 2011-2016 Nostalrius <https://nostalrius.org>
 * Copyright (C) 2016-2017 Elysium Project <https://github.com/elysium-project>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "SQLStorages.h"
#include "Database/SQLStorage.h"
#include "Database/SQLStorageImpl.h"
#include "Database/DatabaseEnv.h"

char const CreatureDataAddonInfofmt[] = "iiiibbis";
char const CreatureDisplayInfoAddonfmt[] = "iffffbi";
char const GameObjectDisplayInfoAddonfmt[] = "iffffff";
char const PageTextfmt[] = "isi";
char const MapEntrysrcfmt[] = "iiiiiiiffss";
char const MapEntrydstfmt[] = "iiiiiiiffsi";
char const ConditionsSrcFmt[] = "iiiiiii";
char const ConditionsDstFmt[] = "iiiiiii";
char const AreaEntryfmt[] = "iiiiiisii";
char const MailTemplatefmt[] = "issssssss";
char const CreatureSpellDatafmt[] = "iiiii";

SQLStorage sCreatureDataAddonStorage(CreatureDataAddonInfofmt, "guid", "creature_addon");
SQLStorage sCreatureDisplayInfoAddonStorage(CreatureDisplayInfoAddonfmt, "display_id", "(SELECT a.`display_id`, a.`bounding_radius`, a.`combat_reach`, a.`speed_walk`, a.`speed_run`, d.`Gender` AS `gender`, a.`display_id_other_gender` FROM `creature_display_info_addon` a JOIN `dbc_creaturedisplayinfo` d ON d.`ID` = a.`display_id`)");
SQLStorage sGameObjectDisplayInfoAddonStorage(GameObjectDisplayInfoAddonfmt, "display_id", "(SELECT `ID` AS `display_id`, `GeoBox1` AS `min_x`, `GeoBox2` AS `min_y`, `GeoBox3` AS `min_z`, `GeoBox4` AS `max_x`, `GeoBox5` AS `max_y`, `GeoBox6` AS `max_z` FROM `dbc_gameobjectdisplayinfo`)");
SQLStorage sPageTextStore(PageTextfmt, "entry", "page_text");
SQLStorage sMapStorage(MapEntrysrcfmt, MapEntrydstfmt, "entry", "(SELECT m.`entry`, CASE WHEN d.`ParentMapID` = -1 THEN 0 ELSE d.`ParentMapID` END AS `parent`, d.`InstanceType` AS `map_type`, m.`linked_zone`, d.`MaxPlayers` AS `player_limit`, m.`reset_delay`, d.`CorpseMapID` AS `ghost_entrance_map`, m.`ghost_entrance_x`, m.`ghost_entrance_y`, d.`MapName_lang` AS `map_name`, m.`script_name` FROM `map_template` m JOIN `dbc_map` d ON d.`ID` = m.`entry`)");
SQLStorage sConditionStorage(ConditionsSrcFmt, ConditionsDstFmt, "condition_entry", "conditions");
SQLStorage sAreaStorage(AreaEntryfmt, "entry", "(SELECT `ID` AS `entry`, `ContinentID` AS `map_id`, `ParentAreaID` AS `zone_id`, `AreaBit` AS `explore_flag`, `Flags1` & ~1073741824 AS `flags`, `ExplorationLevel` AS `area_level`, `AreaName_lang` AS `name`, `FactionGroupMask` AS `team`, `LiquidTypeID4` AS `liquid_type` FROM `dbc_areatable`)");
SQLStorage sMailTemplateStorage(MailTemplatefmt, "entry", "(SELECT d.`ID` AS `entry`, d.`Body_lang` AS `content_default`, IFNULL(l.`content_loc1`, '') AS `content_loc1`, IFNULL(l.`content_loc2`, '') AS `content_loc2`, IFNULL(l.`content_loc3`, '') AS `content_loc3`, IFNULL(l.`content_loc4`, '') AS `content_loc4`, IFNULL(l.`content_loc5`, '') AS `content_loc5`, IFNULL(l.`content_loc6`, '') AS `content_loc6`, IFNULL(l.`content_loc7`, '') AS `content_loc7` FROM `dbc_mailtemplate` d LEFT JOIN `mail_text_template` l ON l.`entry` = d.`ID`)");
SQLStorage sCreatureSpellDataStorage(CreatureSpellDatafmt, "entry", "(SELECT `ID` AS `entry`, `Spells1`, `Spells2`, `Spells3`, `Spells4` FROM `dbc_creaturespelldata`)");
