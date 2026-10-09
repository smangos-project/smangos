#include "Persistence/CharTables.h"
#include "Persistence/EpochWriter.h"
#include "Persistence/SqliteSeed.h"

#include <catch_amalgamated.hpp>

#include <cstdlib>
#include <filesystem>
#include <sqlite3.h>
#include <thread>

using namespace Persistence;

namespace
{
    enum TestTable : uint16 { Rows, Points, Items };

    TableRegistry const& TestRegistry()
    {
        static TableRegistry const registry({
            { "rows", 2, "owner", { { "owner", ColType::Int, false }, { "k", ColType::Int, false }, { "v", ColType::Text, true } } },
            { "points", 0, "owner", { { "owner", ColType::Int, false }, { "x", ColType::Real, false } } },
            { "items", 1, "owner", { { "guid", ColType::Int, false }, { "owner", ColType::Int, false } } },
        });
        return registry;
    }

    char const* const TestSchema =
        "CREATE TABLE rows (owner INTEGER NOT NULL, k INTEGER NOT NULL, v TEXT, PRIMARY KEY (owner, k));"
        "CREATE TABLE points (owner INTEGER NOT NULL, x REAL NOT NULL);"
        "CREATE TABLE items (guid INTEGER NOT NULL PRIMARY KEY, owner INTEGER NOT NULL);";

    std::string TempPath(char const* name)
    {
        std::filesystem::path const path = std::filesystem::temp_directory_path() / name;
        for (char const* suffix : { "", "-wal", "-shm" })
            std::filesystem::remove(path.string() + suffix);
        return path.string();
    }

    void Exec(sqlite3* db, char const* sql)
    {
        char* message = nullptr;
        int const rc = sqlite3_exec(db, sql, nullptr, nullptr, &message);
        INFO(sql << ": " << (message ? message : ""));
        sqlite3_free(message);
        REQUIRE(rc == SQLITE_OK);
    }

    std::string CreateDb(char const* name, char const* sql = TestSchema)
    {
        std::string const path = TempPath(name);
        std::string error;
        sqlite3* db = OpenStockSqlite(path, 4096, true, error);
        REQUIRE(db);
        Exec(db, sql);
        sqlite3_close(db);
        return path;
    }

    // Rows of a query as "a|b|c" lines, NULL as "-"
    std::vector<std::string> Query(std::string const& path, char const* sql)
    {
        std::string error;
        sqlite3* db = OpenStockSqlite(path, 4096, true, error);
        REQUIRE(db);
        sqlite3_stmt* stmt = nullptr;
        REQUIRE(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK);
        std::vector<std::string> rows;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            std::string row;
            for (int i = 0; i < sqlite3_column_count(stmt); ++i)
            {
                unsigned char const* text = sqlite3_column_text(stmt, i);
                row += (i ? "|" : "") + std::string(text ? reinterpret_cast<char const*>(text) : "-");
            }
            rows.push_back(row);
        }
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return rows;
    }

    struct EpochBuilder
    {
        std::unique_ptr<Epoch> epoch;
        RowWriter writer;

        explicit EpochBuilder(uint64 id, size_t shards = 1) : epoch(Make(id, shards)), writer(TestRegistry(), epoch->shards[0]) {}

        static std::unique_ptr<Epoch> Make(uint64 id, size_t shards)
        {
            auto epoch = std::make_unique<Epoch>();
            epoch->id = id;
            epoch->captured = std::chrono::steady_clock::now();
            epoch->shards.resize(shards);
            return epoch;
        }

        EpochBuilder& Rows(uint64 owner, std::vector<std::pair<int64, char const*>> const& rows)
        {
            writer.BeginImage(TestTable::Rows, owner);
            for (auto const& row : rows)
            {
                writer.BeginRow().Int(int64(owner)).Int(row.first);
                if (row.second)
                    writer.Text(row.second);
                else
                    writer.Null();
                writer.EndRow();
            }
            writer.EndImage();
            return *this;
        }

        EpochBuilder& Points(uint64 owner, std::vector<double> const& xs)
        {
            writer.BeginImage(TestTable::Points, owner);
            for (double x : xs)
                writer.BeginRow().Int(int64(owner)).Real(x).EndRow();
            writer.EndImage();
            return *this;
        }

        EpochBuilder& Items(uint64 owner, std::vector<int64> const& guids)
        {
            writer.BeginImage(TestTable::Items, owner);
            for (int64 guid : guids)
                writer.BeginRow().Int(guid).Int(int64(owner)).EndRow();
            writer.EndImage();
            return *this;
        }
    };

    EpochStats SubmitAndWait(EpochWriter& writer, EpochBuilder& builder)
    {
        REQUIRE(writer.Submit(std::move(builder.epoch)));
        writer.Flush();
        INFO(writer.Error());
        REQUIRE_FALSE(writer.Failed());
        std::vector<EpochStats> stats = writer.TakeStats();
        REQUIRE(stats.size() == 1);
        return stats[0];
    }

    WriterConfig Config(std::string const& path)
    {
        WriterConfig config;
        config.path = path;
        config.synchronous = "OFF";
        config.primeFirstImage = false;
        return config;
    }
}

