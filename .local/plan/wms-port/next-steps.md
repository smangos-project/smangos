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

1. [x] **Terrain (Phase 7 exit)** (2026-10-07: hill +22 + dirt paint in 32,48 exported, renders in client, server re-extracted (`wowServer.Extract(0,row=48,col=32)`: row is the tile y) and the character stands on the hill; stock tiles restored afterwards. Drift check verified 2026-10-08 via new `wms.wowServer.Drift(fileX,fileY)`: 0/16625 on unedited tile, 21 verts (max 25.0) after a +25 raise, 0 again after export + re-extract. WDL: export with a big +365 spire changed azeroth.wdl (999 bytes), but the visual check from a distance is not done (could not aim the client camera from Goldshire; /run needs the custom-scripts prompt accepted, batch format is {"actions":[{"key":..},{"wait_ms":..},{"type":..}]}).): in the editor, in the editor, raise a hill and paint a texture in Northshire
   (tile 32,48), export, start the client (`client/autologin.sh`), walk on it, check it from a distance
   (normals, WDL). Re-extract server data (`wow.extract` batch op, run through the UI, incl. dry run) and
   check the Drift layer reads ~0 and the server height matches.
2. [x] **Spawns (Phase 5 exit, verified 2026-10-07 via wms.spawn.Create + Commit, server pointed at sandbox DB, guard visible in client)**: place a creature in Northshire, commit, restart/reload the server, see it in
   the client. Spawn guids must not collide with a running server.
3. [x] **Water (D exit)** (VERIFIED 2026-10-07: re-imported 32,48 with importWater=true, pond added, export keeps stock MH2O (46 water chunks = 42 + pond), client renders the water plane, server re-extract + Breath timer shows it sees the water; stock restored afterwards. Remaining: swim visually, server liquid height check.) (2026-10-07: added `wms.wowImport.CreatePondModel(name,size,liquidTypeId,x,y,z)`; pond at editor (-8935,85.5,135) exported into 32,48 (MH2O now 8 cells) but NOT yet seen in the client or by the server. FINDING: the sandbox tile 32,48 is not flagged WaterImported and has no imported water entities, so export REPLACED the stock stream MH2O (42 cells) with only the pond: re-import the tile with water (wow.import.adt importWater=true) before testing, or fix the export to merge. Also 31_49/32_49 get rewritten only because of doodad ordering (benign). Stock tiles restored in the client afterwards.)**: imported water looks right up close in the viewport (look.sh); add a pond,
   export, swim in it; after re-extract the server sees water (breath bar).
4. [~] **Procedural models (H exit)** (2026-10-08 final: installed and viewed in the client (loads the files, `.client` is a symlink to the Resource dir so the install modifies the pristine copy): only a flat dark patch (the M2 panel, untextured) is visible, the WMO wall was NOT seen (group file tiny, 320-byte MOGP: check it has geometry/batches); server vmaps converted both. Stock obj0 restored from /tmp/stock32, generated dir + fdid lines removed, vmaps re-extracted for map 0. Collision/LOS walk test not done. 2026-10-08 latest: the 'only sky' blocker was NOT the client: autologin picked the last-played character (Wefwefwefa, guid 13, z=-12689, falling in the void); reset its position and the world renders. Installed obj0 (patched) + obj1 + 4 e2e files + fdid lines in .client; the generated files carried stale inner ids (m2 SFID 2000000010 -> 33 at byte 984, wmo GFID 2000000012 -> 35 at byte 222): patched in .client/Content/world/generated, vmap extraction for map 0 then converts them (only a stock unrelated model FILE00028CD9 stays unreadable). The sandbox export must write the real ids inside the files too. Still to do: look at the panels in-game (-8930,-130,92)/(-8915,-130,92), collision/LOS check, restore stock client files. autologin ignores the character arg (always guid 13 on CLAUDE). 2026-10-08 later: id collision sidestepped by allocating real ids 2000000032..35 (database_tool fdid allocate on a scratch copy of the real .world) and patching obj0 FDIDs 2000000009/11 to 32/34; installed into .client and fdid.index, but the client then showed only sky with the character falling, even with stock 32_48 files, stock fdid.index, and re-extracted server tile 48,32 (+ full map 0 vmap tree; a --tile-restricted vmap run leaves a partial 000.vmtree that the server rejects, always run the vmap stage for the whole map). Cause not found; custom.dll was rebuilt at 00:46 by something else. Client files are back to stock, /tmp/stock32 has the 32_48 set.) (2026-10-08: found+fixed CreatePanel*/CreatePond committing the model as a DELETE (CommitAsync(saves, deletes) args swapped) so export gathered no procedural placements; now panels export `world/generated/e2e m2 panel_4.m2` etc. NOT yet installed in the client: the sandbox DB's file_data_id custom ids (2000000003..) collide with the real table's (cubesword/cubehelm use them), so copy the real custom rows into the sandbox table first (or re-export against a sandbox copy of the real .world) before installing tiles + generated + fdid.index into .client. Also: export source map name is case-sensitive: use Export("Azeroth","Azeroth"), lowercase only finds 31_49/32_49; stale TP6/TP7/TestWmo/pond2 entities in the sandbox DB reference missing models (ids 1,4)): an M2 panel/road and a WMO wall in Northshire render in the client;
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

