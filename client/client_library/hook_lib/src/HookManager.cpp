#include "hook_lib/HookManager.hpp"

#include "hook_lib/Log.hpp"

#include <MinHook.h>

#include <mutex>
#include <vector>

std::mutex HookMutex;
std::vector<HookRegistration> HookRegistrations;
bool Initialized = false;
bool Enabled = false;

void* ToPointer(Address address)
{
    return reinterpret_cast<void*>(address);
}

void InitializeHooks()
{
    std::lock_guard<std::mutex> lock(HookMutex);
    if (Initialized)
        return;

    MH_STATUS status = MH_Initialize();
    if (status != MH_OK && status != MH_ERROR_ALREADY_INITIALIZED) {
        HOOK_LOG_ERROR("hook manager initialization failed: %s", MH_StatusToString(status));
        return;
    }

    Initialized = true;
    HOOK_LOG_INFO("hook manager initialized (%s)", status == MH_OK ? "MinHook" : "already initialized");
}

bool InstallHook(HookRegistration registration)
{
    std::lock_guard<std::mutex> lock(HookMutex);

    if (!Initialized) {
        HOOK_LOG_ERROR("hook '%s' install failed: hook manager is not initialized", registration.Name);
        return false;
    }

    if (!registration.Target || !registration.Detour) {
        HOOK_LOG_ERROR("hook '%s' install failed: missing target or detour", registration.Name);
        return false;
    }

    MH_STATUS status = MH_CreateHook(ToPointer(registration.Target), registration.Detour, registration.Original);
    if (status != MH_OK) {
        HOOK_LOG_ERROR("hook '%s' create failed at %p: %s",
                       registration.Name,
                       ToPointer(registration.Target),
                       MH_StatusToString(status));
        return false;
    }

    HookRegistrations.push_back(registration);
    HOOK_LOG_DEBUG("hook '%s' installed at %p", registration.Name, ToPointer(registration.Target));
    return true;
}

bool EnableAllHooks()
{
    std::lock_guard<std::mutex> lock(HookMutex);
    if (Enabled)
        return true;

    MH_STATUS status = MH_EnableHook(MH_ALL_HOOKS);
    if (status != MH_OK) {
        HOOK_LOG_ERROR("enable all hooks failed: %s", MH_StatusToString(status));
        return false;
    }

    Enabled = true;
    HOOK_LOG_INFO("enabled %zu hook registration(s)", HookRegistrations.size());
    return true;
}

bool DisableAllHooks()
{
    std::lock_guard<std::mutex> lock(HookMutex);
    if (!Enabled)
        return true;

    MH_STATUS status = MH_DisableHook(MH_ALL_HOOKS);
    if (status != MH_OK && status != MH_ERROR_DISABLED) {
        HOOK_LOG_ERROR("disable all hooks failed: %s", MH_StatusToString(status));
        return false;
    }

    Enabled = false;
    HOOK_LOG_INFO("disabled hook registrations");
    return true;
}
