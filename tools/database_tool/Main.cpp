#include "Version.h"

#include "Commands/CreateWorldDatabase.h"
#include "Commands/GenerateWms.h"
#include "Commands/PortMigrationFile.h"
#include "Dbc/DbcExport.h"
#include "Dbc/DbcImport.h"
#include "Dbc/InputManifest.h"
#include "Fdid/FileDataIds.h"
#include "Reference/ReferenceBuild.h"
#include "Spec/Evidence.h"
#include "Spec/GapAnalysis.h"
#include "Spec/SpecCheck.h"
#include "Spec/SpecRender.h"
#include "Spec/SpecInventory.h"
#include "Verify/Verify.h"
#include "World/DoltLiteDatabase.h"
#include "World/WorldCompare.h"

#include "Database/DatabaseEnv.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

namespace
{
    std::filesystem::path SourceRoot()
    {
        return std::filesystem::path(DATABASE_TOOL_SOURCE_DIR);
    }

    // Where .world, .client and sql/migrations live; DATABASE_TOOL_ROOT lets tests point at a scratch copy.
    std::filesystem::path DataRoot()
    {
        if (char const* override = std::getenv("DATABASE_TOOL_ROOT"))
            return std::filesystem::path(override);
        return SourceRoot();
    }

    std::filesystem::path DbfilesclientDir()
    {
        return DataRoot() / ".client" / "Content" / "dbfilesclient";
    }

    std::filesystem::path InputsDir()
    {
        return SourceRoot() / "tools" / "database_tool" / "Inputs";
    }

    std::filesystem::path VanillaDbcDir()
    {
        return DataRoot() / ".client" / "vanilla" / "dbc";
    }

    // The stock client db2s (backups win over files `export --client` overwrote), checked
    // against the committed manifest. Empty path and a message in `error` on mismatch.
    std::filesystem::path StockDbfilesclientDir(std::string& error)
    {
        std::filesystem::path view = BuildStockClientView(DbfilesclientDir(),
            DataRoot() / ".world" / "dbfilesclient-stock", DataRoot() / ".world" / "cache" / "client-db2-stock");
        error = VerifyInputManifest(view, InputsDir() / "client-db2.sha256");
        return error.empty() ? view : std::filesystem::path();
    }

    // pin-inputs: rewrites both manifests from the current stock inputs.
    int RunPinInputs()
    {
        std::string error;
        std::filesystem::path view = BuildStockClientView(DbfilesclientDir(),
            DataRoot() / ".world" / "dbfilesclient-stock", DataRoot() / ".world" / "cache" / "client-db2-stock");
        error = WriteInputManifest(view, ".db2", InputsDir() / "client-db2.sha256");
        if (error.empty())
            error = WriteInputManifest(VanillaDbcDir(), ".dbc", InputsDir() / "vanilla-dbc.sha256");
        if (!error.empty())
        {
            std::printf("pin-inputs: %s\n", error.c_str());
            return 1;
        }
        std::printf("pin-inputs: wrote manifests to %s\n", InputsDir().string().c_str());
        return 0;
    }

    bool HasFlag(std::vector<std::string> const& args, std::string const& flag)
    {
        return std::find(args.begin(), args.end(), flag) != args.end();
    }

    std::filesystem::path WorldPath()
    {
        return DataRoot() / ".world" / "world.doltlite";
    }

    std::filesystem::path FdidIndexPath()
    {
        return DataRoot() / ".client" / "Content" / "fdid.index";
    }

    void PrintFdidImport(char const* label, FdidImportResult const& result)
    {
        std::printf("fdid import %s: %u added, %u unchanged, %zu conflicts\n", label, result.added, result.unchanged,
            result.conflicts.size());
        for (std::string const& conflict : result.conflicts)
            std::printf("  conflict: %s\n", conflict.c_str());
    }

    // Returns the number of tables that failed to import.
    uint32_t PrintDbcImportSummary(char const* format, DbcImportResult const& result)
    {
        uint32_t imported = 0, missing = 0, failed = 0;
        for (DbcImportTableResult const& table : result.tables)
        {
            if (!table.fileFound)
                ++missing;
            else if (!table.error.empty())
            {
                std::printf("%s: %s\n", table.tableName.c_str(), table.error.c_str());
                ++failed;
            }
            else
                ++imported;
        }
        std::printf("import %s: %u tables imported, %u missing, %u failed\n", format, imported, missing, failed);
        return failed;
    }

