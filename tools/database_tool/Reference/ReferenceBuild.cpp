#include "ReferenceBuild.h"

#include "Dbc/DbcHotfixes.h"
#include "Dbc/DbcSchema.h"
#include "Dbc/DbcTableMeta.h"
#include "Dbc/Wdbc.h"
#include "Dbc/Wdc5Reader.h"
#include "World/DoltLiteDatabase.h"
#include "World/VersionCollapse.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string_view>

namespace
{
    // One transaction per table is much faster than autocommit-per-row, but a single transaction
    // spanning all ~800 cdbc_ tables' DDL and inserts has been observed to crash DoltLite; commit
    // in batches instead, which keeps both properties.
    constexpr size_t kCommitBatchSize = 40;

    class BatchCommitter
    {
    public:
        explicit BatchCommitter(DoltLiteDatabase& db) : m_db(db) { m_db.Begin(); }
        ~BatchCommitter() { m_db.Commit(); }

        BatchCommitter(BatchCommitter const&) = delete;
        BatchCommitter& operator=(BatchCommitter const&) = delete;

        // Call once per table, whether it succeeded or failed.
        void Tick()
        {
            if (++m_count >= kCommitBatchSize)
            {
                m_db.Commit();
                m_db.Begin();
                m_count = 0;
            }
        }

    private:
        DoltLiteDatabase& m_db;
        size_t m_count = 0;
    };

    struct SourceColumn
    {
        std::string name;
        std::string type;
        int primaryKeyIndex = 0; // 0 = not part of the primary key
    };

    std::vector<SourceColumn> ReadSourceColumns(DoltLiteDatabase& db, std::string const& qualifiedTable)
    {
        std::vector<SourceColumn> columns;
        DoltLiteStatement info = db.Prepare("PRAGMA " + qualifiedTable);
        while (info.Step())
        {
            SourceColumn column;
            column.name = info.ColumnText(1);
            column.type = info.ColumnText(2);
            column.primaryKeyIndex = static_cast<int>(info.ColumnInt64(5));
            columns.push_back(std::move(column));
        }
        return columns;
    }

    std::vector<std::string> PrimaryKeyColumns(std::vector<SourceColumn> const& columns)
    {
        std::vector<SourceColumn const*> pk;
        for (SourceColumn const& column : columns)
            if (column.primaryKeyIndex > 0)
                pk.push_back(&column);
        std::sort(pk.begin(), pk.end(),
                  [](SourceColumn const* a, SourceColumn const* b) { return a->primaryKeyIndex < b->primaryKeyIndex; });

        std::vector<std::string> names;
        for (SourceColumn const* column : pk)
            names.push_back(column->name);
        return names;
    }

    std::string QuoteIdentifierList(std::vector<std::string> const& names)
    {
        std::string joined;
        for (size_t i = 0; i < names.size(); ++i)
            joined += (i ? ", \"" : "\"") + names[i] + "\"";
        return joined;
    }

    // Mirrors a srv_* table's columns, types and primary key from its .world source.
    std::string BuildMirrorDdl(std::string const& tableName, std::vector<SourceColumn> const& columns,
                                std::vector<std::string> const& primaryKey)
    {
        std::ostringstream ddl;
        ddl << "CREATE TABLE \"" << tableName << "\" (";
        for (size_t i = 0; i < columns.size(); ++i)
            ddl << (i ? ", \"" : "\"") << columns[i].name << "\" " << columns[i].type;
        if (!primaryKey.empty())
            ddl << ", PRIMARY KEY(" << QuoteIdentifierList(primaryKey) << ")";
        ddl << ")";
        return ddl.str();
    }

    std::vector<std::string> NonDbcWorldTables(DoltLiteDatabase& reference)
    {
        std::vector<std::string> names;
        DoltLiteStatement stmt = reference.Prepare(
            "SELECT name FROM src.sqlite_master WHERE type='table' "
            "AND name NOT LIKE 'dbc\\_%' ESCAPE '\\' AND name NOT IN ('migrations', 'dbc_files') "
            "AND name NOT LIKE 'dolt\\_%' ESCAPE '\\' AND name NOT LIKE 'sqlite\\_%' ESCAPE '\\' "
            "ORDER BY name");
        while (stmt.Step())
            names.push_back(stmt.ColumnText(0));
        return names;
    }

