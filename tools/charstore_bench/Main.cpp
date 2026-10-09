// Character store writer benchmark (phase 0 M1): seeds a char store file with synthetic players
// and measures steady epoch writes, plus a raw fdatasync probe for storage targets.

#include "Synthetic.h"

#include "Persistence/EpochWriter.h"
#include "Persistence/SqliteSeed.h"

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sqlite3.h>
#include <thread>
#include <unistd.h>

using namespace Persistence;
using namespace CharStoreBench;
using BenchClock = std::chrono::steady_clock;

namespace
{
    struct Args
    {
        std::map<std::string, std::string> values;
        std::vector<std::string> positional;

        std::string Get(std::string const& key, std::string const& fallback = "") const
        {
            auto it = values.find(key);
            return it == values.end() ? fallback : it->second;
        }
        uint64 Num(std::string const& key, uint64 fallback) const
        {
            auto it = values.find(key);
            return it == values.end() ? fallback : std::stoull(it->second);
        }
        bool Has(std::string const& key) const { return values.count(key) != 0; }
    };

    Args Parse(int argc, char** argv)
    {
        static char const* const flags[] = { "--replace", "--full-capture" };
        Args args;
        for (int i = 1; i < argc; ++i)
        {
            std::string const arg = argv[i];
            if (arg.rfind("--", 0) != 0)
                args.positional.push_back(arg);
            else if (std::find(std::begin(flags), std::end(flags), arg) != std::end(flags))
                args.values[arg] = "1";
            else if (i + 1 < argc)
                args.values[arg] = argv[++i];
        }
        return args;
    }

    uint64 Micros(BenchClock::time_point from, BenchClock::time_point to)
    {
        return uint64(std::chrono::duration_cast<std::chrono::microseconds>(to - from).count());
    }

    double Percentile(std::vector<uint64> values, double p)
    {
        if (values.empty())
            return 0;
        std::sort(values.begin(), values.end());
        return double(values[std::min(values.size() - 1, size_t(p * double(values.size())))]);
    }

    uint64 PeakRssKb()
    {
        std::ifstream status("/proc/self/status");
        std::string line;
        while (std::getline(status, line))
            if (line.rfind("VmHWM:", 0) == 0)
                return std::stoull(line.substr(6));
        return 0;
    }

    bool WriterConfigFromArgs(Args const& args, WriterConfig& config, std::string& error)
    {
        config.path = args.Get("--db");
        config.pageSize = uint32(args.Num("--page-size", 4096));
        std::string const mode = args.Get("--mode", "diff");
        config.mode = mode == "full" ? WriteMode::Full : mode == "none" ? WriteMode::None : WriteMode::Diff;
        config.useReplace = args.Has("--replace");
        config.synchronous = args.Get("--sync", "full");
        std::string const checkpoint = args.Get("--checkpoint", "commit");
        config.checkpoint = checkpoint == "commit" ? CheckpointMode::AfterCommit : checkpoint == "thread" ? CheckpointMode::Thread : checkpoint == "auto" ? CheckpointMode::Auto : CheckpointMode::AfterCommit;
        config.walAutocheckpoint = uint32(args.Num("--wal-autocheckpoint", 1000));
        config.checkpointIntervalMs = uint32(args.Num("--checkpoint-ms", 1000));
        config.maxPendingEpochs = size_t(args.Num("--max-pending", 2));
        if (args.Has("--cache-mb"))
            config.extraPragmas += "PRAGMA cache_size = -" + std::to_string(args.Num("--cache-mb", 0) * 1024) + ";";
        if (args.Has("--mmap-mb"))
            config.extraPragmas += "PRAGMA mmap_size = " + std::to_string(args.Num("--mmap-mb", 0) << 20) + ";";
        if (config.path.empty())
            error = "--db is required";
        return error.empty();
    }

    bool HeaderIsWal(std::string const& path)
    {
        std::ifstream file(path, std::ios::binary);
        char header[20] = {};
        file.read(header, sizeof(header));
        return header[18] == 2 && header[19] == 2;
    }