    // Empty means "every database".
    bool WantsDatabase(std::vector<std::string> const& databases, std::string const& name)
    {
        return databases.empty() || std::find(databases.begin(), databases.end(), name) != databases.end();
    }

    std::vector<std::string> SplitCsv(std::string const& csv)
    {
        std::vector<std::string> result;
        std::string current;
        for (char c : csv)
        {
            if (c == ',')
            {
                result.push_back(current);
                current.clear();
            }
            else
            {
                current += c;
            }
        }
        result.push_back(current);
        return result;
    }

    std::vector<std::string> ParseDatabasesFlag(std::vector<std::string> const& args)
    {
        for (size_t i = 0; i < args.size(); ++i)
            if (args[i] == "--databases" && i + 1 < args.size())
                return SplitCsv(args[i + 1]);
        return {};
    }

    int RunCreate(std::vector<std::string> const& args)
    {
        if (!WantsDatabase(ParseDatabasesFlag(args), "world"))
        {
            std::printf("create: only --databases world is implemented so far\n");
            return 1;
        }

        bool const baseOnly = HasFlag(args, "--base-only");
        std::filesystem::path const worldPath = DataRoot() / ".world" / (baseOnly ? "base.doltlite" : "world.doltlite");
        std::filesystem::path const cacheDirectory = DataRoot() / ".world" / "cache";
        std::filesystem::path const emptyMigrations = DataRoot() / ".world" / "no-migrations";
        if (baseOnly)
            std::filesystem::create_directories(emptyMigrations);
        std::filesystem::path const migrationsDirectory = baseOnly ? emptyMigrations : DataRoot() / "sql" / "migrations";
        bool const importDbc = !baseOnly && !HasFlag(args, "--no-dbc-import");

        std::string inputError;
        std::filesystem::path const clientDir = StockDbfilesclientDir(inputError);
        if (!inputError.empty())
        {
            std::printf("create world: %s\n", inputError.c_str());
            return 1;
        }

        WorldCreateResult result = CreateWorldDatabase(worldPath, cacheDirectory, migrationsDirectory, clientDir,
            DataRoot() / "sql" / "hotfixes", importDbc);
        if (!result.error.empty())
        {
            std::printf("create world: %s\n", result.error.c_str());
            return 1;
        }

        std::string listfile;
        for (size_t i = 0; i + 1 < args.size(); ++i)
            if (args[i] == "--listfile")
                listfile = args[i + 1];
        if (!listfile.empty())
        {
            DoltLiteDatabase world(worldPath.string());
            FdidImportResult fdids = ImportFdidFile(world, listfile, "listfile");
            if (!fdids.error.empty())
            {
                std::printf("create world: listfile: %s\n", fdids.error.c_str());
                return 1;
            }
            PrintFdidImport(listfile.c_str(), fdids);
        }

        std::printf("create world: imported %u tables, %llu rows; applied %zu migrations\n",
            result.dumpImport.tablesCreated, static_cast<unsigned long long>(result.dumpImport.rowsInserted),
            result.update.appliedMigrationIds.size());
        if (importDbc)
        {
            PrintDbcImportSummary("db2", result.dbcImport);
            std::printf("create world: applied %llu hotfix statements from %zu files\n",
                static_cast<unsigned long long>(result.hotfixes.statements), result.hotfixes.files.size());
        }
        return 0;
    }

    // Writes Content/fdid.index from file_data_id and drops the old custom.index.
    int RunFdidExportIndex(DoltLiteDatabase& world)
    {
        FdidExportResult result = ExportFdidIndex(world, FdidIndexPath());
        if (!result.error.empty())
        {
            std::printf("fdid export: %s\n", result.error.c_str());
            return 1;
        }
        std::error_code ec;
        std::filesystem::remove(FdidIndexPath().parent_path() / "custom.index", ec);
        std::printf("fdid export: %u rows, %s\n", result.rows, result.written ? "written" : "unchanged");
        return 0;
    }