    void BuildOneSrvTable(DoltLiteDatabase& reference, std::string const& source, ReferenceBuildResult& result)
    {
        std::vector<SourceColumn> columns = ReadSourceColumns(reference, "src.table_info(\"" + source + "\")");
        std::vector<std::string> columnNames;
        for (SourceColumn const& column : columns)
            columnNames.push_back(column.name);
        std::vector<std::string> primaryKey = PrimaryKeyColumns(columns);

        std::string destTable = "srv_" + source;
        reference.Execute("DROP TABLE IF EXISTS \"" + destTable + "\"");
        reference.Execute(BuildMirrorDdl(destTable, columns, primaryKey));

        VersionColumnInfo versionInfo = DetectVersionColumn(columnNames);
        uint64_t rowCount = 0;

        if (versionInfo.kind == VersionColumnKind::None)
        {
            reference.Execute("INSERT INTO \"" + destTable + "\" SELECT * FROM src.\"" + source + "\"");
            DoltLiteStatement count = reference.Prepare("SELECT COUNT(*) FROM \"" + destTable + "\"");
            count.Step();
            rowCount = static_cast<uint64_t>(count.ColumnInt64(0));
        }
        else
        {
            // CollapseTableInto reads and writes within one connection; stage a local copy of
            // the attached source table so it can filter it.
            std::string stageTable = "__reference_stage_" + source;
            reference.Execute("DROP TABLE IF EXISTS \"" + stageTable + "\"");
            reference.Execute(BuildMirrorDdl(stageTable, columns, primaryKey));
            reference.Execute("INSERT INTO \"" + stageTable + "\" SELECT * FROM src.\"" + source + "\"");

            std::vector<std::string> keyColumns;
            for (std::string const& column : primaryKey)
                if (column != versionInfo.column && column != versionInfo.minColumn && column != versionInfo.maxColumn)
                    keyColumns.push_back(column);

            CollapseReport report =
                CollapseTableInto(reference, stageTable, destTable, columnNames, keyColumns, versionInfo);
            rowCount = report.rowsAfter;

            reference.Execute("DROP TABLE \"" + stageTable + "\"");
        }

        result.tables.push_back({ destTable, "srv", rowCount });
    }

    void BuildSrvTables(DoltLiteDatabase& reference, std::filesystem::path const& worldPath,
                         ReferenceBuildResult& result)
    {
        reference.Execute("ATTACH DATABASE '" + worldPath.string() + "' AS src");

        {
            BatchCommitter committer(reference);
            for (std::string const& source : NonDbcWorldTables(reference))
            {
                try
                {
                    BuildOneSrvTable(reference, source, result);
                }
                catch (std::exception const& e)
                {
                    result.tableErrors.push_back(source + ": " + e.what());
                }
                committer.Tick();
            }
        }

        reference.Execute("DETACH DATABASE src");
    }

    // Populates one already-DROPped table from `rows`, per `table`'s field metadata, with
    // `locStringSlots` slots per locstring field (1 for client rows, 9 for vanilla WDBC rows).
    void PopulateDbcTable(DoltLiteDatabase& reference, DbcTableMeta const& table, std::string const& sqlTableName,
                           std::vector<Wdc5Row> const& rows, uint32_t locStringSlots)
    {
        reference.Execute("DROP TABLE IF EXISTS \"" + sqlTableName + "\"");
        reference.Execute(BuildDbcTableDdl(table, sqlTableName, locStringSlots));
        for (std::string const& indexSql : BuildDbcTableIndexDdl(table, sqlTableName, locStringSlots))
            reference.Execute(indexSql);

        if (rows.empty())
            return;

        std::vector<std::string> columnNames;
        for (uint32_t i = 0; i < table.fieldCount; ++i)
            for (std::string const& name : DbcColumnNames(table.fields[i], locStringSlots))
                columnNames.push_back(name);

        std::ostringstream sql;
        sql << "INSERT INTO \"" << sqlTableName << "\" (" << QuoteIdentifierList(columnNames) << ") VALUES (";
        for (size_t i = 0; i < columnNames.size(); ++i)
            sql << (i ? ", ?" : "?");
        sql << ")";

        DoltLiteStatement insert = reference.Prepare(sql.str());
        for (Wdc5Row const& row : rows)
            BindDbcRow(insert, table, row, locStringSlots);
    }

