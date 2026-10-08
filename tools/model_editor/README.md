# model_editor

Blender addon (WoW 1.15 `.m2`/`.wmo`) on top of wowlib. Phase 1 was the toolchain spike; phase 2 the addon skeleton, textures and read-only WMO import; phase 3 static M2 import (animation and all export come later).

## Build wowlib

    tools/model_editor/build-wowlib.sh          # wheel -> .build/wheels (~12 min cold)
    tools/model_editor/build-wowlib.sh --tests  # also runs wowlib's C++ ctest first

Clones wowlib at the pin in the script into `.build/wowlib`, applies `wowlib-patches/*.patch`, and builds the
cp313-abi3 wheel with gcc 16 (`gcc-16`/`g++-16` are symlinked from `gcc`/`g++`). DB tables are trimmed to the
`tww` era (1.15 files follow the TWW format lineage); the Python tests in `tests/python` pass (139) except
`test_db_tables`, which needs every era.

Patch `0001-loose-directory-storage.patch` adds `LooseStorage` and `FileSystemSettings(..., loose_client=True)`:
`client_path` is then a plain extracted tree (`.client/Content`), paths are case-insensitive, the listfile is
`fdid.index`, and writes go to `project_directory` (overlay wins on later reads).

## Spike scripts (`spike/`, plain Python 3.13+, wheel unpacked into `.build/site`)

    mkdir .build/site && (cd .build/site && unzip -q ../wheels/*.whl)
    python3 spike/roundtrip.py creature/wolf/wolf.m2 world/wmo/cameron.wmo
    python3 spike/sweep.py [--limit N] [--json out.json]     # whole corpus, ~80 s
    blender -b --python spike/blender_check.py               # same round trips + numpy->mesh inside Blender

`spike/ext_check/` is the minimal extension (manifest with the bundled wheel); build with
`blender -b --command extension build --source-dir DIR --output-dir OUT` (wheel copied to `DIR/wheels/`, name patched
into `wheels = [...]`), install with `extension install-file -r user_default -e ZIP`. Use
`BLENDER_USER_RESOURCES=/tmp/x` to keep the user's config untouched.

## Results (Blender 5.2.2, Python 3.14.7, numpy 2.5.3)

- `import wowlib` works in `blender -b`, directly and through an extension-bundled wheel. The cp313-abi3 wheel
  loads on Blender's 3.14. This Blender links system libstdc++/libpython, same gcc 16 runtime as the wheel.
- WMO: read/write byte-identical to the shipped file for all 832 root WMOs that read. M2: all 10,211 readable M2
  survive read -> write -> re-read with deep structural equality (root, skins, tracks, NaNs equal) and a
  second write identical to the first. M2 bytes differ from the shipped file (wolf.m2 420,064 vs 464,252): semantic only.
- Failures (corpus 11,098 root files in `fdid.index` that exist on disk): 2 truncated/garbled files
  (`12elw_elwynn_main_tree_apple_d02.m2`, `12tr_amani_bridgewood_plank01.wmo`); 53 `*_lodN.wmo` group-LOD files are not
  roots and are skipped by the sweep. `validate()` reports errors on 36 shipped files (35 M2 `textureTransforms` index out of
  range, 3 WMO `light_index` out of range): real data quirks, so export must not hard-fail on validate.
- 1.15 M2s here have animations embedded (no `.anim`/`.skel`/`.bone` files next to them); humanmale has 143 sequences, 1 skin.
- numpy: WMO vertices/normals/indices are zero-copy structured/uint16 views (`.view(np.float32).reshape(-1, 3)`), texcoords
  and batches are copies/objects; M2 vertices are an opaque record vector (python loop, 3k verts ~6 ms). 50k-vertex
  mesh `foreach_set` takes 34 ms.

## Addon (`addon/wow_model_editor`, Blender >= 5.1 extension)

    tools/model_editor/build-addon.sh [--install DIR] [--dev]   # zip -> .build/dist; DIR = BLENDER_USER_RESOURCES
    tools/model_editor/test.sh [--all] [--filter NAME]          # fresh temp profile, headless tests; --all sweeps every root WMO

Build needs `.build/wheels/<wheel named in blender_manifest.toml>` (built by `build-wowlib.sh` on demand; bump the manifest
when the pin changes). `--install DIR --dev` swaps the installed extension dir for a symlink to the source, so Python edits
only need a Blender restart; the bundled wheel stays in the profile's `extensions/.local/.../site-packages`. Always use a
scratch `BLENDER_USER_RESOURCES` for agents and tests. Preferences: Content dir (default `$MODEL_EDITOR_CONTENT`), output
dir (becomes wowlib's `project_directory`), cache dir (BLP PNGs and the catalogue JSON, default `~/.cache/wow_model_editor`).

Use: 3D view sidebar tab "WoW" (search by path fragment or FDID, Import), or File > Import > WoW model. Imports a WMO as
`<name>.wmo` with `groups`, `portals`, `lights`, `doodads/<set>`, `liquids`, `fogs` collections. Raw fields live in
PropertyGroups (`wow_wmo`, `wow_wmo_group`, `wow_wmo_material`, ...), so export never parses node trees. Group meshes keep
the file's vertex layout; faces outside any batch use the transparent `wow_collision` material. Material nodes are an
approximation (shader id picks diffuse/two-layer/env, blend mode picks key/blend/additive).

`tests/render_model.py` renders an import (`blender -b --python tests/render_model.py -- OUT.png WMO [az el]`) for a visual check.

### M2 import (phase 3)

Imports `<name>.m2` as a collection with one mesh per skin submesh (`<model>_<geoset id>_<submesh>`, grouped under
`geosets/group_N`), an armature `<model>_rig` with weights, preview materials built from the batch stack, and
`attachments`, `events`, `cameras`, `lights`, `emitters` and a hidden `<model>_collision` mesh. Raw fields live in
`wow_m2` (collection), `wow_m2_submesh`, `wow_m2_bone` (Bone), `wow_m2_batches` (Material) and `wow_m2_object`; int point
attributes `wow_vertex` / `wow_skin_vertex` map vertices back to the file. Only skin 0 (or the chosen skin) is imported.

Creature skins come from the client db2s (`dbfilesclient/creaturedisplayinfo`, `creaturemodeldata`,
`creaturedisplayinfogeosetdata`, read by the small `db2.py`, since wowlib has no 1.15 schemas): import option Display
(Auto/None/ID) and the sidebar "WoW M2" panel switch the skin and geoset overrides. Character skins use the
`<model>skin00_00.blp` guess; other replaceable textures show the magenta placeholder. Default geoset visibility
(id 0, plus `group*100+1`) is a heuristic that has not been checked against the client.

WMO import option "Doodad meshes" shows each doodad as a collection instance of a mesh-only M2 kept in the hidden
`wmo_doodad_lib` collection (auto for WMOs with fewer than 500 doodads from the API; stormwind with all 6026 instances takes ~14 s).
Doodad colours are not applied. Preview materials approximate blending modes (mod/mod2x use alpha blend).

`test.sh --all-m2` imports every M2 in the catalogue.

### M2 animation (phase 4)

Sequences are a list on the collection (`wow_m2` sequences, with Play/loop UI in the Sequences panel); each sequence becomes
an action, global loops become loop actions. Bone tracks are fcurves on the `<model>_rig` pose bones (rest pose is identity).
Model tracks (colors, texture weights, texture transforms) and object tracks (attachment visibility, lights, cameras incl.
splines, particle emitters, ribbons) are stored as custom properties (`wow_color_i`, `wow_texweight_i`, `wow_tt_*_i`, ...) on
the armature or the owning object and animated through the same actions; values use fixed16/quaternion codecs matching the
file. Events are pose markers `event_<i>` on sequence/loop actions. Operators `wow.m2_sequence_add/duplicate/remove/key` and
`wow.m2_loop_add/remove` author sequences. Export builds the tracks back from the actions in memory; the round trip is tested
(`test.sh --filter m2_anim`, sweep with `SWEEP_STEP=8 ./test.sh --filter zzz --corpus`).

Limits / phase 6 risks: Blender light colour, energy and range and camera lens are driven (one way) by the `wow_*` properties; edit those, not the Blender data. Preview materials are not driven by
the colour/weight props; models without an armature carry only the model/object tracks; timestamps are milliseconds mapped
to frames at a fixed rate, so edits off the grid are rounded on export.

### WMO export (phase 5)
Select a WMO collection (or an object inside it) and use "Export WMO" in the WoW panel, or `wow.export_wmo`. Files are written to the Output directory; a file whose bytes equal the Content copy is not written, so an unedited export writes nothing.
- Groups are rebuilt (vertex split, batches, BSP) only when an array differs from the source; `Rebuild all groups` forces it. Normals come from the `wow_normal` attribute, falling back to corner normals for new vertices.
- Materials: edited `wow_wmo_material` fields update MOMT; a material without an index is appended. Texture nodes are labelled `texture1..3`; an image without `wow_fdid` is encoded to `world/wmo/custom/<name>.blp` and gets an id from `fdids.allocate` (the `file_data_id` table, ids from 2e9, `World database` preference to override). Edited (dirty) imported images overwrite their own path.
- Root objects: doodads (moved, retargeted, added to their set with group references), lights, fogs, portal vertices/planes and liquid heights/tile flags are written when they differ from the source. Deleted objects are kept in the file with a warning.
- Export checks: `w.validate()` issues and out-of-range material ids (errors/warnings), texture ids missing from every listfile and non-power-of-two new textures (warnings).
- New groups: a mesh object in the `groups` collection with no group index is appended as a new group (file `<wmo>_NNN.wmo`, group fdid from the custom listfile). `New WMO` (`wow.new_wmo`) creates an empty collection whose export writes a new file from scratch.
- LOD group files (`*_lodN.wmo`) are neither imported nor exported; editing a group that has them warns that they are stale.
- Not yet: LOD editing, deleting doodads/lights, new portals, group-name and MOGN edits, in-client verification.

### M2 export (phase 6)
"Export M2" in the WoW panel (`wow.export_m2`) rebuilds the model from the collection: geometry/skins, rig, materials/textures, objects, animation. The source is re-read and each section is applied on top; if nothing differs from the source, nothing is written.
- Geometry edits drop LOD skins unless "rebuild LODs" is set. Findings (errors/warnings) show in the report panel and click-to-select; `m.validate()` errors abort unless "Write anyway" (errors already present in the source are warnings).
- Non-linear animation keys are baked to linear; sequences are validated.
- `New M2` (`wow.new_m2`) copies a template (default the Duskwood lamp post), marks it new and exports it to the given client path with fdids from the `file_data_id` table.
- Not done: shadow-batch rules beyond the fallback, in-client checks of edited/new models, LOD decimation.

### Custom file ids (registry)
The `file_data_id` table of `.world/world.doltlite` (`id`, `path`, `source`, `retired`; custom ids from 2000000000) is the single registry of file ids; the addon (`registry.py`), WorldMapStudio (`WowFileDataIds`) and `database_tool fdid` allocate through it in `BEGIN IMMEDIATE` transactions. The addon pre-allocates every path it writes and fails if wowlib invented an id. `Content/fdid.index` is rebuilt from the table after exports and installs (`database_tool fdid export` does the same). The preference `custom_listfile` names another world database. Ids are never reused: releasing sets `retired`. Nothing here makes dolt commits.

### Install / uninstall (phase 7)
`Install to Content` (`wow.install_model`) copies the last export (or everything in Output) into `.client/Content`, backing up each replaced stock file and recording a manifest in `.client/.model-editor-install` (outside Content). `Uninstall` restores every file hash-identically and removes the new ones. Both refuse while characters are online (mariadb check; skipped if the CLI is missing). Restart the client afterwards.

### Game data (phase 7)
`Write game data` (`wow.write_game_data`, `game_data.py`) builds rows for the active M2 and writes them to `.world/world.doltlite` in one transaction (no migration file, no dolt commit; committing `.world` is up to you):
- Creature: CreatureModelData (ids 60000..65535), CreatureDisplayInfo (ids from 1000000, other columns copied from a template display), `creature_display_info_addon` (radius/reach derived from the model bounds and scale). Game object: GameObjectDisplayInfo (ids from 1000000). No ModelFileData row is written: a new 2e9-id cube renders in the client as a creature and as a game object display without one (checked 2026-10-06).
- Rows in the reserved id ranges are the editor's own and are updated in place; edits of other (stock) rows need "Update stock rows". The db2 files are re-exported to `.client/Content/dbfilesclient`.
- Rules from stock data: CollisionWidth is the collision box width, CollisionHeight its max z; over 50 stock creature models (`creature_derivation_check`) the derived width and height match exactly (median error < 0.02%, except models without a collision box or with hand-edited rows), but stock GeoBox differs from the M2 bounds by about 50% (median), so the editor writes the current mesh bounds. Bounding radius and combat reach are hand data in stock.
- `tests/test_game_data.py` runs all of it on a scratch copy of the world; `database_tool` honours `DATABASE_TOOL_ROOT` for that.

### Item displays (phase 8)
`Write item display` (`wow.write_item`, `game_data.plan_item_display` / `plan_item`) writes the db2 chain of an item into `.world` in one transaction: `ModelFileData` (models share a new ModelResourcesID, ids from 100000), `ComponentModelFileData` (helm race/gender variants: RaceID 1 human..8 troll, GenderIndex 0/1; shoulder sides PositionIndex 0/1, gender 2), `TextureFileData` (one MaterialResourcesID per BLP, ids from 1000000), `ItemDisplayInfo` (ids from 1000000, other columns copied from a template display), `ItemDisplayInfoModelMatRes` (from 500000), and for model-less body armor `ItemDisplayInfoMaterialRes` per ComponentSection 0..7 (from 2000000) with gender variants in `ComponentTextureFileData`. Rows are found again through the first model (or first body texture), so re-running updates them; a stock model keeps its stock resource (retexture only). A test item (`Item`, `ItemSparse`, `ItemAppearance` from 200000, `ItemModifiedAppearance` from 400000, Item ids from 300000; copied from a template item) is created when "Template item" is set. Restart the server (it reads items at start-up) and clear the client `Cache/`.
In-client checks (2026-10-06, `tests/gate_items.py`, `tests/gate_item.py`): a custom sword, shield, helm (human variants), shoulders and a chest with female/male component textures show on a character; no ModelFileData row is needed for creature/game object models, but items resolve models only through it. `Import item display` (`wow.import_item_display`, File > Import and sidebar) imports the model of a stock ItemDisplayInfo (`game_data.item_display_models`, default human male variant); its first BLP texture is applied as the object-skin (type 2) replaceable. Not done: helm orc/dwarf variants checked in the client, race-specific body textures.

### Corpus suite
`test.sh --filter zzz --corpus` imports every M2 and dry-run exports it (this is the unedited-clean check and the forced-rebuild comparison at once, `SWEEP_STEP=N` samples every Nth file, about 15 s per 170 files). Failures are compared with `tests/corpus_baseline.json`; only new ones fail. `--update-baseline` rewrites it from a full run and refuses to add entries without `--allow-new`. The remaining 15 entries are source-data quirks (filename textures without a file id in the cata menu models, a handful of NaN/duplicate-id normals, bounds and attachment lookups). The cat sequence-lookup warning is a stock quirk: export keeps the stock lookup. It also runs the WMO export, WMO forced-rebuild and M2 animation sweeps; `--all-m2-geo` stays separate.

### Phase 6 risks
- New groups/WMOs, LOD files, doodad/light deletion and new portals are not exported; deleted objects stay in the file.
- New doodads outside every group box get no group reference, so they will not render.
