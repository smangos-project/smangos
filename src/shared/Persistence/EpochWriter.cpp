#include "Persistence/EpochWriter.h"
#include "Persistence/SqliteSeed.h"

#include <filesystem>
#include <sqlite3.h>

namespace Persistence
{
    namespace
    {
        using Clock = std::chrono::steady_clock;

        uint64 Micros(Clock::time_point from, Clock::time_point to)
        {
            return uint64(std::chrono::duration_cast<std::chrono::microseconds>(to - from).count());
        }

        uint64 FileSize(std::string const& path)
        {
            std::error_code ec;
            uintmax_t const size = std::filesystem::file_size(path, ec);
            return ec ? 0 : uint64(size);
        }

        std::string Quote(char const* name)
        {
            return std::string("\"") + name + "\"";
        }

        std::string UpsertSql(TableDesc const& table, bool useReplace)
        {
            std::string cols, params;
            for (ColDesc const& col : table.cols)
            {
                cols += (cols.empty() ? "" : ", ") + Quote(col.name);
                params += params.empty() ? "?" : ", ?";
            }
            bool const replace = useReplace && table.keyCols > 0;
            std::string sql = std::string(replace ? "REPLACE" : "INSERT") + " INTO " + Quote(table.name) + " (" + cols + ") VALUES (" + params + ")";
            if (replace || table.keyCols == 0)
                return sql;

            std::string keys, sets;
            for (size_t i = 0; i < table.cols.size(); ++i)
            {
                std::string const name = Quote(table.cols[i].name);
                if (i < table.keyCols)
                    keys += (keys.empty() ? "" : ", ") + name;
                else
                    sets += (sets.empty() ? "" : ", ") + name + " = excluded." + name;
            }
            return sql + " ON CONFLICT (" + keys + ") DO " + (sets.empty() ? "NOTHING" : "UPDATE SET " + sets);
        }

        std::string DeleteKeySql(TableDesc const& table)
        {
            std::string where;
            for (uint8 i = 0; i < table.keyCols; ++i)
                where += (where.empty() ? "" : " AND ") + Quote(table.cols[i].name) + " = ?";
            return "DELETE FROM " + Quote(table.name) + " WHERE " + where;
        }
    }

    EpochWriter::EpochWriter(TableRegistry const& registry, WriterConfig config) : m_registry(registry), m_config(std::move(config))
    {
    }

    EpochWriter::~EpochWriter()
    {
        Stop();
    }

    bool EpochWriter::Start(std::string& error)
    {
        m_db = OpenStockSqlite(m_config.path, m_config.pageSize, true, error);
        if (!m_db)
            return false;
        sqlite3_busy_timeout(m_db, 5000);

        if (!Prepare(error))
        {
            Stop();
            return false;
        }

        m_thread = std::thread(&EpochWriter::Run, this);
        if (m_config.checkpoint == CheckpointMode::Thread)
            m_checkpointThread = std::thread(&EpochWriter::CheckpointLoop, this);
        return true;
    }

    bool EpochWriter::Prepare(std::string& error)
    {
        std::string const pragmas = "PRAGMA synchronous = " + m_config.synchronous + "; PRAGMA wal_autocheckpoint = " +
            std::to_string(m_config.checkpoint == CheckpointMode::Auto ? m_config.walAutocheckpoint : 0);
        if (sqlite3_exec(m_db, pragmas.c_str(), nullptr, nullptr, nullptr) != SQLITE_OK)
        {
            error = std::string("pragmas: ") + sqlite3_errmsg(m_db);
            return false;
        }

        std::vector<std::string> errors;
        if (!m_registry.Validate(m_db, errors))
        {
            error = "schema mismatch:";
            for (std::string const& e : errors)
                error += "\n  " + e;
            return false;
        }

        auto prepare = [this, &error](std::string const& sql, sqlite3_stmt*& stmt)
        {
            if (sqlite3_prepare_v3(m_db, sql.c_str(), -1, SQLITE_PREPARE_PERSISTENT, &stmt, nullptr) == SQLITE_OK)
                return true;
            error = sql + ": " + sqlite3_errmsg(m_db);
            return false;
        };

        m_statements.resize(m_registry.Size());
        for (uint16 id = 0; id < m_registry.Size(); ++id)
        {
            TableDesc const& table = m_registry.Get(id);
            TableStatements& stmts = m_statements[id];
            if (!prepare(UpsertSql(table, m_config.useReplace), stmts.upsert) ||
                (table.keyCols > 0 && !prepare(DeleteKeySql(table), stmts.deleteKey)) ||
                !prepare("DELETE FROM " + Quote(table.name) + " WHERE " + Quote(table.ownerCol) + " = ?", stmts.deleteOwner))
                return false;
        }
        return true;
    }