    std::filesystem::path FindDbcFile(std::filesystem::path const& dir, std::string_view stem)
    {
        auto lower = [](std::string text)
        {
            for (char& c : text)
                c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            return text;
        };

        std::string const wanted = lower(std::string(stem) + ".dbc");
        for (std::filesystem::directory_entry const& entry : std::filesystem::directory_iterator(dir))
            if (lower(entry.path().filename().string()) == wanted)
                return entry.path();
        return {};
    }

    void BuildOneVdbcTable(DoltLiteDatabase& reference, DbcTableMeta const& table,
                            std::filesystem::path const& vanillaDbcDir, WdbcLayout const& layout,
                            ReferenceBuildResult& result)
    {
        std::filesystem::path filePath = FindDbcFile(vanillaDbcDir, table.fileStem);
        if (filePath.empty())
        {
            result.tableErrors.push_back(std::string(table.name) + ": file not found");
            return;
        }

        try
        {
            WdbcReadResult read = ReadWdbcFile(filePath, table, layout);
            if (!read.error.empty())
            {
                result.tableErrors.push_back(std::string(table.name) + ": " + read.error);
                return;
            }

            PopulateDbcTable(reference, table, table.sqlTable, read.rows, layout.locStringSlots);
            result.tables.push_back({ table.sqlTable, "vdbc", static_cast<uint64_t>(read.rows.size()) });
        }
        catch (std::exception const& e)
        {
            result.tableErrors.push_back(std::string(table.name) + ": " + e.what());
        }
    }

    void BuildVdbcTables(DoltLiteDatabase& reference, std::filesystem::path const& vanillaDbcDir,
                         ReferenceBuildResult& result)
    {
        WdbcLayout const layout{ 9 };
        BatchCommitter committer(reference);
        for (DbcTableMeta const& table : GetVanillaDbcTables())
        {
            BuildOneVdbcTable(reference, table, vanillaDbcDir, layout, result);
            committer.Tick();
        }
    }

    // Attaches `partPath` (a self-contained DoltLite file that was populated via `result.tables`'
    // entries for `source`) to `reference` and copies each table across with a plain bulk
    // CREATE TABLE ... AS SELECT, then detaches and deletes it. `reference` is always a freshly
    // created file (BuildReferenceDatabase refuses to overwrite one that exists), so a table
    // under this name can never already be there -- no DROP is needed, and (found by testing)
    // issuing one anyway, even as a no-op "IF EXISTS" against a name that isn't present, made
    // DoltLite (pinned 0.50.12) stop seeing every table in the attached database for the rest of
    // this transaction. A table that still fails to copy is recorded in `result.tableErrors` and
    // dropped from `result.tables`, rather than failing the whole command over one table.
    void MergeIsolatedPart(DoltLiteDatabase& reference, std::filesystem::path const& partPath,
                            char const* source, ReferenceBuildResult& result)
    {
        reference.Execute("ATTACH DATABASE '" + partPath.string() + "' AS mergesrc");
        {
            BatchCommitter committer(reference);
            for (ReferenceTableInfo& info : result.tables)
            {
                if (info.source != source)
                    continue;

                try
                {
                    reference.Execute("CREATE TABLE \"" + info.table + "\" AS SELECT * FROM mergesrc.\"" + info.table + "\"");
                }
                catch (std::exception const& e)
                {
                    result.tableErrors.push_back(info.table + ": merge: " + e.what());
                    info.source.clear(); // filtered out below; never really landed in `reference`
                }
                committer.Tick();
            }
        }
        reference.Execute("DETACH DATABASE mergesrc");
        std::filesystem::remove(partPath);

        std::erase_if(result.tables, [](ReferenceTableInfo const& info) { return info.source.empty(); });
    }

    // Returns false when there is no file to import (not an error: 431 of 844 client files are
    // empty/absent), so the caller can skip ticking the commit batch for it.
    bool BuildOneCdbcTable(DoltLiteDatabase& reference, DbcTableMeta const& table,
                            std::filesystem::path const& clientDbfilesclientDir, ReferenceBuildResult& result)
    {
        std::filesystem::path filePath = clientDbfilesclientDir / (std::string(table.fileStem) + ".db2");
        if (!std::filesystem::exists(filePath))
            return false;

        try
        {
            Wdc5ReadResult read = ReadWdc5File(filePath, table);
            if (!read.error.empty())
            {
                result.tableErrors.push_back(std::string(table.name) + ": " + read.error);
                return true;
            }

            std::string sqlTableName = "cdbc_" + std::string(table.fileStem);
            PopulateDbcTable(reference, table, sqlTableName, read.rows, /*locStringSlots*/ 1);
            result.tables.push_back({ sqlTableName, "cdbc", static_cast<uint64_t>(read.rows.size()) });
        }
        catch (std::exception const& e)
        {
            result.tableErrors.push_back(std::string(table.name) + ": " + e.what());
        }
        return true;
    }

