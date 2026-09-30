-- key: entry
-- expect: allowable_class = CASE WHEN allowable_class = 2047 THEN -1 ELSE allowable_class END
-- expect: allowable_race = CASE WHEN allowable_race = 255 THEN -1 ELSE allowable_race END
-- expect: spellcooldown_1 = CASE WHEN spellid_1 = 0 THEN -1 ELSE spellcooldown_1 END
-- expect: spellcategorycooldown_1 = CASE WHEN spellid_1 = 0 THEN -1 ELSE spellcategorycooldown_1 END
-- expect: spelltrigger_1 = CASE WHEN spellid_1 = 0 THEN 0 ELSE spelltrigger_1 END
-- expect: spellcooldown_2 = CASE WHEN spellid_2 = 0 THEN -1 ELSE spellcooldown_2 END
-- expect: spellcategorycooldown_2 = CASE WHEN spellid_2 = 0 THEN -1 ELSE spellcategorycooldown_2 END
-- expect: spelltrigger_2 = CASE WHEN spellid_2 = 0 THEN 0 ELSE spelltrigger_2 END
-- expect: spellcooldown_3 = CASE WHEN spellid_3 = 0 THEN -1 ELSE spellcooldown_3 END
-- expect: spellcategorycooldown_3 = CASE WHEN spellid_3 = 0 THEN -1 ELSE spellcategorycooldown_3 END
-- expect: spelltrigger_3 = CASE WHEN spellid_3 = 0 THEN 0 ELSE spelltrigger_3 END
-- expect: spellcooldown_4 = CASE WHEN spellid_4 = 0 THEN -1 ELSE spellcooldown_4 END
-- expect: spellcategorycooldown_4 = CASE WHEN spellid_4 = 0 THEN -1 ELSE spellcategorycooldown_4 END
-- expect: spelltrigger_4 = CASE WHEN spellid_4 = 0 THEN 0 ELSE spelltrigger_4 END
-- expect: spellcooldown_5 = CASE WHEN spellid_5 = 0 THEN -1 ELSE spellcooldown_5 END
-- expect: spellcategorycooldown_5 = CASE WHEN spellid_5 = 0 THEN -1 ELSE spellcategorycooldown_5 END
-- expect: spelltrigger_5 = CASE WHEN spellid_5 = 0 THEN 0 ELSE spelltrigger_5 END
SELECT
s.ID AS "entry",
       i.ClassID AS "class",
       i.SubclassID AS "subclass",
       s.OverallQualityID AS "quality",
       s.Flags1 AS "flags",
       s.VendorStackCount AS "buy_count",
       s.BuyPrice AS "buy_price",
       s.SellPrice AS "sell_price",
       s.InventoryType AS "inventory_type",
       s.AllowableClass AS "allowable_class",
       s.AllowableRace AS "allowable_race",
       s.ItemLevel AS "item_level",
       s.RequiredLevel AS "required_level",
       s.RequiredSkill AS "required_skill",
       s.RequiredSkillRank AS "required_skill_rank",
       s.RequiredAbility AS "required_spell",
       s.RequiredPVPRank AS "required_honor_rank",
       s.RequiredPVPMedal AS "required_city_rank",
       s.MinFactionID AS "required_reputation_faction",
       s.MinReputation AS "required_reputation_rank",
       s.MaxCount AS "max_count",
       s.Stackable AS "stackable",
       s.ContainerSlots AS "container_slots",
       CASE WHEN s.StatModifier_bonusStat1 = -1 THEN 0 ELSE s.StatModifier_bonusStat1 END AS "stat_type1",
       s.StatModifier_bonusAmount1 AS "stat_value1",
       CASE WHEN s.StatModifier_bonusStat2 = -1 THEN 0 ELSE s.StatModifier_bonusStat2 END AS "stat_type2",
       s.StatModifier_bonusAmount2 AS "stat_value2",
       CASE WHEN s.StatModifier_bonusStat3 = -1 THEN 0 ELSE s.StatModifier_bonusStat3 END AS "stat_type3",
       s.StatModifier_bonusAmount3 AS "stat_value3",
       CASE WHEN s.StatModifier_bonusStat4 = -1 THEN 0 ELSE s.StatModifier_bonusStat4 END AS "stat_type4",
       s.StatModifier_bonusAmount4 AS "stat_value4",
       CASE WHEN s.StatModifier_bonusStat5 = -1 THEN 0 ELSE s.StatModifier_bonusStat5 END AS "stat_type5",
       s.StatModifier_bonusAmount5 AS "stat_value5",
       CASE WHEN s.StatModifier_bonusStat6 = -1 THEN 0 ELSE s.StatModifier_bonusStat6 END AS "stat_type6",
       s.StatModifier_bonusAmount6 AS "stat_value6",
       CASE WHEN s.StatModifier_bonusStat7 = -1 THEN 0 ELSE s.StatModifier_bonusStat7 END AS "stat_type7",
       s.StatModifier_bonusAmount7 AS "stat_value7",
       CASE WHEN s.StatModifier_bonusStat8 = -1 THEN 0 ELSE s.StatModifier_bonusStat8 END AS "stat_type8",
       s.StatModifier_bonusAmount8 AS "stat_value8",
       CASE WHEN s.StatModifier_bonusStat9 = -1 THEN 0 ELSE s.StatModifier_bonusStat9 END AS "stat_type9",
       s.StatModifier_bonusAmount9 AS "stat_value9",
       CASE WHEN s.StatModifier_bonusStat10 = -1 THEN 0 ELSE s.StatModifier_bonusStat10 END AS "stat_type10",
       s.StatModifier_bonusAmount10 AS "stat_value10",
       s.ItemDelay AS "delay",
       CAST(printf('%.6g', s.ItemRange) AS REAL) AS "range_mod",
       s.AmmunitionType AS "ammo_type",
       CAST(printf('%.6g', s.MinDamage1) AS REAL) AS "dmg_min1",
       CAST(printf('%.6g', s.MaxDamage1) AS REAL) AS "dmg_max1",
       i.DamageType1 AS "dmg_type1",
       CAST(printf('%.6g', s.MinDamage2) AS REAL) AS "dmg_min2",
       CAST(printf('%.6g', s.MaxDamage2) AS REAL) AS "dmg_max2",
       i.DamageType2 AS "dmg_type2",
       CAST(printf('%.6g', s.MinDamage3) AS REAL) AS "dmg_min3",
       CAST(printf('%.6g', s.MaxDamage3) AS REAL) AS "dmg_max3",
       i.DamageType3 AS "dmg_type3",
       CAST(printf('%.6g', s.MinDamage4) AS REAL) AS "dmg_min4",
       CAST(printf('%.6g', s.MaxDamage4) AS REAL) AS "dmg_max4",
       i.DamageType4 AS "dmg_type4",
       CAST(printf('%.6g', s.MinDamage5) AS REAL) AS "dmg_min5",
       CAST(printf('%.6g', s.MaxDamage5) AS REAL) AS "dmg_max5",
       i.DamageType5 AS "dmg_type5",
       COALESCE(x.block, 0) AS "block",
       s.Resistances1 AS "armor",
       s.Resistances2 AS "holy_res",
       s.Resistances3 AS "fire_res",
       s.Resistances4 AS "nature_res",
       s.Resistances5 AS "frost_res",
       s.Resistances6 AS "shadow_res",
       s.Resistances7 AS "arcane_res",
       COALESCE(e1.SpellID, 0) AS "spellid_1",
       COALESCE(e1.TriggerType, 0) AS "spelltrigger_1",
       COALESCE(CASE WHEN e1.Charges > 2147483647 THEN e1.Charges - 4294967296 ELSE e1.Charges END, 0) AS "spellcharges_1",
       COALESCE(x.spellppmrate_1, 0) AS "spellppmrate_1",
       COALESCE(CASE WHEN e1.CoolDownMSec > 2147483647 THEN e1.CoolDownMSec - 4294967296 ELSE e1.CoolDownMSec END, -1) AS "spellcooldown_1",
       COALESCE(e1.SpellCategoryID, 0) AS "spellcategory_1",
       COALESCE(CASE WHEN e1.CategoryCoolDownMSec > 2147483647 THEN e1.CategoryCoolDownMSec - 4294967296 ELSE e1.CategoryCoolDownMSec END, -1) AS "spellcategorycooldown_1",
       COALESCE(e2.SpellID, 0) AS "spellid_2",
       COALESCE(e2.TriggerType, 0) AS "spelltrigger_2",
       COALESCE(CASE WHEN e2.Charges > 2147483647 THEN e2.Charges - 4294967296 ELSE e2.Charges END, 0) AS "spellcharges_2",
       COALESCE(x.spellppmrate_2, 0) AS "spellppmrate_2",
       COALESCE(CASE WHEN e2.CoolDownMSec > 2147483647 THEN e2.CoolDownMSec - 4294967296 ELSE e2.CoolDownMSec END, -1) AS "spellcooldown_2",
       COALESCE(e2.SpellCategoryID, 0) AS "spellcategory_2",
       COALESCE(CASE WHEN e2.CategoryCoolDownMSec > 2147483647 THEN e2.CategoryCoolDownMSec - 4294967296 ELSE e2.CategoryCoolDownMSec END, -1) AS "spellcategorycooldown_2",
       COALESCE(e3.SpellID, 0) AS "spellid_3",
       COALESCE(e3.TriggerType, 0) AS "spelltrigger_3",
       COALESCE(CASE WHEN e3.Charges > 2147483647 THEN e3.Charges - 4294967296 ELSE e3.Charges END, 0) AS "spellcharges_3",
       COALESCE(x.spellppmrate_3, 0) AS "spellppmrate_3",
       COALESCE(CASE WHEN e3.CoolDownMSec > 2147483647 THEN e3.CoolDownMSec - 4294967296 ELSE e3.CoolDownMSec END, -1) AS "spellcooldown_3",
       COALESCE(e3.SpellCategoryID, 0) AS "spellcategory_3",
       COALESCE(CASE WHEN e3.CategoryCoolDownMSec > 2147483647 THEN e3.CategoryCoolDownMSec - 4294967296 ELSE e3.CategoryCoolDownMSec END, -1) AS "spellcategorycooldown_3",
       COALESCE(e4.SpellID, 0) AS "spellid_4",
       COALESCE(e4.TriggerType, 0) AS "spelltrigger_4",
       COALESCE(CASE WHEN e4.Charges > 2147483647 THEN e4.Charges - 4294967296 ELSE e4.Charges END, 0) AS "spellcharges_4",
       COALESCE(x.spellppmrate_4, 0) AS "spellppmrate_4",
       COALESCE(CASE WHEN e4.CoolDownMSec > 2147483647 THEN e4.CoolDownMSec - 4294967296 ELSE e4.CoolDownMSec END, -1) AS "spellcooldown_4",
       COALESCE(e4.SpellCategoryID, 0) AS "spellcategory_4",
       COALESCE(CASE WHEN e4.CategoryCoolDownMSec > 2147483647 THEN e4.CategoryCoolDownMSec - 4294967296 ELSE e4.CategoryCoolDownMSec END, -1) AS "spellcategorycooldown_4",
       COALESCE(e5.SpellID, 0) AS "spellid_5",
       COALESCE(e5.TriggerType, 0) AS "spelltrigger_5",
       COALESCE(CASE WHEN e5.Charges > 2147483647 THEN e5.Charges - 4294967296 ELSE e5.Charges END, 0) AS "spellcharges_5",
       COALESCE(x.spellppmrate_5, 0) AS "spellppmrate_5",
       COALESCE(CASE WHEN e5.CoolDownMSec > 2147483647 THEN e5.CoolDownMSec - 4294967296 ELSE e5.CoolDownMSec END, -1) AS "spellcooldown_5",
       COALESCE(e5.SpellCategoryID, 0) AS "spellcategory_5",
       COALESCE(CASE WHEN e5.CategoryCoolDownMSec > 2147483647 THEN e5.CategoryCoolDownMSec - 4294967296 ELSE e5.CategoryCoolDownMSec END, -1) AS "spellcategorycooldown_5",
       s.Bonding AS "bonding",
       s.PageID AS "page_text",
       s.LanguageID AS "page_language",
       s.PageMaterialID AS "page_material",
       s.StartQuestID AS "start_quest",
       s.LockID AS "lock_id",
       s.SheatheType AS "sheath",
       s.RandomSelect AS "random_property",
       s.ItemSet AS "set_id",
       s.MaxDurability AS "max_durability",
       s.ZoneBound1 AS "area_bound",
       s.InstanceBound AS "map_bound",
       s.DurationInInventory AS "duration",
       CASE WHEN s.BagFamily = 0 THEN 0 ELSE CAST(log2(s.BagFamily) AS INTEGER) + 1 END AS "bag_family",
       COALESCE(x.disenchant_id, 0) AS "disenchant_id",
       COALESCE(x.food_type, 0) AS "food_type",
       COALESCE(x.min_money_loot, 0) AS "min_money_loot",
       COALESCE(x.max_money_loot, 0) AS "max_money_loot",
       COALESCE(x.wrapped_gift, 0) AS "wrapped_gift",
       COALESCE(x.extra_flags, 0) AS "extra_flags",
       COALESCE(x.other_team_entry, 1) AS "other_team_entry"