    void Checkpoint(std::string const& path)
    {
        std::string error;
        if (sqlite3* db = OpenStockSqlite(path, 4096, true, error))
        {
            sqlite3_wal_checkpoint_v2(db, nullptr, SQLITE_CHECKPOINT_TRUNCATE, nullptr, nullptr);
            sqlite3_close(db);
        }
    }

    uint64 FileSize(std::string const& path)
    {
        std::error_code ec;
        uintmax_t const size = std::filesystem::file_size(path, ec);
        return ec ? 0 : uint64(size);
    }

    void WriteEpochCsv(std::string const& path, std::vector<EpochStats> const& stats, std::map<uint64, std::pair<uint64, uint64>> const& capture)
    {
        std::ofstream csv(path);
        csv << "epoch,queue_depth,capture_us,wait_us,lag_us,decode_us,exec_us,commit_us,checkpoint_us,bytes,images,rows_in,"
               "rows_upserted,rows_deleted,rows_primed,rows_unchanged,owner_deletes,wal_bytes,file_bytes\n";
        for (EpochStats const& s : stats)
        {
            auto it = capture.find(s.epochId);
            uint64 const wait = it == capture.end() ? 0 : it->second.second;
            csv << s.epochId << ',' << s.queueDepth << ',' << s.captureUs << ',' << wait << ',' << s.lagUs << ',' << s.decodeUs << ','
                << s.execUs << ',' << s.commitUs << ',' << s.checkpointUs << ',' << s.bytes << ',' << s.images << ',' << s.rowsIn << ','
                << s.rowsUpserted << ',' << s.rowsDeleted << ',' << s.rowsPrimed << ',' << s.rowsUnchanged << ',' << s.ownerDeletes << ','
                << s.walBytes << ',' << s.fileBytes << '\n';
        }
    }

    std::unique_ptr<Epoch> CaptureEpoch(SyntheticRealm const& realm, uint64 id, uint32 first, uint32 end, bool dirtyOnly)
    {
        BenchClock::time_point const start = BenchClock::now();
        auto epoch = std::make_unique<Epoch>();
        epoch->id = id;
        epoch->shards.resize(1);
        RowWriter writer(CharTableRegistry(), epoch->shards[0]);
        for (uint32 player = first; player < end; ++player)
            realm.Capture(writer, player, dirtyOnly);
        epoch->captured = BenchClock::now();
        epoch->captureUs = Micros(start, epoch->captured);
        return epoch;
    }

    int Seed(Args const& args)
    {
        std::string error;
        WriterConfig config;
        if (!WriterConfigFromArgs(args, config, error) || !args.Has("--schema"))
        {
            std::cerr << (error.empty() ? "--schema is required" : error) << '\n';
            return 1;
        }
        config.mode = WriteMode::Full;

        for (char const* suffix : { "", "-wal", "-shm" })
            std::filesystem::remove(config.path + suffix);
        std::filesystem::copy_file(args.Get("--schema"), config.path);
        if (!HeaderIsWal(config.path))
            std::cerr << "warning: " << config.path << " is not in WAL mode\n";

        RowCounts counts = DefaultRowCounts();
        if (args.Has("--rows") && !ParseRowCounts(args.Get("--rows"), counts, error))
        {
            std::cerr << error << '\n';
            return 1;
        }
        uint32 const players = uint32(args.Num("--players", 1000));
        uint32 const batch = uint32(args.Num("--batch", 500));
        SyntheticRealm realm(counts, players, args.Num("--first-guid", 1000000));

        EpochWriter writer(CharTableRegistry(), config);
        if (!writer.Start(error))
        {
            std::cerr << error << '\n';
            return 1;
        }
        BenchClock::time_point const start = BenchClock::now();
        uint64 id = 0;
        for (uint32 first = 0; first < players; first += batch)
            if (!writer.Submit(CaptureEpoch(realm, ++id, first, std::min(players, first + batch), false)))
                break;
        writer.Flush();
        double const seconds = double(Micros(start, BenchClock::now())) / 1e6;
        if (writer.Failed())
        {
            std::cerr << writer.Error() << '\n';
            return 1;
        }
        std::vector<EpochStats> const stats = writer.TakeStats();
        writer.Stop();
        Checkpoint(config.path);

        uint64 rows = 0;
        std::vector<uint64> commits;
        for (EpochStats const& s : stats)
        {
            rows += s.rowsUpserted;
            commits.push_back(s.commitUs);
        }
        std::printf("seed players=%u rows=%llu seconds=%.2f rows_per_s=%.0f commit_p50_ms=%.2f file_mb=%.1f\n", players,
            (unsigned long long)rows, seconds, double(rows) / seconds, Percentile(commits, 0.5) / 1000, double(FileSize(config.path)) / 1048576);
        if (args.Has("--csv"))
            WriteEpochCsv(args.Get("--csv"), stats, {});
        return 0;
    }