    void BuildCdbcTables(DoltLiteDatabase& reference, std::filesystem::path const& clientDbfilesclientDir,
                         ReferenceBuildResult& result)
    {
        BatchCommitter committer(reference);
        for (DbcTableMeta const& table : GetDbcTables())
            if (BuildOneCdbcTable(reference, table, clientDbfilesclientDir, result))
                committer.Tick();
    }

    std::string BuildReferenceTablesDdl()
    {
        return "CREATE TABLE \"reference_tables\" ("
               "\"sql_table\" TEXT PRIMARY KEY, "
               "\"source\" TEXT NOT NULL, "
               "\"row_count\" INTEGER NOT NULL)";
    }

    void WriteReferenceTables(DoltLiteDatabase& reference, std::vector<ReferenceTableInfo> const& tables)
    {
        reference.Execute("DROP TABLE IF EXISTS \"reference_tables\"");
        reference.Execute(BuildReferenceTablesDdl());

        reference.Begin();
        DoltLiteStatement insert =
            reference.Prepare("INSERT INTO \"reference_tables\" (\"sql_table\", \"source\", \"row_count\") "
                               "VALUES (?, ?, ?)");
        for (ReferenceTableInfo const& info : tables)
        {
            insert.BindText(1, info.table);
            insert.BindText(2, info.source);
            insert.BindInt64(3, static_cast<int64_t>(info.rowCount));
            insert.Step();
            insert.Reset();
        }
        reference.Commit();
    }
}

void LoadVanillaDbcTables(DoltLiteDatabase& db, std::filesystem::path const& vanillaDbcDir, ReferenceBuildResult& result)
{
    BuildVdbcTables(db, vanillaDbcDir, result);
}

ReferenceBuildResult BuildReferenceDatabase(std::filesystem::path const& referencePath,
                                             std::filesystem::path const& worldPath,
                                             std::filesystem::path const& vanillaDbcDir,
                                             std::filesystem::path const& clientDbfilesclientDir,
                                             std::filesystem::path const& hotfixesDir)
{
    ReferenceBuildResult result;

    if (std::filesystem::exists(referencePath))
    {
        result.alreadyExisted = true;
        result.error = "already exists: " + referencePath.string() + " (delete it to recreate)";
        return result;
    }

    DoltLiteDatabase reference(referencePath.string());

    try
    {
        BuildSrvTables(reference, worldPath, result);

        // vdbc_* and cdbc_* are each built in their own fresh, self-contained file first, then
        // merged in with CREATE TABLE ... AS SELECT (see MergeIsolatedPart), rather than being
        // populated directly alongside srv_*'s content: doing that crashed DoltLite (pinned
        // 0.50.12) partway through a full real-data run.
        std::filesystem::path vdbcPartPath = referencePath.parent_path() / "reference.vdbc-part.doltlite";
        std::filesystem::remove(vdbcPartPath);
        {
            DoltLiteDatabase vdbcPart(vdbcPartPath.string());
            BuildVdbcTables(vdbcPart, vanillaDbcDir, result);
        }
        MergeIsolatedPart(reference, vdbcPartPath, "vdbc", result);

        std::filesystem::path cdbcPartPath = referencePath.parent_path() / "reference.cdbc-part.doltlite";
        std::filesystem::remove(cdbcPartPath);
        {
            DoltLiteDatabase cdbcPart(cdbcPartPath.string());
            BuildCdbcTables(cdbcPart, clientDbfilesclientDir, result);
            DbcHotfixResult hotfixes = ApplyDbcHotfixes(cdbcPart, hotfixesDir, "cdbc_");
            if (!hotfixes.error.empty())
                throw std::runtime_error("hotfixes: " + hotfixes.error);
        }
        MergeIsolatedPart(reference, cdbcPartPath, "cdbc", result);

        WriteReferenceTables(reference, result.tables);
    }
    catch (std::exception const& e)
    {
        result.error = e.what();
    }

    return result;
}
