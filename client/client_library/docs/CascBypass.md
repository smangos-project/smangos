# Replacing CASC with loose files on disk

Findings from reverse engineering `_classic_era_/WowClassic.exe` (1.15.6.58797).
All addresses are the default image base `0x140000000`.

## The client already has a loose-file override system

The CAS layer (`Engine/Source/CAS/CAS.cpp`, `CASManifest.cpp`) carries a second
file source next to CASC, called the *local manifest*. It is not dev-only dead
code: the shipping client uses it for the Battle.net avatar texture cache
(`WoW/Source/Ui/AvatarTexture.cpp` registers downloaded `Index.json` / avatar
files through it).

A local manifest entry is 0x120 bytes:

```c
struct LocalManifestEntry {
    char path[0x104];   // +0x000  local file path
    int  fileDataId;    // +0x104  build-manifest id, or a synthetic negative id
    bool flag;          // +0x108
};
```

The container object (`g_CasContainer`, `0x143633b98`, 0x148 bytes) holds two
maps built in `CAS_CreateContainer` (`0x1403462a0`):

| offset | map |
| --- | --- |
| `+0x108` / `+0x110` / `+0x118` | `uint64 pathHash -> LocalManifestEntry` |
| `+0x128` / `+0x130` / `+0x138` | `int fileDataId -> LocalManifestEntry*` |

## Where an override wins

`CAS_OpenFile_LocalOverrideAware` (`0x140346550`) is the open path:

```c
if (flags & 1) {                                   // overrides enabled
    if (fileId != 0)
        p = LocalManifest_FindOverrideByFileId(g_CasContainer, fileId);
    if (!p)
        p = LocalManifest_FindOverrideByNameHash(g_CasContainer, entry->nameHash, 0);
    if (p) {
        Os_OpenFileHandle(p, &out, ...);           // plain OS file, CASC untouched
        return;
    }
}
/* ... normal CASC read ... */
```

Two things matter here:

- On a hit the client returns a **plain OS file handle**. Encoding, indices and
  `Data/data/data.NNN` are never consulted, and no BLTE decode happens — the
  file on disk is the raw asset.
- The build-manifest lookup is *not* required to succeed first. A fileId that
  exists only as an override still opens. So overrides can **add** files, not
  just replace them.

## How overrides get registered

Two entry points:

1. **Directory scan.** `CAS_SetLooseFileDirAndScan` (`0x14034aa10`) resolves a
   directory to an absolute path, stores it in `g_LooseFileScanDir`
   (`0x1436388a0`), and `LocalManifest_ScanDirIntoManifest` (`0x140351080`)
   walks it recursively. Every file found is registered by its relative path
   (`LocalManifest_ScanDirEntryCallback`, `0x14034fc00`), skipping the name
   `addons.txt` and these extensions:

   ```
   wtf bak bat bin cfg dat db dbs dll dmp dylib exe idx ilk ini lib md5
   metagen old pak pdb pl pm py sh signed tmp wdb zip
   ```

   The fileId comes from `BuildManifest_LookupFileIdByName` on the path hash;
   if the name is not in the build manifest the entry gets a synthetic
   **negative** id (a counter decrementing from 0) and is still registered.

2. **Explicit registration.** `CAS_AddLocalOverride` (`0x1403443e0`) →
   `LocalManifest_AddOverride` (`0x14034f510`) takes an explicit
   `(fileId, path)` pair. This is the interesting one: it needs no name hash
   and no build-manifest entry.

## What drives the scan directory

`CAS_InitializeContainer` (`0x140345850`) calls `CAS_SetLooseFileDirAndScan`
with its 7th argument. Tracing it back through `CAS_InitializeFromBuildInfo`
(`0x140345c50`) to `CAS_BootstrapStorage` (`0x140236ff0`):

```asm
140237138  lea  rcx, [rbp+0x290]
14023713f  xor  r8d, r8d              ; kind 0 = GetCurrentDirectory
14023714a  mov  edx, 0x104
140237152  call Os_GetDirectory
...
1402371ea  mov  [rsp+0x20], rax       ; -> loose-file scan root
1402371ef  call CAS_InitializeFromBuildInfo
```

So the scan root is **the process working directory**. That is what makes the
long-standing loose `Interface/` and `Fonts/` overrides work.

## Undocumented command-line options

`0x1405184c0` registers the option table. These three are parsed but absent
from the `-help` text:

| option | storage | effect |
| --- | --- | --- |
| `-datadir <path>` | `0x143676b50` | `SetCurrentDirectory`, so it also moves the loose-file scan root |
| `-cascdir <path>` | `0x143676c60` | CASC storage directory |
| `-casrefresh` | flag | forces a CAS refresh |