### Step 1.4 findings (2026-10-08, third client look)
- Reinstalled panels (ids 32..35, obj0 patched, inner ids patched); stock restored afterwards. Server vmaps were stock (no collision) because I did not re-extract.
- Placements in obj0 are fine (MDDF/MODF + MCRD/MCRW refs present). The WMO is a FLAT 12x12 quad at z=92 (bbox height 0), the M2s at z=85.5 and 92; only a dark flat patch at ground level was seen, nothing overhead at (-8915,-130,92) even looking straight up.
- Differences vs stock WMO: root lacks MOSB/MODS/MFOG/MOLT/MOPV.. chunks (stock cameron.wmo has them), group header FogIds[0]=0 with no MFOG, no MOCV, flag 0x2000 unset. Next: add MFOG/MODS/MOSB, try a 3D box instead of a quad, then retest. Debug-test visibility by placing at ground level rather than 11 above.

### Step 1.4 findings (2026-10-08, fourth client look)
- Launching via autologin.sh/launch-wine.sh re-exports the db2s AND rewrites .client/Content/fdid.index from .world, dropping manually appended lines (earlier looks never had ids 32..35 in the index). Use `NO_DB2_EXPORT=1 client/autologin.sh` after patching fdid.index; the client log then shows `name table: 153622`.
- Even with the index right, nothing shows: replacing the panel FDIDs with stock ones (189929 doodad, 107243 WMO) at the same MDDF/MODF positions also shows nothing; stripping empty MMDX/MMID/MWMO/MWID and emptying MCNK did not help; uniqueIds 0x7000000x -> 900000+ did not help; heights 92 -> 86 did not help. The dark patch on the ground is present independent of the panels (probably terrain). So the exported obj0 is the likely culprit, not the models.
- Exported obj0 vs stock: stock lists every doodad in EVERY MCNK its bbox touches (MCRD), exported lists each in fewer chunks (e.g. chunk 13: stock 8 refs, exported 1), so stock objects may also be culled wrongly; stock MCRW is already present. Next: make the writer's MCRD/MCRW cover all touched chunks (or compare a stock-only re-export of obj0 round trip), and test obj0 round trip first with NO new placements.
- Client is off, stock obj0 restored, generated dir removed.

