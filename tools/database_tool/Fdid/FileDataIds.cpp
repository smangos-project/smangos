#include "Fdid/FileDataIds.h"

#include <algorithm>
#include <fstream>
#include <sstream>

std::string NormalizeClientPath(std::string const& path)
{
    std::string result = path;
    for (char& c : result)
    {
        if (c == '\\')
            c = '/';
        else if (c >= 'A' && c <= 'Z')
            c = static_cast<char>(c - 'A' + 'a');
    }
    return result;
}

namespace
{
    struct TransactionGuard
    {
        DoltLiteDatabase& db;
        bool open = true;

        explicit TransactionGuard(DoltLiteDatabase& database) : db(database) { db.BeginImmediate(); }
        ~TransactionGuard()
        {
            if (open)
                try { db.Rollback(); } catch (std::exception const&) {}
        }
        void Commit()
        {
            db.Commit();
            open = false;
        }
    };
}

FdidImportResult ImportFdidFile(DoltLiteDatabase& db, std::filesystem::path const& file, std::string const& source)
{
    FdidImportResult result;
    std::ifstream in(file, std::ios::binary);
    if (!in)
    {
        result.error = "cannot open " + file.string();
        return result;
    }

    try
    {
        TransactionGuard transaction(db);
        DoltLiteStatement byId = db.Prepare("SELECT path, source FROM file_data_id WHERE id = ?");
        DoltLiteStatement byPath = db.Prepare("SELECT id FROM file_data_id WHERE path = ?");
        DoltLiteStatement insert = db.Prepare("INSERT INTO file_data_id (id, path, source) VALUES (?, ?, ?)");
        DoltLiteStatement update = db.Prepare("UPDATE file_data_id SET path = ?, retired = 0 WHERE id = ?");
        bool const stock = source == "listfile";

        std::string line;
        while (std::getline(in, line))
        {
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            size_t const sep = line.find(';');
            if (sep == std::string::npos || sep == 0 || sep + 1 >= line.size()
                || line.find_first_not_of("0123456789") < sep)
                continue;
            int64_t const id = std::stoll(line.substr(0, sep));
            std::string const path = NormalizeClientPath(line.substr(sep + 1));

            std::string existingPath, existingSource;
            bool haveId = false;
            byId.Reset();
            byId.BindInt64(1, id);
            if (byId.Step())
            {
                haveId = true;
                existingPath = byId.ColumnText(0);
                existingSource = byId.ColumnText(1);
            }
            byPath.Reset();
            byPath.BindText(1, path);
            bool const havePath = byPath.Step();
            int64_t const pathId = havePath ? byPath.ColumnInt64(0) : 0;

            if (haveId && existingPath == path)
            {
                ++result.unchanged;
                continue;
            }
            if (haveId && stock && existingSource == "listfile" && (!havePath || pathId == id))
            {
                update.Reset();
                update.BindText(1, path);
                update.BindInt64(2, id);
                update.Step();
                ++result.added;
                continue;
            }
            if (haveId || havePath)
            {
                result.conflicts.push_back(line + " conflicts with " + std::to_string(haveId ? id : pathId) + ";"
                    + (haveId ? existingPath : path));
                continue;
            }
            insert.Reset();
            insert.BindInt64(1, id);
            insert.BindText(2, path);
            insert.BindText(3, source);
            insert.Step();
            ++result.added;
        }
        transaction.Commit();
    }
    catch (std::exception const& e)
    {
        result.error = e.what();
    }
    return result;
}

std::vector<int64_t> AllocateFdids(DoltLiteDatabase& db, std::vector<std::string> const& paths, std::string const& source)
{
    std::vector<int64_t> ids;
    TransactionGuard transaction(db);
    DoltLiteStatement find = db.Prepare("SELECT id, retired FROM file_data_id WHERE path = ?");
    DoltLiteStatement revive = db.Prepare("UPDATE file_data_id SET retired = 0 WHERE id = ?");
    DoltLiteStatement insert = db.Prepare(
        "INSERT INTO file_data_id (id, path, source) SELECT max(coalesce(max(id), 0) + 1, ?), ?, ? "
        "FROM file_data_id WHERE id >= ?");
    for (std::string const& raw : paths)
    {
        std::string const path = NormalizeClientPath(raw);
        find.Reset();
        find.BindText(1, path);
        if (find.Step())
        {
            int64_t const id = find.ColumnInt64(0);
            if (find.ColumnInt64(1))
            {
                revive.Reset();
                revive.BindInt64(1, id);
                revive.Step();
            }
            ids.push_back(id);
            continue;
        }
        insert.Reset();
        insert.BindInt64(1, kFirstCustomId);
        insert.BindText(2, path);
        insert.BindText(3, source);
        insert.BindInt64(4, kFirstCustomId);
        insert.Step();
        find.Reset();
        find.BindText(1, path);
        find.Step();
        ids.push_back(find.ColumnInt64(0));
    }
    transaction.Commit();
    return ids;
}

uint32_t RetireFdids(DoltLiteDatabase& db, std::vector<std::string> const& paths)
{
    uint32_t retired = 0;
    TransactionGuard transaction(db);
    DoltLiteStatement retire = db.Prepare("UPDATE file_data_id SET retired = 1 WHERE path = ? AND retired = 0");
    DoltLiteStatement check = db.Prepare("SELECT changes()");
    for (std::string const& raw : paths)
    {
        retire.Reset();
        retire.BindText(1, NormalizeClientPath(raw));
        retire.Step();
        check.Reset();
        if (check.Step())
            retired += static_cast<uint32_t>(check.ColumnInt64(0));
    }
    transaction.Commit();
    return retired;
}

FdidExportResult ExportFdidIndex(DoltLiteDatabase& db, std::filesystem::path const& indexFile)
{
    FdidExportResult result;
    std::string content;
    try
    {
        DoltLiteStatement rows = db.Prepare("SELECT id, path FROM file_data_id WHERE retired = 0 ORDER BY id");
        while (rows.Step())
        {
            content += std::to_string(rows.ColumnInt64(0));
            content += ';';
            content += rows.ColumnText(1);
            content += '\n';
            ++result.rows;
        }
    }
    catch (std::exception const& e)
    {
        result.error = e.what();
        return result;
    }

    {
        std::ifstream existing(indexFile, std::ios::binary);
        if (existing)
        {
            std::ostringstream buffer;
            buffer << existing.rdbuf();
            if (buffer.str() == content)
                return result;
        }
    }

    std::error_code ec;
    std::filesystem::create_directories(indexFile.parent_path(), ec);
    std::filesystem::path const temp = indexFile.string() + ".tmp";
    {
        std::ofstream out(temp, std::ios::binary | std::ios::trunc);
        out << content;
        if (!out)
        {
            result.error = "cannot write " + temp.string();
            return result;
        }
    }
    std::filesystem::rename(temp, indexFile, ec);
    if (ec)
    {
        result.error = "cannot replace " + indexFile.string() + ": " + ec.message();
        return result;
    }
    result.written = true;
    return result;
}
