#include "Evidence.h"

#include "Spec/SourceScan.h"
#include "Spec/SpecFile.h"
#include "Spec/WireHint.h"
#include "World/DoltLiteDatabase.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>
#include <regex>
#include <set>
#include <sstream>

struct ColumnInfo
{
    std::string name;
    std::string type;
};

static std::vector<std::string> TablesWithPrefix(DoltLiteDatabase& db, std::string const& prefix)
{
    std::vector<std::string> names;
    DoltLiteStatement stmt = db.Prepare(
        "SELECT name FROM sqlite_master WHERE type = 'table' AND name LIKE '" + prefix + "\\_%' ESCAPE '\\' ORDER BY name");
    while (stmt.Step())
        names.push_back(stmt.ColumnText(0));
    return names;
}

static std::vector<ColumnInfo> TableColumnInfos(DoltLiteDatabase& db, std::string const& table)
{
    std::vector<ColumnInfo> columns;
    DoltLiteStatement stmt = db.Prepare("PRAGMA table_info(\"" + table + "\")");
    while (stmt.Step())
        columns.push_back({ stmt.ColumnText(1), stmt.ColumnText(2) });
    return columns;
}

static std::string ReadFile(std::filesystem::path const& path)
{
    std::ifstream in(path);
    std::stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

static std::string ToLowerCopy(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

static std::string RelPath(std::filesystem::path const& path, std::filesystem::path const& repoRoot)
{
    return std::filesystem::relative(path, repoRoot).generic_string();
}

// A source tree, read once and reused for every field-usage lookup and struct/loader search
// this file does -- indexing member accesses this way (below) instead of grepping the tree
// once per candidate field name is what keeps `spec evidence` fast against ~800 files.
struct SourceTree
{
    std::vector<std::filesystem::path> files;
    std::map<std::filesystem::path, std::string> textByFile;
};

static SourceTree LoadSourceTree(std::vector<std::filesystem::path> const& roots)
{
    SourceTree tree;
    for (std::filesystem::path const& root : roots)
    {
        if (!std::filesystem::exists(root))
            continue;
        for (std::filesystem::directory_entry const& entry : std::filesystem::recursive_directory_iterator(root))
        {
            if (!entry.is_regular_file())
                continue;
            std::string ext = entry.path().extension().string();
            if (ext != ".cpp" && ext != ".h")
                continue;
            std::filesystem::path canonical = std::filesystem::weakly_canonical(entry.path());
            tree.files.push_back(canonical);
            tree.textByFile[canonical] = ReadFile(canonical);
        }
    }
    return tree;
}

// fieldName -> (file -> use count), built in one pass over `tree` so that resolving N struct
// fields costs one scan, not N.
static std::map<std::string, std::map<std::filesystem::path, uint32_t>> BuildFieldUsageIndex(SourceTree const& tree)
{
    std::map<std::string, std::map<std::filesystem::path, uint32_t>> index;
    static std::regex const accessRegex(R"([.>]\s*([A-Za-z_]\w*))");
    for (std::filesystem::path const& file : tree.files)
    {
        std::string const& text = tree.textByFile.at(file);
        for (auto it = std::sregex_iterator(text.begin(), text.end(), accessRegex); it != std::sregex_iterator(); ++it)
            ++index[(*it)[1].str()][file];
    }
    return index;
}

struct FieldUsage
{
    uint32_t uses = 0;
    uint32_t files = 0;
};

static FieldUsage LookupFieldUsage(std::map<std::string, std::map<std::filesystem::path, uint32_t>> const& index,
    std::string const& fieldName, std::vector<std::filesystem::path> const& excludeFiles)
{
    FieldUsage usage;
    auto it = index.find(fieldName);
    if (it == index.end())
        return usage;
    for (auto const& [file, count] : it->second)
    {
        bool excluded = std::any_of(excludeFiles.begin(), excludeFiles.end(), [&](std::filesystem::path const& skip)
        { return std::filesystem::weakly_canonical(skip) == file; });
        if (excluded)
            continue;
        usage.uses += count;
        ++usage.files;
    }
    return usage;
}

static std::string FindLookupEntryStructName(SourceTree const& tree, std::string const& varName)
{
    std::regex const pattern(varName + R"(\s*\.\s*LookupEntry\s*<\s*(\w+)\s*>)");
    for (std::filesystem::path const& file : tree.files)
    {
        std::smatch m;
        std::string const& text = tree.textByFile.at(file);
        if (std::regex_search(text, m, pattern))
            return m[1].str();
    }
    return "";
}

static std::filesystem::path FindStructDefinitionFile(SourceTree const& tree, std::string const& structName)
{
    std::regex const pattern(R"(^\s*(?:struct|class)\s+)" + structName + R"(\b)");
    for (std::filesystem::path const& file : tree.files)
    {
        if (file.extension() != ".h")
            continue;
        std::istringstream stream(tree.textByFile.at(file));
        std::string line;
        while (std::getline(stream, line))
        {
            if (std::regex_search(line, pattern))
                return file;
        }
    }
    return {};
}

// Maps each 0-based fmt/column position to the field that covers it (expanding each field's
// declared width). Fails (empty result) if the widths don't add up to exactly `length`.
static bool ResolvePositions(std::vector<StructField> const& structFields, size_t length, std::vector<StructField const*>& perPosition)
{
    perPosition.assign(length, nullptr);
    size_t pos = 0;
    for (StructField const& field : structFields)
    {
        for (uint32_t w = 0; w < field.width; ++w)
        {
            if (pos >= length)
                return false;
            perPosition[pos] = &field;
            ++pos;
        }
    }
    return pos == length;
}

struct ReaderResolution
{
    bool ok = false;
    std::vector<std::string> perColumn;
};

static ReaderResolution ResolveStructReaders(std::string const& structDefText, std::string const& structName, std::string const& fmtString,
    size_t columnCount, std::map<std::string, std::map<std::filesystem::path, uint32_t>> const& usageIndex,
    std::vector<std::filesystem::path> const& excludeFiles, std::map<std::string, std::string>& cache)
{
    ReaderResolution result;
    std::vector<StructField> fields = ParseStructFields(structDefText, structName);
    if (fields.empty() || fmtString.size() != columnCount)
        return result;

    std::vector<StructField const*> perPosition;
    if (!ResolvePositions(fields, fmtString.size(), perPosition))
        return result;

    result.perColumn.reserve(columnCount);
    for (size_t i = 0; i < columnCount; ++i)
    {
        StructField const* field = perPosition[i];
        if (!field || !field->isReal)
        {
            result.perColumn.push_back("none");
            continue;
        }

        std::string cacheKey = structName + "::" + field->name;
        auto cacheIt = cache.find(cacheKey);
        if (cacheIt == cache.end())
        {
            FieldUsage usage = LookupFieldUsage(usageIndex, field->name, excludeFiles);
            std::string value = usage.uses == 0
                ? "none"
                : structName + "::" + field->name + " (" + std::to_string(usage.uses) + " uses in " + std::to_string(usage.files) + " files)";
            cacheIt = cache.emplace(cacheKey, value).first;
        }
        result.perColumn.push_back(cacheIt->second);
    }
    result.ok = true;
    return result;
}

static std::vector<std::filesystem::path> FindPacketCodeFiles(SourceTree const& tree)
{
    std::vector<std::filesystem::path> files;
    for (std::filesystem::path const& file : tree.files)
        if (IsPacketCodeFile(file, tree.textByFile.at(file)))
            files.push_back(file);
    return files;
}

// Pulls the field name back out of a resolved `readers` value ("Struct::field (N uses in M
// files)"); empty for "none" and "manual:..." rows, which have no field to grep for.
static std::string ExtractResolvedFieldName(std::string const& readers)
{
    static std::regex const pattern(R"(^\w+::(\w+) \(\d+ uses in \d+ files\)$)");
    std::smatch m;
    return std::regex_match(readers, m, pattern) ? m[1].str() : "";
}

static std::string ComputeWireHint(std::string const& readers, std::vector<std::filesystem::path> const& packetFiles,
    SourceTree const& tree, std::filesystem::path const& repoRoot, std::map<std::string, std::string>& cache)
{
    std::string fieldName = ExtractResolvedFieldName(readers);
    if (fieldName.empty())
        return "";

    auto cacheIt = cache.find(fieldName);
    if (cacheIt != cache.end())
        return cacheIt->second;

    std::vector<WireHintUse> uses;
    for (std::filesystem::path const& file : packetFiles)
    {
        std::vector<WireHintUse> fileUses = FindFieldUseLines(file, tree.textByFile.at(file), fieldName);
        uses.insert(uses.end(), fileUses.begin(), fileUses.end());
    }

    std::string value = "wire=no";
    if (!uses.empty())
    {
        std::ostringstream out;
        out << "wire=yes";
        for (WireHintUse const& use : uses)
            out << " " << RelPath(use.file, repoRoot) << ":" << use.line << ",";
        value = out.str();
        value.pop_back(); // trailing comma
    }

    cache[fieldName] = value;
    return value;
}

struct DbcTableEvidence
{
    std::string structName;
    std::string fmtString;
    uint32_t loadLine = 0;
};

static std::map<std::string, DbcTableEvidence> BuildDbcTableEvidence(std::string const& fmtText, std::string const& storesText)
{
    std::map<std::string, std::string> fmts = ParseFmtStrings(fmtText);
    std::vector<DbcStorageLink> links = ParseDbcStorageLinks(storesText);
    std::vector<DbcLoadCall> loads = ParseLoadDbcCalls(storesText);

    std::map<std::string, DbcLoadCall const*> loadByVar;
    for (DbcLoadCall const& call : loads)
        loadByVar[call.varName] = &call;

    std::map<std::string, DbcTableEvidence> byFileStem;
    for (DbcStorageLink const& link : links)
    {
        auto loadIt = loadByVar.find(link.varName);
        if (loadIt == loadByVar.end())
            continue; // declared but never loaded -- e.g. AreaTableEntryfmt has no such storage at all
        auto fmtIt = fmts.find(link.fmtName);
        if (fmtIt == fmts.end())
            continue;
        byFileStem[ToLowerCopy(loadIt->second->fileStem)] = { link.structName, fmtIt->second, loadIt->second->line };
    }
    return byFileStem;
}

struct SqlTableEvidence
{
    std::string structName;
    std::string fmtString;
    uint32_t declLine = 0;
};

static std::map<std::string, SqlTableEvidence> BuildSqlTableEvidence(std::string const& sqlStoragesText, SourceTree const& tree)
{
    std::map<std::string, std::string> fmts = ParseFmtStrings(sqlStoragesText);
    std::vector<SqlStorageLink> links = ParseSqlStorageLinks(sqlStoragesText);

    std::map<std::string, SqlTableEvidence> byTable;
    for (SqlStorageLink const& link : links)
    {
        auto fmtIt = fmts.find(link.fmtName);
        if (fmtIt == fmts.end())
            continue;
        std::string structName = FindLookupEntryStructName(tree, link.varName);
        if (structName.empty())
            continue;
        byTable[link.table] = { structName, fmtIt->second, link.line };
    }
    return byTable;
}

struct HandwrittenTableEvidence
{
    bool wholeTableManual = false;
    std::string manualSite; // "<file>:<line>", no "manual:" prefix
    std::map<std::string, std::string> perColumnSite; // column -> "<file>:<line>", no prefix
};

static std::map<std::string, HandwrittenTableEvidence> BuildHandwrittenEvidence(std::filesystem::path const& gameRoot,
    std::filesystem::path const& repoRoot, SourceTree const& tree)
{
    std::vector<HandwrittenQuery> queries = ScanHandwrittenQueries(gameRoot);

    std::map<std::string, HandwrittenTableEvidence> byTable;
    for (HandwrittenQuery const& query : queries)
    {
        std::string site = RelPath(query.file, repoRoot) + ":" + std::to_string(query.line);
        HandwrittenTableEvidence& entry = byTable[query.table];

        if (!query.parsable)
        {
            if (!entry.wholeTableManual)
            {
                entry.wholeTableManual = true;
                entry.manualSite = site;
            }
            continue;
        }

        auto textIt = tree.textByFile.find(std::filesystem::weakly_canonical(query.file));
        if (textIt == tree.textByFile.end())
            continue;
        std::vector<uint32_t> hitLines = FindFieldsAccessLines(textIt->second, query);
        for (size_t i = 0; i < query.columns.size(); ++i)
        {
            if (hitLines[i] == 0 || entry.perColumnSite.count(query.columns[i]))
                continue;
            entry.perColumnSite[query.columns[i]] = RelPath(query.file, repoRoot) + ":" + std::to_string(hitLines[i]);
        }
    }
    return byTable;
}

SpecEvidenceResult BuildEvidence(std::filesystem::path const& referencePath, std::filesystem::path const& repoRoot,
    std::filesystem::path const& specDir)
{
    SpecEvidenceResult result;
    if (!std::filesystem::exists(referencePath))
    {
        result.error = referencePath.string() + " does not exist (run reference first)";
        return result;
    }

    DoltLiteDatabase reference(referencePath.string());

    std::filesystem::path gameRoot = repoRoot / "src" / "game";
    std::filesystem::path scriptsRoot = repoRoot / "src" / "scripts";
    std::filesystem::path dbcfmtPath = gameRoot / "Database" / "DBCfmt.h";
    std::filesystem::path dbcStoresPath = gameRoot / "Database" / "DBCStores.cpp";
    std::filesystem::path dbcStructurePath = gameRoot / "Database" / "DBCStructure.h";
    std::filesystem::path sqlStoragesPath = gameRoot / "Database" / "SQLStorages.cpp";

    SourceTree tree = LoadSourceTree({ gameRoot, scriptsRoot });
    std::map<std::string, std::map<std::filesystem::path, uint32_t>> usageIndex = BuildFieldUsageIndex(tree);

    std::string dbcStructureText = ReadFile(dbcStructurePath);
    std::map<std::string, DbcTableEvidence> dbcTables = BuildDbcTableEvidence(ReadFile(dbcfmtPath), ReadFile(dbcStoresPath));
    std::map<std::string, SqlTableEvidence> sqlTables = BuildSqlTableEvidence(ReadFile(sqlStoragesPath), tree);
    std::map<std::string, HandwrittenTableEvidence> handwritten = BuildHandwrittenEvidence(gameRoot, repoRoot, tree);

    std::map<std::string, std::string> readerCache;
    std::vector<SpecEvidenceRow> rows;

    for (std::string const& qualified : TablesWithPrefix(reference, "vdbc"))
    {
        std::string table = qualified.substr(5);
        std::vector<ColumnInfo> columns = TableColumnInfos(reference, qualified);

        auto dbcIt = dbcTables.find(table);
        ReaderResolution resolution;
        std::string wholeTableManual;
        if (dbcIt != dbcTables.end())
        {
            resolution = ResolveStructReaders(dbcStructureText, dbcIt->second.structName, dbcIt->second.fmtString,
                columns.size(), usageIndex, { std::filesystem::weakly_canonical(dbcStructurePath), std::filesystem::weakly_canonical(dbcStoresPath) },
                readerCache);
            if (!resolution.ok)
                wholeTableManual = "manual:" + RelPath(dbcStoresPath, repoRoot) + ":" + std::to_string(dbcIt->second.loadLine);
        }

        for (size_t i = 0; i < columns.size(); ++i)
        {
            std::string readers = "none";
            if (dbcIt != dbcTables.end())
                readers = resolution.ok ? resolution.perColumn[i] : wholeTableManual;
            rows.push_back({ "vdbc", table, columns[i].name, columns[i].type, readers, "", "", "", "", "" });
            ++result.rowsWritten;
        }
    }

    for (std::string const& qualified : TablesWithPrefix(reference, "srv"))
    {
        std::string table = qualified.substr(4);
        std::vector<ColumnInfo> columns = TableColumnInfos(reference, qualified);

        auto sqlIt = sqlTables.find(table);
        if (sqlIt != sqlTables.end())
        {
            std::filesystem::path structFile = FindStructDefinitionFile(tree, sqlIt->second.structName);
            ReaderResolution resolution;
            if (!structFile.empty())
            {
                resolution = ResolveStructReaders(tree.textByFile.at(structFile), sqlIt->second.structName, sqlIt->second.fmtString,
                    columns.size(), usageIndex, { structFile, std::filesystem::weakly_canonical(sqlStoragesPath) }, readerCache);
            }
            std::string wholeTableManual = "manual:" + RelPath(sqlStoragesPath, repoRoot) + ":" + std::to_string(sqlIt->second.declLine);

            for (size_t i = 0; i < columns.size(); ++i)
            {
                rows.push_back({ "srv", table, columns[i].name, columns[i].type, resolution.ok ? resolution.perColumn[i] : wholeTableManual,
                                  "", "", "", "", "" });
                ++result.rowsWritten;
            }
            continue;
        }

        auto handIt = handwritten.find(table);
        for (size_t i = 0; i < columns.size(); ++i)
        {
            std::string readers = "none";
            if (handIt != handwritten.end())
            {
                if (handIt->second.wholeTableManual)
                    readers = "manual:" + handIt->second.manualSite;
                else
                {
                    auto colIt = handIt->second.perColumnSite.find(columns[i].name);
                    if (colIt != handIt->second.perColumnSite.end())
                        readers = "manual:" + colIt->second;
                }
            }
            rows.push_back({ "srv", table, columns[i].name, columns[i].type, readers, "", "", "", "", "" });
            ++result.rowsWritten;
        }
    }

    std::vector<std::filesystem::path> packetFiles = FindPacketCodeFiles(tree);
    std::map<std::string, std::string> wireHintCache;
    for (SpecEvidenceRow& row : rows)
        row.wireHint = ComputeWireHint(row.readers, packetFiles, tree, repoRoot, wireHintCache);

    WriteEvidenceTsv(specDir / "evidence.tsv", rows);
    return result;
}
