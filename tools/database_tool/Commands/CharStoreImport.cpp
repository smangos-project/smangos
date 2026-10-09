#include "Commands/CharStoreImport.h"

#include "Sql/MysqlDdlTranslator.h"
#include "World/DoltLiteDatabase.h"

#include "Persistence/SqliteSeed.h"

#include <mysql.h>
#include <sqlite3.h>

#include <algorithm>
#include <memory>
#include <sstream>
#include <stdexcept>

namespace
{
    struct MysqlCloser
    {
        void operator()(MYSQL* mysql) const { mysql_close(mysql); }
    };
    using MysqlHandle = std::unique_ptr<MYSQL, MysqlCloser>;

    struct ResultFree
    {
        void operator()(MYSQL_RES* result) const { mysql_free_result(result); }
    };
    using MysqlResult = std::unique_ptr<MYSQL_RES, ResultFree>;

    MysqlHandle Connect(std::string const& info)
    {
        std::vector<std::string> parts;
        std::stringstream stream(info);
        for (std::string part; std::getline(stream, part, ';');)
            parts.push_back(part);
        if (parts.size() != 5)
            throw std::runtime_error("--mysql must be \"host;port;user;pass;db\"");

        MYSQL* mysql = mysql_init(nullptr);
        if (!mysql)
            throw std::runtime_error("mysql_init failed");
        MysqlHandle handle(mysql);
        mysql_options(mysql, MYSQL_SET_CHARSET_NAME, "utf8mb4");

        char const* host = parts[0].c_str();
        unsigned int port = 0;
        char const* socket = nullptr;
        if (parts[0] == ".")
        {
            // same convention as DatabaseMysql: "." means the unix socket given as port
            host = "localhost";
            socket = parts[1].c_str();
        }
        else
            port = static_cast<unsigned int>(std::stoul(parts[1]));

        if (!mysql_real_connect(mysql, host, parts[2].c_str(), parts[3].c_str(), parts[4].c_str(), port, socket, 0))
            throw std::runtime_error(std::string("cannot connect to MySQL: ") + mysql_error(mysql));
        return handle;
    }

    MysqlResult Query(MYSQL* mysql, std::string const& sql, bool stream = false)
    {
        if (mysql_real_query(mysql, sql.data(), static_cast<unsigned long>(sql.size())) != 0)
            throw std::runtime_error(sql + ": " + mysql_error(mysql));
        MysqlResult result(stream ? mysql_use_result(mysql) : mysql_store_result(mysql));
        if (!result)
            throw std::runtime_error(sql + ": " + mysql_error(mysql));
        return result;
    }

    enum class BindKind { Integer, Real, Text, Blob };

    BindKind KindOf(MYSQL_FIELD const& field)
    {
        switch (field.type)
        {
            case MYSQL_TYPE_TINY:
            case MYSQL_TYPE_SHORT:
            case MYSQL_TYPE_LONG:
            case MYSQL_TYPE_INT24:
            case MYSQL_TYPE_LONGLONG:
            case MYSQL_TYPE_YEAR:
                return BindKind::Integer;
            case MYSQL_TYPE_FLOAT:
            case MYSQL_TYPE_DOUBLE:
                return BindKind::Real;
            case MYSQL_TYPE_TINY_BLOB:
            case MYSQL_TYPE_MEDIUM_BLOB:
            case MYSQL_TYPE_LONG_BLOB:
            case MYSQL_TYPE_BLOB:
            case MYSQL_TYPE_STRING:
            case MYSQL_TYPE_VAR_STRING:
                return field.charsetnr == 63 ? BindKind::Blob : BindKind::Text; // 63 = binary
            default:
                return BindKind::Text; // DECIMAL, dates, ...: sqlite's column affinity converts
        }
    }

    void Check(sqlite3* db, int rc, char const* what)
    {
        if (rc != SQLITE_OK && rc != SQLITE_DONE && rc != SQLITE_ROW)
            throw std::runtime_error(std::string(what) + ": " + sqlite3_errmsg(db));
    }

    uint64_t CopyRows(MYSQL* mysql, sqlite3* db, std::string const& table)
    {
        MysqlResult result = Query(mysql, "SELECT * FROM `" + table + "`", true);
        unsigned int const columns = mysql_num_fields(result.get());
        MYSQL_FIELD const* fields = mysql_fetch_fields(result.get());

        std::vector<BindKind> kinds;
        std::string sql = "INSERT INTO `" + table + "` (";
        for (unsigned int i = 0; i < columns; ++i)
        {
            kinds.push_back(KindOf(fields[i]));
            sql += (i ? ", `" : "`") + std::string(fields[i].name) + "`";
        }
        sql += ") VALUES (";
        for (unsigned int i = 0; i < columns; ++i)
            sql += i ? ", ?" : "?";
        sql += ")";

        sqlite3_stmt* raw = nullptr;
        Check(db, sqlite3_prepare_v2(db, sql.c_str(), -1, &raw, nullptr), sql.c_str());
        std::unique_ptr<sqlite3_stmt, int (*)(sqlite3_stmt*)> stmt(raw, sqlite3_finalize);

        uint64_t rows = 0;
        while (MYSQL_ROW row = mysql_fetch_row(result.get()))
        {
            unsigned long const* lengths = mysql_fetch_lengths(result.get());
            for (unsigned int i = 0; i < columns; ++i)
            {
                int const index = static_cast<int>(i) + 1;
                if (!row[i])
                {
                    sqlite3_bind_null(raw, index);
                    continue;
                }
                switch (kinds[i])
                {
                    case BindKind::Integer:
                        if (fields[i].flags & UNSIGNED_FLAG)
                            sqlite3_bind_int64(raw, index, static_cast<sqlite3_int64>(std::stoull(row[i])));
                        else
                            sqlite3_bind_int64(raw, index, std::stoll(row[i]));
                        break;
                    case BindKind::Real:
                        sqlite3_bind_double(raw, index, std::stod(row[i]));
                        break;
                    case BindKind::Text:
                        sqlite3_bind_text(raw, index, row[i], static_cast<int>(lengths[i]), SQLITE_STATIC);
                        break;
                    case BindKind::Blob:
                        sqlite3_bind_blob(raw, index, row[i], static_cast<int>(lengths[i]), SQLITE_STATIC);
                        break;
                }
            }
            Check(db, sqlite3_step(raw), table.c_str());
            sqlite3_reset(raw);
            ++rows;
        }
        if (mysql_errno(mysql))
            throw std::runtime_error(table + ": " + mysql_error(mysql));
        return rows;
    }