    // fdid import --listfile FILE | --custom FILE...; fdid export; fdid allocate PATH...; fdid retire PATH...
    int RunFdid(std::vector<std::string> const& args)
    {
        if (args.empty())
        {
            std::printf("fdid: subcommand required (import, export, allocate, retire)\n");
            return 1;
        }
        if (!std::filesystem::exists(WorldPath()))
        {
            std::printf("fdid: %s does not exist (run create first)\n", WorldPath().string().c_str());
            return 1;
        }

        std::string const subcommand = args[0];
        std::vector<std::string> const rest(args.begin() + 1, args.end());
        DoltLiteDatabase world(WorldPath().string());
        try
        {
            if (subcommand == "export")
                return RunFdidExportIndex(world);

            if (subcommand == "import")
            {
                bool failed = false;
                std::string mode;
                for (std::string const& arg : rest)
                {
                    if (arg == "--listfile" || arg == "--custom")
                    {
                        mode = arg;
                        continue;
                    }
                    std::string const source = mode == "--listfile" ? "listfile" : "import";
                    if (mode.empty())
                    {
                        std::printf("fdid import: --listfile FILE or --custom FILE... required\n");
                        return 1;
                    }
                    FdidImportResult result = ImportFdidFile(world, arg, source);
                    if (!result.error.empty())
                    {
                        std::printf("fdid import: %s\n", result.error.c_str());
                        failed = true;
                        continue;
                    }
                    PrintFdidImport(arg.c_str(), result);
                }
                return failed || mode.empty() ? 1 : 0;
            }

            if (subcommand == "allocate" || subcommand == "retire")
            {
                if (rest.empty())
                {
                    std::printf("fdid %s: at least one path required\n", subcommand.c_str());
                    return 1;
                }
                if (subcommand == "retire")
                {
                    std::printf("fdid retire: %u retired\n", RetireFdids(world, rest));
                    return 0;
                }
                std::vector<int64_t> const ids = AllocateFdids(world, rest, "import");
                for (size_t i = 0; i < ids.size(); ++i)
                    std::printf("%lld;%s\n", static_cast<long long>(ids[i]), NormalizeClientPath(rest[i]).c_str());
                return 0;
            }
        }
        catch (std::exception const& e)
        {
            std::printf("fdid %s: %s\n", subcommand.c_str(), e.what());
            return 1;
        }

        std::printf("fdid: unknown subcommand: %s\n", subcommand.c_str());
        return 1;
    }

    // import --format db2|dbc [--tables A,B] [--dir D]
    int RunImport(std::vector<std::string> const& args)
    {
        std::string format;
        std::vector<std::string> onlyTables;
        std::filesystem::path directory;
        for (size_t i = 0; i < args.size(); ++i)
        {
            if (args[i] == "--format" && i + 1 < args.size())
                format = args[++i];
            else if (args[i] == "--tables" && i + 1 < args.size())
                onlyTables = SplitCsv(args[++i]);
            else if (args[i] == "--dir" && i + 1 < args.size())
                directory = args[++i];
        }

        if (format != "db2" && format != "dbc")
        {
            std::printf("import: --format db2|dbc is required\n");
            return 1;
        }

        if (directory.empty())
        {
            std::string inputError;
            directory = format == "db2" ? StockDbfilesclientDir(inputError) : VanillaDbcDir();
            if (!inputError.empty())
            {
                std::printf("import: %s\n", inputError.c_str());
                return 1;
            }
        }

        std::filesystem::path const worldPath = DataRoot() / ".world" / "world.doltlite";
        if (!std::filesystem::exists(worldPath))
        {
            std::printf("import: %s does not exist (run create --base-only first)\n", worldPath.string().c_str());
            return 1;
        }

        DoltLiteDatabase world(worldPath.string());
        DbcImportResult result = format == "db2"
            ? ImportDb2Files(world, directory, onlyTables)
            : ImportDbcFiles(world, directory, onlyTables);
        if (!result.error.empty())
        {
            std::printf("import: %s\n", result.error.c_str());
            return 1;
        }

        return PrintDbcImportSummary(format.c_str(), result) ? 1 : 0;
    }

