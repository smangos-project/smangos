#include "Database/StatementStats.h"

#include <algorithm>
#include <cctype>
#include <cstring>

namespace
{
    thread_local bool s_inTransaction = false;

    char const* SkipSpace(char const* p)
    {
        while (*p && std::isspace(uint8(*p)))
            ++p;
        return p;
    }

    // Reads one identifier or keyword, without backticks, and advances p.
    std::string NextToken(char const*& p)
    {
        p = SkipSpace(p);
        std::string token;
        if (*p == '`')
        {
            ++p;
            while (*p && *p != '`')
                token += *p++;
            if (*p == '`')
                ++p;
            return token;
        }
        while (*p && !std::isspace(uint8(*p)) && *p != '(' && *p != ',' && *p != ';')
            token += *p++;
        return token;
    }

    std::string Upper(std::string s)
    {
        std::transform(s.begin(), s.end(), s.begin(), [](char c) { return char(std::toupper(uint8(c))); });
        return s;
    }

    uint32 Percentile(std::vector<uint32> const& sorted, double q)
    {
        if (sorted.empty())
            return 0;
        size_t i = size_t(q * double(sorted.size() - 1) + 0.5);
        return sorted[std::min(i, sorted.size() - 1)];
    }

    uint64 UnixMs()
    {
        return uint64(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    }
}

StatementStats::StatementStats(std::string csvPath, uint32 intervalMs)
    : m_csvPath(std::move(csvPath)), m_interval(intervalMs), m_lastFlush(std::chrono::steady_clock::now())
{
    m_file = fopen(m_csvPath.c_str(), "a");
    if (m_file)
    {
        fseek(m_file, 0, SEEK_END);
        if (ftell(m_file) == 0)
            fprintf(m_file, "unix_ms,kind,key,count,count_in_tx,bytes,p50,p99,max,total\n");
    }
}

StatementStats::~StatementStats()
{
    Flush();
    if (m_file)
        fclose(m_file);
}

bool StatementStats::InTransaction()
{
    return s_inTransaction;
}

void StatementStats::SetInTransaction(bool inTransaction)
{
    s_inTransaction = inTransaction;
}

std::string StatementStats::ParseKey(char const* sql)
{
    char const* p = sql;
    std::string verb = Upper(NextToken(p));
    std::string word;
    if (verb == "INSERT" || verb == "REPLACE")
    {
        do
            word = NextToken(p);
        while (!word.empty() && (Upper(word) == "INTO" || Upper(word) == "IGNORE" || Upper(word) == "LOW_PRIORITY" || Upper(word) == "DELAYED" || Upper(word) == "HIGH_PRIORITY"));
    }
    else if (verb == "UPDATE")
    {
        do
            word = NextToken(p);
        while (!word.empty() && (Upper(word) == "IGNORE" || Upper(word) == "LOW_PRIORITY"));
    }
    else if (verb == "DELETE")
    {
        do
            word = NextToken(p);
        while (!word.empty() && Upper(word) != "FROM");
        word = NextToken(p);
    }
    else
        return verb;

    return verb + " " + word;
}

void StatementStats::Count(std::string const& key, size_t bytes)
{
    Counter& c = m_counters[key];
    ++c.statements;
    if (s_inTransaction)
        ++c.statementsInTx;
    c.bytes += bytes;
}

void StatementStats::RecordPlain(std::string const& sql)
{
    std::string key = ParseKey(sql.c_str());
    std::lock_guard<std::mutex> guard(m_mutex);
    Count(key, sql.size());
    MaybeFlushLocked();
}

void StatementStats::RegisterPrepared(int stmtIndex, std::string const& fmt)
{
    std::string key = ParseKey(fmt.c_str());
    std::lock_guard<std::mutex> guard(m_mutex);
    if (m_preparedKeys.size() <= size_t(stmtIndex))
        m_preparedKeys.resize(stmtIndex + 1);
    m_preparedKeys[stmtIndex] = std::move(key);
}

void StatementStats::RecordPrepared(int stmtIndex, size_t paramBytes)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    if (stmtIndex >= 0 && size_t(stmtIndex) < m_preparedKeys.size() && !m_preparedKeys[stmtIndex].empty())
        Count(m_preparedKeys[stmtIndex], paramBytes);
    else
        Count("PREPARED #" + std::to_string(stmtIndex), paramBytes);
    MaybeFlushLocked();
}

void StatementStats::RecordTransaction(uint32 statements, uint64 us)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    Samples& s = m_timings["transaction"];
    s.values.push_back(uint32(std::min<uint64>(us, UINT32_MAX)));
    s.extra += statements;
    MaybeFlushLocked();
}

void StatementStats::RecordTiming(char const* name, uint64 us)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    m_timings[name].values.push_back(uint32(std::min<uint64>(us, UINT32_MAX)));
    MaybeFlushLocked();
}

void StatementStats::Flush()
{
    std::lock_guard<std::mutex> guard(m_mutex);
    FlushLocked();
}

void StatementStats::MaybeFlushLocked()
{
    if (std::chrono::steady_clock::now() - m_lastFlush >= m_interval)
        FlushLocked();
}

void StatementStats::FlushLocked()
{
    m_lastFlush = std::chrono::steady_clock::now();
    if (!m_file)
    {
        m_counters.clear();
        m_timings.clear();
        return;
    }

    uint64 const ts = UnixMs();
    for (auto const& entry : m_counters)
        fprintf(m_file, "%llu,stmt,%s,%llu,%llu,%llu,,,,\n", (unsigned long long)ts, entry.first.c_str(),
            (unsigned long long)entry.second.statements, (unsigned long long)entry.second.statementsInTx, (unsigned long long)entry.second.bytes);

    // timings in microseconds; for transactions count_in_tx is the number of statements they ran
    for (auto& entry : m_timings)
    {
        std::vector<uint32>& v = entry.second.values;
        std::sort(v.begin(), v.end());
        uint64 total = 0;
        for (uint32 x : v)
            total += x;
        fprintf(m_file, "%llu,timing,%s,%llu,%llu,,%u,%u,%u,%llu\n", (unsigned long long)ts, entry.first.c_str(),
            (unsigned long long)v.size(), (unsigned long long)entry.second.extra,
            Percentile(v, 0.5), Percentile(v, 0.99), v.empty() ? 0 : v.back(), (unsigned long long)total);
    }
    fflush(m_file);

    m_counters.clear();
    m_timings.clear();
}
