#ifndef DATABASE_TOOL_TOOLCONFIG_H
#define DATABASE_TOOL_TOOLCONFIG_H

#include <string>

// The MySQL connection strings this tool needs out of a mangosd.conf-style
// file, using the same keys and "host;port;user;pass;db" format the server
// itself reads (src/shared/Database/Database.cpp SqlConnection::Initialize).
struct ToolConfig
{
    std::string loginDatabaseInfo;
    std::string characterDatabaseInfo;
    std::string logsDatabaseInfo;
};

// Returns false if the file couldn't be opened. Missing keys are left blank.
bool LoadToolConfig(std::string const& path, ToolConfig& out);

#endif
