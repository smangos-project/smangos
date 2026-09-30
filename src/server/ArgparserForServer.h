#ifndef MANGOS_ARGPARSERFORSERVER_H
#define MANGOS_ARGPARSERFORSERVER_H

#include <string>
#include "nonstd/expected.hpp"

struct ServerServices
{
    bool mangosd = false;
    bool bnetserver = false;
};

struct ServerStartupArguments
{
    std::string configFilePath; // common.conf
    std::string dumpStoresDir; // mangosd: write the loaded world stores here, then exit
    std::string serverDir;
    std::string services; // comma-separated list, empty when not given
};

/// Makes the paths absolute, fills defaults and changes into the server directory.
bool EnterServerDirectory(ServerStartupArguments& args);

/// Picks the services from --services, else Server.Services of common.conf (default: both).
bool ResolveServices(ServerStartupArguments const& args, ServerServices& services);

/// Loads common.conf, then the file of each selected service from <server dir>/config.
bool LoadServerConfig(ServerStartupArguments const& args, ServerServices const& services);

/// Warns when common.conf is older than this build expects. Needs the config loaded.
void CheckCommonConfVersion();

/// Creates the PID file named by PidFile, if any.
bool CreateServerPidFile();

/// Applies UseProcessors and ProcessPriority (Windows only).
void ApplyProcessSettings();

/// Returns a parsed ServerStartupArguments or exit value
nonstd::expected<ServerStartupArguments, int /* exit value */> ParseServerStartupArguments(int argc, char** argv);

#endif // MANGOS_ARGPARSERFORSERVER_H
