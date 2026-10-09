#ifndef MANGOS_PERSISTENCE_EPOCHWRITER_H
#define MANGOS_PERSISTENCE_EPOCHWRITER_H

#include "Persistence/EpochBuffer.h"

#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

struct sqlite3;
struct sqlite3_stmt;

namespace Persistence
{
    enum class WriteMode : uint8
    {
        Diff,   // write rows that changed since the owner's previous image
        Full,   // write every captured row
        None,   // decode and drop
    };

    enum class CheckpointMode : uint8
    {
        Auto,           // sqlite's wal_autocheckpoint inside COMMIT
        AfterCommit,    // PASSIVE checkpoint by the writer after every epoch
        Thread,         // PASSIVE checkpoint on a second connection every checkpointIntervalMs
    };

    struct WriterConfig
    {
        std::string path;
        uint32 pageSize = 4096;
        WriteMode mode = WriteMode::Diff;
        bool useReplace = false;        // REPLACE instead of INSERT ... ON CONFLICT DO UPDATE
        bool primeFirstImage = true;    // an owner's first image only records hashes (rows are already stored)
        std::string synchronous = "FULL";
        CheckpointMode checkpoint = CheckpointMode::Auto;
        uint32 walAutocheckpoint = 1000;
        uint32 checkpointIntervalMs = 1000;
        size_t maxPendingEpochs = 2;
    };

    struct EpochStats
    {
        uint64 epochId = 0;
        uint32 queueDepth = 0;      // epochs queued at pickup, this one included
        uint64 captureUs = 0;
        uint64 lagUs = 0;           // capture end to pickup
        uint64 decodeUs = 0;        // grouping, decode and diff
        uint64 execUs = 0;          // statements
        uint64 commitUs = 0;
        uint64 checkpointUs = 0;
        uint64 bytes = 0;
        uint32 images = 0;
        uint64 rowsIn = 0;
        uint64 rowsUpserted = 0;
        uint64 rowsDeleted = 0;
        uint64 rowsPrimed = 0;
        uint64 rowsUnchanged = 0;
        uint64 ownerDeletes = 0;    // keyless tables: owner's rows replaced
        uint64 walBytes = 0;
        uint64 fileBytes = 0;
    };

    // Writes epochs into a stock sqlite file on its own thread, one transaction per epoch.
    class EpochWriter
    {
        public:
            EpochWriter(TableRegistry const& registry, WriterConfig config);
            ~EpochWriter();

            bool Start(std::string& error);
            // Blocks while maxPendingEpochs are queued; waitUs receives the blocked time.
            // False once the writer failed or stopped.
            bool Submit(std::unique_ptr<Epoch> epoch, uint64* waitUs = nullptr);
            // Waits until every submitted epoch is written.
            void Flush();
            void Stop();

            bool Failed() const { return m_failed; }
            std::string Error() const;
            std::vector<EpochStats> TakeStats();
            uint64 CheckpointThreadUs() const { return m_checkpointThreadUs; }
            uint64 CheckpointThreadRuns() const { return m_checkpointThreadRuns; }

        private:
            struct TableStatements
            {
                sqlite3_stmt* upsert = nullptr;
                sqlite3_stmt* deleteKey = nullptr;
                sqlite3_stmt* deleteOwner = nullptr;
            };

            struct ImageKey
            {
                uint16 table;
                uint64 owner;
                bool operator==(ImageKey const& other) const { return table == other.table && owner == other.owner; }
            };

            struct ImageKeyHash
            {
                size_t operator()(ImageKey const& key) const { return std::hash<uint64>()(key.owner * 64 + key.table); }
            };

            struct PrevImage
            {
                std::unordered_map<std::string, uint64> rows;   // encoded key -> row hash
                uint64 hash = 0;                                // keyless tables: hash of all rows
            };

            struct RowRef
            {
                uint16 table;
                uint8 const* data;
                size_t size;
            };

            struct KeyDelete
            {
                uint16 table;
                std::string key;
            };

            struct OwnerDelete
            {
                uint16 table;
                uint64 owner;
            };

            bool Prepare(std::string& error);
            void Run();
            void CheckpointLoop();
            bool Process(Epoch& epoch, EpochStats& stats);
            bool Exec(char const* sql);
            bool Step(sqlite3_stmt* stmt);
            bool BindRow(sqlite3_stmt* stmt, TableDesc const& table, uint8 const* data, size_t size, uint8 cols);
            void Fail(std::string error);
            size_t KeySize(TableDesc const& table, uint8 const* data, size_t size) const;

            TableRegistry const& m_registry;
            WriterConfig m_config;
            sqlite3* m_db = nullptr;
            std::vector<TableStatements> m_statements;
            std::unordered_map<ImageKey, PrevImage, ImageKeyHash> m_prev;

            std::thread m_thread;
            mutable std::mutex m_mutex;
            std::condition_variable m_wake;     // writer: work or stop
            std::condition_variable m_progress; // submitters and Flush: space or idle
            std::deque<std::unique_ptr<Epoch>> m_queue;
            bool m_busy = false;
            bool m_stopping = false;
            std::atomic<bool> m_failed{ false };
            std::string m_error;
            std::vector<EpochStats> m_stats;

            std::thread m_checkpointThread;
            std::condition_variable m_checkpointWake;
            std::atomic<uint64> m_checkpointThreadUs{ 0 };
            std::atomic<uint64> m_checkpointThreadRuns{ 0 };
    };
}

#endif
