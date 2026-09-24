// Custom Catch2 main for tests-conformance (bot-phase-1.md, "Conformance"): --target/--config/
// --strict (env fallbacks REALBOTS_TARGET/REALBOTS_CONFIG) on top of whatever else Catch2's own
// CLI already understands (bot-examples.md #7: "extra Catch2 filters still work on top"). Loads
// the target's Tags and expectations/<target>.txt, and excludes:
//   - every [protocol]/[gameplay]-tagged scenario the target's own Tags don't list, always
//     (this is a structural capability mismatch, not a "not yet ported" placeholder -- --strict
//     never overrides it);
//   - every scenario named in expectations/<target>.txt, unless --strict.
// Prints what it left out and why before running, same shape as bot-examples.md #7's transcript.
#include "ConformanceRun.h"
#include "Config/Config.h"
#include "Crypto/InitializeCrypto.h"
#include "Log.h"
#include "RealBotMgr.h"
#include "RealBotTarget.h"
#include <catch_amalgamated.hpp>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

char const* g_mainLogFileName = "tests-conformance.log";

namespace
{
bool HasTag(Catch::TestCaseInfo const& info, std::string const& tag)
{
    for (Catch::Tag const& t : info.tags)
        if (t.original == tag)
            return true;
    return false;
}

void Trim(std::string& s)
{
    size_t const first = s.find_first_not_of(" \t\r");
    size_t const last = s.find_last_not_of(" \t\r");
    s = (first == std::string::npos) ? std::string() : s.substr(first, last - first + 1);
}

// "name | reason" per non-comment, non-blank line (bot-examples.md #8's format); '#' lines
// (including the file's own column header) are pure comments.
std::unordered_map<std::string, std::string> LoadExpectations(std::string const& path)
{
    std::unordered_map<std::string, std::string> out;
    std::ifstream file(path);
    if (!file)
        return out;

    std::string line;
    while (std::getline(file, line))
    {
        size_t const first = line.find_first_not_of(" \t");
        if (first == std::string::npos || line[first] == '#')
            continue;

        size_t const bar = line.find('|');
        if (bar == std::string::npos)
            continue;

        std::string name = line.substr(0, bar);
        std::string reason = line.substr(bar + 1);
        Trim(name);
        Trim(reason);
        if (!name.empty())
            out[name] = reason;
    }
    return out;
}

// Where expectations/<target>.txt lives, independent of the working directory (bot-phase-2.md,
// "Targets"): next to the config file, then one level up (the .dist sits in targets/, the
// expectations beside it), then the source tree this binary was built from. First hit wins.
std::string FindExpectationsFile(std::string const& configPath, std::string const& targetName)
{
    namespace fs = std::filesystem;
    std::error_code ec;
    std::string const leaf = "expectations/" + targetName + ".txt";

    std::vector<fs::path> candidates;
    fs::path const configDir = fs::absolute(fs::path(configPath), ec).parent_path();
    if (!configDir.empty())
    {
        candidates.push_back(configDir / leaf);
        candidates.push_back(configDir.parent_path() / leaf);
    }
#ifdef CONFORMANCE_SOURCE_DIR
    candidates.push_back(fs::path(CONFORMANCE_SOURCE_DIR) / leaf);
#endif

    for (fs::path const& candidate : candidates)
        if (fs::exists(candidate, ec))
            return candidate.string();

    return leaf; // not found anywhere; LoadExpectations returns empty for a missing file
}
}

