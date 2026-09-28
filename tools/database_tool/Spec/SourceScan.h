#ifndef DATABASE_TOOL_SPEC_SOURCESCAN_H
#define DATABASE_TOOL_SPEC_SOURCESCAN_H

#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

// One member of a DBC/SQLStorage struct, in fmt-string position order. `width` is how many
// raw fmt/column positions it spans (array length, or 1 for a scalar); `isReal` is false for
// a commented-out member or a bare `// N` / `// N-M` position marker with no declaration at
// all (both stand for an `x` fmt character: consumed, but not a C++ field).
struct StructField
{
    std::string name;
    uint32_t width = 1;
    bool isReal = false;
};

// Walks the body of `struct StructName { ... }` or `class StructName { ... }` in `text`
// positionally, in source order: real members, commented-out members and bare position-marker
// comments all consume width; constructors, methods and access specifiers consume none. `#if`
// blocks inside the body (e.g. ItemSetEntry's build-gated tail fields) are resolved the same
// way as ParseDbcFmt. Numbered position comments (WMOAreaTableEntry's `// 10 ...`) are not
// needed to drive this walk -- they cross-check it instead, in tests. Returns an empty vector
// if the struct/class isn't found in `text`.
std::vector<StructField> ParseStructFields(std::string const& text, std::string const& structName);

// DBCfmt.h (and SQLStorages.cpp, which uses the same syntax): `char const <name>fmt[] =
// "...";` declarations, keyed by `<name>fmt`. A name defined under `#if`/`#elif`/`#else` picks
// one branch: this project's vanilla target is a fixed build, CLIENT_BUILD_1_12_1 (5875,
// cmake/options.cmake's default and the highest build this codebase defines), so a `>`/`==`/
// `<=` comparison against an earlier CLIENT_BUILD_1_x_y can always be resolved without reading
// CMake output. A condition this can't read (e.g. `defined(__GNUC__)`) falls back to the
// block's last textual branch.
std::map<std::string, std::string> ParseFmtStrings(std::string const& text);

struct DbcStorageLink
{
    std::string varName;
    std::string structName;
    std::string fmtName;
    uint32_t line = 0;
};

// DBCStores.cpp: `DBCStorage <Struct> var(Fmt);` declarations.
std::vector<DbcStorageLink> ParseDbcStorageLinks(std::string const& text);

struct DbcLoadCall
{
    std::string varName;
    std::string fileStem; // the .dbc file name, without extension, as written in the call
    uint32_t line = 0;
};

// DBCStores.cpp: `LoadDBC(locales, bar, errlist, var, path, "File.dbc");` calls in
// LoadDBCStores(), the only evidence that a format actually gets read.
std::vector<DbcLoadCall> ParseLoadDbcCalls(std::string const& text);

struct SqlStorageLink
{
    std::string varName;
    std::string fmtName; // the destination fmt, when a src/dst pair is given
    std::string key;
    std::string table;
    uint32_t line = 0;
};

// SQLStorages.cpp: `SQLStorage sXStorage(fmt[, dstFmt], "key", "table");` declarations. The
// load is `SELECT *` (SQLStorageImpl.h:242), so format position is column ordinal, same as a
// DBCStorage.
std::vector<SqlStorageLink> ParseSqlStorageLinks(std::string const& text);

// Counts `->fieldName` and `.fieldName` occurrences (whole-identifier match) in `text`. Field
// names collide across unrelated structs (many DBC structs have a `flags` member); that noise
// is accepted here rather than attempting type-aware analysis. Multi-file aggregation (and
// excluding a struct's own definition/loader file) is the caller's job -- see Evidence.cpp,
// which indexes a whole source tree once and looks candidate field names up in it, rather than
// rescanning the tree per field.
uint32_t CountFieldUsesInText(std::string const& text, std::string const& fieldName);

struct HandwrittenQuery
{
    std::string table;
    std::filesystem::path file;
    uint32_t line = 0;
    std::vector<std::string> columns; // empty when `parsable` is false
    bool parsable = false;
};

// Scans every `*.cpp` under `root` for `SELECT col1, col2, ... FROM table` string literals,
// including adjacent string-literal concatenation. A query is `parsable` only when its column
// list is plain identifiers (no `*`, function calls, subqueries or JOIN); anything else is
// still reported (table + site), for a whole-table `manual:` fallback.
std::vector<HandwrittenQuery> ScanHandwrittenQueries(std::filesystem::path const& root);

// For a `parsable` query, scans up to `searchWindowLines` lines of `text` after `query.line`
// for a `fields[N]` use of each column ordinal (`fields` is this codebase's near-universal
// result-row variable name; other names are not recognized). A column with no match keeps
// line 0. Any match -- a clean `member = fields[N].GetX();` assignment or not -- is reported
// the same way: the plan treats both as `manual:`, since there is no reusable struct field to
// count uses of here.
std::vector<uint32_t> FindFieldsAccessLines(std::string const& text, HandwrittenQuery const& query, uint32_t searchWindowLines = 100);

#endif
