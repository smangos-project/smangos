# WMS catalog expansion checklist

Order (value first):
- [x] 1 Loot templates: creature/gameobject/reference/item/skinning/pickpocketing/fishing/disenchant/mail (all share columns entry,item,ChanceOrQuestChance,groupid,mincountOrRef,maxcount,condition_id; key entry+item+groupid)
- [x] 2 Vendor/trainer: npc_vendor, npc_vendor_template, npc_trainer, npc_trainer_template
- [x] 3 Gossip: gossip_menu + gossip_menu_option done + npc_text + broadcast_text done
- [x] 4 Quest relations (4 catalogs; areatrigger_involvedrelation skipped, one quest per trigger)
- [x] 5 Spawn extras: creature_addon, equip, pool_template, game_event, creature_groups, creature_linking done; pool membership done; game_event_creature/gameobject done; game_event_creature_data, mail, quest left
- [x] 6 Player: Start Spells/Items/Actions, Level Stats, Class Level Stats (entry = race*1000+class); playercreateinfo position row not done
- [x] 7 More DBC: Item Set, Lock, Enchantment, Talent, TalentTab, Holiday (RecordBackedFactory, record copy)
- [~] 8 AI: Creature Spells + Creature AI Event done (RecordBackedFactory). Left: generic_scripts, creature_ai_scripts, creature_movement; from 5: creature_groups, creature_linking, pool membership, game_event_*; also TalentTab, playercreateinfo position row
- [ ] 9 Commit the untracked SpellVisual/SpellVisualKit catalog dirs once they build and test

## Design findings (so far, nothing implemented)
- ThinDbcFactory<TEntity,TRecord> (src/Catalog/ThinDbcFactory.cs) assumes one row per int id (Find(ID) on update). Loot/vendor tables have many rows per entry and no master row, so they need a sibling base, e.g. `ChildRowFactory<TEntity,TRecord>`: entity = ID (entry) + List<row entries>; implement ILazyCatalogEntityFactory + ICatalogBrowser (Search = DISTINCT entry with row counts, Open loads rows where entry==id, Stage = delete-all-for-entry then re-add current rows (or diff), StageDelete removes all rows; IsSaved via SavedEntityStaging).
- Child list editing + undo pattern to copy: Spell.cs ReplaceEffectsCommand = SetFieldCommand<List<..>>, UI in SpellFactory.DrawFields (Add/Remove buttons, Apply(context, cmd) = cmd.Apply(); context.EditSessions.Record(cmd)).
- Field widgets: F.Int/Float/Link(label,get,set,"Item") in CatalogFieldSpec.cs; links to catalogs by CatalogName (Item, Quest, ...). Link loot rows' item to "Item"; mincountOrRef < 0 means reference loot entry.
- Generated records exist for every table (src/Database/Generated/*Record.cs, e.g. CreatureLootTemplateRecord).
- Wire creature_template LootId/VendorId/TrainerId fields to Link into the new catalogs.
- Tests go beside ServerTemplateCatalogTests.cs (round-trip via internal CallToEntity/CallWrite); run `tools/map_editor/wms-test.sh --filter 'WoW Catalog'`.
- Loot done (cf425f6b4); creature_template LootId links to "Creature Loot". Item column in loot rows is a plain int (no Item link yet).
- Vendor/trainer done on shared EntryRowsFactory<TRecord,TTable,TRow> (Catalog/EntryRows); trainer tables are keyless in the DB so TrainerTableKeys (ITableConfiguration) keys them on (entry,spell). New child-row catalog = subclass EntryRowTable + EntryRowsFactory. Script mutators (e.AddRow(..)) currently fail in the sandbox with a Closure cast error, for Spell.AddEffect too (pre-existing).
