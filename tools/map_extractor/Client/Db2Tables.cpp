#include "Client/Db2Tables.h"

#include "Dbc/DbcSchema.h"
#include "Dbc/Wdc5Reader.h"

namespace
{
    struct Table
    {
        DbcTableMeta const* meta = nullptr;
        std::vector<std::string> columns;
        std::vector<Wdc5Row> rows;

        int Col(char const* name) const
        {
            for (size_t i = 0; i < columns.size(); ++i)
                if (columns[i] == name)
                    return int(i);
            return -1;
        }
    };

    std::string LoadTable(ClientStorage const& client, char const* name, Table& out)
    {
        out.meta = FindDbcTable(name);
        if (!out.meta)
            return std::string("no metadata for ") + name;
        for (uint32_t i = 0; i < out.meta->fieldCount; ++i)
            for (std::string& c : DbcColumnNames(out.meta->fields[i]))
                out.columns.push_back(std::move(c));

        Wdc5ReadResult read = ReadWdc5File(client.Root() / "dbfilesclient" / (std::string(out.meta->fileStem) + ".db2"), *out.meta);
        if (!read.error.empty())
            return std::string(name) + ": " + read.error;
        out.rows = std::move(read.rows);
        return {};
    }

    int64_t I(Wdc5Row const& row, int col) { return col >= 0 ? row[size_t(col)].intValue : 0; }
}

std::string Db2Tables::Load(ClientStorage const& client)
{
    Table table;
    if (std::string err = LoadTable(client, "Map", table); !err.empty())
        return err;
    {
        int id = table.Col("ID"), dir = table.Col("Directory"), parent = table.Col("ParentMapID");
        for (Wdc5Row const& row : table.rows)
        {
            MapRecord m;
            m.id = uint32_t(I(row, id));
            m.directory = row[size_t(dir)].stringValue;
            m.parentMapId = uint32_t(I(row, parent));
            maps.push_back(std::move(m));
        }
    }

    table = {};
    if (std::string err = LoadTable(client, "LiquidType", table); !err.empty())
        return err;
    {
        int id = table.Col("ID"), bank = table.Col("SoundBank"), mat = table.Col("MaterialID");
        for (Wdc5Row const& row : table.rows)
            liquidTypes[uint32_t(I(row, id))] = { uint8_t(I(row, bank)), uint8_t(I(row, mat)) };
    }

    table = {};
    if (std::string err = LoadTable(client, "LiquidMaterial", table); !err.empty())
        return err;
    {
        int id = table.Col("ID"), flags = table.Col("Flags"), lvf = table.Col("LVF");
        for (Wdc5Row const& row : table.rows)
            liquidMaterials[uint32_t(I(row, id))] = { uint8_t(I(row, flags)), uint8_t(I(row, lvf)) };
    }

    table = {};
    if (std::string err = LoadTable(client, "LiquidObject", table); !err.empty())
        return err;
    {
        int id = table.Col("ID"), type = table.Col("LiquidTypeID");
        for (Wdc5Row const& row : table.rows)
            liquidObjects[uint32_t(I(row, id))] = uint16_t(I(row, type));
    }

    table = {};
    if (std::string err = LoadTable(client, "GameObjectDisplayInfo", table); !err.empty())
        return err;
    {
        int id = table.Col("ID"), name = table.Col("ModelName"), fdid = table.Col("FileDataID");
        for (Wdc5Row const& row : table.rows)
            gameObjectDisplays[uint32_t(I(row, id))] = { row[size_t(name)].stringValue, uint32_t(I(row, fdid)) };
    }
    return {};
}
