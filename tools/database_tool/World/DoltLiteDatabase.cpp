#include "DoltLiteDatabase.h"

#include <sqlite3.h>

#include <utility>

namespace
{
    [[noreturn]] void ThrowLastError(sqlite3* db)
    {
        throw DoltLiteError(sqlite3_errmsg(db));
    }
}

DoltLiteError::DoltLiteError(std::string const& message)
    : std::runtime_error(message)
{
}

DoltLiteStatement::DoltLiteStatement(sqlite3_stmt* stmt)
    : m_stmt(stmt)
{
}

DoltLiteStatement::DoltLiteStatement(DoltLiteStatement&& other) noexcept
    : m_stmt(std::exchange(other.m_stmt, nullptr))
{
}

DoltLiteStatement& DoltLiteStatement::operator=(DoltLiteStatement&& other) noexcept
{
    if (this != &other)
    {
        if (m_stmt)
            sqlite3_finalize(m_stmt);
        m_stmt = std::exchange(other.m_stmt, nullptr);
    }
    return *this;
}

DoltLiteStatement::~DoltLiteStatement()
{
    if (m_stmt)
        sqlite3_finalize(m_stmt);
}

void DoltLiteStatement::BindInt64(int index, int64_t value)
{
    if (sqlite3_bind_int64(m_stmt, index, value) != SQLITE_OK)
        ThrowLastError(sqlite3_db_handle(m_stmt));
}

void DoltLiteStatement::BindDouble(int index, double value)
{
    if (sqlite3_bind_double(m_stmt, index, value) != SQLITE_OK)
        ThrowLastError(sqlite3_db_handle(m_stmt));
}

void DoltLiteStatement::BindText(int index, std::string const& value)
{
    if (sqlite3_bind_text(m_stmt, index, value.data(), static_cast<int>(value.size()), SQLITE_TRANSIENT) != SQLITE_OK)
        ThrowLastError(sqlite3_db_handle(m_stmt));
}

void DoltLiteStatement::BindNull(int index)
{
    if (sqlite3_bind_null(m_stmt, index) != SQLITE_OK)
        ThrowLastError(sqlite3_db_handle(m_stmt));
}

bool DoltLiteStatement::Step()
{
    int result = sqlite3_step(m_stmt);
    if (result == SQLITE_ROW)
        return true;
    if (result == SQLITE_DONE)
        return false;
    ThrowLastError(sqlite3_db_handle(m_stmt));
}

void DoltLiteStatement::Reset()
{
    sqlite3_reset(m_stmt);
}

bool DoltLiteStatement::ColumnIsNull(int index) const
{
    return sqlite3_column_type(m_stmt, index) == SQLITE_NULL;
}

int64_t DoltLiteStatement::ColumnInt64(int index) const
{
    return sqlite3_column_int64(m_stmt, index);
}

double DoltLiteStatement::ColumnDouble(int index) const
{
    return sqlite3_column_double(m_stmt, index);
}

std::string DoltLiteStatement::ColumnText(int index) const
{
    const unsigned char* text = sqlite3_column_text(m_stmt, index);
    int bytes = sqlite3_column_bytes(m_stmt, index);
    return text ? std::string(reinterpret_cast<const char*>(text), static_cast<size_t>(bytes)) : std::string();
}

DoltLiteDatabase::DoltLiteDatabase(std::string const& path)
    : m_db(nullptr)
{
    if (sqlite3_open(path.c_str(), &m_db) != SQLITE_OK)
    {
        DoltLiteError error(sqlite3_errmsg(m_db));
        sqlite3_close(m_db);
        throw error;
    }
}

DoltLiteDatabase::~DoltLiteDatabase()
{
    sqlite3_close(m_db);
}

void DoltLiteDatabase::Execute(std::string const& sql)
{
    char* errorMessage = nullptr;
    if (sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &errorMessage) != SQLITE_OK)
    {
        DoltLiteError error(errorMessage ? errorMessage : "unknown error");
        sqlite3_free(errorMessage);
        throw error;
    }
}

DoltLiteStatement DoltLiteDatabase::Prepare(std::string const& sql)
{
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_db, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr) != SQLITE_OK)
        ThrowLastError(m_db);
    return DoltLiteStatement(stmt);
}

void DoltLiteDatabase::Begin()
{
    Execute("BEGIN");
}

void DoltLiteDatabase::Commit()
{
    Execute("COMMIT");
}

void DoltLiteDatabase::Rollback()
{
    Execute("ROLLBACK");
}

std::string DoltLiteDatabase::DoltCommit(std::string const& message)
{
    DoltLiteStatement statement = Prepare("SELECT dolt_commit('-A', '-m', ?)");
    statement.BindText(1, message);
    if (!statement.Step())
        throw DoltLiteError("dolt_commit returned no rows");
    return statement.ColumnText(0);
}
