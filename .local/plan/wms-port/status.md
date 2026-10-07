# WMS port status (rewritten 2026-10-07)

Overall plan: `plan.md` (phases 0-8). What to do next: `next-steps.md`. This file: what exists and the
facts needed to work on it. Code: `WMS = tools/map_editor/WorldMapStudioPlugins/WoWMapStudio/src`.
CataBox reference: `CB = .local/reference/CataBox/src/tools/map_editor/WorldMapStudioPlugins/WoWMapStudio/src`.

## What is built

Nearly everything below is unit-tested but has **not** been checked in the real client or by hand in
the editor (see next-steps.md step 1).

- **Catalogs**: simple db2s (Map, AreaTable, Emote, Faction, Liquid*, LightSkybox, LightParams, SkillLine,
  Quest info/sort, CharTitle, CurrencyType, TerrainType, GroundEffect, SpellDuration/Range/Radius/CastTimes),
  Creature/GameObject Template (spawn palettes), Quest (quest_template), Item (dbc_itemsparse, edit only),
  Spell (`Catalog/Spell`: dbc_spell + name/misc/cooldowns/categories/levels/power/effects/aura options/
  interrupts/class options/target restrictions/reagents, enum + flag pickers, all 16 attribute words).
  `ThinDbcFactory` has LoadExtrasAsync/StageExtras/StageDeleteExtras hooks for satellite tables.
- **Assets**: M2/WMO/BLP loading through FDIDs (chunked MD21, MOMT/MODI FDIDs).
- **Import**: ADT tiles (`wms.wowImport.Run`), incl. MH2O water as `wow.water.surface` procedural models.
- **Entities on vmangos tables**: creature/gameobject spawns (drawn with their models; NPCs with
  CreatureDisplayInfoExtra get only the baked skin), tele, taxi, area trigger, graveyard, waypoints,
  spell_target_position.
- **Viewport**: zone/area header, light/sky preview (LightParams/LightData), server preview layers
  (height, collision, navmesh, liquid, drift).
- **Terrain tools** (`Landscape/`): height sculpt, vertex colour paint, texture paint (4 slots/chunk).
  Holes: script only (`wms.adtHoles`).
- **Export** (`wms.wowImport.Export`): rewrites edited tiles (heights, normals, holes, area, layers+alpha,
  vertex colours, placements, MH2O, `_obj1` removals), WDL entries, new tiles from a blank template + WDT
  MAIN/MAID, tile removal, new map (`wow.export.map`: WDT from donor, dbc_map/map_template/game_tele).
- **Procedural models**: `M2ModelWriter` (with collision), `WmoModelWriter` + `WmoBspBuilder` (+ MOGN),
  `ProceduralModelExporter`, `WowProceduralPlacements` (MDDF/MODF). Script helpers
  `wms.wowImport.CreatePanelModel` / `CreatePanelWmoModel`.
- **Server tooling**: `wow.extract` batch op (wraps `WowServerExtractor`, dry run logs staleness).
- **Started, untracked (2026-10-07)**: `Catalog/SpellVisual`, `Catalog/SpellVisualKit` (someone else may
  be working on them; check `git status`/`git log` before touching).
- `WorldMap/WorldMapProjection.cs` is the only piece of the world map editor (G6).

## Verified in the real client so far
- A procedural WMO panel (MODF repointed by hand in Stormwind, then restored).
- Creature/gameobject models render in the editor at Northshire; Elwynn 32,48 import shows terrain+trees.
- Unedited import -> export writes 0 tiles.

## Important: export writes into the live client
The real project (`tools/map_editor/wms-project.json`) has `wow.clientContent = .client/Content`, and the
exporter writes ADT/WDT/WDL/models **straight into it with no backup**. Before exporting against the real
project, either back up the affected files or use the sandbox below. See next-steps.md step 0.

## Sandbox for editor experiments
`.local/plan/wms-port/sandbox/` has copies of the `/tmp/wmsdbg` scripts (`/tmp` may be wiped):
- `/tmp/wmsdbg/content`: symlinks to every entry of `.client/Content` except `world/maps/azeroth`, which
  holds real copies of tiles 32_48, 31_49, 32_49 (+ wdt/wdl). `project.json` points `wow.clientContent`
  there and the editor DB at `/tmp/wmsdbg/world.doltlite` (a copy of `.world/world.doltlite`; map 1 has
  those 3 tiles imported, plus leftover TestPanel entities).
- `up.sh` (restart + wait), `stop.sh`, `js.sh 'code'` (port 8799), `shot.sh out.png`,
  `look.sh camx camy camz tx ty tz` (aim camera). Adjust paths in the scripts if /tmp/wmsdbg moves.
- Typical flow: `wms.map.Open(1)`, `wms.viewport.SetPosition(-8800,650,266)`,
  `wms.view.SetViewDistanceChunks(16)`, `wms.wowMap.WaitForStreaming()`, then query/edit, `wms.session.Commit()`,
  `wms.wowImport.Export("azeroth","Azeroth")`.

## Gotchas
- `wms-run.sh` without `--no-build` rebuilds; check the build succeeded, otherwise the old binary runs
  ("No public methods" errors = stale binary). Never `pkill -f godot-mono` inline (kills the shell).
- Entities stream in asynchronously: `scene.All()` right after `map.Open` is empty. `Focus(entity from
  All())` fails; use look.sh.
- wowlib (`WoWLib.dll` 0.0.11) types clash with ours (`M2Vertex`, `M2Material`): use aliases.
  Reflect a wowlib type with a small `dotnet fsi` script (old one at `/tmp/refl.fsx`, may be gone).
- Do not use CataBox's M2Writer/M2SkinWriter: they write WotLK MD20; 1.15 needs chunked MD21 + SkinCataPlus.
- `_obj1` layout: MLFD (12 u32: doodad level offsets[3], lengths[3], WMO offsets[3], lengths[3]), MLDD (36 B,
  uid at +4), MLDX (28 B bounds), MLDL (4 B), MLMD (40 B, uid at +4), MLMX (28 B).
- Stock 1.15 ADTs never use MH2O vertex format HeightUv; zero UVs are harmless. With no authored water
  MH2O is left alone unless the tile is flagged `WaterImported` (wow_adt_import_tiles).
- `autologin.sh` picks the last played character and ignores the CHARACTER argument.
