#ifndef DATABASE_TOOL_PORTMIGRATIONFILE_H
#define DATABASE_TOOL_PORTMIGRATIONFILE_H

#include <filesystem>
#include <string>

struct PortMigrationFileResult
{
    bool flagged = false;
    std::string flagReason; // empty unless flagged
    std::string error;      // empty unless path itself isn't a <id>_world.sql migration
};

// Ports one legacy `<id>_world.sql` migration file in place (PortMysqlMigration):
// overwrites path with the SQLite-dialect output on success, leaves it
// untouched when flagged for a hand port (world-sql.md's "port-migration").
PortMigrationFileResult PortMigrationFile(std::filesystem::path const& path);

#endif
