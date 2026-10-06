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
- `wow.extract` batch op (`Server/WowExtractBatchOperation`): map, optional tile, stages, force, dry run, wraps `WowServerExtractor`. Compiles, not run through the UI. Not done: staleness status header, tile-rect (map_extractor only has `--tile`), drift layer exists already. Next: F3 light preview, G, H, F5.

F3 done: LightParams catalog (read-only, LightData time keys -> bands), WowLightComponent env source auto-attached to WowLight, WowSkyEvaluation, WowOutdoorLighting; light falloff now /36. Weather slots, band editing, zone light env source not ported. Not seen in viewport.
