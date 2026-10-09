#include "Persistence/SqliteSeed.h"

#include <cstdio>
#include <filesystem>
#include <sqlite3.h>

namespace Persistence
{
    namespace
    {
        void PutBE32(std::vector<uint8>& image, size_t offset, uint32 value)
        {
            image[offset] = uint8(value >> 24);
            image[offset + 1] = uint8(value >> 16);
            image[offset + 2] = uint8(value >> 8);
            image[offset + 3] = uint8(value);
        }

        void PutBE16(std::vector<uint8>& image, size_t offset, uint32 value)
        {
            // 65536 is stored as 1 (page size) or 0 (cell content offset) by the callers
            image[offset] = uint8(value >> 8);
            image[offset + 1] = uint8(value);
        }
    }

    std::vector<uint8> EmptySqliteImage(uint32 pageSize)
    {
        std::vector<uint8> image(pageSize, 0);
        static char const magic[16] = "SQLite format 3";
        std::copy(magic, magic + 16, image.begin());
        PutBE16(image, 16, pageSize == 65536 ? 1 : pageSize);
        image[18] = 1;              // write version: legacy (rollback journal)
        image[19] = 1;              // read version
        image[20] = 0;              // reserved bytes per page
        image[21] = 64;             // max embedded payload fraction
        image[22] = 32;             // min embedded payload fraction
        image[23] = 32;             // leaf payload fraction
        PutBE32(image, 24, 1);      // file change counter
        PutBE32(image, 28, 1);      // database size in pages
        PutBE32(image, 40, 1);      // schema cookie
        PutBE32(image, 92, 1);      // version-valid-for (= change counter)
        PutBE32(image, 96, SQLITE_VERSION_NUMBER);

        // page 1: empty table-leaf b-tree page, content area starts at the page end
        image[100] = 0x0D;
        PutBE16(image, 105, pageSize == 65536 ? 0 : pageSize);
        return image;
    }

    bool EnsureStockSqliteFile(std::string const& path, uint32 pageSize, std::string& error)
    {
        if (pageSize < 512 || pageSize > 65536 || (pageSize & (pageSize - 1)))
        {
            error = "invalid sqlite page size " + std::to_string(pageSize);
            return false;
        }

        std::error_code ec;
        if (std::filesystem::exists(path, ec) && std::filesystem::file_size(path, ec) > 0)
            return true;

        std::vector<uint8> const image = EmptySqliteImage(pageSize);
        FILE* file = std::fopen(path.c_str(), "wb");
        if (!file)
        {
            error = "cannot create " + path;
            return false;
        }
        bool const ok = std::fwrite(image.data(), 1, image.size(), file) == image.size();
        if (std::fclose(file) != 0 || !ok)
        {
            error = "cannot write " + path;
            return false;
        }
        return true;
    }

    bool IsStockEngine(sqlite3* db, std::string& engine)
    {
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, "SELECT doltlite_engine()", -1, &stmt, nullptr) != SQLITE_OK)
        {
            // not doltlite: plain sqlite
            engine = "sqlite";
            return true;
        }
        engine.clear();
        if (sqlite3_step(stmt) == SQLITE_ROW)
            if (unsigned char const* text = sqlite3_column_text(stmt, 0))
                engine = reinterpret_cast<char const*>(text);
        sqlite3_finalize(stmt);
        return engine == "orig";
    }

    sqlite3* OpenStockSqlite(std::string const& path, uint32 pageSize, std::string& error)
    {
        if (!EnsureStockSqliteFile(path, pageSize, error))
            return nullptr;

        sqlite3* db = nullptr;
        if (sqlite3_open(path.c_str(), &db) != SQLITE_OK)
        {
            error = "cannot open " + path + ": " + sqlite3_errmsg(db);
            sqlite3_close(db);
            return nullptr;
        }

        std::string engine;
        if (!IsStockEngine(db, engine))
        {
            error = path + " opened with engine '" + engine + "', expected the stock sqlite engine";
            sqlite3_close(db);
            return nullptr;
        }
        return db;
    }
}
