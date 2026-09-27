#ifndef DB_SCHEMA_SETUP_DBDRESOLVE_H
#define DB_SCHEMA_SETUP_DBDRESOLVE_H

#include "DbdBuild.h"
#include "DbdFile.h"

#include <optional>
#include <string>
#include <vector>

// A version-block field joined with its column, ready for a generator to consume.
struct DbdResolvedField
{
    std::string name;
    std::string type; // "int", "float", "string" or "locstring"
    int bits = 0;      // 0 when the field has no explicit size
    bool isUnsigned = false;
    int arrayLength = 1;
    bool isId = false;
    bool isRelation = false;
    bool isNonInline = false;
    std::string foreignTable;
    std::string foreignColumn;
    bool verified = true;
    std::string columnComment;
    std::string fieldComment;

    bool HasForeignKey() const { return !foreignTable.empty(); }
};

struct DbdTable
{
    std::string name;
    std::vector<DbdBuild> matchedBuilds;
    std::vector<DbdBuildRange> matchedBuildRanges;
    std::vector<std::string> layoutHashes;
    std::string comment;
    std::vector<DbdResolvedField> fields;
};

enum class DbdResolveStatus
{
    Resolved,
    NoDefinition,
    Ambiguous, // more than one version block matched the same build
};

struct DbdResolveResult
{
    DbdResolveStatus status = DbdResolveStatus::NoDefinition;
    std::optional<DbdTable> table;
};

// Resolves `tableName` (a `DbdFile` parsed from `<tableName>.dbd`) against a build.
// `buildsInPriorityOrder` is tried in order; the first build with exactly one matching version
// block wins. A build matching more than one block is an immediate error (Ambiguous), even if a
// later build in the list would have resolved cleanly.
DbdResolveResult ResolveDbdTable(DbdFile const& file, std::string const& tableName,
                                  std::vector<DbdBuild> const& buildsInPriorityOrder);

#endif