The CAS data directory itself defaults to `../Data` (`CAS_CreateContainer`).

## CAS error codes

Table at `0x142d09d90`:

```
0 Success                     6 Canceled
1 General Failure             7 Missing TACT Cryptkey
2 Can't find file in build manifest
3 Loose file not found and CAS is in loose file only mode.
4 Unknown file type           8 Container needs repair
5 Deleted remote file         9 OS File Error
                             10 Code/Data mismatch
```

## buildinfo.json

After CAS init, `CAS_InitializeContainer` opens FileDataID `0x4f9920`
(5216544) — `buildinfo.json` — and reads `dataBuild` and `clientBuild` from it
into `0x14342f340` / `0x14342f344`. The extracted copy in `Content/` is
`{"dataBuild":58725,"clientBuild":58797}`.

## The flags gate

Every read path guards its local-manifest lookup on bit 0 of a flags argument,
and most call sites leave it clear — of the 43 call sites across the three
synchronous open functions, only 15 set it. Each gate compiles to:

```asm
test  <reg>b, 1                 ; flags & 1
jz    <casc path>               ; 74 xx, or 0F 84 xx xx xx xx
[test <reg>,<reg> / jz]         ; sometimes
mov   rcx, [g_CasContainer]     ; the local-manifest lookup
```

There are **seven** of them:

| address | function |
| --- | --- |
| `0x140344605` | in `0x1403445e0` |
| `0x140345182` | `CAS_GetFileIdByName` |
| `0x1403465d7` | open by FileDataId (handle out-param) |
| `0x140346773` | open by FileDataId (bool) |
| `0x1403468cc` | open by name |
| `0x140347497` | **async streaming open** |
| `0x14034766a` | in `0x140347640` |

`CascBypass.cpp` finds them by shape rather than by seven hard-coded patterns —
an earlier attempt hard-coded three and silently missed the async path — and
replaces each `jz` with NOPs. That is safe rather than a redirect: every lookup
block already falls through to the original CASC read when no override matches.

## Registration order: the scan destroys the map

`LocalManifest_ScanDirIntoManifest` does not merge into the maps, it **resets
them**: it walks both bucket arrays freeing every `chained_hash_node`, zeroes
the buckets, and clears the counts at `+0x118` and `+0x138`, before walking the
directory. Anything registered before it is gone.

This is worth stating plainly because it fails silently and convincingly.
Registering first, `CAS_AddLocalOverride` returns a valid FileDataId for all
141,760 entries and a readback taken at that moment verifies 8/8 — and then the
scan wipes the lot, every asset quietly falls back to CASC, and the game looks
completely normal. The same readback taken *after* the scan returns 0/9.

So `custom.dll` lets the client's own scan run first and registers `Content/` on
top of it. The consequence is that `Content/` shadows a hand-placed loose file
of the same FileDataId rather than the other way round, which is the right way
round here: `Content/` plus its index is the supported way to supply files.

## Verified behaviour

On 1.15.6.58797, with `Data/` still present:

- 141,760 overrides register in ~220 ms, none rejected.
- A readback taken after the client's own scan verifies 9/9 sampled ids,
  including `fonts/frizqt__.ttf` (615960) and `buildinfo.json` (5216544).
- Replacing `Content/fonts/frizqt__.ttf` with `morpheus.ttf` (and clearing
  `_classic_era_/Fonts/*.slug`) renders **every glyph on the login screen** in
  Morpheus. The client is reading the file off disk.

Note that negative tests prove nothing here, so use "wrong content" tests: the
async path falls through to CASC when an override file fails to open, and a
missing override simply misses the map. Hiding files therefore looks identical
to the bypass not working at all.

The font path is a good worked example — `FUN_1405ea2c0` loads a font with
`FUN_1403466f0(fileDataId, &buffer, &size, 1, 0x1b)`, i.e. through an
override-aware open with bit 0 already set, then names its glyph cache
`<id>.slug`, or `override_<contenthash>.slug` when the id is one of the
synthetic negative ones the directory scan hands out.

## Loose-file-only mode

This is the default; `--casc` turns it off. It runs with `Data/` absent
(verified 2026-09-22 21:20: login screen renders fully, no storage reads). Without it, a file missing from
`Content/` falls back to CASC, and with `Data/` deleted the client builds a
fresh empty storage and **streams the game from the CDN**. That was observed on
2026-09-22: 51 connections to `archive.wow.tools` and new empty `Data/data/*.idx`.

