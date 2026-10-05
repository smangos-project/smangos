# WoWMapStudio

WorldMapStudio plugin for editing the smangos world: the `.world/world.doltlite` database, client assets and terrain.
It has no project file of its own; the core compiles `WorldMapStudioPlugins/**/*.cs`.

Run and test with `../wms-run.sh` and `../wms-test.sh`.

## Contents

- `Models/`, `Export/`: coordinates, tile addressing, placement transforms, client/server data paths.
- `WorldMap/`, `Viewport/`, `Component/`: world map projection, go-to header items, point markers.
- `Server/`: readers for the extractor output the server loads, on the vmangos formats: `.map` (`WowMapTile`), `.mmap`/`.mmtile` (`WowNavMeshTile`), `.vmtile`/`.vmo` (`Collision/`), plus the preview mesh builders. Golden fixtures live in `src/Server/Fixtures`.
- `Database/`: `WorldStorage`, `Models/WowMapBindings` (map to continent bindings).

## Not yet ported

Catalogs and generated DB records (Phase 2), asset formats (Phase 3), terrain/export, the Server preview layers and staleness checks (Phase 6).

## Naming

Tables owned by this plugin in the editor DB are prefixed `wms_wow_` (CataBox used `wow_`).
