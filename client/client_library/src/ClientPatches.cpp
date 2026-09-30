// The patches Arctium Game Launcher (arctiumcpp) applied to the client,
// redone from inside the process.
//
// Arctium launched WowClassic.exe suspended, patched it through
// WriteProcessMemory, resumed it, waited for the packer to unpack, and then
// neutralized self-integrity checks and remap guards so it could patch code
// too. The dejunked client has no packer and no integrity checks (that
// pattern matches nothing), and everything Arctium looked for sits in plain
// .rdata/.text. Most patches below target a pinned RVA (WowClassic.exe
// 1.15.6.58797) and verify Arctium's byte pattern is actually there before
// writing, rather than scanning the section for it: same fail-loud contract
// (a mismatch logs a clear error instead of patching), without the cost or
// collision risk of a full-section scan for every site. CollectRemapCheckPatches
// is the exception -- it deliberately scans, see its own comment for why.
//
// Timing: custom_launcher.exe loads custom.dll before the client's entry
// point, so these run before anything reads them. That matters: net init
// builds its ConnectTo RSA key object from .rdata at startup, so a later
// patch of the modulus has no effect and entering the world fails. The
// client's TLS callbacks do run before us, and they have already remapped
// its image SEC_NO_CHANGE, so we undo that first (see
// CollectRemapCheckPatches).
//
// Not ported: the version/CDN URL rewrite (--versionurl/--cdnsurl). Its
// only reader, the TACT bootstrap (WowClassic.exe+0x237130), only goes
// online when CAS init from .build.info fails.

#include "ClientPatches.hpp"

#include "custom/LaunchProtocol.hpp"
#include "hook_lib/HookManager.hpp"
#include "hook_lib/ImageRemap.hpp"
#include "hook_lib/Log.hpp"
#include "hook_lib/Memory.hpp"
#include "hook_lib/Pattern.hpp"
#include "hook_lib/ProcessImage.hpp"

#include <windows.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <cwchar>
#include <span>
#include <string_view>
#include <vector>


// ConnectTo / ChangeProtocol RSA modulus, replacing the one starting with
// 91 D5 9B B7 D4 E1 83 A5 (Patches::Common::RsaModulus in arctiumcpp).
constexpr std::array<std::uint8_t, 256> RsaModulus = {
    0x5F, 0xD6, 0x80, 0x0B, 0xA7, 0xFF, 0x01, 0x40, 0xC7, 0xBC, 0x8E, 0xF5, 0x6B, 0x27, 0xB0, 0xBF,
    0xF0, 0x1D, 0x1B, 0xFE, 0xDD, 0x0B, 0x1F, 0x3D, 0xB6, 0x6F, 0x1A, 0x48, 0x0D, 0xFB, 0x51, 0x08,
    0x65, 0x58, 0x4F, 0xDB, 0x5C, 0x6E, 0xCF, 0x64, 0xCB, 0xC1, 0x6B, 0x2E, 0xB8, 0x0F, 0x5D, 0x08,
    0x5D, 0x89, 0x06, 0xA9, 0x77, 0x8B, 0x9E, 0xAA, 0x04, 0xB0, 0x83, 0x10, 0xE2, 0x15, 0x4D, 0x08,
    0x77, 0xD4, 0x7A, 0x0E, 0x5A, 0xB0, 0xBB, 0x00, 0x61, 0xD7, 0xA6, 0x75, 0xDF, 0x06, 0x64, 0x88,
    0xBB, 0xB9, 0xCA, 0xB0, 0x18, 0x8B, 0x54, 0x13, 0xE2, 0xCB, 0x33, 0xDF, 0x17, 0xD8, 0xDA, 0xA9,
    0xA5, 0x60, 0xA3, 0x1F, 0x4E, 0x27, 0x05, 0x98, 0x6F, 0xAA, 0xEE, 0x14, 0x3B, 0xF3, 0x97, 0xA8,
    0x12, 0x02, 0x94, 0x0D, 0x84, 0xDC, 0x0E, 0xF1, 0x76, 0x23, 0x95, 0x36, 0x13, 0xF9, 0xA9, 0xC5,
    0x48, 0xDB, 0xDA, 0x86, 0xBE, 0x29, 0x22, 0x54, 0x44, 0x9D, 0x9F, 0x80, 0x7B, 0x07, 0x80, 0x30,
    0xEA, 0xD2, 0x83, 0xCC, 0xCE, 0x37, 0xD1, 0xD1, 0xCF, 0x85, 0xBE, 0x91, 0x25, 0xCE, 0xC0, 0xCC,
    0x55, 0xC8, 0xC0, 0xFB, 0x38, 0xC5, 0x49, 0x03, 0x6A, 0x02, 0xA9, 0x9F, 0x9F, 0x86, 0xFB, 0xC7,
    0xCB, 0xC6, 0xA5, 0x82, 0xA2, 0x30, 0xC2, 0xAC, 0xE6, 0x98, 0xDA, 0x83, 0x64, 0x43, 0x7F, 0x0D,
    0x13, 0x18, 0xEB, 0x90, 0x53, 0x5B, 0x37, 0x6B, 0xE6, 0x0D, 0x80, 0x1E, 0xEF, 0xED, 0xC7, 0xB8,
    0x68, 0x9B, 0x4C, 0x09, 0x7B, 0x60, 0xB2, 0x57, 0xD8, 0x59, 0x8D, 0x7F, 0xEA, 0xCD, 0xEB, 0xC4,
    0x60, 0x9F, 0x45, 0x7A, 0xA9, 0x26, 0x8A, 0x2F, 0x85, 0x0C, 0xF2, 0x19, 0xC6, 0x53, 0x92, 0xF7,
    0xF0, 0xB8, 0x32, 0xCB, 0x5B, 0x66, 0xCE, 0x51, 0x54, 0xB4, 0xC3, 0xD3, 0xD4, 0xDC, 0xB3, 0xEE,
};

