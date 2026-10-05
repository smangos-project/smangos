#include "GenerateWms.h"

#include "World/DoltLiteDatabase.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <set>
#include <sstream>

namespace
{
    bool IsInternalTable(std::string const& name)
    {
        return name.rfind("sqlite_", 0) == 0 || name.rfind("dolt_", 0) == 0 || name.rfind("wms_", 0) == 0;
    }

    std::string Upper(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return std::toupper(c); });
        return value;
    }

    std::string Quote(std::string const& identifier)
    {
        std::string out = "\"";
        for (char c : identifier)
        {
            if (c == '"')
                out += '"';
            out += c;
        }
        return out + "\"";
    }

    std::string ClrTypeFor(std::string const& sqlType)
    {
        std::string const type = Upper(sqlType);
        if (type.find("INT") != std::string::npos)
            return "int";
        if (type.find("REAL") != std::string::npos || type.find("FLOA") != std::string::npos || type.find("DOUB") != std::string::npos)
            return "float";
        if (type.find("BLOB") != std::string::npos)
            return "byte[]";
        return "string";
    }

    std::string Escape(std::string const& value)
    {
        std::string out;
        for (char c : value)
        {
            if (c == '"' || c == '\\')
                out += '\\';
            out += c;
        }
        return out;
    }
}

std::string WmsPascalCase(std::string const& name)
{
    std::string out;
    bool upperNext = true;
    for (unsigned char c : name)
    {
        if (!std::isalnum(c))
        {
            upperNext = true;
            continue;
        }
        out += upperNext ? static_cast<char>(std::toupper(c)) : static_cast<char>(c);
        upperNext = false;
    }
    if (out.empty() || std::isdigit(static_cast<unsigned char>(out[0])))
        out.insert(out.begin(), '_');
    return out;
}

std::string WmsIntegerType(int64_t minValue, int64_t maxValue)
{
    return minValue >= INT32_MIN && maxValue <= INT32_MAX ? "int" : "long";
}

std::vector<WmsTable> ReadWmsTables(DoltLiteDatabase& world)
{
    std::vector<std::string> names;
    {
        DoltLiteStatement stmt = world.Prepare("SELECT name FROM sqlite_schema WHERE type = 'table' ORDER BY name");
        while (stmt.Step())
            names.push_back(stmt.ColumnText(0));
    }

    std::vector<WmsTable> tables;
    for (std::string const& name : names)
    {
        if (IsInternalTable(name))
            continue;

        WmsTable table;
        table.name = name;
        table.record = WmsPascalCase(name) + "Record";

        {
            DoltLiteStatement stmt = world.Prepare(
                "SELECT name, type, \"notnull\", pk FROM pragma_table_info(" + std::string("'") + name + "') ORDER BY cid");
            std::set<std::string> used;
            while (stmt.Step())
            {
                WmsColumn column;
                column.name = stmt.ColumnText(0);
                column.sqlType = stmt.ColumnText(1);
                column.notNull = stmt.ColumnInt64(2) != 0;
                column.primaryKeyOrder = static_cast<int>(stmt.ColumnInt64(3));
                column.clrType = ClrTypeFor(column.sqlType);
                column.property = WmsPascalCase(column.name);
                if (column.property == table.record)
                    column.property += "Value";
                while (!used.insert(column.property).second)
                    column.property += "_";
                table.columns.push_back(std::move(column));
            }
        }

        // Several columns per query keeps the scan of the big tables to one pass.
        std::string select;
        std::vector<size_t> integerColumns;
        for (size_t i = 0; i < table.columns.size(); ++i)
        {
            if (table.columns[i].clrType != "int")
                continue;
            select += (select.empty() ? "" : ", ") + std::string("MIN(") + Quote(table.columns[i].name) + "), MAX(" +
                Quote(table.columns[i].name) + ")";
            integerColumns.push_back(i);
        }
        if (!integerColumns.empty())
        {
            DoltLiteStatement stmt = world.Prepare("SELECT " + select + " FROM " + Quote(name));
            if (stmt.Step())
            {
                for (size_t n = 0; n < integerColumns.size(); ++n)
                {
                    int const lo = static_cast<int>(n * 2);
                    if (stmt.ColumnIsNull(lo))
                        continue;
                    table.columns[integerColumns[n]].clrType = WmsIntegerType(stmt.ColumnInt64(lo), stmt.ColumnInt64(lo + 1));
                }
            }
        }

        tables.push_back(std::move(table));
    }
    return tables;
}