    int Steady(Args const& args)
    {
        std::string error;
        WriterConfig config;
        if (!WriterConfigFromArgs(args, config, error))
        {
            std::cerr << error << '\n';
            return 1;
        }
        if (!HeaderIsWal(config.path))
            std::cerr << "warning: " << config.path << " is not in WAL mode\n";

        RowCounts counts = DefaultRowCounts();
        if (args.Has("--rows") && !ParseRowCounts(args.Get("--rows"), counts, error))
        {
            std::cerr << error << '\n';
            return 1;
        }
        uint32 const players = uint32(args.Num("--players", 1000));
        uint32 const changes = uint32(args.Num("--changes", 5));
        uint32 const keyChangePct = uint32(args.Num("--key-change-pct", 0));
        uint32 const epochs = uint32(args.Num("--epochs", 60));
        uint32 const warmup = uint32(args.Num("--warmup", 5));
        uint32 const intervalMs = uint32(args.Num("--interval-ms", 0));
        uint32 const readerMs = uint32(args.Num("--reader-ms", 0));
        bool const dirtyOnly = !args.Has("--full-capture");
        SyntheticRealm realm(counts, players, args.Num("--first-guid", 1000000));
        std::mt19937_64 rng(args.Num("--seed", 1));

        EpochWriter writer(CharTableRegistry(), config);
        if (!writer.Start(error))
        {
            std::cerr << error << '\n';
            return 1;
        }

        // the first full image of every player primes the diff state
        BenchClock::time_point const primeStart = BenchClock::now();
        writer.Submit(CaptureEpoch(realm, 0, 0, players, false));
        writer.Flush();
        double const primeSeconds = double(Micros(primeStart, BenchClock::now())) / 1e6;
        writer.TakeStats();

        std::atomic<bool> readerStop{ false };
        std::vector<uint64> readLatency;
        std::thread reader;
        if (readerMs)
            reader = std::thread([&]
            {
                sqlite3* db = OpenStockSqlite(config.path, config.pageSize, true, error);
                if (!db)
                    return;
                sqlite3_busy_timeout(db, 5000);
                std::vector<sqlite3_stmt*> stmts;
                for (uint16 table = 0; table < CharTableCount; ++table)
                {
                    TableDesc const& desc = CharTableRegistry().Get(table);
                    std::string const sql = std::string("SELECT * FROM \"") + desc.name + "\" WHERE \"" + desc.ownerCol + "\" = ?";
                    sqlite3_stmt* stmt = nullptr;
                    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
                    stmts.push_back(stmt);
                }
                std::mt19937_64 readerRng(7);
                while (!readerStop)
                {
                    BenchClock::time_point const start = BenchClock::now();
                    sqlite3_exec(db, "BEGIN", nullptr, nullptr, nullptr);
                    int64 const guid = int64(realm.Guid(uint32(readerRng() % players)));
                    for (sqlite3_stmt* stmt : stmts)
                    {
                        sqlite3_bind_int64(stmt, 1, guid);
                        while (sqlite3_step(stmt) == SQLITE_ROW) {}
                        sqlite3_reset(stmt);
                    }
                    sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr);
                    readLatency.push_back(Micros(start, BenchClock::now()));
                    std::this_thread::sleep_for(std::chrono::milliseconds(readerMs));
                }
                for (sqlite3_stmt* stmt : stmts)
                    sqlite3_finalize(stmt);
                sqlite3_close(db);
            });