    // export --format db2|dbc [--tables A,B] [--dir D] [--client]
    int RunExport(std::vector<std::string> const& args)
    {
        bool toClient = false;
        std::string format;
        std::vector<std::string> onlyTables;
        std::filesystem::path directory = DataRoot() / ".world" / "dbfilesclient";
        for (size_t i = 0; i < args.size(); ++i)
        {
            if (args[i] == "--format" && i + 1 < args.size())
                format = args[++i];
            else if (args[i] == "--tables" && i + 1 < args.size())
                onlyTables = SplitCsv(args[++i]);
            else if (args[i] == "--dir" && i + 1 < args.size())
                directory = args[++i];
            else if (args[i] == "--client")
            {
                toClient = true;
                directory = DbfilesclientDir();
            }
        }

        if (toClient && format != "db2")
        {
            std::printf("export: --client needs --format db2\n");
            return 1;
        }

        if (format != "db2" && format != "dbc")
        {
            std::printf("export: --format db2|dbc is required\n");
            return 1;
        }

        std::filesystem::path const worldPath = DataRoot() / ".world" / "world.doltlite";
        if (!std::filesystem::exists(worldPath))
        {
            std::printf("export: %s does not exist (run create --base-only first)\n", worldPath.string().c_str());
            return 1;
        }

        std::error_code ec;
        std::filesystem::create_directories(directory, ec);
        if (ec)
        {
            std::printf("export: cannot create %s: %s\n", directory.string().c_str(), ec.message().c_str());
            return 1;
        }

        DoltLiteDatabase world(worldPath.string());
        ExportDb2Options options;
        if (toClient)
        {
            options.onlyChanged = true;
            options.backupDir = DataRoot() / ".world" / "dbfilesclient-stock";
        }
        DbcExportResult result = format == "db2"
            ? ExportDb2Files(world, directory, onlyTables, GetDbcTables(), options)
            : ExportDbcFiles(world, directory, onlyTables);
        if (!result.error.empty())
        {
            std::printf("export: %s\n", result.error.c_str());
            return 1;
        }

        uint32_t exported = 0, missing = 0, failed = 0, unchanged = 0;
        for (DbcExportTableResult const& table : result.tables)
        {
            if (table.unchanged)
                ++unchanged;
            else if (!table.hasFilesRow)
                ++missing;
            else if (!table.error.empty())
            {
                std::printf("%s: %s\n", table.tableName.c_str(), table.error.c_str());
                ++failed;
            }
            else
                ++exported;
        }
        std::printf("export %s: %u tables exported, %u unchanged, %u with no dbc_files row, %u failed\n",
            format.c_str(), exported, unchanged, missing, failed);
        if (toClient && RunFdidExportIndex(world) != 0)
            return 1;
        return failed ? 1 : 0;
    }

    int RunUpdate(std::vector<std::string> const& args)
    {
        if (!WantsDatabase(ParseDatabasesFlag(args), "world"))
        {
            std::printf("update: only --databases world is implemented so far\n");
            return 1;
        }

        std::filesystem::path const worldPath = DataRoot() / ".world" / "world.doltlite";
        std::filesystem::path const migrationsDirectory = DataRoot() / "sql" / "migrations";

        if (!std::filesystem::exists(worldPath))
        {
            std::printf("update world: %s does not exist (run create --base-only first)\n", worldPath.string().c_str());
            return 1;
        }

        DoltLiteDatabase world(worldPath.string());
        WorldUpdateResult result = UpdateWorldDatabase(world, migrationsDirectory);
        if (!result.error.empty())
        {
            std::printf("update world: %s: %s\n", result.failedFile.c_str(), result.error.c_str());
            return 1;
        }

        std::printf("update world: applied %zu migrations\n", result.appliedMigrationIds.size());
        return 0;
    }

