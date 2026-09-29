-- key: entry, slot, spell_id
SELECT "entry", "slot", "availability", "spell_id", "cooldown_min", "cooldown_max", "patch_min", "patch_max" FROM "creature_charm_spells" WHERE "patch_min" <= 10 AND 10 <= "patch_max"
