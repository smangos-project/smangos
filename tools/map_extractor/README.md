# map_extractor

Builds the server's `maps/`, `vmaps/` and `mmaps/` from the extracted 1.15 client (`.client/Content`, see `casc_extractor`).

    build/tools/map_extractor/map_extractor [--input .client/Content] [--output .server/data] [--stages map,vmap,mmap]
        [--map ID,...] [--tile R,C | --tiles R1,C1,R2,C2 | --around X,Y,RADIUS] [--threads N] [--force] [--dry-run]

`R,C` are the numbers in `MMMRRCC.map`: Northshire Abbey is `--map 0 --tile 48,32`. Run `--help` for all options.

## Stages
- **map**: ADT -> `maps/MMMRRCC.map` (z1.4). Full run takes seconds.
- **vmap**: WMO/M2/obj0/WDT -> raw models in `Buildings/`, assembled per map into `vmaps/` (vmtree, vmtile, vmo). A range run reassembles the map from only the selected tiles, so use the whole map for a real server.
- **mmap**: Recast/Detour via `contrib/mmap` -> `mmaps/MMM.mmap`, `MMMRRCC.mmtile`. Needs the map and vmap output of the tile and its 8 neighbours. About a minute per dense tile per core, so a full run takes long; use ranges. Config defaults to `contrib/mmap/config.json` and `offmesh.txt` (`--mmap-config`, `--offmesh`). Transport/elevator gameobject meshes (`go*.mmtile`) are not built.

## Cache
`.cache/` under the output holds fingerprints of input file stamps and options per tile (and per map for vmaps). Unchanged work is skipped; `--force` redoes the selection, `--dry-run` lists what would be redone and why.
mmap tiles are redone when the map or vmap files of their 3x3 neighbourhood, the vmtree, config, offmesh file or `--skip-liquid` change.

## Known gaps
Parent maps (`Map.ParentMapID`) are not handled; one model (`FILE00028CD9.m2`) fails to convert and is reported as unreadable.
