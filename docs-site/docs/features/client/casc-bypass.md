# CASC bypass

**Code:** `client/client_library/src/CascBypass.cpp`, notes in `client/client_library/docs/CascBypass.md`

The retail-engine client reads its game data from CASC storage. smangos instead makes it read loose files from `.client/Content/`, so the client can run on data you extracted and modified yourself. The client already has a loose-file override system (it uses it for avatar caches); `custom.dll` points it at the extracted files.

## What you can do

- Run the client without a full CASC install, using the output of `casc_extractor`.
- Replace or add any client file (db2, textures, models, UI) by changing it on disk.
- Read `client/client_library/docs/CascBypass.md` for the reverse-engineering findings behind it.