    // compare-world --mysql "<host;port;user;pass;db>"
    int RunCompareWorld(std::vector<std::string> const& args)
    {
        std::string mysqlInfo;
        for (size_t i = 0; i < args.size(); ++i)
            if (args[i] == "--mysql" && i + 1 < args.size())
                mysqlInfo = args[i + 1];

        if (mysqlInfo.empty())
        {
            std::printf("compare-world: --mysql \"<host;port;user;pass;db>\" is required\n");
            return 1;
        }

        std::filesystem::path const worldPath = DataRoot() / ".world" / "world.doltlite";
        if (!std::filesystem::exists(worldPath))
        {
            std::printf("compare-world: %s does not exist (run create --base-only first)\n", worldPath.string().c_str());
            return 1;
        }

        DoltLiteDatabase world(worldPath.string());

        DatabaseMysql mysql;
        if (!mysql.Initialize(mysqlInfo.c_str(), 1, 0))
        {
            std::printf("compare-world: could not connect to MySQL: %s\n", mysqlInfo.c_str());
            return 1;
        }

        WorldCompareResult result = CompareWorldWithMysql(world, mysql);

        for (std::string const& table : result.tablesOnlyInWorld)
            std::printf("only in .world: %s\n", table.c_str());
        for (std::string const& table : result.tablesOnlyInMysql)
            std::printf("only in mysql: %s\n", table.c_str());

        for (WorldTableComparison const& table : result.tableComparisons)
        {
            if (!table.columnsMatch)
            {
                std::printf("%s: columns differ\n  .world: ", table.table.c_str());
                for (std::string const& col : table.worldColumns)
                    std::printf("%s ", col.c_str());
                std::printf("\n  mysql:  ");
                for (std::string const& col : table.mysqlColumns)
                    std::printf("%s ", col.c_str());
                std::printf("\n");
                continue;
            }

            if (!table.rowCountMatches)
                std::printf("%s: row count differs (.world=%llu, mysql=%llu)\n", table.table.c_str(),
                    static_cast<unsigned long long>(table.worldRowCount),
                    static_cast<unsigned long long>(table.mysqlRowCount));

            for (WorldRowDifference const& diff : table.rowDifferences)
                std::printf("%s: row %llu, column %s differs (.world=%s, mysql=%s)\n", table.table.c_str(),
                    static_cast<unsigned long long>(diff.rowIndex), diff.column.c_str(),
                    diff.worldValue.c_str(), diff.mysqlValue.c_str());
        }

        std::printf("%zu tables compared, %zu only in .world, %zu only in mysql\n",
            result.tableComparisons.size(), result.tablesOnlyInWorld.size(), result.tablesOnlyInMysql.size());

        if (result.Identical())
        {
            std::printf("compare-world: identical\n");
            return 0;
        }
        return 1;
    }

    // reference
    int RunReference(std::vector<std::string> const& args)
    {
        (void)args;

        std::filesystem::path const worldPath = DataRoot() / ".world" / "base.doltlite";
        std::filesystem::path const referencePath = DataRoot() / ".world" / "reference.doltlite";
        std::filesystem::path const vanillaDbcDir = DataRoot() / ".client" / "vanilla" / "dbc";

        if (!std::filesystem::exists(worldPath))
        {
            std::printf("reference: %s does not exist (run create --base-only first)\n", worldPath.string().c_str());
            return 1;
        }

        std::string inputError = VerifyInputManifest(vanillaDbcDir, InputsDir() / "vanilla-dbc.sha256");
        std::filesystem::path const clientDir = inputError.empty() ? StockDbfilesclientDir(inputError) : std::filesystem::path();
        if (!inputError.empty())
        {
            std::printf("reference: %s\n", inputError.c_str());
            return 1;
        }

        ReferenceBuildResult result = BuildReferenceDatabase(referencePath, worldPath, vanillaDbcDir, clientDir,
            DataRoot() / "sql" / "hotfixes");
        for (std::string const& error : result.tableErrors)
            std::printf("%s\n", error.c_str());
        if (!result.error.empty())
        {
            std::printf("reference: %s\n", result.error.c_str());
            return 1;
        }

        uint32_t srv = 0, vdbc = 0, cdbc = 0;
        for (ReferenceTableInfo const& table : result.tables)
        {
            if (table.source == "srv") ++srv;
            else if (table.source == "vdbc") ++vdbc;
            else if (table.source == "cdbc") ++cdbc;
        }
        std::printf("reference: %u srv_* tables, %u vdbc_* tables, %u cdbc_* tables, %zu errors\n", srv, vdbc, cdbc,
            result.tableErrors.size());
        return result.tableErrors.empty() ? 0 : 1;
    }

    std::filesystem::path SpecDir()
    {
        return SourceRoot() / "tools" / "database_tool" / "Spec";
    }