### Step 1.4 (2026-10-08, fifth session)
- Committed 1dd60c68e: RebuildRefs lists doodads in all chunks within 15 units (stand-in for model bounds).
- Sandbox export (`wms.wowImport.Export("Azeroth","Azeroth")`, no await; poll `wms.batch.Status(id)`) compares against the file already in the overlay `/tmp/wmsdbg/content/world/maps/azeroth`, so a second export writes "0 edited tiles". To re-export: copy `/tmp/stock32/azeroth_32_48*` over the overlay first. Done once: new obj0 (44956 bytes, new refs) is now in the overlay, NOT yet installed in .client.
- wms-test.sh with a scratch config never becomes ready (20 min); use `--attach --port 8799` on the sandbox editor (no rebuild, so it tests the old assembly unless the editor was restarted: `/tmp/wmsdbg/up.sh` rebuilds).
- Next: install overlay obj0 + generated + the 4 fdid lines into .client (patch obj0 FDIDs to 32/34, inner ids m2 byte 984, wmo byte 222), `NO_DB2_EXPORT=1 client/autologin.sh`, look at panels; else round-trip test.

### Step 1.4 ROOT CAUSE (2026-10-08, sixth session)
- The 1.15 client draws doodads from `_obj1` MLDD (and MLMD for LOD WMOs), NOT from obj0 MDDF: setting all obj0 MDDF heights to 5000 changed nothing, doing it to obj1 MLDD made every tree/lamp vanish (collision wireframes stayed). obj0 is still required (missing file = streaming error WOW51900322 at login).
- Hand-built obj1 (stock + MLDD/MLDX/MLDL entries for the panels, MLMD/MLMX for the WMO, MLFD lengths bumped; MLDX/MLMX bounds are WORLD coords: wx=17066.667-fileZ, wy=17066.667-fileX, wz=fileY) made a panel shadow appear on the ground; the panel itself not yet seen (camera could not be tilted up: `/run` via /batch did nothing).
- Implemented `AdtObj1Patcher.AddMissing(obj1, obj0)` (+test, 44/44 WoW Export pass) and call it from `WowAdtTileWriter.Write`; not yet exported+installed. Client is back to stock (obj0/obj1 from /tmp/stock32, generated dir and fdid lines removed).
- Next: re-export (copy /tmp/stock32/azeroth_32_48* over the overlay first, `wms.wowImport.Export("Azeroth","Azeroth")`), install obj0+obj1 (patch FDIDs 9->32, 11->34 in obj0/obj1; inner ids m2 byte 984 -> 33, wmo byte 222 -> 35) + generated + 4 fdid lines, `NO_DB2_EXPORT=1 client/autologin.sh`; the char (guid 13) stands at (-8922,-127,81), panels at (-8930,-130,92) (above and 8 away: move the character e.g. onto z=92 or look up). Check Write really has the obj0 in fs (AddMissing is skipped when it is absent).

- 2026-10-08 (step 1.4, sixth check): export with AddMissing ran in the sandbox (obj1 54032 -> 54372, MLDD +3, MLMD +2; obj0 44956); installed (FDIDs 9->32, 11->34, inner ids 33/35, 4 fdid lines), character at (-8930,-148..-130, 81). Camera pitched via `/run MoveViewUpStart(1)` (click Yes on the custom-script prompt at client px (774,228) after every login; type after `enter` + 400 ms wait; MoveViewUp overshoots to straight down). Still NO panel visible (neither M2 at z 92 nor WMO wall); character stands on ground. Client files restored to stock. Next idea: check whether MLDD/MLMD ids for FDIDs >= 2000000000 are accepted (client may drop unknown/negative-id entries), try a stock FDID model at the same position through the same obj1 path to isolate whether the writer or the custom model is at fault.

### Step 1.4 (seventh session, 2026-10-08)
- Stock obj1 MLDD entries always have flags 0x20 set (u16 at +34; ours only 0x40), stock MLMD flags 0x0c (ours 0x08). AdtObj1Patcher now ORs 0x20 / 0x04 into added entries (committed). Installed obj0/obj1 (flags forced) + generated + fdid lines via /tmp/inst.py with `NO_DB2_EXPORT=1 client/autologin.sh`: still no panel visible from the start position, but the camera could not be pointed at (-8930,-130,92) (`/batch` takes `{"actions":[...]}`, typed /run did not execute). Client files restored to stock, guid 13 back at Northshire start.
- Next: isolate with a stock FDID model in the same obj1 entry (e.g. 189929 doodad at the panel position), and put the character at ground level right below/near it with a screenshot of the sky; or compare MLDL/MLFD semantics (MLFD counts: ours {5,5,0,5}).
- RESULT (user screenshot, fly mode): with the flag fix (0x20 MLDD / 0x04 MLMD) the WMO panel IS drawn: a flat black square floating in front of the abbey door. So obj1 entries needed those flags. Black = untextured material. The M2 panels were not identified yet. Next: texture the WMO material, check M2s, vmap collision (step 1.5).

