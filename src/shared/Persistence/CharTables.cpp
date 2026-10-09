#include "Persistence/CharTables.h"

namespace Persistence
{
    TableRegistry const& CharTableRegistry()
    {
        constexpr ColType I = ColType::Int;
        constexpr ColType R = ColType::Real;
        constexpr ColType T = ColType::Text;

        // column lists follow the character schema, primary key first
        static TableRegistry const registry({
            { "characters", 1, "guid", {
                { "guid", I, false }, { "account", I, false }, { "name", T, false }, { "race", I, false }, { "class", I, false },
                { "gender", I, false }, { "level", I, false }, { "xp", I, false }, { "money", I, false },
                { "character_flags", I, false }, { "zone", I, false }, { "map", I, false }, { "instance", I, false },
                { "position_x", R, false }, { "position_y", R, false }, { "position_z", R, false }, { "orientation", R, false },
                { "transport_guid", I, false }, { "transport_x", R, false }, { "transport_y", R, false },
                { "transport_z", R, false }, { "transport_o", R, false }, { "known_taxi_mask", T, true },
                { "current_taxi_path", T, true }, { "online", I, false }, { "played_time_total", I, false },
                { "played_time_level", I, false }, { "create_time", I, false }, { "logout_time", I, false },
                { "rest_bonus", R, false }, { "reset_talents_multiplier", I, false }, { "reset_talents_time", I, false },
                { "death_expire_time", I, false }, { "stable_slots", I, false }, { "bank_bag_slots", I, false },
                { "extra_flags", I, false }, { "honor_rank_points", R, false }, { "honor_highest_rank", I, false },
                { "honor_standing", I, false }, { "honor_last_week_hk", I, false }, { "honor_last_week_cp", R, false },
                { "honor_stored_hk", I, false }, { "honor_stored_dk", I, false }, { "watched_faction", I, false },
                { "drunk", I, false }, { "health", I, false }, { "power1", I, false }, { "power2", I, false },
                { "power3", I, false }, { "power4", I, false }, { "power5", I, false }, { "explored_zones", T, true },
                { "equipment_cache", T, true }, { "ammo_id", I, false }, { "action_bars", I, false },
                { "deleted_account", I, true }, { "deleted_name", T, true }, { "deleted_time", I, true },
                { "world_phase_mask", I, true }
            } },
            { "character_customizations", 2, "guid", {
                { "guid", I, false }, { "chrCustomizationOptionID", I, false }, { "chrCustomizationChoiceID", I, false }
            } },
            { "character_battleground_data", 1, "guid", {
                { "guid", I, false }, { "instance_id", I, false }, { "team", I, false }, { "join_x", R, false },
                { "join_y", R, false }, { "join_z", R, false }, { "join_o", R, false }, { "join_map", I, false }
            } },
            { "character_inventory", 1, "guid", {
                { "item_guid", I, false }, { "guid", I, false }, { "bag", I, false }, { "slot", I, false },
                { "item_id", I, false }
            } },
            { "item_instance", 1, "owner_guid", {
                { "guid", I, false }, { "item_id", I, false }, { "owner_guid", I, false }, { "creator_guid", I, false },
                { "gift_creator_guid", I, false }, { "count", I, false }, { "duration", I, false }, { "charges", T, true },
                { "flags", I, false }, { "enchantments", T, false }, { "random_property_id", I, false },
                { "durability", I, false }, { "text", I, false }, { "generated_loot", I, true }
            } },
            { "character_queststatus", 2, "guid", {
                { "guid", I, false }, { "quest", I, false }, { "status", I, false }, { "rewarded", I, false },
                { "explored", I, false }, { "timer", I, false }, { "mob_count1", I, false }, { "mob_count2", I, false },
                { "mob_count3", I, false }, { "mob_count4", I, false }, { "item_count1", I, false }, { "item_count2", I, false },
                { "item_count3", I, false }, { "item_count4", I, false }, { "reward_choice", I, false }
            } },
            { "character_spell", 2, "guid", {
                { "guid", I, false }, { "spell", I, false }, { "active", I, false }, { "disabled", I, false }
            } },
            { "character_spell_cooldown", 2, "guid", {
                { "guid", I, false }, { "spell", I, false }, { "spell_expire_time", I, false }, { "category", I, false },
                { "category_expire_time", I, false }, { "item_id", I, false }
            } },
            { "character_aura", 4, "guid", {
                { "guid", I, false }, { "caster_guid", I, false }, { "item_guid", I, false }, { "spell", I, false },
                { "stacks", I, false }, { "charges", I, false }, { "base_points0", R, false }, { "base_points1", R, false },
                { "base_points2", R, false }, { "periodic_time0", I, false }, { "periodic_time1", I, false },
                { "periodic_time2", I, false }, { "max_duration", I, false }, { "duration", I, false },
                { "effect_index_mask", I, false }
            } },
            { "character_skills", 2, "guid", {
                { "guid", I, false }, { "skill", I, false }, { "value", I, false }, { "max", I, false }
            } },
            { "character_reputation", 2, "guid", {
                { "guid", I, false }, { "faction", I, false }, { "standing", I, false }, { "flags", I, false }
            } },
            { "character_honor_cp", 0, "guid", {
                { "guid", I, false }, { "victim_type", I, false }, { "victim_id", I, false }, { "cp", R, false },
                { "date", I, false }, { "type", I, false }
            } },
            { "character_action", 2, "guid", {
                { "guid", I, false }, { "button", I, false }, { "action", I, false }, { "type", I, false }
            } },
            { "character_tutorial", 1, "account", {
                { "account", I, false }, { "tut0", I, false }, { "tut1", I, false }, { "tut2", I, false }, { "tut3", I, false },
                { "tut4", I, false }, { "tut5", I, false }, { "tut6", I, false }, { "tut7", I, false }
            } },
            { "character_social", 3, "guid", {
                { "guid", I, false }, { "friend", I, false }, { "flags", I, false }
            } },
            { "mail", 1, "receiver_guid", {
                { "id", I, false }, { "message_type", I, false }, { "stationery", I, false }, { "mail_template_id", I, false },
                { "sender_guid", I, false }, { "receiver_guid", I, false }, { "subject", T, true }, { "item_text_id", I, false },
                { "has_items", I, false }, { "expire_time", I, false }, { "deliver_time", I, false }, { "money", I, false },
                { "cod", I, false }, { "checked", I, false }
            } },
            { "mail_items", 2, "receiver_guid", {
                { "mail_id", I, false }, { "item_guid", I, false }, { "item_id", I, false }, { "receiver_guid", I, false }
            } },
            { "character_pet", 1, "owner_guid", {
                { "id", I, false }, { "entry", I, false }, { "owner_guid", I, false }, { "display_id", I, true },
                { "created_by_spell", I, false }, { "pet_type", I, false }, { "level", I, false }, { "xp", I, false },
                { "react_state", I, false }, { "loyalty_points", I, false }, { "loyalty", I, false },
                { "training_points", I, false }, { "name", T, true }, { "renamed", I, false }, { "slot", I, false },
                { "current_health", I, false }, { "current_mana", I, false }, { "current_happiness", I, false },
                { "save_time", I, false }, { "reset_talents_cost", I, false }, { "reset_talents_time", I, false },
                { "action_bar_data", T, true }, { "teach_spell_data", T, true }
            } },
            { "pet_spell", 2, "guid", {
                { "guid", I, false }, { "spell", I, false }, { "active", I, false }
            } },
            { "pet_aura", 4, "guid", {
                { "guid", I, false }, { "caster_guid", I, false }, { "item_guid", I, false }, { "spell", I, false },
                { "stacks", I, false }, { "charges", I, false }, { "base_points0", R, false }, { "base_points1", R, false },
                { "base_points2", R, false }, { "periodic_time0", I, false }, { "periodic_time1", I, false },
                { "periodic_time2", I, false }, { "max_duration", I, false }, { "duration", I, false },
                { "effect_index_mask", I, false }
            } },
            { "pet_spell_cooldown", 2, "guid", {
                { "guid", I, false }, { "spell", I, false }, { "time", I, false }
            } },
            { "character_homebind", 1, "guid", {
                { "guid", I, false }, { "map", I, false }, { "zone", I, false }, { "position_x", R, false },
                { "position_y", R, false }, { "position_z", R, false }
            } },
            { "character_instance", 2, "guid", {
                { "guid", I, false }, { "instance", I, false }, { "permanent", I, false }
            } }
        });
        return registry;
    }
}