        std::map<uint64, std::pair<uint64, uint64>> capture;   // epoch -> capture us, back-pressure wait us
        BenchClock::time_point const start = BenchClock::now();
        BenchClock::time_point measured = start;
        BenchClock::time_point next = start;
        for (uint32 i = 1; i <= warmup + epochs; ++i)
        {
            if (i == warmup + 1)
                measured = BenchClock::now();
            realm.Mutate(changes, keyChangePct, rng);
            std::unique_ptr<Epoch> epoch = CaptureEpoch(realm, i, 0, players, dirtyOnly);
            realm.ClearDirty();
            uint64 const captureUs = epoch->captureUs;
            uint64 waitUs = 0;
            if (!writer.Submit(std::move(epoch), &waitUs))
                break;
            capture[i] = { captureUs, waitUs };
            if (intervalMs)
            {
                next += std::chrono::milliseconds(intervalMs);
                std::this_thread::sleep_until(next);
            }
        }
        writer.Flush();
        double const wallSeconds = double(Micros(measured, BenchClock::now())) / 1e6;
        readerStop = true;
        if (reader.joinable())
            reader.join();
        if (writer.Failed())
        {
            std::cerr << writer.Error() << '\n';
            return 1;
        }

        std::vector<EpochStats> stats = writer.TakeStats();
        stats.erase(std::remove_if(stats.begin(), stats.end(), [warmup](EpochStats const& s) { return s.epochId <= warmup; }), stats.end());
        writer.Stop();

        std::vector<uint64> write, commit, checkpoint, captureUs, waits;
        uint64 rowsWritten = 0, rowsIn = 0, busyUs = 0, walMax = 0;
        for (EpochStats const& s : stats)
        {
            uint64 const total = s.decodeUs + s.execUs + s.commitUs + s.checkpointUs;
            write.push_back(total);
            commit.push_back(s.commitUs);
            checkpoint.push_back(s.checkpointUs);
            captureUs.push_back(capture[s.epochId].first);
            waits.push_back(capture[s.epochId].second);
            rowsWritten += s.rowsUpserted + s.rowsDeleted;
            rowsIn += s.rowsIn;
            busyUs += total;
            walMax = std::max(walMax, s.walBytes);
        }
        double const n = std::max<double>(1, double(stats.size()));
        uint64 waitTotal = 0;
        for (uint64 w : waits)
            waitTotal += w;

