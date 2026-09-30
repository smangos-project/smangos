#pragma once

// In-process replacements for the memory patches Arctium Game Launcher used
// to apply to a suspended client from the outside. See ClientPatches.cpp.
// launchedEarly: loaded by custom_launcher.exe, i.e. before the client's
// entry point (otherwise some patches come too late and a warning is logged).
bool ApplyClientPatches(bool launchedEarly);
