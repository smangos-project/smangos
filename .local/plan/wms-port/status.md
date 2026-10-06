# WMS port status

## Session 4 (2026-10-06)
- B5 done as script-only: `wms.adtHoles` (Cells/Cut/Fill/CutAt/FillAt), `Landscape/AdtHoles`. No brush tool.
- C2 done (removal only): `Export/Adt/AdtObj1Patcher`. Stock `_obj1` = MLFD (12 u32: doodad level offsets[3], lengths[3], WMO offsets[3], lengths[3]),
  MLDD (36B, same as MDDF, uid at +4, different order), MLDX (28B bounds), MLDL (4B), MLMD (40B, uid +4), MLMX (28B). Export drops removed uids and fixes MLFD ranges; added placements get no LOD entry.
- Next: D (water, MH2O), E, F, G, H per next-steps.md. Live-editor import check (A2/A3) and all client verification still pending by user's choice.

## D (water) findings, not started in code
- Core already has the procedural stack (`WorldMapStudio/src/Procedural`, `Network`, `PolygonTriangulator`, `ProceduralOutputBuilder`, `MeshParameter`), so CataBox's `Water/*` ports on it.
- Our `WowAdtImportBatchOperation` already has a comment about water models going into the same commit (~line 588), but no water code; `ImportedChunk.Liquid` (ImportedChunkLiquid) is read by `WowAdtTileReader.ConvertLiquid`.
- CataBox source to port (CB/...): `Import/Mapping/WaterMapper.cs` (pure, needs `WowLiquidRasterizer.SubchunksPerCell` and `WowChunkGridOrientation.ToEditorGrid`; reads MapChunkLiquid -> bodies of quads),
  `WowAdtImportBatchOperation` water parts (BuildWaterModelsAsync/BuildWaterEntities/DeleteExistingWaterAsync, importWater setting, version bump),
  `Water/WaterSurfaceMeshFunction`, `WaterLatticeSnap`, `WaterAttributeParameters`, `WowLiquidParameters`, `WowLiquidMaterialType`, `Mh2oFormat` (Rendering/Water), `WowLiquidRasterizer` + `Mh2o/Mh2oChunkWriter`, `Export/Adt/WowWaterTileBuilder`, `WowMh2oExportChannel`.
- Our wowlib already has MapChunkLiquid/LiquidInstance/Mh2OData, so CataBox's own `Water/Mh2o/*` copies are not needed.
- Suggested order: WaterMapper + tests -> liquid parameters/material (1.15 LiquidType/LiquidMaterial) -> surface function + import wiring -> rasterizer -> export (ADTBfaPlus.Water).

## Session 4 - D (water)
- D1 done: `WaterMapper` (+tests), `WowChunkGridOrientation.ToEditorGrid`, `WaterSurfaceMeshFunction`, liquid params/lattice snap/`Mh2oFormat`, `WowLiquidMaterialType`, `LiquidTextureLibrary` (frames from `LiquidTypeXTexture` FDIDs via `LiquidType.FrameFileDataIds`, lake_a fallback), tint updater. Import wired into `WowAdtImportBatchOperation` (importer v6, "Import water" setting, re-import deletes old water). Elwynn 32,48 imports 2 bodies on a scratch DB copy; WoW Import tests pass (34).
- Not verified: rendering of the water in the viewport (`wms.scene.All()` is empty after import+Reload+Map.Open in a scratch run, the known live-editor issue), shader/depth-buffer behaviour on 4.x.
- Left in D: D3 `RiverSurfaceMeshFunction` (CataBox Water/RiverSurfaceMeshFunction.cs), D4 export (`WowLiquidRasterizer` + `Mh2oChunkWriter` onto wowlib `Mh2OData`; `WowAdtTileWriter` has no MH2O yet; need export-channel plumbing or direct gather of ProceduralComponent water models), D5 server liquid preview.

