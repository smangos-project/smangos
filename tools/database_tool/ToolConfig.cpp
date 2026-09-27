#include "ToolConfig.h"

#include "Config/Config.h"

bool LoadToolConfig(std::string const& path, ToolConfig& out)
{
    if (!sConfig.LoadFromFile(path))
        return false;

    out.loginDatabaseInfo = sConfig.GetStringDefault("LoginDatabase.Info", "");
    out.characterDatabaseInfo = sConfig.GetStringDefault("CharacterDatabase.Info", "");
    out.logsDatabaseInfo = sConfig.GetStringDefault("LogsDatabase.Info", "");
    return true;
}
