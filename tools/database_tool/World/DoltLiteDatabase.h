#ifndef DATABASE_TOOL_DOLTLITEDATABASE_H
#define DATABASE_TOOL_DOLTLITEDATABASE_H

#include <cstdint>
#include <stdexcept>
#include <string>

struct sqlite3;
struct sqlite3_stmt;

// Thrown for any DoltLite/sqlite3 failure, with the library's own message.
struct DoltLiteError : std::runtime_error
{
    explicit DoltLiteError(std::string const& message);
};

// A prepared statement. Binds and columns are 0-based, matching sqlite3_bind_*
// (1-based) and sqlite3_column_* (0-based) respectively, exactly as the C API does.
class DoltLiteStatement
{
public:
    DoltLiteStatement(DoltLiteStatement&& other) noexcept;
    DoltLiteStatement& operator=(DoltLiteStatement&& other) noexcept;
    ~DoltLiteStatement();

    DoltLiteStatement(DoltLiteStatement const&) = delete;
    DoltLiteStatement& operator=(DoltLiteStatement const&) = delete;

    void BindInt64(int index, int64_t value);
    void BindDouble(int index, double value);
    void BindText(int index, std::string const& value);
    void BindNull(int index);

    // Advances to the next row. Returns false once the statement is exhausted.
    bool Step();
    void Reset();

    int ColumnCount() const;
    std::string ColumnName(int index) const;
    bool ColumnIsNull(int index) const;
    // sqlite3 fundamental type of the current value: 1 integer, 2 float, 3 text, 4 blob, 5 null.
    int ColumnType(int index) const;
    int64_t ColumnInt64(int index) const;
    double ColumnDouble(int index) const;
    std::string ColumnText(int index) const;

private:
    friend class DoltLiteDatabase;
    explicit DoltLiteStatement(sqlite3_stmt* stmt);

    sqlite3_stmt* m_stmt;
};

// RAII wrapper over a DoltLite (versioned SQLite) database file. DoltLite
// exposes version control as plain SQL (dolt_commit, dolt_log, ...) on an
// ordinary sqlite3 connection, so this is otherwise just the sqlite3 C API.
// Kept free of anything database_tool-specific so a later server-side
// DatabaseDoltLite can reuse it.
class DoltLiteDatabase
{
public:
    explicit DoltLiteDatabase(std::string const& path);
    ~DoltLiteDatabase();

    DoltLiteDatabase(DoltLiteDatabase const&) = delete;
    DoltLiteDatabase& operator=(DoltLiteDatabase const&) = delete;

    // Runs sql with no bound parameters and no result rows.
    void Execute(std::string const& sql);

    DoltLiteStatement Prepare(std::string const& sql);

    void Begin();
    void Commit();
    void Rollback();

    sqlite3* Handle() const { return m_db; }

private:
    sqlite3* m_db;
};

#endif
