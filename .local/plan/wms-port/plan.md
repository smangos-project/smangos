# Plan: port CataBox's WoWMapStudio plugin to smangos (1.15, vmangos schema, DoltLite)

Status: planning only, nothing implemented (2026-10-05).

## What exists

| Piece | Where | State |
|---|---|---|
| CataBox plugin (reference) | `.local/reference/CataBox/src/tools/map_editor/WorldMapStudioPlugins/WoWMapStudio` | ~96k lines hand-written C# + 537 generated EF records (TC world + Cata DBC dumps). Targets WMS core of 2026-09-21 (old hash `6e58952`, history since rewritten), MySQL/dolt sql-server, MPQ, 4.3.4 formats. |
| Our WMS core | `tools/map_editor/WorldMapStudio` submodule @ `75694a5` (origin/main, 2026-09-25) | Still MySQL/dolt sql-server (Pomelo). No plugin folder yet. |
| DoltLite WMS | branch `doltlite-migration` (15 commits on top of `75694a5`) in `/mnt/data/Dev/WoW/Classic3/tools/map_editor/WorldMapStudio` | **Local only, not on origin.** EF Core Sqlite.Core + `DoltHub.Doltlite` 0.50.12, per-storage `StorageLocation` (file@branch), background-read work. |
| wowlib | `.local/reference/wowlib` | C++26 (gcc 16, we have 16.2.1) with welder-generated **C# bindings / NuGet `WoWLib`**: WMO, M2(+skin/anim/skel), ADT (split), WDT, WDL, BLP, WDC5, CASC. Classic builds map onto the retail lineage by build number: 1.15 = 11.x (TWW) column, which is marked "implemented, not corpus-verified". |
| World DB | `.world/world.doltlite` (built by `tools/database_tool`) | 978 tables: ~140 vmangos server tables + every 1.15 db2 as `dbc_<lowercase>` (WoWDBDefs column names, e.g. `MapName_lang`, `ModelID`). `database_tool export --format db2 --client` writes db2s to `.client/Content/dbfilesclient`. |
| Client data | `.client/Content` | Loose lowercase files by listfile path (casc_extractor). Client reads them via `custom.dll` CASC bypass. Split ADTs (`_tex0/_obj0/_obj1`), FileDataID references everywhere. |
| Server data | `tools/map_extractor` -> `.server/data` | maps/vmaps/mmaps, incremental, has a `.cache` fingerprint per tile (CataBox's Staleness reads "extractor manifests" -> must be adapted to this cache). |

## Main differences to bridge

1. **Storage**: MySQL/dolt server -> DoltLite file. Raw SQL in the plugin is small (`CataStorage`, `WowWaypointPathFactory`, `ISchemaMaintenance` index statements); the rest is EF Core and ports by switching provider.
2. **Schema**: TrinityCore 4.3.4 world (`tc_*`) + Cata DBC dumps (`dbc_*` Cata layout) -> vmangos world + 1.15 db2 layout. Every catalog/entity factory is touched. Examples:
   - `tc_creature` -> `creature` (vmangos: `id..id5`, `spawntimesecsmin/max`, `wander_distance`, `movement_type`, no phase/spawnMask); `creature_addon`, `creature_movement`/`creature_movement_template` instead of `waypoint_data`; no SmartAI (`creature_ai_events`/`*_scripts` instead).
   - `tc_game_tele` -> `game_tele`, `graveyard_zone` -> `game_graveyard_zone` + `world_safe_locs` (server table, not the db2), `areatrigger_*` similar, `spell_target_position` similar.
   - Creature display: `CreatureDisplayInfoExtra` + `CharSections` -> `ChrCustomization*` (Choice/Element/Material/Geoset/SkinnedModel) + `CreatureDisplayInfoOption`; model/texture paths -> `ModelFileData`/`TextureFileData`/FileDataIDs.
   - World map: `WorldMapArea/Continent/Overlay/Transforms` -> `UiMap*`, `UiMapAssignment`, `WorldMapOverlay(+Tile)`.
   - Spell: Cata split tables partly exist differently in 1.15 (`SpellMisc`, `SpellEffect`, `SpellXSpellVisual`...); catalog editors need re-mapping, and `Field_1_15_4_*` unknown columns must survive round-trips.
   - Quests are vmangos `quest_template`. There is no `item_template`: items live in `dbc_item`/`dbc_itemsparse` (+ `item_extra`, loot tables).
3. **Client formats**: Cata 4.3.4 filename-based files -> modern FileDataID-based files. ADT `_obj0` uses `MDDF/MODF` with FDID flag, `_tex0` uses `MDID/MHID` (no `MTEX`), WDT has `MAID` (per-tile FDIDs for every split file, minimap, lod). M2 is MD21 chunked (`SFID`/`TXID`/`AFID`/`SKID`...), WMO root has `GFID`/`MOMT` with FDIDs. CataBox's hand-written readers/writers (`CataAdtTileReader`, `AdtRootWriter`, `M2Reader`, `WmoRootReader`, `BLPTextureLoader`, `WdlWriter`...) are replaced by wowlib.
4. **Asset access**: MPQ/StormLib -> loose files (`FileSystem` source already in core) + a FileDataID resolver (listfile + `dbc_modelfiledata`/`dbc_texturefiledata`/`dbc_files`?). New files need new FDIDs: an allocator in a custom range, written to the listfile the client/bypass uses (check `client/client_library/docs/CascBypass.md` for how unknown FDIDs resolve).
5. **Export targets**: ADT/WDT/WDL/ZMP export -> `.client/Content/world/maps/...`; db2 export goes through `database_tool export --format db2 --client` (don't write db2 from C#).

## Decisions (answered 2026-10-05)

- **D1. DoltLite WMS**: DoltLite is WMS's main backend. `doltlite-migration` (d6b01c6) fast-forwarded into `main` in our submodule (`tools/map_editor/WorldMapStudio`, fetched from the read-only Classic3 checkout). User pushes it to origin (agent pushes are blocked); then Phase 0 bumps the submodule pin to ad4e421 (history rewritten 2026-10-05: author fixed, all commits signed, d6b01c6 -> ad4e421; old history in branch backup/pre-resign).
- **D2/D3. Editor data**: lives in `.world`, like the old WMS (CataBox: `EditorStorage` on the `.world` dolt repo, `CataStorage` sharing that connection, so editor and world tables are in one DB). Here: the `Editor` storage's `DatabasePath` in wms-project.json points at `.world/world.doltlite`, and `WorldStorage` (renamed `CataStorage`) shares its connection. `database_tool create` refuses to overwrite an existing world.doltlite, so editor tables survive everything except deleting `.world/`. Still: `WorldStorage.EnsureSchema` must not create/alter world tables (only check them); editor-only indexes on world tables go in `sql/migrations/*_world.sql`. Check the world migration runner and `.world/baseline`/`golden` diffs ignore the extra editor tables.
- **D8. Table ownership (2026-10-05)**: each storage declares which tables it owns. The editor creates (and migrates) only its own `wms_*` tables. Tables it doesn't own (the world tables) come into the editor as `database_tool`-generated EF models; the editor only validates them against the live DB and, on a mismatch, tells the user what to do (regenerate / recreate `.world`) instead of offering SQL. Never drops or alters a table it doesn't own. Dolt commits don't matter for now: `.world` is recreated often (editor data in it is lost then, accepted).
- **D4. Generated records**: the editor follows the world tables, so `database_tool` generates the WMS EF Core sources (record classes + `ITableConfiguration`s, `Vm*Record` / `Db2*Record`) for all tables, from the same schema metadata it already has (`tools/database_tool/generated/DbcTables.g.cpp` for db2, world schema for the rest). Replaces CataBox's `DBImport.csx`. Regenerate when the world schema changes.
- **D5. wowlib**: agreed: try the NuGet `WoWLib` package; fall back to source only if it doesn't work.
- **D6. Terrain round-trip**: (agreed) CataBox's import -> landscape -> export model.
- **D7. Scope**: agreed: DB path first (catalogs + spawns as markers, see first slice), since everything else depends on storage; terrain after.
- **Implementation**: Phase 0 done and committed (2026-10-05). Phase 1 planned in `phase1.md`; waiting for the go-ahead. Carry the OwnsConnection->OwnsLocation drift notes into Phase 1.

## Phases

Each phase ends in something runnable with tests (`[EditorTest]` + plain unit tests). Order follows dependencies.

### Phase 0 - Host setup (small) - DONE
Detailed plan: `phase0.md` (submodule pin already done; found a shared-file blocker in the core).
- Get the DoltLite WMS into the submodule (D1). Verify it builds and runs on Linux with Godot 4.7 mono and that `DoltHub.Doltlite` loads the linux-x64 native lib.
- Create `tools/map_editor/WorldMapStudioPlugins/WoWMapStudio/` (core globs `../WorldMapStudioPlugins/**/*.cs`), `tools/map_editor/wms-project.json` pointing at `.world/*.doltlite`, `.client/Content`, `.server/data`, axis preset X fwd/Y left/Z up.
- Port `wms-test.ps1` to a bash script (we are on Linux/fish).
- Exit: empty plugin compiles, editor opens the project with both storages.

### Phase 1 - Plugin skeleton + foundations (copy & adapt, no schema yet) - DONE
Detailed plan: `phase1.md` (server readers must be ported to vmangos formats; plugin editor tables renamed `wms_wow_*`).
- Copy format-independent code verbatim first, compile, fix core API drift (core moved ~4 days + DoltLite): `Models/Wow*Coordinates`, `WowAxisPreset`, `WowMapBindings`, `WowTileAddressing`, `Viewport/*` header items, `Component/WowPointMarker`, `WorldMap/WorldMapProjection`, `Server/*` readers (formats must be checked against our extractor; `WowServerFormats` versions), utility tests.
- Drop: `Archives/MPQ*`, `StormLib`, `native/`, `Database/Generated` (regenerated in Phase 2).
- Exit: plugin compiles against core, its pure unit tests pass.
- Done notes: editor tables renamed `wow_*` -> `wms_wow_*`. `Server/` has the readers (map, mmtile, vmtile/vmo) and mesh builders (collision, navmesh) only; the preview layers, staleness, drift and extract operations are Phase 6. vmtile names are `MMM_XX_YY` with XX = fileX; a tile also lists spawns up to one tile outside it.

### Phase 2 - Database layer - DONE
- `WorldStorage` (was `CataStorage`): own `StorageLocation` -> `world.doltlite` (D2), verify-only schema (D3, D8): `ExpectedSchema()` = the generated model, mismatches go through Phase 0's non-owned-table path.
- New `database_tool` command (e.g. `generate-wms`) emitting C# records + table configurations into the plugin's `src/Database/Generated` (D4), instead of `tools/DBImport.csx`. Map SQLite affinity to CLR types carefully (vmangos `INTEGER` columns that are unsigned 32-bit, `REAL` floats, `_lang` text).
- Port `ThinDbcFactory` + the simple one-table catalogs that still exist in 1.15 with renamed columns: AreaTable, Map, Faction, Emote, LiquidType, LiquidObject, LightParams/LightSkybox, SkillLine, QuestInfo, QuestSort, CharTitle, CurrencyTypes, CameraShakes, TerrainType, GroundEffect*, SpellIcon/Duration/Range/Radius/CastTimes...
- Progress: `database_tool generate-wms` (reads the live world.doltlite, one `<Table>Record.cs` per table, INTEGER -> int unless the data needs long) and `WorldStorage`/`WorldDbContext` done; `ThinDbcFactory` + Map and AreaTable catalogs ported (1.15 `Flags1/Flags2`, no Map `Flags`). Tests need the real project config now (scratch has no world tables, so the editor stalls on the migration screen). Simple catalogs ported too (Emote, Faction, LiquidType/Object, LightSkybox, SkillLine, QuestInfo/Sort, CharTitle, CurrencyType, TerrainType, GroundEffect*, SpellDuration/Range/Radius/CastTimes); SkillLine and GroundEffectDoodad use 1.15 file-ID columns. Not in 1.15, skipped: SpellIcon, CameraShakes, LightParams bands. LightParams (needs a Light data rework) is left. Exit check: row edited and dolt-committed on a copy, `export --format db2` wrote it into emotes.db2 (real-client load not re-tested; the db2 writer is unchanged).
- Exit: catalog browser shows and edits 1.15 db2 rows; a commit lands as a dolt commit in `world.doltlite`; `database_tool export --format db2 --client` turns it into a db2 the client accepts (see "db2 files the client accepts" in project notes).

### Phase 3 - Asset layer on wowlib
- FileDataID resolver service: listfile (`.local/listfile.txt` or the one casc_extractor uses) + `dbc_modelfiledata`/`dbc_texturefiledata`; path<->FDID both ways.
- `BLPTextureLoader` -> wowlib BLP decode (keep the Godot `ITextureLoader` surface).
- `M2ModelLoader`/`WmoModelLoader` -> wowlib M2 (MD21 + .skin) and WMO (root+groups, FDID-based). Keep CataBox's Godot-side rendering (`Rendering/M2`, `Rendering/Wmo`, materials, blend modes) and adapt to modern material/shader ids.
- Character/creature appearance: rewrite `CreatureAppearanceResolver`, `CharacterTextureCompositor`, `CharacterTextureLayerResolver`, `DressUp/*` on `ChrCustomization*` + `ChrModel` + `CreatureDisplayInfoExtra`/`CreatureDisplayInfoOption` + `ItemDisplayInfoMaterialRes`/`ItemDisplayInfoModelMatRes`. This is the largest single rewrite in the port; can be deferred (render creatures by `CreatureDisplayInfo` model + texture variations first, NPCs with extra display info as a placeholder).
- Exit: browsing/placing a doodad M2 and a WMO renders; a creature display renders.
- Progress: BLP, FDID resolver, M2 (wowlib) and WMO (CataBox byte readers, not wowlib) loaders done; AssetSystem loads real M2+WMO; `CreatureAppearanceResolver` (CreatureDisplayInfo -> CreatureModelData FDID + TextureVariationFileDataID1-3) done. Remaining: visual check in editor, character/NPC-extra appearance, wowlib round-trip audit.

### Phase 4 - Terrain import (read-only world)
- Replace `CataAdtTileReader`/`WdtReader`/`WdlReader`/`McalDecoder` with a wowlib-backed `IAdtTileReader` (root + `_tex0` + `_obj0`; `_obj1`/`_lod` ignored). Keep `Import/Mapping/*` (height, holes, textures, vertex colours, water MH2O, placements, area) and its tests, swapping texture paths for FDIDs.
- WDT `MAID`/`MPHD` flags; doodad/WMO placements by FDID.
- Progress: `Import/Adt/` has `WowClientFiles` (shared wowlib FileSystem on the real CASC in `.client`; NuGet has no loose-directory mode, ADT read needs it), `WowWdt` (flags, tile presence, MAID), `WowAdtTileReader` (wowlib ADT -> FDID-based `ImportedTile`, alpha already 64x64, MH2O as plain data) and an Elwynn test. `Import/Mapping/` has the pure mappers (height, hole, texture, vertex colour, placement, area; tests pass, textures keyed by FDID). `AdtTileComponent` (+type, persistence), `AdtImportCatalog`, `AdtImportTileStore`, `AdtReimportGuard` and `WowAdtProfile` are copied and compile (catalog not yet exercised against FDID textures; catalog/tile-store may still key textures by path). `WowAdtImportBatchOperation` (+tests), `WowImportScriptApi`, `WowExportEntityIds` and the WMO doodad-set partial are ported and compile (materials keyed by FDID, texture/model paths resolved through the listfile; water import removed for now). Not done: `WaterMapper`/water import, running an actual Elwynn import (azeroth 32,48) and visual check.
- Done: Elwynn tile import verified on real data; water import deferred.
- Exit: Elwynn tiles import and match the client visually (screenshot side by side via the control server).

### Phase 5 - Spawns and world entities on vmangos tables
- `WowCreatureSpawn` <- `creature` (+ `creature_addon`, `creature_template`, `creature_display_info_addon`), `WowGameObjectSpawn` <- `gameobject` (+ `gameobject_template`, quaternion rotation0-3).
- `WowWaypointPath` <- `creature_movement` / `creature_movement_template`; drop `WowSmartWaypointPath` (no SmartAI) or replace with `script_waypoint`/`script_escort_data`.
- `WowGameTele` <- `game_tele`; `WowGraveyard` <- `world_safe_locs` + `game_graveyard_zone`; `WowAreaTrigger` <- `dbc_areatrigger` + `areatrigger_teleport/tavern/involvedrelation/bg_entrance`; `WowSpellTargetPosition` <- `spell_target_position`; `WowTaxiNode/Path` <- `dbc_taxinodes/taxipath/taxipathnode` (+ `taxi_path_transitions`); `WowLight`/`WowZoneLight` <- `dbc_light` (+ `lightdata`), `dbc_zonelight`/`zonelightpoint`; `WowSoundEmitter` <- `dbc_soundemitters`.
- vmangos specifics: `pool_*`, `game_event_creature/gameobject` membership, `creature_groups`, patch/`spawn_flags`. Decide which to surface.
- Server-side ids: guid allocation for `creature`/`gameobject` must not collide with the running server; reloads go through the server console fifo (`.reload` commands) where useful.
- Progress: point-marker entities with factories + view categories: WowCreatureSpawn, WowGameObjectSpawn, WowGameTele, WowGraveyard, WowTaxiNode, WowAreaTrigger (+ areatrigger_teleport), WowLight, WowSoundEmitter; path entities WowTaxiPath, WowWaypointPath (shared Entity/Path). Tables without autoincrement get max+1 ids in `PrepareBatchAsync`. Not done: WowZoneLight, game_graveyard_zone link, area trigger volume mesh, inspectors, spawn-commit-to-client check. Commit test (taxi node, area trigger, light, sound emitter) passes on a scratch DB copy.
- Exit: place a creature in the editor, commit, restart (or reload) the server, see it in the real client via `client/autologin.sh`.

### Phase 6 - Server data loop
- `Server/Preview` layers against our `.server/data` (`.map`/`.vmtile`/`.vmo`/`.mmtile` versions from `tools/map_extractor`), `Server/Extract` runs `build/tools/map_extractor/map_extractor --map --tiles`, `Staleness` reads map_extractor's `.cache` fingerprints.
- Exit: drift/collision/navmesh layers render for Northshire.

### Phase 7 - Terrain export
- ADT writers (root/`_tex0`/`_obj0`, regenerate `_obj1`/`_lod` or drop and clear the WDT `MAID` slots), WDT (`MAID`, new tiles need new FDIDs), WDL, ZMP (`Map.ZmpFileDataID`), MH2O, minimap tiles (optional). All via wowlib writers; CataBox writers become the semantic reference for what to fill.
- FDID allocator + listfile update for new files; check the CASC bypass picks up new FDIDs.
- Exit: edit a hill in Northshire, export, restart client, walk on it; re-extract server data and the server agrees (height drift layer is zero).

### Phase 8 - Remaining editors
- Quest catalog on vmangos `quest_template` (+ `creature_questrelation`/`involvedrelation`, `gameobject_*relation`, `quest_*_scripts`).
- Item catalog on `dbc_item`/`dbc_itemsparse` (+ `item_extra`), ItemDisplayInfo.
- Spell + SpellVisual* catalogs re-mapped to the 1.15 split tables.
- World map editor on `UiMap*`; DungeonMap -> `UiMapArt`/`UiMapXMapArt`.
- Cinematic camera import/export (M2 camera writer stays, FDID-based `CinematicCamera` db2).
- Procedural model export (M2/WMO writers) via wowlib writers.

## Risks
- wowlib 11.x lineage is unverified against a real corpus; budget for upstream fixes (run its round-trip audit on `.client/Content` early, in Phase 3).
- CataBox plugin targets a WMS core that diverged (history rewrite + DoltLite); expect compile churn in Phase 1.
- Lossy terrain mapping: modern ADTs have features Cata didn't (`MCDD`, `MTXP` height textures, `MLDD`/LOD, blend meshes). Export must preserve or regenerate them; otherwise round-tripped tiles look worse than stock.
- Character appearance under ChrCustomization is a large rewrite.
- Concurrent writers: the server reads `world.doltlite` while the editor writes; check DoltLite locking (`.world.doltlite-lock`) and busy timeouts.

## Suggested first slice
Phase 0 + 1 + the Phase 2 storage/generator + one catalog (Map or AreaTable), then Phase 5 creature spawns rendered as markers (no models). That proves the DB path end to end before any format work.