int main(int argc, char** argv)
{
    std::string targetName;
    std::string configPath = "tests-conformance.conf";
    bool strict = false;

    if (char const* env = std::getenv("REALBOTS_TARGET"))
        targetName = env;
    if (char const* env = std::getenv("REALBOTS_CONFIG"))
        configPath = env;

    bool configGiven = std::getenv("REALBOTS_CONFIG") != nullptr;

    std::vector<char*> passthrough;
    passthrough.push_back(argv[0]);
    for (int i = 1; i < argc; ++i)
    {
        std::string const arg = argv[i];
        if (arg == "--target" && i + 1 < argc)
            targetName = argv[++i];
        else if (arg == "--config" && i + 1 < argc)
        {
            configPath = argv[++i];
            configGiven = true;
        }
        else if (arg == "--strict")
            strict = true;
        else
            passthrough.push_back(argv[i]);
    }

    // Default config: the working directory's tests-conformance.conf, else the one next to the binary.
    if (!configGiven && !std::filesystem::exists(configPath))
    {
        std::error_code ec;
        std::filesystem::path const besideExe = std::filesystem::absolute(std::filesystem::path(argv[0]), ec).parent_path() / configPath;
        if (std::filesystem::exists(besideExe, ec))
            configPath = besideExe.string();
    }

    if (targetName.empty())
    {
        std::cerr << "tests-conformance: --target <name> is required (or REALBOTS_TARGET)\n";
        return EXIT_FAILURE;
    }

    if (sConfig.LoadFromFile(configPath))
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "Loaded config %s", configPath.c_str());
    else
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "No config at %s -- using RealBots.* defaults", configPath.c_str());

    sLog.OpenWorldLogFiles();

    if (!Crypto::InitializeCryptoAndPrintVersion())
        return EXIT_FAILURE;

    sRealBotMgr.Start();
    ConformanceRun::SetTarget(targetName);

    RealBotTarget const* target = sRealBotMgr.GetTargets().Find(targetName);
    if (!target)
    {
        std::cerr << "tests-conformance: no such target '" << targetName << "' in " << configPath << "\n";
        sRealBotMgr.Stop();
        return EXIT_FAILURE;
    }

    bool const runsProtocol = target->HasTag("protocol");
    bool const runsGameplay = target->HasTag("gameplay");
    std::string const expectationsPath = FindExpectationsFile(configPath, targetName);
    std::unordered_map<std::string, std::string> const expectations =
        strict ? std::unordered_map<std::string, std::string>{} : LoadExpectations(expectationsPath);

    Catch::Session session;
    int const cliResult = session.applyCommandLine(int(passthrough.size()), passthrough.data());
    if (cliResult != 0)
    {
        sRealBotMgr.Stop();
        return cliResult;
    }

    std::vector<std::string> exclusionTerms;
    std::vector<std::pair<std::string, std::string>> excluded; // name, reason

    for (Catch::TestCaseHandle const& handle : Catch::getRegistryHub().getTestCaseRegistry().getAllTests())
    {
        Catch::TestCaseInfo const& info = handle.getTestCaseInfo();

        std::string reason;
        if (!runsProtocol && HasTag(info, "protocol"))
            reason = "not applicable to " + targetName + " ([protocol])";
        else if (!runsGameplay && HasTag(info, "gameplay"))
            reason = "not applicable to " + targetName + " ([gameplay])";
        else if (auto it = expectations.find(info.name); it != expectations.end())
        {
            reason = it->second;
            exclusionTerms.push_back("~\"" + info.name + "\"");
        }

        if (!reason.empty())
            excluded.emplace_back(info.name, reason);
    }

    if (!runsProtocol)
        exclusionTerms.push_back("~[protocol]");
    if (!runsGameplay)
        exclusionTerms.push_back("~[gameplay]");

    // Appending here (rather than replacing) ANDs with whatever test-spec the user's own extra
    // Catch2 args already put in configData().testsOrTags -- confirmed against
    // dep/catch2/extras/catch_amalgamated.cpp's TestSpecParser: separate parse() calls (one per
    // testsOrTags entry) keep accumulating into the same filter until a comma or the end, so
    // multiple entries AND together exactly like multiple CLI args would.
    for (std::string const& term : exclusionTerms)
        session.configData().testsOrTags.push_back(term);

    if (!strict)
        std::cout << "expectations: " << expectationsPath << (expectations.empty() ? " (none loaded)" : "") << "\n";
    std::cout << "target " << targetName << ": " << excluded.size()
        << " scenario" << (excluded.size() == 1 ? "" : "s") << " excluded\n";
    for (auto const& [name, reason] : excluded)
        std::cout << "  " << name << "\t" << reason << "\n";

    int const result = session.run();

    sRealBotMgr.Stop();
    return result;
}
