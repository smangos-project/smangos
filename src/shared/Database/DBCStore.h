/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2009-2011 MaNGOSZero <https://github.com/mangos/zero>
 * Copyright (C) 2011-2016 Nostalrius <https://nostalrius.org>
 * Copyright (C) 2016-2017 Elysium Project <https://github.com/elysium-project>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef DBCSTORE_H
#define DBCSTORE_H

#include "Platform/Define.h"
#include "FieldFormat.h"
#include <algorithm>
#include <deque>
#include <string>
#include <utility>
#include <vector>

template<class T>
class DBCStorage
{
    public:
        explicit DBCStorage(char const* f) : nCount(0), fmt(f), indexTable(nullptr), m_dataTable(nullptr) { }
        ~DBCStorage() { Clear(); }

        T const* LookupEntry(uint32 id) const { return (id>=nCount)?nullptr:indexTable[id]; }
        bool InsertEntry(uint32 id, T* data) { if (id >= nCount) return false; indexTable[id] = data; return true; }

        uint32  GetNumRows() const { return nCount; }
        char const* GetFormat() const { return fmt; }

        // Takes rows mapped from the world database; strings must outlive the store (see InternString).
        bool LoadRows(std::vector<std::pair<uint32, T>> const& rows)
        {
            Clear();
            if (rows.empty())
                return false;

            uint32 maxId = 0;
            for (auto const& row : rows)
                maxId = std::max(maxId, row.first);

            nCount = maxId + 1;
            m_dataTable = reinterpret_cast<T*>(new char[sizeof(T) * rows.size()]);
            indexTable = reinterpret_cast<T**>(new char[sizeof(T*) * nCount]());
            T* out = m_dataTable;
            for (auto const& row : rows)
            {
                *out = row.second;
                indexTable[row.first] = out++;
            }
            return true;
        }

        char const* InternString(std::string const& value)
        {
            m_strings.push_back(value);
            return m_strings.back().c_str();
        }

        void Clear()
        {
            if (!indexTable)
                return;

            delete[] ((char*)indexTable);
            indexTable = nullptr;
            delete[] ((char*)m_dataTable);
            m_dataTable = nullptr;
            m_strings.clear();
            nCount = 0;
        }

        void EraseEntry(uint32 id) { indexTable[id] = nullptr; }

    private:
        uint32 nCount;
        char const* fmt;
        T** indexTable;
        T* m_dataTable;
        std::deque<std::string> m_strings;
};

#endif