CAS has a mode of its own for this. `CAS_LoadManifests` (`0x140345a70`) takes a
flag as its fifth argument; when set it opens no storage, reads no `.root`, and
latches the flag into `0x143633bbc` so a lookup that misses the local manifest
reports CAS error 3, "Loose file not found and CAS is in loose file only mode",
instead of going to the archives. Both callers hard-code it off as
`mov byte ptr [rsp+0x20], 0` just before calling `CAS_InitializeContainer`
(`0x140345e3a` and `0x1403461ef`).

Flipping those two immediates is not quite enough, because the mode is
unfinished in this build:

- It skips filling the manifest buffer but still parses it, so CAS init dies on
  *"build manifest is truncated"*. `custom.dll` hooks the loader and substitutes
  a 20-byte synthetic root — magic `TSFM`, header size, version 2, two zero
  counts — which yields a valid empty manifest. That is safe to probe: the
  bucket allocator floors at 8, so `BuildManifest_FindEntry`'s `bucketCount - 1`
  mask stays in bounds and simply finds nothing.
- CAS init then reads `buildinfo.json` (FileDataID 5216544) through the build
  manifest only, with no local-manifest check, and a fatal follows
  (`ERROR #134 (0x85100086)`). Skipping just the fatal is not enough — control
  falls into the JSON block with a null buffer and the parser dereferences it
  (ACCESS_VIOLATION at `WowClassic.exe+0x710E29`), and jumping to the teardown
  would destroy a context that was never constructed. The fatal is replaced with
  a jmp past the whole block, and `dataBuild`/`clientBuild` are written from
  `Content/buildinfo.json` instead. They are not cosmetic: `dataBuild` is what
  the client sends when requesting hotfixes, and the built-in default (58797) is
  not this build's data build (58725).

## Names: the build manifest is the name → FileDataId table

An empty synthetic manifest is not enough. The build manifest is where the
client turns a **name** into a FileDataId: `BuildManifest_LookupFileIdByName`
(`0x140349950`) hashes the path with `CAS_HashPathKey` (`0x14034ce80`) and looks
the hash up. `CAS_GetFileIdByName`, open-by-name, and the working-directory scan
(which registers each file with id 0 and asks the manifest for the real one) all
depend on it. With it empty, everything looked up by name misses.

`CAS_HashPathKey`: uppercase `a-z`, `/` → `\`, then lookup3 `hashlittle2`
(`0x1426b2210`) with both seeds 0, returned as `(c << 32) | b`. That is the
standard WoW root name hash, identical to CascLib's `CalcNormNameHash`.

The same hash keys the local manifest's path map. `LocalManifest_AddOverride`
normalises `path` (`0x1403501a0`, per-extension key transform, default is a
copy) and hashes it; the stored file path is `basePath` joined with `path`. So an
override registered as `(relative path, Content dir)` is *keyed* by the
manifest entry's name hash. It is only *found* by name, though, once the
entry's "named" flag (`+0x108`) is set: `LocalManifest_FindOverrideByNameHash`
(`0x1403508f0`) skips entries without it, and `LocalManifest_AddOverride` sets
it only when called with FileDataId 0. `custom.dll` therefore registers every
file twice, first with its id and then with 0. The second call finds the
existing entry, sets the flag and re-inserts it under its current id. One registered with an absolute path and no base
is keyed by the hash of `D:\...` and is only reachable by id.

Manifest parsing (`FUN_140351270`, blocks in `FUN_14034d9a0`), run once for the
text locale and once for the speech locale (mask `1 << locale`):

- header `TSFM`, header size, version 1 or 2, total count, named count;
- v1 block: `uint32 count, contentFlags, localeFlags`, then `int32` id deltas
  (`id = prev + 1 + delta`), `count` × 16-byte CKey, `count` × `uint64` name
  hash; v2 block headers are the 17-byte `{count, locale, cf1, cf2, cf3}`;
- a block is kept when content-flag bit 31 is clear for the text pass (set for
  speech), it has none of `0x191` (`0x110` in some modes), and
  `localeFlags & (1 << locale)` is non-zero;
- entries are 0x2C bytes: name hash, id, locale, cf1, cf2, cf3, CKey.

`custom.dll` now generates this manifest from `Content/`. It is one v1 block
with content flags 0 and every locale bit set. The CKeys are synthetic, built
from id, size and mtime, since no archive stands behind them.

## What is left

See `progress.md` at the repo root. The three "Unable to open %s: %s" DB2
fatals are still softened. With files removable at will, that is now the
intended behaviour: a missing DB2 leaves an empty table, as stock does.