### Step 1.4 (eighth session, 2026-10-08)
- State check only: client off, stock files in place, /tmp/inst.py and /tmp/stock32 still exist, sandbox editor (port 8799) running. WMO panel visible (black = untextured). Remaining: textured WMO export, M2 panel visibility, vmap collision/LOS (step 1.5), then step 1.5 new map.

### Step 1.4 (ninth session, 2026-10-08)
- WMO panel renders TEXTURED in the client: the e2e panel's MOMT Texture1 was 0 (black = no texture), not a writer bug. Patched Texture1 to stock FDID 189080 via /tmp/inst.py: hazard-stripe panel floats near the abbey door. Writer already emits Texture1 from the surface texture path.
- Viewing tips: control server `/batch` actions are `{"key":{"keys":"escape"}}`, `{"drag":{"from":[x,y],"to":[x,y],"button":"right"}}` (right-drag turns/pitches the camera, ~100px = ~30deg pitch). Put the character offline-in-DB at (-8944,-130,83.6) orientation 0 to face the abbey door/panel.
- Client restored to stock (obj0/obj1, generated, fdid lines); character back at Northshire start. Remaining: M2 panels not identified, vmap collision (step 1.5), then new map.

### Step 1.4 (tenth session, 2026-10-08): M2 panels
- Tried the M2 path with a STOCK model (189929 canopy) in the added obj1 MLDD entries at the panel positions, with flags 0x260 (stock) and small uids: still nothing drawn. So the problem is the added MLDD entries (or how doodads are culled), not the custom M2 model. WMO via MLMD works.
- MLDD entries have MDDF layout (rot floats at +20/+24/+28, scale u16 +32, flags u16 +34); stock rot y is e.g. 232, ours -180 (probably fine). MLFD = m2 offsets[3], m2 lengths[3], wmo offsets[3], wmo lengths[3].
- Ideas not yet tried: move an existing stock MLDD entry (edit pos+MLDX) to see if edits show; check whether doodads draw from obj0 MDDF only within a near range (obj0 5000-height test may have been masked); try MLDL non-zero values / other LOD level for the added entries.
- Camera: right-drag pitch is unreliable (goes to first person); `scroll` action {"scroll":{"x":..,"y":..,"delta":N}} exists. Escape opens the game menu once chat is closed.
- Client restored to stock, character 13 back at Northshire start.

### Step 1.4 (eleventh session, 2026-10-08): ROOT CAUSE of invisible added doodads
- Client ignores MLDD entries APPENDED at the end of the array (stock model, correct flags/bounds: not drawn). Replacing a slot or INSERTING at the front draws them. AdtObj1Patcher.AddMissing now inserts new doodad entries at level 0's start (committed, 44/44 WoW Export tests pass). WMO (MLMD) appends work.
- With the new obj1 and the exported custom M2 (fdid 2000000032) the client hangs on the loading screen (connected, never enters world). Swapping that FDID for stock 189793 in obj1 loads fine and the doodad panels are drawn at the panel positions. So the custom M2 panel file (generated/e2e m2 panel_4.m2 + skin, inner id patch 984->33) makes the client hang: next, diff its M2 structure against a stock M2 (chunks, header version, skin/SFID, bounds).
- Install with /tmp/inst2.py (inst.py without flag forcing) + `NO_DB2_EXPORT=1 client/autologin.sh`. Never run plain autologin during an install (rewrites fdid.index). Client restored to stock.