    // spec init
    int RunSpecInit(std::vector<std::string> const& args)
    {
        (void)args;

        std::filesystem::path const referencePath = DataRoot() / ".world" / "reference.doltlite";
        SpecInitResult result = UpdateSpecInventory(referencePath, SpecDir());
        if (!result.error.empty())
        {
            std::printf("spec init: %s\n", result.error.c_str());
            return 1;
        }

        for (std::string const& missing : result.missingColumns)
            std::printf("columns.tsv: %s is no longer in the reference database\n", missing.c_str());
        for (std::string const& missing : result.missingDbcColumns)
            std::printf("dbc-columns.tsv: %s is no longer in the reference database\n", missing.c_str());
        std::printf("spec init: %u columns.tsv rows added, %u dbc-columns.tsv rows added\n",
            result.columnsAdded, result.dbcColumnsAdded);
        return 0;
    }

    // spec set --source S --table T [--column C] [--disposition D] [--target T] [--rule R]
    //          [--reason "..."] [--reviewed yes|no] [--overwrite]
    int RunSpecSet(std::vector<std::string> const& args)
    {
        SpecSetFilter filter;
        SpecSetValues values;
        bool overwrite = false;

        for (size_t i = 0; i < args.size(); ++i)
        {
            auto takeValue = [&]() -> std::string { return i + 1 < args.size() ? args[++i] : std::string(); };
            if (args[i] == "--source") filter.source = takeValue();
            else if (args[i] == "--table") filter.table = takeValue();
            else if (args[i] == "--column") filter.column = takeValue();
            else if (args[i] == "--disposition") values.disposition = takeValue();
            else if (args[i] == "--target") values.target = takeValue();
            else if (args[i] == "--rule") values.rule = takeValue();
            else if (args[i] == "--reason") values.reason = takeValue();
            else if (args[i] == "--reviewed") values.reviewed = takeValue();
            else if (args[i] == "--overwrite") overwrite = true;
        }

        if (filter.source.empty() || filter.table.empty())
        {
            std::printf("spec set: --source and --table are required\n");
            return 1;
        }

        SpecSetResult result = SetSpecColumns(SpecDir(), filter, values, overwrite);
        if (!result.error.empty())
        {
            std::printf("spec set: %s\n", result.error.c_str());
            return 1;
        }

        std::printf("spec set: %u rows matched, %u updated\n", result.matched, result.updated);
        return 0;
    }

    // spec evidence
    int RunSpecEvidence(std::vector<std::string> const& args)
    {
        (void)args;

        std::filesystem::path const referencePath = DataRoot() / ".world" / "reference.doltlite";
        SpecEvidenceResult result = BuildEvidence(referencePath, SourceRoot(), SpecDir());
        if (!result.error.empty())
        {
            std::printf("spec evidence: %s\n", result.error.c_str());
            return 1;
        }

        std::printf("spec evidence: %u rows written, %u pairs joined, %u pairs skipped\n", result.rowsWritten,
            result.pairsJoined, result.pairsSkipped);
        return 0;
    }

#ifndef DB_SCHEMA_SETUP_DEFINITIONS_DIR
#define DB_SCHEMA_SETUP_DEFINITIONS_DIR ""
#endif

    // spec gaps
    int RunSpecGaps(std::vector<std::string> const& args)
    {
        (void)args;

        std::filesystem::path const referencePath = DataRoot() / ".world" / "reference.doltlite";
        SpecGapResult result = BuildGaps(referencePath, DB_SCHEMA_SETUP_DEFINITIONS_DIR, SpecDir());
        if (!result.error.empty())
        {
            std::printf("spec gaps: %s\n", result.error.c_str());
            return 1;
        }

        std::printf("spec gaps: %u rows written\n", result.rowsWritten);
        return 0;
    }

    // spec check [--final]
    int RunSpecCheck(std::vector<std::string> const& args)
    {
        bool final = std::find(args.begin(), args.end(), "--final") != args.end();

        std::filesystem::path const referencePath = DataRoot() / ".world" / "reference.doltlite";
        SpecCheckResult result;
        try
        {
            result = CheckSpec(referencePath, SpecDir(), final);
        }
        catch (std::exception const& e)
        {
            result.error = e.what();
        }
        if (!result.error.empty())
        {
            std::printf("spec check: %s\n", result.error.c_str());
            return 1;
        }

        constexpr uint32_t printedPerCheck = 25;
        uint32_t perCheck[10] = {};
        for (SpecCheckProblem const& problem : result.problems)
        {
            if (++perCheck[problem.check] <= printedPerCheck)
                std::printf("[%u] %s\n", problem.check, problem.message.c_str());
        }
        for (uint32_t check = 1; check < 10; ++check)
        {
            if (perCheck[check] > printedPerCheck)
                std::printf("[%u] ... and %u more\n", check, perCheck[check] - printedPerCheck);
        }

        std::printf("spec check: %u columns.tsv rows, %zu problems\n", result.columnsChecked, result.problems.size());
        return result.problems.empty() ? 0 : 1;
    }