## Session 5 - D4/D5
- D4 done: `WowLiquidRasterizer`, `WaterSurfacePatch`, `WowWaterTileBuilder`, `WowWaterPatches`; `WowAdtTileWriter.ApplyWater` rewrites only cells whose liquid differs from the file. Caveats: HeightUv UVs are written as zeros; if the map has no authored water at all, MH2O is left untouched (deleting all water doesn't strip it).
- D5 done: `WowServerLiquidMesh` + "Liquid" toggle in the server preview header item (one quad per wet cell from `maps/*.map`). Only the 0002035 fixture is available (no 0002537/0004830 fixtures yet).
- Still open: live-editor empty scene diagnosis, visual water check, E, F/G, H.

## Session 6 - E3/E4 and empty-scene diagnosis
- E3: export setting "Removed tiles" (drops overlay files, clears WDT MAIN/MAID). E4: `wow.export.map` batch op (WDT from donor without tiles, dbc_map + map_template + game_tele rows, ids from 3000); export op has "Donor map for new tiles". Untested end to end.
- Empty scene: NOT a bug. On a scratch copy, import 32,48 + `wms.editor.Reload()` + `wms.map.Open(1)` + `wms.viewport.SetPosition(-8800,650,266)` + `wms.wowMap.WaitForStreaming()` gives 255 entities (terrain, m2/wmo, creatures, water "ADT Water 32_48 #1"); screenshot shows terrain with trees. Entities stream in asynchronously, so `scene.All()` right after Open is empty. Water not yet inspected up close (Focus(entity from All()) fails; use look.sh with a Create()d entity).

## Session 7 - F1/F2
- F1/F2 done (minimal): `Component/WowSpawnModelComponent` + `SpawnAppearances` replace the point marker on creature and gameobject spawns (same marker type ids). Creature = `creature_template.display_id1` (+`display_scale1`) -> CreatureDisplayInfo -> CreatureModelData FDID; gameobject = `GameObjectDisplayInfo.FileDataID` x template size. Placeholder box until loaded/unresolved. Checked in the live editor at Northshire (NPC and objects render). Not done: NPCs with `CreatureDisplayInfoExtra` (F5), equipment, display_id2-4 variants, thumbnails.
- Next: F3 light preview, F4 spell_target_position, F6, G catalogs, H.

## Session 8 - F4
- F4 done: `WowSpellTargetPosition` entity (+factory, inspector, view category) on `spell_target_position` (key id + target_map, no effect index). Create doesn't validate the map (Stage does). Commit path untested against a DB. Next: F3 light preview, F6, G, H.

## Session 8 - F6 (partial)
- `wow.extract` batch op (`Server/WowExtractBatchOperation`): map, optional tile, stages, force, dry run, wraps `WowServerExtractor`. Compiles, not run through the UI. Dry run now logs a staleness summary. Not done: tile-rect (map_extractor only has `--tile`), drift layer exists already. Next: F3 light preview, G, H, F5.

F3 done: LightParams catalog (read-only, LightData time keys -> bands), WowLightComponent env source auto-attached to WowLight, WowSkyEvaluation, WowOutdoorLighting; light falloff now /36. Weather slots, band editing, zone light env source not ported. Not seen in viewport.
- G1: `Creature Template` / `GameObject Template` catalogs (ThinDbcFactory keyed by Entry, common fields only, no palette UI in spawn tools yet).
- G2/G3: `Quest` (quest_template) and `Item` (dbc_itemsparse, edit only) catalogs.
- Next: H1/H2 procedural export (CataBox Export/Models: ProceduralModelExporter, M2Writer, WMO writers, WowExportChannelSystem, WowModelExportChannels; port onto wowlib writers), then F5.

## Session: H1 research (no code)
- Do NOT copy CataBox M2Writer/M2SkinWriter: they emit WotLK MD20 (v264) + external skin; the 1.15 client loads chunked MD21 + SkinCataPlus, and our M2Reader reads exactly that. Build with wowlib instead.
- wowlib C# (net8, WoWLib.dll 0.0.11) write path: `M2RootLegionPlus` (Vertices, Textures, Materials, TextureLookupTable, BoneLookupTable, BoundingBox, ... all `WoWLib.Vector<T>` with Add/Clear/CopyFrom), `SkinCataPlus.Profile` (`M2SkinProfileCataPlus`: Vertices, Indices, Submeshes, Batches), `M2ChunkedFile.ForVersion(ClientEra)` (SkinFdids, TextureFdids). Unknown yet: how MD21 chunk + chunked file combine on Write, and the submesh element type (reflect with /tmp/refl.fsx: `dotnet fsi refl.fsx <type>`; may be gone).
- Alternative: start from a template model like the Python addon (m2_export.new_m2 copies a small doodad then rewrites geometry) and call `M2.Write(fs,key)`.
- ProceduralModelExporter, WowExportChannelSystem, WowModelExportChannels, WowExportRun are still to port (reference Export/). WMO writers (WmoRootWriter/WmoGroupWriter/WmoBspBuilder) should likewise target 1.15 WMO via wowlib; see tools/model_editor/addon/wow_model_editor/wmo_*.py for the working Python equivalent.

- H1 started: `Export/Models/M2ModelWriter` (wowlib `M2RootLegionPlus` + `SkinCataPlus` + `M2ChunkedFile`, MD21 assembled by hand) with round-trip tests through M2Reader; WoW Export tests pass (37). Wowlib `M2Vertex`/`M2Material` clash with ours: use aliases. Skin `Bones` take a `FixedArray<byte>` copied from `M2Vertex.BoneIndices`. Still to port: ProceduralModelExporter, export channel system, WMO writers.
- H1: `ProceduralModelExporter` (M2 only; takes a `WowFileDataIds`, allocates ids for `.m2` and `00.skin`, writes the skin id into the chunked file's SFID, texture strings resolved via `IdOf`). Tested with a live ArrayMesh. WMO output, export channel system (`WowExportChannelSystem`, `WowModelExportChannels`, `WowExportRun`) and WMO writers still to port.
- H2: no channel registry (our export already gathers per tile). `WowProceduralPlacements.Gather` turns procedural entities whose output format is M2 into generated files (via `ProceduralModelExporter`) + `EditorPlacement`s (keyed by entity RecordId) that the ADT export feeds to `PlacementExtractor`. Compiles, existing tests pass, NOT run end to end. WMO output still needs the WMO writers.

## E2E check of procedural M2 export (in progress, not done)
- Sandbox made so the real client data isn't touched: `/tmp/wmsdbg/content` = symlinks to `.client/Content` except `world/maps/azeroth`, where the three imported tiles' ADT files (32_48, 31_49, 32_49) and wdt/wdl are real copies; `/tmp/wmsdbg/project.json` points `wow.clientContent` there and the editor DB at the scratch `/tmp/wmsdbg/world.doltlite` (map 1 "Azeroth" has those 3 tiles imported). Start with `/tmp/wmsdbg/restart.sh`, script port 8799, `/tmp/wmsdbg/js.sh`. /tmp may be gone; rebuild the same way.
- Blocker: no script API creates a procedural model (catalog Add) and the live context isn't reachable from [EditorTest]. Next step: add a small `[ScriptFunction]` (or temp one) that creates a `ProceduralModel` (function `builtin.mesh.panel_network`, Formats `{"mesh":"wow.format.m2"}` via `ProceduralFormats`, a VertexNetwork with one face, texture param set on the surface material), adds a MapSceneEntity with `ProceduralComponent` on map 1 near tile 32,48, commits, then `wms.wowImport.Export("azeroth","Azeroth")` and check `/tmp/wmsdbg/content/world/generated/*.m2` + the tile's obj0 MDDF, then load via the client only after installing deliberately.

## H2 end-to-end check (done)
- `wms.wowImport.CreatePanelModel(name, size, texturePath, x, y, z)` creates + persists a panel_network ProceduralModel (M2 format) and places an entity in the open session (then `wms.session.Commit()`). Export wrote `world/generated/<name>_<id>.m2` + `00.skin`, and tile 32_48 obj0 MDDF gained entries with the custom FDID (787 vs 785 placements). Not loaded in the real client yet.
- Gotcha: wms-run.sh rebuilds only when not `--no-build`; check `dotnet build` actually succeeded or the old binary keeps running (arg-binding errors "No public methods" = stale binary). Never `pkill -f godot-mono` inline.
- Still open: WMO writers (WMO output format), F5, palette UI, procedural placements not removed from earlier scratch runs.

## Palette UI (already covered)
- Spawn menu picker is core (`SpawnMenu.Browse` via `ISpawnFactory.PickerCatalogName`); creature and gameobject spawn factories already return "Creature Template"/"Game Object Template", so the palette exists. No work left there.
- Remaining: WMO writers (port wmo_*.py onto wowlib), F5 (CreatureDisplayInfoExtra appearance).
- F5 (minimal): CreatureAppearanceResolver binds CreatureDisplayInfoExtra.BakeMaterialResourcesID -> TextureFileData FDID onto the Skin slot. Not seen in viewport; equipment/hair layers not done.
