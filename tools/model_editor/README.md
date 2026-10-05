# model_editor

Blender addon (WoW 1.15 `.m2`/`.wmo`) on top of wowlib. Phase 1 is the toolchain spike; no addon features yet.

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