FROM dbc_itemsparse s
JOIN dbc_item i ON i.ID = s.ID
LEFT JOIN item_extra x ON x.entry = s.ID
LEFT JOIN (SELECT m.ItemID, MIN(a.ItemDisplayInfoID) AS ItemDisplayInfoID FROM dbc_itemmodifiedappearance m
           JOIN dbc_itemappearance a ON a.ID = m.ItemAppearanceID
           WHERE m.ItemAppearanceModifierID = 0 AND m.OrderIndex = 0 AND a.ItemDisplayInfoID <> 0 GROUP BY m.ItemID) d ON d.ItemID = s.ID
LEFT JOIN dbc_itemeffect e1 ON e1.ParentItemID = s.ID AND e1.LegacySlotIndex = 0
LEFT JOIN dbc_itemeffect e2 ON e2.ParentItemID = s.ID AND e2.LegacySlotIndex = 1
LEFT JOIN dbc_itemeffect e3 ON e3.ParentItemID = s.ID AND e3.LegacySlotIndex = 2
LEFT JOIN dbc_itemeffect e4 ON e4.ParentItemID = s.ID AND e4.LegacySlotIndex = 3
LEFT JOIN dbc_itemeffect e5 ON e5.ParentItemID = s.ID AND e5.LegacySlotIndex = 4