TEST_CASE("RowWriter and RowReader round trip every column type", "[shared][charstore]")
{
    TableRegistry const registry({
        { "all", 1, "i", { { "i", ColType::Int, false }, { "n", ColType::Int, true }, { "r", ColType::Real, false },
                           { "t", ColType::Text, true }, { "b", ColType::Blob, false } } },
    });
    EpochBuffer buffer;
    RowWriter writer(registry, buffer);
    uint8 const blob[] = { 0, 1, 255 };
    writer.BeginImage(0, 42);
    writer.BeginRow().Int(-5).Null().Real(1.5).Text("hello").Blob(blob, sizeof(blob)).EndRow();
    writer.BeginRow().Int(INT64_MIN).Int(INT64_MAX).Real(-0.25).Null().Blob(nullptr, 0).EndRow();
    writer.EndImage();
    writer.BeginImage(0, 43);
    writer.EndImage();

    REQUIRE(buffer.images.size() == 2);
    CHECK(buffer.images[0].owner == 42);
    CHECK(buffer.images[0].rows == 2);
    CHECK(buffer.images[1].rows == 0);
    CHECK(buffer.images[1].begin == buffer.images[1].end);

    std::vector<std::vector<Value>> rows;
    ForEachRow(buffer, buffer.images[0], [&](uint8 const* data, size_t size)
    {
        RowReader reader(registry.Get(0), data, size);
        rows.emplace_back();
        Value value;
        while (reader.Next(value))
            rows.back().push_back(value);
        CHECK(reader.Offset() == size);
    });
    REQUIRE(rows.size() == 2);
    REQUIRE(rows[0].size() == 5);
    CHECK(rows[0][0].i == -5);
    CHECK(rows[0][1].null);
    CHECK(rows[0][2].r == 1.5);
    CHECK(rows[0][3].bytes == "hello");
    CHECK(rows[0][4].bytes == std::string_view("\0\1\xff", 3));
    CHECK(rows[1][0].i == INT64_MIN);
    CHECK_FALSE(rows[1][1].null);
    CHECK(rows[1][1].i == INT64_MAX);
    CHECK(rows[1][2].r == -0.25);
    CHECK(rows[1][3].null);
    CHECK(rows[1][4].bytes.empty());
}

TEST_CASE("TableRegistry reports schema mismatches", "[shared][charstore]")
{
    std::string const path = CreateDb("charstore_registry.db",
        "CREATE TABLE rows (owner INTEGER NOT NULL, k INTEGER NOT NULL, v INTEGER, extra INTEGER, PRIMARY KEY (k, owner));"
        "CREATE TABLE points (owner INTEGER NOT NULL);");
    std::string error;
    sqlite3* db = OpenStockSqlite(path, 4096, true, error);
    REQUIRE(db);

    std::vector<std::string> errors;
    CHECK_FALSE(TestRegistry().Validate(db, errors));
    auto has = [&errors](std::string const& message) { return std::find(errors.begin(), errors.end(), message) != errors.end(); };
    CHECK(has("rows: owner has primary key position 2, expected 1"));
    CHECK(has("rows: k has primary key position 1, expected 2"));
    CHECK(has("rows: v is int, expected text"));
    CHECK(has("rows: extra column extra"));
    CHECK(has("points: missing column x"));
    CHECK(has("items: missing table"));
    CHECK(errors.size() == 6);

    errors.clear();
    Exec(db, "DROP TABLE rows; DROP TABLE points;");
    Exec(db, TestSchema);
    CHECK(TestRegistry().Validate(db, errors));
    CHECK(errors.empty());
    sqlite3_close(db);
}

TEST_CASE("CharTableRegistry matches its own column lists", "[shared][charstore]")
{
    TableRegistry const& registry = CharTableRegistry();
    REQUIRE(registry.Size() == CharTableCount);
    CHECK(std::string(registry.Get(Characters).name) == "characters");
    CHECK(std::string(registry.Get(Instance).name) == "character_instance");
    CHECK(std::string(registry.Get(Inventory).cols[0].name) == "item_guid");
    CHECK(registry.Get(HonorCp).keyCols == 0);
    CHECK(registry.Get(Aura).keyCols == 4);
}

