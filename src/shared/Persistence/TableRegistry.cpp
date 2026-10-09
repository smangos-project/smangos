#include "Persistence/TableRegistry.h"

#include <algorithm>
#include <cctype>
#include <sqlite3.h>

namespace Persistence
{
    namespace
    {
        struct SchemaCol
        {
            std::string name;
            std::string type;
            bool notNull;
            int pk;
        };

        // sqlite column affinity rules (datatype3.html 3.1), NUMERIC folded into Int
        ColType Affinity(std::string type)
        {
            std::transform(type.begin(), type.end(), type.begin(), [](unsigned char c) { return char(std::toupper(c)); });
            if (type.find("INT") != std::string::npos)
                return ColType::Int;
            if (type.find("CHAR") != std::string::npos || type.find("CLOB") != std::string::npos || type.find("TEXT") != std::string::npos)
                return ColType::Text;
            if (type.empty() || type.find("BLOB") != std::string::npos)
                return ColType::Blob;
            if (type.find("REAL") != std::string::npos || type.find("FLOA") != std::string::npos || type.find("DOUB") != std::string::npos)
                return ColType::Real;
            return ColType::Int;
        }

        char const* TypeName(ColType type)
        {
            switch (type)
            {
                case ColType::Int: return "int";
                case ColType::Real: return "real";
                case ColType::Text: return "text";
                case ColType::Blob: return "blob";
            }
            return "?";
        }
    }

    bool TableRegistry::Validate(sqlite3* db, std::vector<std::string>& errors) const
    {
        size_t const errorsBefore = errors.size();
        for (TableDesc const& table : m_tables)
        {
            std::vector<SchemaCol> schema;
            sqlite3_stmt* stmt = nullptr;
            if (sqlite3_prepare_v2(db, "SELECT name, type, \"notnull\", pk FROM pragma_table_info(?)", -1, &stmt, nullptr) != SQLITE_OK)
            {
                errors.push_back(std::string("table_info: ") + sqlite3_errmsg(db));
                return false;
            }
            sqlite3_bind_text(stmt, 1, table.name, -1, SQLITE_STATIC);
            while (sqlite3_step(stmt) == SQLITE_ROW)
            {
                auto text = [stmt](int i) { unsigned char const* t = sqlite3_column_text(stmt, i); return std::string(t ? reinterpret_cast<char const*>(t) : ""); };
                schema.push_back({ text(0), text(1), sqlite3_column_int(stmt, 2) != 0, sqlite3_column_int(stmt, 3) });
            }
            sqlite3_finalize(stmt);

            std::string const prefix = std::string(table.name) + ": ";
            if (schema.empty())
            {
                errors.push_back(prefix + "missing table");
                continue;
            }

            for (size_t i = 0; i < table.cols.size(); ++i)
            {
                ColDesc const& col = table.cols[i];
                auto it = std::find_if(schema.begin(), schema.end(), [&col](SchemaCol const& s) { return s.name == col.name; });
                if (it == schema.end())
                {
                    errors.push_back(prefix + "missing column " + col.name);
                    continue;
                }
                ColType const actual = Affinity(it->type);
                if (actual != col.type)
                    errors.push_back(prefix + col.name + " is " + TypeName(actual) + ", expected " + TypeName(col.type));
                if (it->notNull == col.nullable)
                    errors.push_back(prefix + col.name + (col.nullable ? " is NOT NULL, expected nullable" : " is nullable, expected NOT NULL"));
                int const expectedPk = i < table.keyCols ? int(i + 1) : 0;
                if (it->pk != expectedPk)
                    errors.push_back(prefix + col.name + " has primary key position " + std::to_string(it->pk) + ", expected " + std::to_string(expectedPk));
            }

            for (SchemaCol const& s : schema)
                if (std::none_of(table.cols.begin(), table.cols.end(), [&s](ColDesc const& c) { return s.name == c.name; }))
                    errors.push_back(prefix + "extra column " + s.name);

            if (std::none_of(table.cols.begin(), table.cols.end(), [&table](ColDesc const& c) { return std::string(c.name) == table.ownerCol; }))
                errors.push_back(prefix + "owner column " + table.ownerCol + " is not a column");
        }
        return errors.size() == errorsBefore;
    }
}
