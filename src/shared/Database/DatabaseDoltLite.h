#ifndef DATABASEDOLTLITE_H
#define DATABASEDOLTLITE_H

#include "Database.h"

#include <memory>

struct DoltLiteHandle;

class DoltLiteConnection : public SqlConnection
{
    public:
        DoltLiteConnection(Database& db, std::shared_ptr<DoltLiteHandle> handle) : SqlConnection(db), m_handle(std::move(handle)) {}
        ~DoltLiteConnection() override;

        bool OpenConnection(bool reconnect) override;

        std::unique_ptr<QueryResult> Query(std::string const& sql) override;
        std::unique_ptr<QueryNamedResult> QueryNamed(std::string const& sql) override;
        bool Execute(std::string const& sql) override;

        unsigned long escape_string(char* to, char const* from, unsigned long length) override;

        bool BeginTransaction() override;
        bool CommitTransaction() override;
        bool RollbackTransaction() override;

    private:
        std::shared_ptr<DoltLiteHandle> m_handle;
};

// A DoltLite (SQLite) database file. All connections share one handle, so the file is opened once.
class DatabaseDoltLite : public Database
{
    public:
        DatabaseDoltLite() = default;
        ~DatabaseDoltLite() override;

        // infoString is the path of the database file.
        bool Initialize(char const* infoString, int nConns = 1, int nWorkers = 1) override;

    protected:
        SqlConnection* CreateConnection() override;

    private:
        std::shared_ptr<DoltLiteHandle> m_handle;
};

#endif
