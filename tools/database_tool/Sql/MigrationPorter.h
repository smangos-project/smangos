#ifndef DATABASE_TOOL_MIGRATIONPORTER_H
#define DATABASE_TOOL_MIGRATIONPORTER_H

#include <string>

struct PortedMigration
{
    std::string content;    // the new SQLite-dialect file content; empty when flagged
    bool flagged = false;
    std::string flagReason; // empty unless flagged
};

// Ports one legacy world migration file's content (still wrapped in the
// DROP PROCEDURE/DELIMITER/CREATE PROCEDURE envelope every world migration
// has) to the SQLite dialect world-sql.md documents: the envelope is
// stripped, leaving the `INSERT INTO migrations` line and the migration's
// own body, with the mechanical rewrites from world-sql.md's
// "port-migration" table applied (string literal re-escaping, @variable
// substitution, TRUNCATE/CONCAT/INSERT IGNORE, CREATE TABLE via
// MysqlDdlTranslator). id is the migration's own id (from its filename),
// used to locate the envelope's own `INSERT INTO `migrations` VALUES
// ('<id>')` marker.
//
// Anything the table calls out as needing a hand port -- ALTER TABLE, a
// multi-table UPDATE, or an @variable assignment whose right-hand side isn't
// a literal/copy/increment -- comes back flagged instead of ported, with
// content left empty so a caller never overwrites the original file.
PortedMigration PortMysqlMigration(std::string const& id, std::string const& fileContent);

#endif