        std::string const label = args.Get("--label", "steady");
        char line[1024];
        std::snprintf(line, sizeof(line),
            "%s,%u,%u,%u,%s,%s,%d,%s,%zu,%.0f,%.0f,%.0f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.1f,%.1f,%.2f,%.2f,%.2f,%llu,%.2f",
            label.c_str(), players, changes, keyChangePct, config.synchronous.c_str(), args.Get("--checkpoint", "auto").c_str(),
            config.useReplace ? 1 : 0, dirtyOnly ? "dirty" : "full", stats.size(), rowsIn / n, rowsWritten / n,
            busyUs ? double(rowsWritten) * 1e6 / double(busyUs) : 0.0,
            Percentile(write, 0.5) / 1000, Percentile(write, 0.99) / 1000, Percentile(write, 1.0) / 1000,
            Percentile(commit, 0.5) / 1000, Percentile(commit, 0.99) / 1000, Percentile(checkpoint, 0.99) / 1000,
            Percentile(captureUs, 0.5) / 1000, Percentile(captureUs, 0.99) / 1000, double(waitTotal) / 1000,
            double(walMax) / 1048576, double(FileSize(config.path)) / 1048576,
            Percentile(readLatency, 0.5) / 1000, Percentile(readLatency, 0.99) / 1000, primeSeconds,
            (unsigned long long)(PeakRssKb() / 1024), wallSeconds);
        static char const* const header =
            "label,players,changes,key_change_pct,sync,checkpoint,replace,capture,epochs,rows_in_per_epoch,rows_written_per_epoch,"
            "writer_rows_per_s,write_p50_ms,write_p99_ms,write_max_ms,commit_p50_ms,commit_p99_ms,checkpoint_p99_ms,capture_p50_ms,"
            "capture_p99_ms,backpressure_ms,wal_max_mb,file_mb,read_p50_ms,read_p99_ms,prime_s,peak_rss_mb,wall_s";
        std::printf("%s\n%s\n", header, line);

        if (args.Has("--summary"))
        {
            std::string const path = args.Get("--summary");
            bool const fresh = FileSize(path) == 0;
            std::ofstream summary(path, std::ios::app);
            if (fresh)
                summary << header << '\n';
            summary << line << '\n';
        }
        if (args.Has("--csv"))
            WriteEpochCsv(args.Get("--csv"), stats, capture);
        return 0;
    }

    int FsyncProbe(Args const& args)
    {
        if (args.positional.size() < 2)
        {
            std::cerr << "fsync-probe DIR [--count N] [--size BYTES]\n";
            return 1;
        }
        std::string const path = args.positional[1] + "/.fsync-probe";
        uint32 const count = uint32(args.Num("--count", 10000));
        std::vector<char> block(size_t(args.Num("--size", 4096)), 'x');
        int const fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0)
        {
            std::cerr << "cannot create " << path << '\n';
            return 1;
        }
        std::vector<uint64> latency;
        latency.reserve(count);
        for (uint32 i = 0; i < count; ++i)
        {
            if (::write(fd, block.data(), block.size()) != ssize_t(block.size()))
                break;
            BenchClock::time_point const start = BenchClock::now();
            ::fdatasync(fd);
            latency.push_back(Micros(start, BenchClock::now()));
        }
        ::close(fd);
        std::filesystem::remove(path);
        uint64 total = 0;
        for (uint64 l : latency)
            total += l;
        std::printf("fsync-probe dir=%s count=%zu mean_us=%.0f p50_us=%.0f p99_us=%.0f max_us=%.0f\n", args.positional[1].c_str(),
            latency.size(), latency.empty() ? 0.0 : double(total) / double(latency.size()), Percentile(latency, 0.5),
            Percentile(latency, 0.99), Percentile(latency, 1.0));
        return 0;
    }
}

int main(int argc, char** argv)
{
    Args const args = Parse(argc, argv);
    std::string const command = args.positional.empty() ? "" : args.positional[0];
    if (command == "seed")
        return Seed(args);
    if (command == "steady")
        return Steady(args);
    if (command == "fsync-probe")
        return FsyncProbe(args);

    std::cerr <<
        "charstore_bench seed --schema FILE --db FILE [--players N] [--batch N] [--rows table=n,...]\n"
        "charstore_bench steady --db FILE [--players N] [--changes N] [--key-change-pct N] [--epochs N] [--warmup N]\n"
        "                [--interval-ms N] [--reader-ms N] [--full-capture] [--label L] [--summary FILE] [--csv FILE]\n"
        "charstore_bench fsync-probe DIR [--count N] [--size BYTES]\n"
        "writer options: --mode diff|full|none --sync full|normal|off --replace --checkpoint auto|commit|thread\n"
        "                --checkpoint-ms N --wal-autocheckpoint N --cache-mb N --mmap-mb N --max-pending N --page-size N\n";
    return 1;
}