### Step 1.4 (twelfth session, 2026-10-08): custom M2 hang FIXED
- Cause: M2 header skin-profile count (u32 at MD20 offset 68) was 0 although SFID lists a skin; client hung on load. M2ModelWriter now sets it to 1 (test added). Patched installed copy (inst4.py) loads fine. Texture FDID 0 was not the cause. Panel visual not checked (client control server hung); client restored to stock, character 13 reset.
- Next: build+run WoW Export tests, re-export, view M2 panels; then step 1.5 vmap re-extract.

### Step 1.4/1.5 (twelfth session, later)
- Fresh sandbox export (fixed M2 writer) installed WITHOUT manual patches loads into the world. Panels at (-8930,-130,92): USER TO CHECK VISUALLY LATER (client left running + installed; control server on 8790 hung, no screenshots). Restore stock afterwards: copy /tmp/stock32/azeroth_32_48{,_obj0,_obj1,_tex0}.adt into .client/Content/world/maps/azeroth/, rm -rf .client/Content/world/generated, `sed -i '/^20000000[0-9][0-9];/d' .client/Content/fdid.index`, reset character 13 to -8949.95,-132.493,83.53, kill client (script file, no inline pkill).
- `map_extractor --output /tmp/sdata --stages vmap --map 0` (scratch output) converts both panel models (FILE77359420.m2.vmo, FILE77359422.wmo.vmo). Not yet run on live .server/data nor tested for collision/LOS in the server.

### Step 1.4 (thirteenth session, 2026-10-08)
- ROOT CAUSE of vanishing distant trees: stock obj1 MLDD/MLDX/MLDL are sorted by bounds radius DESCENDING; front insertion broke the order. AdtObj1Patcher.AddMissing now inserts at the sorted position (committed 976152a6d + offset fix). Trees are back (user confirmed); WMO panel shows.
- M2 panel exported with TXID 0 (the e2e panel has no texture): patched TXID to 189080 in .client generated m2 to test; awaiting user answer. wms-test.sh with scratch config hangs; use `--attach --port 8799` after /tmp/wmsdbg/up.sh.
- Client currently running with the installed panels; restore stock afterwards (see twelfth-session note).

- 2026-10-08: M2 panel fixed (commit 91f8ee5c0: seq bounds, skin boneInfluences/BoneCountMax 0, flags 0x202080): renders as flat dark textured quad in client. Its TXID is still 0 from the e2e panel (test-only). Client restored to stock, character 13 reset. Next: step 1.5 (vmap extract of map 0 on live .server/data with panels installed, collision/LOS), then new map.

### Step 1.4/1.5 (thirteenth session)
- Re-exported with the fixed M2 writer, installed (inst2.py), full map 0 vmap extract (5 models converted, WMO panel vmo has a 2-triangle quad). In the client the WMO panel renders textured; `.gps` (works from the client chat via `/batch {"actions":[{"key":{"keys":"enter"}},{"type":".gps"},...]}` for CLAUDE) reports GroundZ/FloorZ 92.0 with VMap data at the panel: server collision height sees the panel. LOS block not tested. Client restored to stock, character 13 reset.

