#ifndef MANGOS_PERSISTENCE_TABLEREGISTRY_H
#define MANGOS_PERSISTENCE_TABLEREGISTRY_H

#include "Common.h"

#include <string>
#include <vector>

struct sqlite3;

namespace Persistence
{
    enum class ColType : uint8 { Int, Real, Text, Blob };

    struct ColDesc
    {
        char const* name;
        ColType type;
        bool nullable;
    };

    // Columns are listed primary key first (keyCols of them, in key order). keyCols 0 means the
    // table has no key: an owner's rows are replaced as a whole. ownerCol holds the id the
    // capture code passes as image owner.
    struct TableDesc
    {
        char const* name;
        uint8 keyCols;
        char const* ownerCol;
        std::vector<ColDesc> cols;
    };

    class TableRegistry
    {
        public:
            explicit TableRegistry(std::vector<TableDesc> tables) : m_tables(std::move(tables)) {}

            uint16 Size() const { return uint16(m_tables.size()); }
            TableDesc const& Get(uint16 id) const { return m_tables[id]; }

            // Compares every table with the schema of db. Appends one message per mismatch.
            bool Validate(sqlite3* db, std::vector<std::string>& errors) const;

        private:
            std::vector<TableDesc> m_tables;
    };
}

#endif
