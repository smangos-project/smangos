# Extractors

**Code:** `tools/casc_extractor`, `tools/map_extractor`

Both target the 1.15 client.

## casc_extractor

Reads the client's CASC storage and writes the files named in a listfile to `.client/Content/`. Reruns skip finished files, `--cdn` fetches files missing locally, and `--zero-encrypted` zero-fills db2 sections whose key is unknown.

## map_extractor

Builds the server's `maps/`, `vmaps/` and `mmaps/` from `.client/Content/` in three stages (`map`, `vmap`, `mmap`).

- **Range-based:** `--map ID`, `--tile R,C`, `--tiles R1,C1,R2,C2` or `--around X,Y,RADIUS` limit the work. For example Northshire Abbey is `--map 0 --tile 48,32`.
- **Incremental:** a cache of input fingerprints per tile skips unchanged work. `--force` redoes it and `--dry-run` shows what would be redone.

```
build/tools/map_extractor/map_extractor --map 0 --tile 48,32 --stages map,vmap,mmap
```

See `tools/map_extractor/README.md` and the [Setup](../setup.md) page.
