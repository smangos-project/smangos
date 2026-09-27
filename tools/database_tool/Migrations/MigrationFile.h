#ifndef DATABASE_TOOL_MIGRATIONFILE_H
#define DATABASE_TOOL_MIGRATIONFILE_H

#include <filesystem>
#include <string>
#include <vector>

// One file from sql/migrations, named `<id>_<db>.sql`. id is a
// YYYYmmddHHMMSS timestamp, but it's only ever compared as text, never
// treated as a date or number, matching how vmangos itself identifies
// migrations.
struct MigrationFile
{
    std::string id;   // e.g. "20210515110157"
    std::string db;   // "world", "characters", "logon" or "logs"
    std::filesystem::path path;

    // True while the file still has the DROP PROCEDURE/DELIMITER/CREATE
    // PROCEDURE envelope vmangos wraps every migration in. World migrations
    // lose this once ported to SQLite dialect (MigrationPorter); the other
    // three databases stay MySQL and keep it forever.
    bool hasLegacyEnvelope = false;
};

// Parses one migration filename (just the name, not a full path). Returns
// false for anything else found in sql/migrations (README, merge.sh, the
// generated *_db_updates.sql files, ...).
bool TryParseMigrationFilename(std::string const& filename, std::string& id, std::string& db);

// Reads every `<id>_<db>.sql` migration for one database out of directory,
// sorted by id (== filename order, since ids are timestamps).
std::vector<MigrationFile> DiscoverMigrations(std::filesystem::path const& directory, std::string const& db);

#endif