    void EpochWriter::Stop()
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_stopping = true;
        }
        m_wake.notify_all();
        m_progress.notify_all();
        m_checkpointWake.notify_all();
        if (m_thread.joinable())
            m_thread.join();
        if (m_checkpointThread.joinable())
            m_checkpointThread.join();

        for (TableStatements& stmts : m_statements)
        {
            sqlite3_finalize(stmts.upsert);
            sqlite3_finalize(stmts.deleteKey);
            sqlite3_finalize(stmts.deleteOwner);
        }
        m_statements.clear();
        if (m_db)
            sqlite3_close(m_db);
        m_db = nullptr;
    }

    bool EpochWriter::Submit(std::unique_ptr<Epoch> epoch, uint64* waitUs)
    {
        Clock::time_point const start = Clock::now();
        std::unique_lock<std::mutex> lock(m_mutex);
        m_progress.wait(lock, [this] { return m_failed || m_stopping || m_queue.size() < m_config.maxPendingEpochs; });
        if (waitUs)
            *waitUs = Micros(start, Clock::now());
        if (m_failed || m_stopping)
            return false;
        m_queue.push_back(std::move(epoch));
        lock.unlock();
        m_wake.notify_one();
        return true;
    }

    void EpochWriter::Flush()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_progress.wait(lock, [this] { return m_failed || (m_queue.empty() && !m_busy); });
    }

    std::string EpochWriter::Error() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_error;
    }

    std::vector<EpochStats> EpochWriter::TakeStats()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return std::move(m_stats);
    }

    void EpochWriter::Fail(std::string error)
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_failed)
                return;
            m_error = std::move(error);
            m_failed = true;
            m_queue.clear();
        }
        m_progress.notify_all();
    }

    void EpochWriter::Run()
    {
        for (;;)
        {
            std::unique_ptr<Epoch> epoch;
            EpochStats stats;
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_wake.wait(lock, [this] { return m_stopping || !m_queue.empty(); });
                if (m_queue.empty())
                    return;
                stats.queueDepth = uint32(m_queue.size());
                epoch = std::move(m_queue.front());
                m_queue.pop_front();
                m_busy = true;
            }
            m_progress.notify_all();

            stats.epochId = epoch->id;
            stats.captureUs = epoch->captureUs;
            stats.lagUs = Micros(epoch->captured, Clock::now());
            bool const ok = !m_failed && Process(*epoch, stats);
            epoch.reset();

            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_busy = false;
                if (ok)
                    m_stats.push_back(stats);
            }
            m_progress.notify_all();
        }
    }

    void EpochWriter::CheckpointLoop()
    {
        std::string error;
        sqlite3* db = OpenStockSqlite(m_config.path, m_config.pageSize, true, error);
        if (!db)
        {
            Fail("checkpoint connection: " + error);
            return;
        }
        sqlite3_busy_timeout(db, 5000);

        std::unique_lock<std::mutex> lock(m_mutex);
        while (!m_stopping)
        {
            m_checkpointWake.wait_for(lock, std::chrono::milliseconds(m_config.checkpointIntervalMs));
            if (m_stopping)
                break;
            lock.unlock();
            Clock::time_point const start = Clock::now();
            sqlite3_wal_checkpoint_v2(db, nullptr, SQLITE_CHECKPOINT_PASSIVE, nullptr, nullptr);
            m_checkpointThreadUs += Micros(start, Clock::now());
            ++m_checkpointThreadRuns;
            lock.lock();
        }
        lock.unlock();
        sqlite3_close(db);
    }

    bool EpochWriter::Exec(char const* sql)
    {
        if (sqlite3_exec(m_db, sql, nullptr, nullptr, nullptr) == SQLITE_OK)
            return true;
        Fail(std::string(sql) + ": " + sqlite3_errmsg(m_db));
        return false;
    }

    bool EpochWriter::Step(sqlite3_stmt* stmt)
    {
        int const rc = sqlite3_step(stmt);
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        if (rc == SQLITE_DONE)
            return true;
        Fail(std::string(sqlite3_sql(stmt)) + ": " + sqlite3_errmsg(m_db));
        return false;
    }

    bool EpochWriter::BindRow(sqlite3_stmt* stmt, TableDesc const& table, uint8 const* data, size_t size, uint8 cols)
    {
        RowReader reader(table, data, size);
        Value value;
        for (int i = 1; i <= cols && reader.Next(value); ++i)
        {
            int rc;
            if (value.null)
                rc = sqlite3_bind_null(stmt, i);
            else switch (value.type)
            {
                case ColType::Int: rc = sqlite3_bind_int64(stmt, i, value.i); break;
                case ColType::Real: rc = sqlite3_bind_double(stmt, i, value.r); break;
                case ColType::Text: rc = sqlite3_bind_text(stmt, i, value.bytes.data(), int(value.bytes.size()), SQLITE_STATIC); break;
                default: rc = sqlite3_bind_blob(stmt, i, value.bytes.data(), int(value.bytes.size()), SQLITE_STATIC); break;
            }
            if (rc != SQLITE_OK)
            {
                Fail(std::string(table.name) + ": bind: " + sqlite3_errmsg(m_db));
                return false;
            }
        }
        return true;
    }

    size_t EpochWriter::KeySize(TableDesc const& table, uint8 const* data, size_t size) const
    {
        RowReader reader(table, data, size);
        Value value;
        for (uint8 i = 0; i < table.keyCols; ++i)
            reader.Next(value);
        return reader.Offset();
    }

    bool EpochWriter::Process(Epoch& epoch, EpochStats& stats)
    {
        Clock::time_point const start = Clock::now();

        // several images of one (table, owner) in an epoch form one image
        std::unordered_map<ImageKey, size_t, ImageKeyHash> groupIndex;
        std::vector<std::pair<ImageKey, std::vector<std::pair<EpochBuffer const*, ImageRef const*>>>> groups;
        for (EpochBuffer const& shard : epoch.shards)
        {
            stats.bytes += shard.bytes.size();
            for (ImageRef const& image : shard.images)
            {
                ++stats.images;
                stats.rowsIn += image.rows;
                ImageKey const key{ image.table, image.owner };
                auto inserted = groupIndex.emplace(key, groups.size());
                if (inserted.second)
                    groups.push_back({ key, {} });
                groups[inserted.first->second].second.push_back({ &shard, &image });
            }
        }

        std::vector<OwnerDelete> ownerDeletes;
        std::vector<KeyDelete> keyDeletes;
        std::vector<RowRef> upserts;
        for (auto const& group : groups)
        {
            ImageKey const& key = group.first;
            TableDesc const& table = m_registry.Get(key.table);
            auto forEachRow = [&group](auto&& f)
            {
                for (auto const& image : group.second)
                    ForEachRow(*image.first, *image.second, f);
            };

            if (m_config.mode == WriteMode::None)
            {
                forEachRow([&table](uint8 const* data, size_t size)
                {
                    RowReader reader(table, data, size);
                    Value value;
                    while (reader.Next(value)) {}
                });
                continue;
            }

            if (m_config.mode == WriteMode::Full)
            {
                if (table.keyCols == 0)
                    ownerDeletes.push_back({ key.table, key.owner });
                forEachRow([&](uint8 const* data, size_t size) { upserts.push_back({ key.table, data, size }); });
                continue;
            }

            auto prevIt = m_prev.find(key);
            bool const known = prevIt != m_prev.end();
            bool const prime = !known && m_config.primeFirstImage;
            PrevImage& prev = known ? prevIt->second : m_prev[key];

            if (table.keyCols == 0)
            {
                // keyless: any change replaces all of the owner's rows
                uint64 hash = 0;
                size_t const first = upserts.size();
                forEachRow([&](uint8 const* data, size_t size)
                {
                    hash += Fnv1a(data, size);
                    upserts.push_back({ key.table, data, size });
                });
                bool const write = !prime && !(known && hash == prev.hash);
                prev.hash = hash;
                if (write)
                    ownerDeletes.push_back({ key.table, key.owner });
                else
                {
                    (prime ? stats.rowsPrimed : stats.rowsUnchanged) += upserts.size() - first;
                    upserts.resize(first);
                }
                continue;
            }

            std::unordered_map<std::string, uint64> rows;
            rows.reserve(prev.rows.size());
            forEachRow([&](uint8 const* data, size_t size)
            {
                std::string rowKey(reinterpret_cast<char const*>(data), KeySize(table, data, size));
                uint64 const hash = Fnv1a(data, size);
                if (prime)
                    ++stats.rowsPrimed;
                else
                {
                    auto old = prev.rows.find(rowKey);
                    if (old == prev.rows.end() || old->second != hash)
                        upserts.push_back({ key.table, data, size });
                    else
                        ++stats.rowsUnchanged;
                }
                rows[std::move(rowKey)] = hash;
            });
            if (!prime)
                for (auto const& old : prev.rows)
                    if (!rows.count(old.first))
                        keyDeletes.push_back({ key.table, old.first });
            prev.rows.swap(rows);
        }

        Clock::time_point const decoded = Clock::now();
        stats.decodeUs = Micros(start, decoded);
        if (ownerDeletes.empty() && keyDeletes.empty() && upserts.empty())
            return true;

        if (!Exec("BEGIN"))
            return false;

        // all deletes first: a key that moved to another owner is deleted and upserted in the same epoch
        bool ok = true;
        for (OwnerDelete const& del : ownerDeletes)
        {
            sqlite3_stmt* stmt = m_statements[del.table].deleteOwner;
            ok = sqlite3_bind_int64(stmt, 1, int64(del.owner)) == SQLITE_OK && Step(stmt);
            if (!ok)
                break;
        }
        for (size_t i = 0; ok && i < keyDeletes.size(); ++i)
        {
            KeyDelete const& del = keyDeletes[i];
            TableDesc const& table = m_registry.Get(del.table);
            sqlite3_stmt* stmt = m_statements[del.table].deleteKey;
            ok = BindRow(stmt, table, reinterpret_cast<uint8 const*>(del.key.data()), del.key.size(), table.keyCols) && Step(stmt);
        }
        for (size_t i = 0; ok && i < upserts.size(); ++i)
        {
            RowRef const& row = upserts[i];
            TableDesc const& table = m_registry.Get(row.table);
            sqlite3_stmt* stmt = m_statements[row.table].upsert;
            ok = BindRow(stmt, table, row.data, row.size, uint8(table.cols.size())) && Step(stmt);
        }
        if (!ok)
        {
            sqlite3_exec(m_db, "ROLLBACK", nullptr, nullptr, nullptr);
            if (!m_failed)
                Fail("statement failed");
            return false;
        }
        stats.ownerDeletes = ownerDeletes.size();
        stats.rowsDeleted = keyDeletes.size();
        stats.rowsUpserted = upserts.size();

        Clock::time_point const executed = Clock::now();
        stats.execUs = Micros(decoded, executed);
        if (!Exec("COMMIT"))
            return false;
        Clock::time_point const committed = Clock::now();
        stats.commitUs = Micros(executed, committed);

        if (m_config.checkpoint == CheckpointMode::AfterCommit)
        {
            sqlite3_wal_checkpoint_v2(m_db, nullptr, SQLITE_CHECKPOINT_PASSIVE, nullptr, nullptr);
            stats.checkpointUs = Micros(committed, Clock::now());
        }
        stats.walBytes = FileSize(m_config.path + "-wal");
        stats.fileBytes = FileSize(m_config.path);
        return true;
    }
}