### Step 1.6 new map (started, thirteenth session)
- Sandbox: `wms.batch.Run("wow.export.map",{donorMap:"Azeroth",directory:"smangostest",name:"Smangos Test"})` (omit mapId: a 0 double fails GetValue<int>) created map 3000 in the sandbox DB (dbc_map/map_template/game_tele + WDT). Then `wow.export.adt {sourceMap,targetMap:"smangostest",donorMap:"Azeroth",newTiles:"32,32;33,32;32,33;33,33"}` failed on the missing WDL; fixed (WowWdlWriter.Write skips maps without a WDL, uncommitted until the commit below) and re-run. Overlay now has smangostest_{32,33}_{32,33}.adt (+obj0/obj1/tex0, 302 KB each) at /tmp/wmsdbg/content/world/maps/smangostest; check the WDT is there (a 2nd run says "no imported tiles" because the WDT already has the tiles).
- Next: install overlay into .client (maps/smangostest + wdt + fdid.index lines), `database_tool export --format db2 --client` for the dbc_map row, map_extractor --map 3000 (map,vmap,mmap), server needs map 3000 in world DB (map_template), `tele name` smangostest, walk, compare heights; tile removal.
- 2026-10-08 (14th session, step 1.6 install): map 3000 server data extracted fine (`map_extractor --world .local/dtroot/.world/world.doltlite --map 3000`: 4 map tiles + mmaps, no vmaps needed). Scratch root `.local/dtroot` (copy of sandbox world.doltlite + symlinks; /tmp is full, don't copy there) used with `DATABASE_TOOL_ROOT=$PWD/.local/dtroot database_tool export --format db2 --client --tables Map` -> its map.db2 has row 3000. `.local/instmap.sh` installs map.db2 + fdid lines (WDT/ADT/obj0/tex0 lines come from the sandbox index, obj1 + lgt/occ/fogs/mpv lines are hand-added ids 26..33: the exporter doesn't allocate FDIDs for new tiles' obj1 or the companion wdts, fix it) + copies the overlay. Server test: set `WorldDatabase.Info` in .server/config/common.conf to the scratch DB, `tele name Wefwefwefa smangostest` (server accepts, char lands at 0,0,0 on map 3000).
- BLOCKER: client shows 'streaming error WOW51900322' / Error txt `Bad zone ID 3000`: new blank tiles have AreaId 0 on all chunks (WowBlankTile.Create areaId param unused), so zone falls back to the map id, which has no AreaTable row. Fix: wow.export.map should create an AreaTable row (ContinentID=map, parent 0, name) and the export should pass its id into WowBlankTile.Create; then re-export AreaTable.db2 too (and server dbc_areatable). Never run plain `client/autologin.sh` during an install (it re-exports db2 + fdid.index from the real .world and wipes the install lines): use NO_DB2_EXPORT=1.
- State restored: client map.db2 stock, smangostest files + index lines removed, config reverted, guid 13 back in Northshire.
- 2026-10-08 (15th session): fix committed (2a91cc6c3): wow.export.adt now finds/creates a dbc_areatable row (ContinentID = map id, parent 0, id >= 90000) for maps with new tiles and passes it to WowBlankTile.Create. 'WoW Export' 44/44 pass. NOT yet retested in the client: restart the sandbox editor (/tmp/wmsdbg/up.sh), delete the overlay smangostest tiles + remove tiles from sandbox WDT state (or re-run wow.export.map), re-export tiles, regenerate AreaTable.db2 + Map.db2 (`database_tool export --format db2 --client --tables Map,AreaTable` with DATABASE_TOOL_ROOT=.local/dtroot), install with .local/instmap.sh (add AreaTable.db2), NO_DB2_EXPORT=1 autologin. obj1/companion-wdt FDID allocation for new tiles still open (WriteNew writes no obj1; instmap.sh hand-adds lines).
- 2026-10-08 (15th session, retest): fresh map 'smangostwo' id 3001 via sandbox (area 90000 created, db2s Map+AreaTable exported from .local/dtroot, server data extracted, server pointed at dtroot via `WorldDatabase.Info = "../.local/dtroot/.world/world.doltlite"` (relative to .server)). `.local/instmap.sh DIR` / `.local/uninstmap.sh DIR` install/restore (sandbox fdid ids 26+ collide with real ones, script strips colliding index lines; backup in .local/instbak). RESULT: client STILL shows streaming error WOW51900322 on teleport, even with all MCNK areaIds patched to stock area 12, so the area was not (the only) cause; no Error txt written this time. Suspects: WDT (companion lgt/occ/fogs/mpv ids copied from donor, obj1 ids 0 in WDT), tile files vs donor, map.db2 row. Next: diff smangostwo.wdt/ADTs against a stock small map; try a WDT with the donor's MAID layout; check client Connection.log. State restored: client files stock, config reverted, guid 13 in Northshire.
