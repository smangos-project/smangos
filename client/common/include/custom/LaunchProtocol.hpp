#pragma once

#include <cstdio>
#include <cwchar>
#include <string>
#include <string_view>
#include <vector>
#include <windows.h>
#include <shellapi.h>

// How custom_launcher.exe and custom.dll talk: the launcher creates this
// event before resuming the client, and custom.dll sets it once DllMain has
// finished patching and hooking. The launcher treats a timeout as failure.
inline void FormatReadyEventName(wchar_t* buffer, std::size_t size, DWORD processId)
{
    swprintf(buffer, size, L"Local\\CustomClientReady_%lu", processId);
}

// Options meant for custom.dll rather than the client. The client's own
// command-line parser rejects anything it doesn't know ("CommandLine Parse
// Error: -looseonly(0): Invalid argument"), so the launcher takes these out of
// the client's command line and passes them in this environment variable,
// which the client process inherits.
constexpr wchar_t DllArgumentsVariable[] = L"CUSTOM_DLL_ARGS";

// Returns true for a custom.dll option; takesValue says whether the next
// argument belongs to it.
inline bool IsDllOption(std::wstring_view argument, bool& takesValue)
{
    struct Option {
        wchar_t const* name;
        bool takesValue;
    };
    static constexpr Option Options[] = {
        { L"--casc", false },
        { L"--looseonly", false },
        { L"--staticauthseed", false },
        { L"--contentdir", true },
        { L"-contentdir", true },
    };

    for (Option const& option : Options) {
        if (argument.size() == std::wcslen(option.name) &&
            _wcsnicmp(argument.data(), option.name, argument.size()) == 0) {
            takesValue = option.takesValue;
            return true;
        }
    }
    return false;
}

// Every argument custom.dll should look at: the client's command line (for
// runs without the launcher) followed by the launcher-supplied options.
inline std::vector<std::wstring> DllArguments()
{
    std::vector<std::wstring> arguments;

    auto append = [&](wchar_t const* commandLine) {
        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(commandLine, &argc);
        if (!argv)
            return;
        for (int i = 1; i < argc; ++i) // argv[0] is the program name
            arguments.emplace_back(argv[i]);
        LocalFree(argv);
    };

    append(GetCommandLineW());

    DWORD const size = GetEnvironmentVariableW(DllArgumentsVariable, nullptr, 0);
    if (size > 1) {
        std::wstring value(size, L'\0');
        value.resize(GetEnvironmentVariableW(DllArgumentsVariable, value.data(), size));
        append((L"custom.dll " + value).c_str());
    }

    return arguments;
}