TEST_CASE("CharTableRegistry validates against an imported character store", "[shared][charstore]")
{
    // set CHARSTORE_TEST_DB to a file written by `database_tool charstore import`
    char const* path = std::getenv("CHARSTORE_TEST_DB");
    if (!path)
        SKIP("CHARSTORE_TEST_DB not set");

    std::string error;
    sqlite3* db = OpenStockSqlite(path, 4096, true, error);
    REQUIRE(db);
    std::vector<std::string> errors;
    bool const valid = CharTableRegistry().Validate(db, errors);
    sqlite3_close(db);
    for (std::string const& e : errors)
        UNSCOPED_INFO(e);
    CHECK(valid);
}

TEST_CASE("EpochWriter diff writes changes, deletes dropped keys and leaves absent owners alone", "[shared][charstore]")
{
    std::string const path = CreateDb("charstore_diff.db");
    EpochWriter writer(TestRegistry(), Config(path));
    std::string error;
    REQUIRE(writer.Start(error));

    EpochBuilder first(1);
    first.Rows(1, { { 1, "a" }, { 2, nullptr } }).Rows(2, { { 1, "b" } }).Points(1, { 1.0, 2.0 });
    EpochStats stats = SubmitAndWait(writer, first);
    CHECK(stats.rowsIn == 5);
    CHECK(stats.rowsUpserted == 5);
    CHECK(stats.ownerDeletes == 1);
    CHECK(Query(path, "SELECT * FROM rows ORDER BY owner, k") == std::vector<std::string>{ "1|1|a", "1|2|-", "2|1|b" });

    EpochBuilder second(2);
    second.Rows(1, { { 1, "a" }, { 3, "c" } }).Points(1, { 2.0, 1.0 });
    stats = SubmitAndWait(writer, second);
    CHECK(stats.rowsUpserted == 1);
    CHECK(stats.rowsDeleted == 1);
    CHECK(stats.rowsUnchanged == 3);
    CHECK(stats.ownerDeletes == 0);
    CHECK(Query(path, "SELECT * FROM rows ORDER BY owner, k") == std::vector<std::string>{ "1|1|a", "1|3|c", "2|1|b" });

    // empty image deletes the owner's rows, two images of one owner merge, keyless table replaced
    EpochBuilder third(3);
    third.Rows(1, {}).Rows(2, { { 1, "b" } }).Rows(2, { { 2, "d" } }).Points(1, { 3.0 });
    stats = SubmitAndWait(writer, third);
    CHECK(stats.images == 4);
    CHECK(stats.rowsDeleted == 2);
    CHECK(stats.rowsUpserted == 2);
    CHECK(stats.ownerDeletes == 1);
    CHECK(Query(path, "SELECT * FROM rows ORDER BY owner, k") == std::vector<std::string>{ "2|1|b", "2|2|d" });
    CHECK(Query(path, "SELECT owner, x FROM points") == std::vector<std::string>{ "1|3.0" });
    writer.Stop();
}

TEST_CASE("EpochWriter primes an owner's first image", "[shared][charstore]")
{
    std::string const path = CreateDb("charstore_prime.db");
    {
        std::string error;
        sqlite3* db = OpenStockSqlite(path, 4096, true, error);
        Exec(db, "INSERT INTO rows VALUES (1, 1, 'stored'), (1, 2, 'x'); INSERT INTO points VALUES (1, 5.0);");
        sqlite3_close(db);
    }

    WriterConfig config = Config(path);
    config.primeFirstImage = true;
    EpochWriter writer(TestRegistry(), config);
    std::string error;
    REQUIRE(writer.Start(error));

    EpochBuilder first(1);
    first.Rows(1, { { 1, "a" }, { 2, "x" } }).Points(1, { 5.0 });
    EpochStats stats = SubmitAndWait(writer, first);
    CHECK(stats.rowsPrimed == 3);
    CHECK(stats.rowsUpserted == 0);
    CHECK(Query(path, "SELECT v FROM rows ORDER BY k") == std::vector<std::string>{ "stored", "x" });

    EpochBuilder second(2);
    second.Rows(1, { { 1, "a" }, { 2, "y" } }).Points(1, { 5.0 });
    stats = SubmitAndWait(writer, second);
    CHECK(stats.rowsUpserted == 1);
    CHECK(stats.rowsUnchanged == 2);
    CHECK(Query(path, "SELECT v FROM rows ORDER BY k") == std::vector<std::string>{ "stored", "y" });
}

