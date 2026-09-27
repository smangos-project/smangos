#ifndef DB_SCHEMA_SETUP_DBDFILE_H
#define DB_SCHEMA_SETUP_DBDFILE_H

#include "DbdBuild.h"

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

// A parse failure in a .dbd file, with the location it was found at.
struct DbdParseError : std::runtime_error
{
    std::string file;
    int line;

    DbdParseError(std::string file, int line, std::string const& message);
};

// One entry from the leading COLUMNS block.
struct DbdColumn
{
    std::string type; // "int", "float", "string" or "locstring"
    std::string foreignTable;
    std::string foreignColumn;
    bool verified = true;
    std::string comment;

    bool HasForeignKey() const { return !foreignTable.empty(); }
};

// One field line inside a version block, exactly as written (before joining with its column).
struct DbdFieldDefinition
{
    std::string name;
    int bits = 0;             // 0 when the line has no <size>
    bool isUnsigned = false;
    int arrayLength = 0;      // 0 when the line has no [length] (scalar)
    bool isId = false;
    bool isRelation = false;
    bool isNonInline = false;
    std::string comment;
};

// One `BUILD`/`LAYOUT` block: the builds it applies to and its ordered field list.
struct DbdVersionBlock
{
    std::vector<DbdBuild> builds;
    std::vector<DbdBuildRange> buildRanges;
    std::vector<std::string> layoutHashes;
    std::string comment;
    std::vector<DbdFieldDefinition> fields;
};

struct DbdFile
{
    std::unordered_map<std::string, DbdColumn> columns;
    std::vector<DbdVersionBlock> versions;
};

// Parses DBD source text. `fileName` is used only to attribute errors.
DbdFile ParseDbdText(std::string const& text, std::string const& fileName);

// Reads and parses a .dbd file from disk.
DbdFile ParseDbdFile(std::string const& path);

#endif
