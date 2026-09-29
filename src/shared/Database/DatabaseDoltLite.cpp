#include "DatabaseEnv.h"
#include "Errors.h"
#include "Log.h"

#include <mutex>
#include <sqlite3.h>

struct DoltLiteHandle
{
    ~DoltLiteHandle() { sqlite3_close(db); }

    sqlite3* db = nullptr;
    std::recursive_mutex mutex;
};

class QueryResultDoltLite : public QueryResult
{
    public:
        QueryResultDoltLite(std::vector<std::string> cells, std::vector<bool> nulls, std::vector<Field::DataTypes> types) :
            QueryResult(nulls.size() / std::max<size_t>(types.size(), 1), types.size()),
            m_cells(std::move(cells)), m_nulls(std::move(nulls)), m_types(std::move(types))
        {
            mCurrentRow = new Field[mFieldCount];
            for (uint32 i = 0; i < mFieldCount; ++i)
                mCurrentRow[i].SetType(m_types[i]);
        }

        ~QueryResultDoltLite() override { delete[] mCurrentRow; }

        bool NextRow() override
        {
            if (m_row >= mRowCount)
                return false;

            size_t base = m_row++ * mFieldCount;
            for (uint32 i = 0; i < mFieldCount; ++i)
                mCurrentRow[i].SetValue(m_nulls[base + i] ? nullptr : m_cells[base + i].c_str());
            return true;
        }

    private:
        std::vector<std::string> m_cells;
        std::vector<bool> m_nulls;
        std::vector<Field::DataTypes> m_types;
        uint64 m_row = 0;
};

static Field::DataTypes ConvertType(int type)
{
    switch (type)
    {
        case SQLITE_INTEGER: return Field::DB_TYPE_INTEGER;
        case SQLITE_FLOAT:   return Field::DB_TYPE_FLOAT;
        case SQLITE_TEXT:
        case SQLITE_BLOB:    return Field::DB_TYPE_STRING;
        default:             return Field::DB_TYPE_UNKNOWN;
    }
}

DoltLiteConnection::~DoltLiteConnection()
{
    FreePreparedStatements();
}

bool DoltLiteConnection::OpenConnection(bool /*reconnect*/)
{
    m_database = m_host;
    return m_handle && m_handle->db;
}

std::unique_ptr<QueryResult> DoltLiteConnection::Query(std::string const& sql)
{
    std::unique_ptr<QueryNamedResult> named = QueryNamed(sql);
    if (!named)
        return nullptr;
    return named->ReleaseQuery();
}

std::unique_ptr<QueryNamedResult> DoltLiteConnection::QueryNamed(std::string const& sql)
{
    std::lock_guard guard(m_handle->mutex);

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_handle->db, sql.c_str(), int(sql.size()), &stmt, nullptr) != SQLITE_OK)
    {
        sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "SQL: %s", sql.c_str());
        sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "SQL ERROR: %s", sqlite3_errmsg(m_handle->db));
        return nullptr;
    }

    int const columns = sqlite3_column_count(stmt);
    QueryFieldNames names;
    for (int i = 0; i < columns; ++i)
        names.emplace_back(sqlite3_column_name(stmt, i));

    std::vector<std::string> cells;
    std::vector<bool> nulls;
    std::vector<Field::DataTypes> types(columns, Field::DB_TYPE_UNKNOWN);
    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        for (int i = 0; i < columns; ++i)
        {
            int type = sqlite3_column_type(stmt, i);
            bool isNull = type == SQLITE_NULL;
            if (!isNull && types[i] == Field::DB_TYPE_UNKNOWN)
                types[i] = ConvertType(type);

            nulls.push_back(isNull);
            if (isNull)
                cells.emplace_back();
            else
                cells.emplace_back(reinterpret_cast<char const*>(sqlite3_column_text(stmt, i)), size_t(sqlite3_column_bytes(stmt, i)));
        }
    }

    if (rc != SQLITE_DONE)
    {
        sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "SQL: %s", sql.c_str());
        sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "SQL ERROR: %s", sqlite3_errmsg(m_handle->db));
        sqlite3_finalize(stmt);
        return nullptr;
    }
    sqlite3_finalize(stmt);

    if (nulls.empty())
        return nullptr;

    auto result = std::make_unique<QueryResultDoltLite>(std::move(cells), std::move(nulls), std::move(types));
    result->NextRow();
    return std::make_unique<QueryNamedResult>(std::move(result), std::move(names));
}

bool DoltLiteConnection::Execute(std::string const& sql)
{
    std::lock_guard guard(m_handle->mutex);

    char* error = nullptr;
    if (sqlite3_exec(m_handle->db, sql.c_str(), nullptr, nullptr, &error) != SQLITE_OK)
    {
        sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "SQL: %s", sql.c_str());
        sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "SQL ERROR: %s", error ? error : "unknown error");
        sqlite3_free(error);
        return false;
    }
    return true;
}

unsigned long DoltLiteConnection::escape_string(char* to, char const* from, unsigned long length)
{
    char* out = to;
    for (unsigned long i = 0; i < length; ++i)
    {
        if (from[i] == '\'')
            *out++ = '\'';
        *out++ = from[i];
    }
    *out = '\0';
    return out - to;
}

bool DoltLiteConnection::BeginTransaction()
{
    return Execute("BEGIN");
}

bool DoltLiteConnection::CommitTransaction()
{
    return Execute("COMMIT");
}

bool DoltLiteConnection::RollbackTransaction()
{
    return Execute("ROLLBACK");
}

DatabaseDoltLite::~DatabaseDoltLite()
{
    StopServer();
}

bool DatabaseDoltLite::Initialize(char const* infoString, int nConns, int nWorkers)
{
    m_handle = std::make_shared<DoltLiteHandle>();
    if (sqlite3_open(infoString, &m_handle->db) != SQLITE_OK)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Cannot open database file %s: %s", infoString, sqlite3_errmsg(m_handle->db));
        return false;
    }

    // One shared handle: a single async worker keeps writes and transactions in order.
    return Database::Initialize(infoString, nConns, 1);
}

SqlConnection* DatabaseDoltLite::CreateConnection()
{
    return new DoltLiteConnection(*this, m_handle);
}