// GameCrypto Ed25519 public key (Patches::Common::CryptoEdPublicKey).
constexpr std::array<std::uint8_t, 32> CryptoEdPublicKey = {
    0x02, 0x59, 0x6F, 0x0D, 0x0C, 0x06, 0x1A, 0x8B, 0x30, 0x74, 0x59, 0x88, 0xFD, 0x72, 0xC5, 0x9E,
    0x29, 0xEC, 0x36, 0x7F, 0xB0, 0xF3, 0x41, 0xF2, 0x8E, 0x0F, 0x08, 0xD0, 0x37, 0xBA, 0xFC, 0x69,
};

// The client appends this to a portal hostname that has no dot and isn't an
// IP address ("us" -> "us.actual.battle.net"). Zeroing it keeps a bare
// hostname like "localhost" as-is. An IP portal never reaches this path.
constexpr std::string_view PortalSuffix{ ".actual.battle.net\0", 19 };

// Registry key the client reads Battle.net launch options from (LOCALE,
// REGION, and the -launcherlogin web token). The client copies it with
// fixed-size 16-byte loads, so the replacement must be exactly as long.
constexpr std::string_view LauncherLoginKey = "Software\\Blizzard Entertainment\\Battle.net\\Launch Options\\";
constexpr std::string_view CustomLauncherLoginKey = "Software\\Custom Game Server Dev\\Battle.net\\Launch Options\\";
static_assert(LauncherLoginKey.size() == CustomLauncherLoginKey.size());

// The seed that --staticauthseed servers expect: 179D3DC3235629D07113A9B3867F97A7.
// Without the flag the client keeps its own built-in seed. That seed is a
// constant too (6411844E2DE7014EEC55D0C96DAF1190 for 1.15.6.58797), so a
// server can also just be configured with it instead.
constexpr std::array<std::uint8_t, 16> StaticAuthSeed = {
    0x17, 0x9D, 0x3D, 0xC3, 0x23, 0x56, 0x29, 0xD0, 0x71, 0x13, 0xA9, 0xB3, 0x86, 0x7F, 0x97, 0xA7,
};

ProcessImage Image;

unsigned long long ImageOffset(void const* address)
{
    return static_cast<unsigned long long>(static_cast<std::byte const*>(address) - Image.BaseAddress);
}

bool PatchData(char const* name, std::size_t rva, BytePattern const& pattern, std::span<std::byte const> replacement)
{
    if (std::byte* target = VerifyPatternAt(Image, rva, pattern)) {
        if (!IsInsideImage(Image, target, replacement.size())) {
            HOOK_LOG_ERROR("client patch '%s': replacement runs past the end of the image", name);
            return false;
        }
        if (!WriteBytes(target, replacement))
            return false;

        HOOK_LOG_INFO("client patch '%s' applied at WowClassic.exe+0x%llx (%zu byte(s))", name, ImageOffset(target), replacement.size());
        return true;
    }

    // Launching through Arctium Game Launcher still works: it has already
    // written the same bytes before we get here.
    BytePattern const replacementPattern(replacement.begin(), replacement.end());
    if (VerifyPatternAt(Image, rva, replacementPattern)) {
        HOOK_LOG_INFO("client patch '%s' already applied at WowClassic.exe+0x%llx", name, ImageOffset(Image.BaseAddress + rva));
        return true;
    }

    HOOK_LOG_ERROR("client patch '%s': bytes at +0x%zx don't match original or replacement", name, rva);
    return false;
}

