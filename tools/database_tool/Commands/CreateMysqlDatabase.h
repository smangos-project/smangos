#ifndef DATABASE_TOOL_CREATEMYSQLDATABASE_H
#define DATABASE_TOOL_CREATEMYSQLDATABASE_H

#include <filesystem>
#include <string>
#include <vector>

struct MysqlSetupResult
{
    bool databaseCreated = false;     // the schema didn't exist before this call
    bool baseScriptImported = false;  // this call ran baseScriptPath (only happens once, ever, per database)
    std::vector<std::string> appliedMigrationIds;
    std::string error; // empty on success
};

// Creates (if missing) and brings one MySQL database (logon/characters/logs)
// fully up to date, the way vmangos' own recipe does:
//   1. CREATE DATABASE IF NOT EXISTS ... DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci
//   2. the base script, but only the first time (detected by the `migrations`
//      table it creates not existing yet)
//   3. every pending migration in filename order (MigrationRunner)
//
// Connects twice: once with no database selected (to create it, since
// mysql_real_connect fails outright for a database that doesn't exist yet),
// then again with connectionInfo's own database once it exists -- a
// DatabaseMysql connection pool's database is fixed at connect time, and a
// later `USE` only affects whichever single physical connection runs it, not
// the pool DirectExecute()/Query() otherwise pick.
// connectionInfo's password can't be empty: shared/Util.cpp's StrSplit drops
// empty tokens, so an empty password shifts every field after it left by one
// (the database name silently becomes the password instead).
MysqlSetupResult CreateAndUpdateMysqlDatabase(
    std::string const& connectionInfo, // "host;port;user;pass;db", as in mangosd.conf
    std::filesystem::path const& baseScriptPath,
    std::filesystem::path const& migrationsDirectory,
    std::string const& migrationDbSuffix); // "logon"/"characters"/"logs" -- selects <id>_<suffix>.sql files

#endif