    uint64_t MysqlCount(MYSQL* mysql, std::string const& table)
    {
        MysqlResult result = Query(mysql, "SELECT COUNT(*) FROM `" + table + "`");
        MYSQL_ROW row = mysql_fetch_row(result.get());
        return row && row[0] ? std::stoull(row[0]) : 0;
    }
}

std::string WithoutRowidIfComposite(std::string const& createTable)
{
    size_t const key = createTable.find("\n  PRIMARY KEY (");
    if (key == std::string::npos)
        return createTable;
    size_t const end = createTable.find(')', key);
    if (createTable.find(',', key) > end)
        return createTable;
    return createTable + " WITHOUT ROWID";
}

CharStoreImportResult ImportCharStore(CharStoreImportOptions const& options)
{
    CharStoreImportResult result;
    try
    {
        std::error_code ec;
        if (std::filesystem::exists(options.out, ec) && std::filesystem::file_size(options.out, ec) > 0)
            throw std::runtime_error(options.out.string() + " already exists");

        MysqlHandle mysql = Connect(options.mysqlInfo);

        std::vector<std::string> tables;
        {
            MysqlResult list = Query(mysql.get(), "SHOW FULL TABLES WHERE Table_type = 'BASE TABLE'");
            while (MYSQL_ROW row = mysql_fetch_row(list.get()))
                tables.push_back(row[0]);
        }
        if (!options.tables.empty())
        {
            for (std::string const& wanted : options.tables)
                if (std::find(tables.begin(), tables.end(), wanted) == tables.end())
                    throw std::runtime_error("no table `" + wanted + "` in the MySQL database");
            tables = options.tables;
        }

        std::string error;
        if (!Persistence::EnsureStockSqliteFile(options.out.string(), options.pageSize, true, error))
            throw std::runtime_error(error);

        DoltLiteDatabase db(options.out.string());
        std::string engine;
        if (!Persistence::IsStockEngine(db.Handle(), engine))
            throw std::runtime_error("opened with engine '" + engine + "', expected the stock sqlite engine");

        db.Begin();

        std::vector<std::string> deferredIndexes;
        for (std::string const& table : tables)
        {
            MysqlResult create = Query(mysql.get(), "SHOW CREATE TABLE `" + table + "`");
            MYSQL_ROW row = mysql_fetch_row(create.get());
            if (!row || !row[1])
                throw std::runtime_error("SHOW CREATE TABLE `" + table + "` returned nothing");

            TranslatedTable translated = TranslateMysqlCreateTable(row[1]);
            std::string createTable = translated.statements.front();
            if (options.withoutRowid)
                createTable = WithoutRowidIfComposite(createTable);
            db.Execute(createTable);
            deferredIndexes.insert(deferredIndexes.end(), translated.statements.begin() + 1, translated.statements.end());

            CharStoreTableResult& tableResult = result.tables.emplace_back();
            tableResult.table = table;
            tableResult.warnings = std::move(translated.warnings);
            if (options.schemaOnly)
                continue;

            tableResult.rows = CopyRows(mysql.get(), db.Handle(), table);
            uint64_t const expected = MysqlCount(mysql.get(), table);
            if (tableResult.rows != expected)
                throw std::runtime_error("`" + table + "`: copied " + std::to_string(tableResult.rows) +
                    " rows, MySQL has " + std::to_string(expected));
        }

        for (std::string const& index : deferredIndexes)
            db.Execute(index);
        db.Commit();

        DoltLiteStatement integrity = db.Prepare("PRAGMA integrity_check");
        if (!integrity.Step() || integrity.ColumnText(0) != "ok")
            throw std::runtime_error("integrity_check failed: " + integrity.ColumnText(0));
        DoltLiteStatement journal = db.Prepare("PRAGMA journal_mode");
        if (!journal.Step() || journal.ColumnText(0) != "wal")
            throw std::runtime_error("journal mode is " + journal.ColumnText(0) + ", expected wal");
    }
    catch (std::exception const& e)
    {
        result.error = e.what();
    }
    return result;
}