template <std::size_t N>
std::span<std::byte const> AsBytes(std::array<std::uint8_t, N> const& bytes)
{
    return std::as_bytes(std::span(bytes));
}

std::span<std::byte const> AsBytes(std::string_view text)
{
    return std::as_bytes(std::span(text.data(), text.size()));
}

// Arctium's anchor for the auth seed provider. Net init registers it right
// after passing the 'WoW' program id to another setter:
//
//   B9 57 6F 57 00       mov  ecx, 'WoW'
//   E8 ?? ?? ?? ??       call ...
//   48 8D 0D ?? ?? ?? ?? lea  rcx, [AuthSeedWrapper]
//
// and the wrapper tail-calls the real provider:
//
//   48 8B D1           mov  rdx, rcx        ; output buffer
//   33 C9              xor  ecx, ecx
//   E9 ?? ?? ?? ??     jmp  AuthSeedProvider
std::byte* FindAuthSeedProvider()
{
    // Net init's registration call, WowClassic.exe+0x235e31 in 1.15.6.58797.
    std::byte* anchor =
        VerifyPatternAt(Image, 0x235e31, ParsePattern("B9 57 6F 57 00 E8 ? ? ? ? 48 8D 0D ? ? ? ?"));
    if (!anchor) {
        HOOK_LOG_ERROR("static auth seed: bytes at +0x235e31 don't match");
        return nullptr;
    }

    std::byte* wrapper = anchor + 17 + ReadValue<std::int32_t>(anchor + 13);
    static BytePattern const wrapperPattern = ParsePattern("48 8B D1 33 C9 E9");
    if (!IsInsideImage(Image, wrapper, wrapperPattern.size() + 4) ||
        FindPattern({ wrapper, wrapperPattern.size() }, wrapperPattern) != wrapper) {
        HOOK_LOG_ERROR("static auth seed: unexpected wrapper at WowClassic.exe+0x%llx", ImageOffset(wrapper));
        return nullptr;
    }

    return wrapper + 10 + ReadValue<std::int32_t>(wrapper + 6);
}

// The real provider only writes the 16-byte seed to its second argument.
// It also checks its return address, which is why the whole function is
// replaced here instead of being called through a trampoline.
void DetourAuthSeedProvider(std::uint64_t, std::uint8_t* seed)
{
    std::memcpy(seed, StaticAuthSeed.data(), StaticAuthSeed.size());
}

// Whole-token, case-insensitive match against the process command line.
// Flags are never quoted, so a plain whitespace split is enough.
// The client samples checks on its own SEC_NO_CHANGE remap (Arctium's
// Patterns::Windows::Remap) at the tail of many functions:
//
//   rdtsc ; ... ; rdi = tsc % 100
//   48 83 FF 1E          cmp  rdi, 30
//   0F 83 ?? ?? ?? ??    jae  <epilogue>      ; 70%: skip the check
//   <check>                                   ; 30%: inspect the mapping
//
// Once MakeMappedViewPatchable has swapped the view those checks fail --
// randomly, since only 30% of calls reach them -- and the connection to
// the server doesn't survive it. Arctium neutralized the same sites
// whenever it had to remap. Here each jae becomes an unconditional jmp:
//
//   E9 <rel32 + 1>  90
struct RemapCheckPatch
{
    std::byte* Site = nullptr;
    std::array<std::byte, 6> Bytes = {};
};

// Scans rather than targets fixed RVAs: this shape recurs at the tail of many
// functions, in a count that isn't known ahead of time, so there is no single
// site (or fixed list of sites) to pin -- the whole point is to catch every
// occurrence in one pass.
std::vector<RemapCheckPatch> CollectRemapCheckPatches(std::span<std::byte> text)
{
    static BytePattern const pattern = ParsePattern("48 ? ? 1E 0F 83");

    std::vector<RemapCheckPatch> patches;
    for (std::byte* match = FindPattern(text, pattern); match;
         match = FindPattern(text.subspan(static_cast<std::size_t>(match + 1 - text.data())), pattern)) {
        std::byte* site = match + 4;
        std::int32_t const rel = ReadValue<std::int32_t>(site + 2);

        RemapCheckPatch patch;
        patch.Site = site;
        patch.Bytes[0] = std::byte{ 0xE9 };
        std::int32_t const jmpRel = rel + 1;
        std::memcpy(&patch.Bytes[1], &jmpRel, sizeof(jmpRel));
        patch.Bytes[5] = std::byte{ 0x90 };
        patches.push_back(patch);
    }

    return patches;
}

