# WMS port: next steps (rewritten 2026-10-07)

Read `status.md` first (what exists, sandbox, gotchas). `plan.md` is the overall plan.
Priority: most features are written but never seen working end to end. **Verify before building more.**
Each step ends in something checked in the real client/server (screenshots published), or a written-up
bug. Fix what breaks before moving on. Tick items here as they're done.

## Step 0: safe export into the client (done 2026-10-07)
- [x] `WowExportBackup` saves each stock file once before an export overwrites it (copy in
  `.client/.wms-export-backup`, manifest of originals/absent files); `wow.export.restore` batch op puts everything back.
  Covers `world/maps/<map>`, `world/generated`, `fdid.index`. Used by `wow.export.adt` and `wow.export.map`.
- [x] Both ops refuse to run while a WowClassic.exe process exists (no character DB access from the plugin).

## Step 1: end-to-end checks of what's built (in this order)

1. [x] **Terrain (Phase 7 exit)** (2026-10-07: hill +22 + dirt paint in 32,48 exported, renders in client, server re-extracted (`wowServer.Extract(0,row=48,col=32)`: row is the tile y) and the character stands on the hill; stock tiles restored afterwards. Drift check verified 2026-10-08 via new `wms.wowServer.Drift(fileX,fileY)`: 0/16625 on unedited tile, 21 verts (max 25.0) after a +25 raise, 0 again after export + re-extract. Still open: WDL from distance.): in the editor, in the editor, raise a hill and paint a texture in Northshire
   (tile 32,48), export, start the client (`client/autologin.sh`), walk on it, check it from a distance
   (normals, WDL). Re-extract server data (`wow.extract` batch op, run through the UI, incl. dry run) and
   check the Drift layer reads ~0 and the server height matches.
2. [x] **Spawns (Phase 5 exit, verified 2026-10-07 via wms.spawn.Create + Commit, server pointed at sandbox DB, guard visible in client)**: place a creature in Northshire, commit, restart/reload the server, see it in
   the client. Spawn guids must not collide with a running server.
3. [x] **Water (D exit)** (VERIFIED 2026-10-07: re-imported 32,48 with importWater=true, pond added, export keeps stock MH2O (46 water chunks = 42 + pond), client renders the water plane, server re-extract + Breath timer shows it sees the water; stock restored afterwards. Remaining: swim visually, server liquid height check.) (2026-10-07: added `wms.wowImport.CreatePondModel(name,size,liquidTypeId,x,y,z)`; pond at editor (-8935,85.5,135) exported into 32,48 (MH2O now 8 cells) but NOT yet seen in the client or by the server. FINDING: the sandbox tile 32,48 is not flagged WaterImported and has no imported water entities, so export REPLACED the stock stream MH2O (42 cells) with only the pond: re-import the tile with water (wow.import.adt importWater=true) before testing, or fix the export to merge. Also 31_49/32_49 get rewritten only because of doodad ordering (benign). Stock tiles restored in the client afterwards.)**: imported water looks right up close in the viewport (look.sh); add a pond,
   export, swim in it; after re-extract the server sees water (breath bar).
4. [ ] **Procedural models (H exit)**: an M2 panel/road and a WMO wall in Northshire render in the client;
   run the vmap extractor on them (collision present, not NoCollision), walk on the WMO, server LoS blocks.
   Check MOGN group names don't upset the client.
5. [ ] **New map (E exit)**: create a 2x2-tile map with `wow.export.map` + new tiles,
   `database_tool export --format db2 --client`, map_extractor + mmaps, `.tele` there, walk; server
   heights agree. Also tile removal.
6. [ ] **Spell editor**: open an existing spell, edit, save, reopen; satellite rows updated in place; a new
   spell gets rows with fresh ids; delete removes them; links (cast time, duration, range) resolve; the
   effect editor undoes. Then `database_tool export --client` and see the change in game.
7. [ ] **Editor look-through** (one screenshot each): catalog row edit, Goldshire M2+WMO, NPC appearance
   (CreatureDisplayInfoExtra skin), markers around Northshire, server preview toggles, light preview.
8. [~] Data: `WaterImported` column was missing in old DBs and exports crashed (fixed: EditorStorage now owns plugin tables mapped by its entities; the migration screen offers AddColumn; the sandbox DB was patched by hand; the real `.world` needs the Apply click on next editor start; generated `WowAdtImportTilesRecord` in WorldStorage still lacks the column, regenerate with generate-wms after migrating `.world`). Clean up TestPanel
   entities in the sandbox DB.

## Step 2: close small gaps found useful during step 1
- Hole brush tool (today holes are script only).
- `_obj1` LOD entries for newly added placements (large WMOs at least), so they show from far away.
- WMO doodad sets for procedural WMOs (wowlib `SmoDoodadSet.name` is read-only; needs a workaround).
- NPC appearance: equipment and hair layers (F5 rest: `ChrModel`, `ChrCustomization*`,
  `NpcModelItemSlotDisplayInfo`, `ItemDisplayInfoMaterialRes`/`ModelMatRes`).
- Spell icons shown as images instead of bare FDIDs.

## Step 3: remaining editors
- **G5 spell visuals**: an editor over SpellXSpellVisual -> SpellVisual -> SpellVisualEvent ->
  SpellVisualKit -> SpellVisualKitEffect -> ModelAttach/ProceduralEffect, SpellChainEffects,
  SpellVisualMissile. Design it fresh for the 1.15 graph (don't port `CB/Catalog/SpellVisual*`); keep unknown
  `Field_1_15_*` columns. `Catalog/SpellVisual(Kit)` were started 2026-10-07; check whether another job owns it.
- **G6 world map editor**: `CB/WorldMap` retargeted to UiMap, UiMapAssignment, UiMapArt/ArtTile/XMapArt,
  WorldMapOverlay/Tile, UiMapLink, UiMapGroup(Member). `WorldMap/WorldMapProjection.cs` exists.
- **G4 small catalogs**: MailTemplate + mail_loot_template, battlegrounds (BattlemasterList +
  battleground_template), CinematicCamera (M2 camera via wowlib) + CinematicSequences.
- **G7** race/class (ChrRaces/ChrClasses + playercreateinfo*): last, low value.

## Deferred (decided)
- Distant terrain textures (`world/maptextures`) stay stale until they bother us; then bake (C4).
- ZMP (`interface/worldmap/*.zmp`, E5): only once a new/edited area shows wrong on the world map.
- MCSH shadows: kept as is unless they look wrong.
- Dropped: MPQ, StormLib, SmartAI paths, Cata-only catalogs, SpellIcon/CameraShakes (not in 1.15).
