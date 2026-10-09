#ifndef MANGOS_STATEMENTSTATS_H
#define MANGOS_STATEMENTSTATS_H

#include "Common.h"

#include <chrono>
#include <cstdio>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

// Counts executed write statements per (verb, table) and records timings; appends
// one CSV block per interval. Used to measure the write load of a database.
class StatementStats
{
    public:
        StatementStats(std::string csvPath, uint32 intervalMs);
        ~StatementStats();

        void RecordPlain(std::string const& sql);
        void RegisterPrepared(int stmtIndex, std::string const& fmt);
        void RecordPrepared(int stmtIndex, size_t paramBytes);
        void RecordTransaction(uint32 statements, uint64 us);
        void RecordTiming(char const* name, uint64 us);

        void Flush();

        // "INSERT characters", "DELETE character_aura", ... from the leading tokens of a statement
        static std::string ParseKey(char const* sql);

        static bool InTransaction();
        static void SetInTransaction(bool inTransaction);

        class ScopedTiming
        {
            public:
                ScopedTiming(StatementStats* stats, char const* name)
                    : m_stats(stats), m_name(name), m_start(stats ? std::chrono::steady_clock::now() : std::chrono::steady_clock::time_point()) {}
                ~ScopedTiming()
                {
                    if (m_stats)
                        m_stats->RecordTiming(m_name, uint64(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - m_start).count()));
                }
            private:
                StatementStats* m_stats;
                char const* m_name;
                std::chrono::steady_clock::time_point m_start;
        };

    private:
        struct Counter
        {
            uint64 statements = 0;
            uint64 statementsInTx = 0;
            uint64 bytes = 0;
        };
        struct Samples
        {
            std::vector<uint32> values;
            uint64 extra = 0;
        };

        void Count(std::string const& key, size_t bytes);
        void MaybeFlushLocked();
        void FlushLocked();

        std::mutex m_mutex;
        std::string m_csvPath;
        FILE* m_file = nullptr;
        std::chrono::milliseconds m_interval;
        std::chrono::steady_clock::time_point m_lastFlush;
        std::unordered_map<std::string, Counter> m_counters;
        std::unordered_map<std::string, Samples> m_timings;
        std::vector<std::string> m_preparedKeys;
};

#endif