bool HasCommandLineFlag(std::wstring_view flag)
{
    for (std::wstring const& argument : DllArguments()) {
        if (argument.size() == flag.size() && _wcsnicmp(argument.data(), flag.data(), flag.size()) == 0)
            return true;
    }
    return false;
}

bool InstallStaticAuthSeedHook()
{
    std::byte* provider = FindAuthSeedProvider();
    if (!provider)
        return false;

    HookRegistration hook;
    hook.Name = "AuthSeedProvider";
    hook.Target = reinterpret_cast<Address>(provider);
    hook.Detour = reinterpret_cast<void*>(&DetourAuthSeedProvider);
    if (!InstallHook(hook))
        return false;

    HOOK_LOG_INFO("static auth seed: provider at WowClassic.exe+0x%llx now returns 179D3DC3235629D07113A9B3867F97A7",
                  ImageOffset(provider));
    return true;
}


bool ApplyClientPatches(bool launchedEarly)
{
    Image = GetProcessImage();

    std::span<std::byte> rdata = FindSection(Image, ".rdata");
    std::span<std::byte> text = FindSection(Image, ".text");
    if (rdata.empty() || text.empty()) {
        HOOK_LOG_ERROR("client patches: .rdata/.text section not found");
        return false;
    }

    if (!launchedEarly)
        HOOK_LOG_WARN("client patches: not started by custom_launcher.exe -- data the client reads at startup "
                      "(e.g. the ConnectTo RSA key) may already have been consumed");

    // Even when loaded by custom_launcher.exe the client's TLS callbacks have
    // already run, and by then it has swapped its image for a SEC_NO_CHANGE
    // mapping (.text and .rdata share its first view): undo that, disabling
    // the client's checks for that mapping in the same breath.
    MEMORY_BASIC_INFORMATION textInfo = {};
    VirtualQuery(text.data(), &textInfo, sizeof(textInfo));
    if (textInfo.Type == MEM_MAPPED) {
        std::vector<RemapCheckPatch> remapChecks = CollectRemapCheckPatches(text);
        std::vector<PendingWrite> remapCheckWrites;
        for (RemapCheckPatch const& patch : remapChecks)
            remapCheckWrites.push_back({ patch.Site, patch.Bytes });

        if (!MakeMappedViewPatchable(text.data(), remapCheckWrites) || !MakeMappedViewPatchable(rdata.data()))
            return false;

        HOOK_LOG_INFO("client patch 'Remap checks' disabled %zu site(s)", remapChecks.size());
    } else {
        HOOK_LOG_INFO("client patches: image not remapped yet, patching in place");
    }

    // RVAs are for 1.15.6.58797; see the file header on why they're pinned
    // rather than found by scanning.
    bool ok = PatchData("ConnectTo RsaModulus", 0x2df5ea0, ParsePattern("91 D5 9B B7 D4 E1 83 A5"), AsBytes(RsaModulus));
    ok = PatchData("GameCrypto Ed25519 PublicKey",
                   0x2e41ee0,
                   ParsePattern("15 D6 18 BD 7D B5 77 BD 9A 8D 45 76 9C 59 E4 FC 63"),
                   AsBytes(CryptoEdPublicKey)) && ok;

    static constexpr std::array<std::uint8_t, PortalSuffix.size()> ZeroedPortalSuffix = {};
    ok = PatchData("Login Portal", 0x2e429c0, LiteralPattern(PortalSuffix), AsBytes(ZeroedPortalSuffix)) && ok;

    ok = PatchData("Launcher Login Registry", 0x2e130f0, LiteralPattern(LauncherLoginKey), AsBytes(CustomLauncherLoginKey)) && ok;

    // Opt-in, like Arctium's --staticauthseed: add it to the client's
    // command line.
    if (HasCommandLineFlag(L"--staticauthseed"))
        ok = InstallStaticAuthSeedHook() && ok;

    return ok;
}
