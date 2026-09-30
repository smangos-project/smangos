#ifndef MANGOS_ARGPARSERFORSERVER_H
#define MANGOS_ARGPARSERFORSERVER_H

#include <string>
#include "nonstd/expected.hpp"

enum class ServiceDaemonAction
{
    NotSet,
    Start,
#ifdef WIN32
    Install,
    Uninstall,
#else
    Stop,
#endif
};

struct ServerStartupArguments
{
    ServiceDaemonAction inputServiceMode; // will be empty when no mode was set
    std::string configFilePath;
    std::string dumpStoresDir; // mangosd: write the loaded world stores here, then exit
    std::string serverDir;
};

/// Makes the paths absolute, fills defaults and changes into the server directory.
bool EnterServerDirectory(ServerStartupArguments& args, char const* defaultConfigName);

/// Loads <server dir>/config/common.conf, then the service's own file (args.configFilePath).
bool LoadServerConfig(ServerStartupArguments const& args);

/// Warns when common.conf is older than this build expects. Needs the config loaded.
void CheckCommonConfVersion();

/// Returns a parsed ServerStartupArguments or exit value
nonstd::expected<ServerStartupArguments, int /* exit value */> ParseServerStartupArguments(int argc, char** argv);

#endif // MANGOS_ARGPARSERFORSERVER_H