TEST_CASE("EpochWriter applies deletes before upserts when a key changes owner", "[shared][charstore]")
{
    std::string const path = CreateDb("charstore_move.db");
    for (bool useReplace : { false, true })
    {
        WriterConfig config = Config(path);
        config.useReplace = useReplace;
        EpochWriter writer(TestRegistry(), config);
        std::string error;
        REQUIRE(writer.Start(error));

        EpochBuilder first(1);
        first.Items(1, { 10, 11 }).Items(2, {});
        SubmitAndWait(writer, first);

        // B's image comes first, A's delete of item 10 must not remove B's row
        EpochBuilder second(2);
        second.Items(2, { 10 }).Items(1, { 11 });
        EpochStats const stats = SubmitAndWait(writer, second);
        CHECK(stats.rowsDeleted == 1);
        CHECK(stats.rowsUpserted == 1);
        CHECK(Query(path, "SELECT guid, owner FROM items ORDER BY guid") == std::vector<std::string>{ "10|2", "11|1" });
        writer.Stop();
        sqlite3* db = OpenStockSqlite(path, 4096, true, error);
        Exec(db, "DELETE FROM items");
        sqlite3_close(db);
    }
}

TEST_CASE("EpochWriter full and none modes", "[shared][charstore]")
{
    std::string const path = CreateDb("charstore_modes.db");
    for (WriteMode mode : { WriteMode::Full, WriteMode::None })
    {
        WriterConfig config = Config(path);
        config.mode = mode;
        config.checkpoint = CheckpointMode::AfterCommit;
        EpochWriter writer(TestRegistry(), config);
        std::string error;
        REQUIRE(writer.Start(error));
        for (uint64 id = 1; id <= 2; ++id)
        {
            EpochBuilder builder(id);
            builder.Rows(1, { { 1, "a" } }).Points(1, { 1.0 });
            EpochStats const stats = SubmitAndWait(writer, builder);
            CHECK(stats.rowsUpserted == (mode == WriteMode::Full ? 2 : 0));
        }
        CHECK(Query(path, "SELECT COUNT(*) FROM points") == std::vector<std::string>{ "1" });
    }
}

TEST_CASE("EpochWriter merges shards and runs the checkpoint thread", "[shared][charstore]")
{
    std::string const path = CreateDb("charstore_shards.db");
    WriterConfig config = Config(path);
    config.checkpoint = CheckpointMode::Thread;
    config.checkpointIntervalMs = 1;
    config.maxPendingEpochs = 1;
    EpochWriter writer(TestRegistry(), config);
    std::string error;
    REQUIRE(writer.Start(error));

    for (uint64 id = 1; id <= 20; ++id)
    {
        auto epoch = EpochBuilder::Make(id, 2);
        RowWriter a(TestRegistry(), epoch->shards[0]);
        RowWriter b(TestRegistry(), epoch->shards[1]);
        a.BeginImage(TestTable::Rows, 1);
        a.BeginRow().Int(1).Int(1).Text(std::to_string(id)).EndRow();
        a.EndImage();
        b.BeginImage(TestTable::Rows, 2);
        b.BeginRow().Int(2).Int(1).Text(std::to_string(id)).EndRow();
        b.EndImage();
        uint64 waitUs = 0;
        REQUIRE(writer.Submit(std::move(epoch), &waitUs));
    }
    writer.Flush();
    REQUIRE_FALSE(writer.Failed());
    std::vector<EpochStats> const stats = writer.TakeStats();
    REQUIRE(stats.size() == 20);
    CHECK(stats.back().epochId == 20);
    CHECK(stats.back().rowsUpserted == 2);
    for (int i = 0; i < 1000 && writer.CheckpointThreadRuns() == 0; ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    writer.Stop();
    CHECK(writer.CheckpointThreadRuns() > 0);
    CHECK(Query(path, "SELECT * FROM rows ORDER BY owner") == std::vector<std::string>{ "1|1|20", "2|1|20" });
}

TEST_CASE("EpochWriter refuses a schema that does not match", "[shared][charstore]")
{
    std::string const path = CreateDb("charstore_bad.db", "CREATE TABLE rows (owner INTEGER NOT NULL);");
    EpochWriter writer(TestRegistry(), Config(path));
    std::string error;
    CHECK_FALSE(writer.Start(error));
    CHECK(error.find("rows: missing column k") != std::string::npos);
}