    // spec render
    int RunSpecRender(std::vector<std::string> const& args)
    {
        (void)args;

        SpecRenderResult result = RenderSpec(SpecDir(), SourceRoot() / ".local" / "plan" / "bot" / "spec");
        if (!result.error.empty())
        {
            std::printf("spec render: %s\n", result.error.c_str());
            return 1;
        }

        std::printf("spec render: %u pages written\n", result.pagesWritten);
        return 0;
    }

    // spec query "<select>" [--world]
    int RunSpecQuery(std::vector<std::string> const& args)
    {
        if (args.empty())
        {
            std::printf("spec query: a SELECT statement is required\n");
            return 1;
        }

        bool const useWorld = args.size() > 1 && args[1] == "--world";
        std::string const& sql = args[0];
        size_t first = sql.find_first_not_of(" \t\n");
        if (first == std::string::npos || (sql.compare(first, 6, "SELECT") != 0 && sql.compare(first, 6, "select") != 0 &&
            sql.compare(first, 4, "WITH") != 0 && sql.compare(first, 4, "with") != 0))
        {
            std::printf("spec query: only SELECT statements are allowed\n");
            return 1;
        }

        try
        {
            DoltLiteDatabase db((DataRoot() / ".world" / (useWorld ? "world.doltlite" : "reference.doltlite")).string());
            DoltLiteStatement stmt = db.Prepare(sql);
            int const count = stmt.ColumnCount();
            while (stmt.Step())
            {
                for (int i = 0; i < count; ++i)
                    std::printf("%s%s", i ? "\t" : "", stmt.ColumnIsNull(i) ? "NULL" : stmt.ColumnText(i).c_str());
                std::printf("\n");
            }
        }
        catch (std::exception const& e)
        {
            std::printf("spec query: %s\n", e.what());
            return 1;
        }
        return 0;
    }

    int RunSpec(std::vector<std::string> const& args)
    {
        if (args.empty())
        {
            std::printf("spec: subcommand required (init, set, evidence, gaps, check, render, query)\n");
            return 1;
        }

        std::string const subcommand = args[0];
        std::vector<std::string> const rest(args.begin() + 1, args.end());

        if (subcommand == "init")
            return RunSpecInit(rest);
        if (subcommand == "set")
            return RunSpecSet(rest);
        if (subcommand == "evidence")
            return RunSpecEvidence(rest);
        if (subcommand == "gaps")
            return RunSpecGaps(rest);
        if (subcommand == "check")
            return RunSpecCheck(rest);
        if (subcommand == "query")
            return RunSpecQuery(rest);
        if (subcommand == "render")
            return RunSpecRender(rest);

        std::printf("spec: unknown subcommand: %s\n", subcommand.c_str());
        return 1;
    }

