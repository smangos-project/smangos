#pragma once

// Registers every file in the extracted Content/ tree with the client's own
// local-manifest override map, so assets are read off disk instead of out of
// CASC. See docs/CascBypass.md for how the mechanism was found.
bool InstallCascBypassHook();
