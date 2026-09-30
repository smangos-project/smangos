#pragma once

#include "hook_lib/Memory.hpp"

struct HookRegistration
{
    const char* Name = "";
    Address Target = 0;
    void* Detour = nullptr;
    void** Original = nullptr;
};

void InitializeHooks();
bool InstallHook(HookRegistration registration);
bool EnableAllHooks();
bool DisableAllHooks();