    // verify [--final] [--update] [--checks V1,V3] [--tables a,b]
    // verify init [--vdbc Name,Name|all]
    int RunVerifyCommand(std::vector<std::string> const& args)
    {
        std::filesystem::path const referencePath = DataRoot() / ".world" / "reference.doltlite";
        std::filesystem::path const verifyDir = SourceRoot() / "tools" / "database_tool" / "Verify";

        if (!args.empty() && args[0] == "init")
        {
            std::vector<std::string> vanillaTables;
            for (size_t i = 1; i + 1 < args.size(); ++i)
                if (args[i] == "--vdbc")
                    vanillaTables = SplitCsv(args[i + 1]);

            VerifyInitResult result = InitVerifyMappings(referencePath, SpecDir(), verifyDir, vanillaTables);
            if (!result.error.empty())
            {
                std::printf("verify init: %s\n", result.error.c_str());
                return 1;
            }
            for (std::string const& skipped : result.skipped)
                std::printf("skipped %s\n", skipped.c_str());
            std::printf("verify init: %u mapping files written\n", result.written);
            return 0;
        }

        VerifyOptions options;
        for (size_t i = 0; i < args.size(); ++i)
        {
            if (args[i] == "--final")
                options.final = true;
            else if (args[i] == "--update")
                options.update = true;
            else if (args[i] == "--checks" && i + 1 < args.size())
                options.checks = SplitCsv(args[++i]);
            else if (args[i] == "--tables" && i + 1 < args.size())
                options.tables = SplitCsv(args[++i]);
            else
            {
                std::printf("verify: unknown argument: %s\n", args[i].c_str());
                return 1;
            }
        }

        VerifyResult result = RunVerify(DataRoot() / ".world" / "world.doltlite", referencePath, SpecDir(), verifyDir, options);
        if (!result.error.empty())
        {
            std::printf("verify: %s\n", result.error.c_str());
            return 1;
        }

        for (std::string const& note : result.notes)
            std::printf("note: %s\n", note.c_str());
        for (VerifyProblem const& problem : result.problems)
            std::printf("[%s] %s: %s\n", problem.check.c_str(), problem.table.c_str(), problem.message.c_str());
        std::printf("verify: %u tables compared, %zu problems\n", result.tablesCompared, result.problems.size());
        return result.problems.empty() ? 0 : 1;
    }

    int RunPortMigration(std::vector<std::string> const& args)
    {
        if (args.empty())
        {
            std::printf("port-migration: no files given\n");
            return 1;
        }

        bool anyFlagged = false;
        for (std::string const& arg : args)
        {
            PortMigrationFileResult result = PortMigrationFile(arg);
            if (!result.error.empty())
            {
                std::printf("%s: %s\n", arg.c_str(), result.error.c_str());
                anyFlagged = true;
            }
            else if (result.flagged)
            {
                std::printf("%s: flagged: %s\n", arg.c_str(), result.flagReason.c_str());
                anyFlagged = true;
            }
        }

        return anyFlagged ? 2 : 0;
    }


    // generate-wms [--out DIR]
    int RunGenerateWms(std::vector<std::string> const& args)
    {
        std::filesystem::path directory = SourceRoot() / "tools" / "map_editor" / "WorldMapStudioPlugins" / "WoWMapStudio" /
            "src" / "Database" / "Generated";
        for (size_t i = 0; i < args.size(); ++i)
            if (args[i] == "--out" && i + 1 < args.size())
                directory = args[++i];

        std::filesystem::path const worldPath = DataRoot() / ".world" / "world.doltlite";
        if (!std::filesystem::exists(worldPath))
        {
            std::printf("generate-wms: %s does not exist (run create first)\n", worldPath.string().c_str());
            return 1;
        }

        DoltLiteDatabase world(worldPath.string());
        WmsGenerateResult result = GenerateWmsSources(world, directory);
        if (!result.error.empty())
        {
            std::printf("generate-wms: %s\n", result.error.c_str());
            return 1;
        }
        std::printf("generate-wms: wrote %zu tables to %s\n", result.tables, directory.string().c_str());
        return 0;
    }
}

int main(int argc, char** argv)
{
    std::vector<std::string> args(argv + 1, argv + argc);
    if (args.empty())
    {
        std::printf("database_tool %s\n", DatabaseToolVersion());
        return 0;
    }

    std::string const command = args[0];
    std::vector<std::string> const rest(args.begin() + 1, args.end());

    if (command == "create")
        return RunCreate(rest);
    if (command == "update")
        return RunUpdate(rest);
    if (command == "port-migration")
        return RunPortMigration(rest);
    if (command == "compare-world")
        return RunCompareWorld(rest);
    if (command == "pin-inputs")
        return RunPinInputs();
    if (command == "reference")
        return RunReference(rest);
    if (command == "spec")
        return RunSpec(rest);
    if (command == "import")
        return RunImport(rest);
    if (command == "export")
        return RunExport(rest);
    if (command == "fdid")
        return RunFdid(rest);
    if (command == "generate-wms")
        return RunGenerateWms(rest);
    if (command == "verify")
        return RunVerifyCommand(rest);

    std::printf("unknown command: %s\n", command.c_str());
    return 1;
}