std::string RenderWmsTable(WmsTable const& table)
{
    std::vector<WmsColumn const*> key;
    for (WmsColumn const& column : table.columns)
        if (column.primaryKeyOrder > 0)
            key.push_back(&column);
    std::sort(key.begin(), key.end(), [](WmsColumn const* a, WmsColumn const* b) { return a->primaryKeyOrder < b->primaryKeyOrder; });

    std::ostringstream out;
    out << "using System.ComponentModel.DataAnnotations.Schema;\n"
        << "using Microsoft.EntityFrameworkCore;\n\n"
        << "namespace WorldMapStudio.WoW;\n\n"
        << "/// <summary>EF Core row for the `" << table.name << "` table, generated by `database_tool generate-wms`.</summary>\n"
        << "[Table(\"" << table.name << "\")]\n";
    if (key.empty())
    {
        out << "[Keyless]\n";
    }
    else
    {
        out << "[PrimaryKey(";
        for (size_t i = 0; i < key.size(); ++i)
            out << (i ? ", " : "") << "nameof(" << key[i]->property << ")";
        out << ")]\n";
    }
    out << "public sealed class " << table.record << "\n{\n";
    for (size_t i = 0; i < table.columns.size(); ++i)
    {
        WmsColumn const& column = table.columns[i];
        bool const isKey = column.primaryKeyOrder > 0;
        bool const nullable = !column.notNull && !isKey;
        out << (i ? "\n" : "") << "    [Column(\"" << Escape(column.name) << "\")]\n";
        if (isKey && key.size() == 1 && column.clrType != "string")
            out << "    [DatabaseGenerated(DatabaseGeneratedOption.None)]\n";
        out << "    public " << column.clrType << (nullable ? "?" : "") << " " << column.property << " { get; set; }";
        if (column.clrType == "string" && !nullable)
            out << " = string.Empty;";
        if (column.clrType == "byte[]" && !nullable)
            out << " = [];";
        out << "\n";
    }
    out << "}\n\n"
        << "public sealed partial class WorldDbContext\n{\n"
        << "    public DbSet<" << table.record << "> " << WmsPascalCase(table.name) << " => Set<" << table.record << ">();\n}\n\n"
        << "/// <summary>Registers `" << table.name << "` into WorldStorage's model.</summary>\n"
        << "[Subsystem(nameof(WorldStorage))]\n"
        << "public sealed class " << WmsPascalCase(table.name) << "TableConfiguration : ITableConfiguration\n{\n"
        << "    public " << WmsPascalCase(table.name) << "TableConfiguration(WorldStorage storage)\n    {\n    }\n\n"
        << "    public void Configure(ModelBuilder model)\n    {\n"
        << "        model.Entity<" << table.record << ">();\n    }\n}\n";
    return out.str();
}

WmsGenerateResult GenerateWmsSources(DoltLiteDatabase& world, std::filesystem::path const& outputDirectory)
{
    WmsGenerateResult result;
    try
    {
        std::vector<WmsTable> const tables = ReadWmsTables(world);

        std::error_code ec;
        std::filesystem::create_directories(outputDirectory, ec);
        for (auto const& entry : std::filesystem::directory_iterator(outputDirectory, ec))
            if (entry.path().extension() == ".cs")
                std::filesystem::remove(entry.path(), ec);

        for (WmsTable const& table : tables)
        {
            std::ofstream file(outputDirectory / (table.record + ".cs"), std::ios::binary);
            if (!file)
            {
                result.error = "cannot write " + (outputDirectory / (table.record + ".cs")).string();
                return result;
            }
            file << RenderWmsTable(table);
        }
        result.tables = tables.size();
    }
    catch (std::exception const& e)
    {
        result.error = e.what();
    }
    return result;
}
